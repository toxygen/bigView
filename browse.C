//////////////////////////////////////////////////////////////////////////
//////////////////////////////// browse.C ////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <sstream>
#include <GLX.h>
#include <X11/keysym.h>
#include <math.h> // for floor,ceil
#include <unistd.h> // for getopt
#include <assert.h>
#include "defs.h"
#include "ppm.h"
#include "ostr.h"
#include "PageReader.h"
using namespace std;

#define DEBUG 1
#include "debug.h"

int sourceWidth=0;
int sourceHeight=0;
int sourceFormat=GL_RGB;
int sourceType=GL_UNSIGNED_BYTE;
glx* env = 0;
Raw::PageReader* readerThread=0;
ThreadedQueue<Raw::Page> workQueue;
unsigned char* pixels=0;
pthread_mutex_t pageLock=PTHREAD_MUTEX_INITIALIZER;
int levelOfDetail=0;
int curRow=0,curCol=0;
bool swapPixels=false;
bool signedPixels=false;

void
draw(glx* env, void* user)
{
  FANCYMESG("draw");

  pthread_mutex_lock(&pageLock);
  glRasterPos2i(0,0);
  glDrawPixels( Raw::PAGE_SIZE, Raw::PAGE_SIZE,
                sourceFormat, sourceType,
                pixels);
  int mipwidth,mipheight;
  Raw::PageReader::mipmapSize(levelOfDetail,
			      sourceWidth, sourceHeight,
			      mipwidth, mipheight);

  float srcAspect = (float)mipwidth/(float)mipheight;
  float dstAspect = env->aspect();
  
  glDisable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0,dstAspect,0,1,-1,1);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  glColor3f(0,0,1);
  glLineWidth(1);
  
  float xNext = 0.1 + srcAspect*0.1;
  
  glBegin(GL_LINE_LOOP);
  glVertex3f(0.1f,0.1f,0.1f);
  glVertex3f(0.1f,0.2f,0.1f);
  glVertex3f(xNext,0.2f,0.1f);
  glVertex3f(xNext,0.1f,0.1f);
  glEnd();
  
  int x = curCol*256;
  int y = curRow*256;

  float xMin = 0.1f+((float)x/(float)mipwidth)*srcAspect*0.1f;
  float xMax = 0.1f+((float)(x+256)/(float)mipwidth)*srcAspect*0.1f;
  float yMin = 0.1f+((float)y/(float)mipheight)*0.1f;
  float yMax = 0.1f+((float)(y+256)/(float)mipheight)*0.1f;
  
  glBegin(GL_LINE_LOOP);
  glVertex3f(xMin,yMin,0.0f);
  glVertex3f(xMin,yMax,0.0f);
  glVertex3f(xMax,yMax,0.0f);
  glVertex3f(xMax,yMin,0.0f);
  glEnd();
  
  glLineWidth(1);
  
  glBegin(GL_LINES);
  for(int i = 0 ; i < 10 ; i++ ){
    float t = (float)((float)i/9.0f)*(yMax-yMin);
    glVertex3f(xMin,yMin+t,0.0);
    glVertex3f(xMax,yMin+t,0.0);
  }
  glEnd();

  pthread_mutex_unlock(&pageLock);
}

void 
swapShorts(void* buf, int numShorts)
{
  register unsigned short* src = (unsigned short*)buf;
  for(int i = 0 ; i < numShorts ; i++, src++ ){
    unsigned short lo = *src & 0x00ff;
    unsigned short hi = *src & 0xff00;
    *src = (lo<<8) | (hi>>8);
  }
}

void
pageLoaded(Raw::PageReader*, Raw::Page* page, void* user)
{
  FANCYMESG("pageLoaded");
  VAR(page->itsCoord);

  pthread_mutex_lock(&pageLock);
  if( pixels!=0 ){
    delete [] pixels;
    pixels=0;
  }
  pixels = page->itsPixels;
  page->itsPixels=0;
  delete page;

  int count = Raw::PAGE_SIZE * Raw::PAGE_SIZE;
  if( swapPixels )
    swapShorts((unsigned short*)pixels, count);

  int mni=0;
  int mxi=0;
  short slo=0x7fff,shi=-slo;
  short* sptr = (short*) pixels;  
  unsigned short lo=0xffff,hi=0x0;
  unsigned short* ptr = (unsigned short*) pixels; 
  for(int i=0 ; i<count ; ++i ){
    int c = i % Raw::PAGE_SIZE;
    int r = (int)(i / Raw::PAGE_SIZE);
    //cout << "["<<c<<","<<r<<"]:";
    if( signedPixels ){
      //cout << sptr[i] << endl;
      if( sptr[i]<slo ) {mni=i;slo = sptr[i];}
      if( sptr[i]>shi ) {mxi=i;shi = sptr[i];} 
    } else {
      //cout << ptr[i] << endl;
      if( ptr[i]<lo ) {mni=i;lo = ptr[i];}
      if( ptr[i]>hi ) {mxi=i;hi = ptr[i];}  
    }
  } 

  int col = mni % Raw::PAGE_SIZE;
  int row = (int)(mni / Raw::PAGE_SIZE);
  if( signedPixels )
    cout << "lo = " << slo << " at [" << col << ","<<row<<"]"<<endl;
  else
    cout << "lo = " << lo << " at [" << col << ","<<row<<"]"<<endl;
  
  col = mxi % Raw::PAGE_SIZE;
  row = (int)(mxi / Raw::PAGE_SIZE);

  if( signedPixels )
    cout << "hi = " << shi << " at [" << col << ","<<row<<"]"<<endl;
  else
    cout << "hi = " << hi << " at [" << col << ","<<row<<"]"<<endl;

  if( signedPixels ){
    cout << "normalizing...";
    sptr = (short*) pixels;  
    for(int i=0 ; i<count ; ++i ){
      float ft = (float)(sptr[i]-slo)/(float)(shi-slo);
      sptr[i] = (short)(ft*(float)0x7fff);
    }
    cout << "done"<<endl;
  }
  pthread_mutex_unlock(&pageLock);
  if( env )
    env->wakeup();
}

void setProjection(glx* env,void* user)
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0,255,0,255,-1,1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void handleEvent(glx* env, XEvent *event, void*)
{
  KeySym sym;
  XComposeStatus status;
  char buffer[20];
  int buf_len = 20;
  XKeyEvent *kep = (XKeyEvent *)(event);
  XLookupString(kep, buffer, buf_len, &sym, &status);
  int pageCols,pageRows;

  Raw::PageReader::pageCounts(levelOfDetail,Raw::PAGE_SIZE,
			      sourceWidth,sourceHeight,pageCols,pageRows);

  switch( sym ){
    case XK_q:
      exit(0);
      break;
    case XK_Left:
	--curCol;
	if( curCol>=0 ){
	  Raw::Page* p = 
	    new Raw::Page(Raw::Coord(curCol,curRow),0,0,levelOfDetail);
	  workQueue.add( p );
	} else
	  curCol=0;
	break;
    case XK_Right:
	++curCol;
	if( curCol<pageCols ){
	  Raw::Page* p = 
	    new Raw::Page(Raw::Coord(curCol,curRow),0,0,levelOfDetail);
	  if( ! workQueue.exists(*p) )
	    workQueue.add( p );
	  else
	    delete p;
	} else
	  curCol=pageCols-1;
	break;
    case XK_Up:
      if( kep->state & ShiftMask)
      {
	if( pageCols>1 || pageRows>1 ){
	  levelOfDetail++;
	  curRow=0;
	  curCol=0;
	  Raw::Page* p = 
	    new Raw::Page(Raw::Coord(curCol,curRow),0,0,levelOfDetail);	  
	  if( ! workQueue.exists(*p) )
	    workQueue.add( p );
	  else
	    delete p;	    
	  env->wakeup();
	}
      }
      else
      {
	++curRow;
	if( curRow<pageRows ){
	  Raw::Page* p = 
	    new Raw::Page(Raw::Coord(curCol,curRow),0,0,levelOfDetail); 
	  if( ! workQueue.exists(*p) )
	    workQueue.add( p );
	  else
	    delete p;	    
	} else
	  curRow=pageRows-1;
      }
      break;
    case XK_Down:
      if( kep->state & ShiftMask)
      {
	levelOfDetail--;
	if(levelOfDetail<0)levelOfDetail=0;
	curRow=0;
	curCol=0;
	Raw::Page* p = 
	  new Raw::Page(Raw::Coord(curCol,curRow),0,0,levelOfDetail);
	if( ! workQueue.exists(*p) )
	  workQueue.add( p );
	else
	  delete p;	    
	env->wakeup();
      }
      else
      {
	--curRow;
	if( curRow>=0 ){
	  Raw::Page* p = 
	    new Raw::Page(Raw::Coord(curCol,curRow),0,0,levelOfDetail);
	  if( ! workQueue.exists(*p) )
	    workQueue.add( p );
	  else
	    delete p;	    
	} else
	  curRow=0;
      }
      break;
  }
}

void initGL(glx* env,void* user)
{
  while( ! readerThread->ready() )
    ;
  Raw::Page* p = new Raw::Page(Raw::Coord(0,0),0,0,levelOfDetail);
  workQueue.add( p );
  env->addDrawFunc(draw);
}

int calcSourceDepth(int frmt, int type)
{
  int bpc; //bytes per component
  int depth;

  switch( type ){
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
      bpc=1;
      break;
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
      bpc=2;
      break;
    case GL_INT:
    case GL_UNSIGNED_INT:
    case GL_FLOAT:
      bpc=4;
      break;
    case GL_DOUBLE:
      bpc=8;
      break;
    default: assert(0);
  }

  switch( frmt ){
    case GL_RGB:
      depth=3*bpc;
      break;
    case GL_BGR:
      depth=3*bpc;
      break;
    case GL_RGBA:
      depth=4*bpc;
      break;
    case GL_BGRA:
      depth=4*bpc;
      break;
    case GL_LUMINANCE:
      depth=1*bpc;
      break;
    default: assert(0);
  }
  return depth;
}

int OpenGLKeyword(string str)
{
  // component size...
  if( ! str.compare("GL_BYTE") )           return GL_BYTE;
  if( ! str.compare("GL_UNSIGNED_BYTE") )  return GL_UNSIGNED_BYTE;
  if( ! str.compare("GL_SHORT") )          return GL_SHORT;
  if( ! str.compare("GL_UNSIGNED_SHORT") ) return GL_UNSIGNED_SHORT;
  if( ! str.compare("GL_INT") )            return GL_INT;
  if( ! str.compare("GL_UNSIGNED_INT") )   return GL_UNSIGNED_INT;
  if( ! str.compare("GL_FLOAT") )          return GL_FLOAT;
  if( ! str.compare("GL_DOUBLE") )         return GL_DOUBLE;

  // component format
  if( ! str.compare("GL_RGB") )       return GL_RGB;
  if( ! str.compare("GL_BGR,") )      return GL_BGR;
  if( ! str.compare("GL_RGBA") )      return GL_RGBA;
  if( ! str.compare("GL_BGRA") )      return GL_BGRA;
  if( ! str.compare("GL_LUMINANCE") ) return GL_LUMINANCE;
}

string OpenGLKeyword(int value)
{
  switch(value){
    case GL_BYTE:           return string("GL_BYTE"); break;
    case GL_UNSIGNED_BYTE:  return string("GL_UNSIGNED_BYTE"); break;
    case GL_SHORT:          return string("GL_SHORT"); break;
    case GL_UNSIGNED_SHORT: return string("GL_UNSIGNED_SHORT"); break;
    case GL_INT:            return string("GL_INT"); break;
    case GL_UNSIGNED_INT:   return string("GL_UNSIGNED_INT"); break;
    case GL_FLOAT:          return string("GL_FLOAT"); break;
    case GL_RGB:            return string("GL_RGB"); break;
    case GL_LUMINANCE:      return string("GL_LUMINANCE"); break;
  }
}

int
main(int argc, char** argv)
{
  int c;
  int offset=0;
  int pageDim=Raw::PAGE_SIZE;

  if( argc==1 ){
    cerr << "usage: browse -w<height> -w<int> paged.raw"<<endl;
    cerr << "usage: browse ppm.paged.raw"<<endl;
    return 0;
  }

  while( (c = getopt(argc,argv,"Ssh:w:")) != -1) 
  {
    switch( c ){
      case 'S':
	swapPixels=true;
	break;
      case 's':
	signedPixels=true;
	break;
      case 'f': // format
	sourceFormat = OpenGLKeyword(optarg);
	break;
      case 't':
	sourceType = OpenGLKeyword(optarg);
	break;
      case 'h':
        sourceHeight = atol(optarg);
	cerr << "=== Height : " << sourceHeight << " ===" << endl;
        break;
      case 'w':
        sourceWidth = atol(optarg);
	cerr << "=== Width : " << sourceWidth << " ===" << endl;
        break;
    }
  }
  if( sourceWidth==0 && sourceHeight==0 ){
    PPM::Format format;
    int cpp,bpc;
    int sizeX,sizeY;
    int off;
    try {
      PPM::ppmHeader(argv[optind],&format,&cpp,&bpc,&sizeX,&sizeY,&off);
      offset=off;
    } catch( exception& e ){throw;}
    sourceWidth=sizeX;
    sourceHeight=sizeY;
    sourceFormat = (cpp==1)?GL_LUMINANCE:GL_RGB;
    switch(bpc){
      case 8:
	sourceType = signedPixels ? GL_BYTE : GL_UNSIGNED_BYTE;
	break;
      case 16:
	sourceType = signedPixels ? GL_SHORT : GL_UNSIGNED_SHORT;
	break;
      default:
	assert(0);
    }
  }

  int srcDepth = calcSourceDepth(sourceFormat,sourceType);
  int numRows=(int)(sourceHeight/pageDim) + 1;
  int numCols=(int)(sourceWidth/pageDim) + 1;
  cout << "numRows = " << numRows << endl;
  cout << "numCols = " << numCols << endl;
  cout << " Format = " << OpenGLKeyword(sourceFormat) << endl;
  cout << "  Type  = " << OpenGLKeyword(sourceType) << endl;

  int pageSize = pageDim*pageDim*srcDepth;

  try {
    readerThread = 
      new Raw::PageReader(workQueue,argv[optind],Raw::PAGE_SIZE,true,
			  sourceWidth,sourceHeight,
			  sourceFormat,sourceType,offset);
  } catch( exception& e ){return 0;}

  readerThread->addListener(pageLoaded);

  env = new glx(initGL);
  env->setSize(Raw::PAGE_SIZE,Raw::PAGE_SIZE);
  env->showAxis(false);
  env->addProjFunc(setProjection);
  env->addEventFunc(handleEvent);
  env->mainLoop();
}
