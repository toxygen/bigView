#include <iostream>
#include <fstream>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <string>
#include <OpenEnv2D.h>
#include <vector>
#include <iterator>
#include <string>
#include "ppm.h"

using namespace std;

enum Format{UNKNOWN,PLAIN_PPM,BINARY_PPM,PLAIN_PGM,BINARY_PGM};

# define FANCYMESG(m) {cout << "===== " << m << " =====" << endl;}
# define VAR(v)  {cout << #v << " = " << v << endl;}

//const int maxWinX = 1000;
//const int maxWinY = 1000;
const int maxWinX = 1920;
const int maxWinY = 1200;

vector<string> inputFilenames;
vector<unsigned char*> inputPixels;
unsigned char* extraction=0;
int srcSizeX=0;
int srcSizeY=0;
int srcSizeZ=0;
int dstSizeX=0;
int dstSizeY=0;
int srcOffsetX=0;
int srcOffsetY=0;
float startX=0;
float startY=0;
int winW=0, winH=0;
int bitsPerComponent=0;
int componentsPerPixel=0;
int bytesPerPixel=0;

vector<unsigned char*>::iterator curPixels;
vector<string>::iterator curName;
int showNames=1;
bool signedPixels=false;
bool swapPixels=false;
bool forceNormalize=false;

//////////////////////////////////////////////////////////////////////////
/////////////////////////// METHOD DECLARATIONS //////////////////////////
//////////////////////////////////////////////////////////////////////////

void eatSpace(ifstream&);
int pow2(int);
unsigned char* readBinaryPixels(ifstream&,int,int,int);
unsigned char* readPlainPixels(ifstream&,int,int,int);
void draw(int*, void*);
void fillArray(unsigned char*,unsigned char*,int, int,int,int);
void startMove(float, float, void*);
void trackMove(float, float, void*);
int load(string);
void swapShorts(void* buf, int numShorts);

//////////////////////////////////////////////////////////////////////////
/////////////////////////// METHOD DEFINITIONS ///////////////////////////
//////////////////////////////////////////////////////////////////////////

void 
draw(envBase* env, int* flags, void* user)
{
  GLenum componentType = (componentsPerPixel==1)?GL_LUMINANCE:GL_RGB;
  GLenum componentSize;

  switch(bitsPerComponent){
    case 8:
      componentSize = signedPixels ? GL_BYTE : GL_UNSIGNED_BYTE;
      break;
    case 16:
      componentSize = (signedPixels && !forceNormalize) ? 
	GL_SHORT : GL_UNSIGNED_SHORT;
      break;
      
  }
  int h = env->getWinHeight();
  int w = env->getWinWidth();
  if( extraction==0 )
    return;
  glDisable(GL_DEPTH_TEST);

  fillArray(inputPixels[ flags[FRAME]%inputPixels.size() ],extraction,
	    srcOffsetX,srcOffsetY,dstSizeX,dstSizeY);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glOrtho(0,dstSizeX,0,dstSizeY,-1,1);
  glRasterPos2i(0,0);
  glDrawPixels( dstSizeX, dstSizeY,
		componentType, componentSize,
		extraction);

  if( w != srcSizeX || h != srcSizeY ){
    float srcAspect = (float)srcSizeX/(float)srcSizeY;
    float dstAspect = env->getAspectRatio();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,dstAspect,0,1,-1,1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(0,0,1);
    glLineWidth(3);

    float xNext = 0.1 + srcAspect*0.1;

    glBegin(GL_LINE_LOOP);
    glVertex3f(0.1f,0.1f,0.0f);
    glVertex3f(0.1f,0.2f,0.0f);
    glVertex3f(xNext,0.2f,0.0f);
    glVertex3f(xNext,0.1f,0.0f);
    glEnd();

    float xMin = 0.1f+((float)srcOffsetX/(float)srcSizeX)*srcAspect*0.1f;
    float xMax = 0.1f+((float)(srcOffsetX+dstSizeX)/(float)srcSizeX)*srcAspect*0.1f;
    float yMin = 0.1f+((float)srcOffsetY/(float)srcSizeY)*0.1f;
    float yMax = 0.1f+((float)(srcOffsetY+dstSizeY)/(float)srcSizeY)*0.1f;
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
  }

  if( showNames ){
    int index=flags[FRAME]%inputFilenames.size();
    env->showMessage(30,30,inputFilenames[index].c_str() );
  }
}

void 
fillArray(unsigned char* srcBytes,unsigned char* dstBytes,
	  int srcX, int srcY,
	  int dstWidth,int dstHeight)
{
  off_t srcBase = (u_int64_t)srcY * srcSizeX + srcX;
  int size = bytesPerPixel;
  for(off_t row = 0 ; row < dstHeight ; row++ ){
    off_t srcIndex = (off_t)srcBase + row*srcSizeX;
    off_t dstIndex = (off_t)row*dstWidth;
    memcpy(&dstBytes[dstIndex*size],&srcBytes[srcIndex*size],dstWidth*size);
  }  
}
void startMove(envBase*,float x, float y, void*)
{
  startX = x;
  startY = y;
}

void trackMove(envBase* base,float x, float y, void*)
{
  OpenEnv2D* env = dynamic_cast<OpenEnv2D*>(base);
  float deltaX = startX - x;//x - startX;
  float deltaY = startY - y;//y - startY;
  float w = (float)env->getWinWidth();
  float h = (float)env->getWinHeight();
  float aspect = w/h;  
  srcOffsetX += (int)(deltaX*aspect*w);
  srcOffsetY += (int)(deltaY*h);

  if( srcOffsetX < 0 )
    srcOffsetX = 0;
  if( srcOffsetX > srcSizeX-dstSizeX )
    srcOffsetX = srcSizeX-dstSizeX;

  if( srcOffsetY < 0 )
    srcOffsetY = 0;
  if( srcOffsetY > srcSizeY-dstSizeY )
    srcOffsetY = srcSizeY-dstSizeY;
    
  //cout << "srcOffsetX = " << srcOffsetX << endl;
  //cout << "srcOffsetY = " << srcOffsetY << endl;
  fillArray(*curPixels,extraction,srcOffsetX,srcOffsetY,dstSizeX,dstSizeY);
  env->wakeup();
}

int
main(int argc, char** argv)
{
  int c;

  if( argc == 1 ){
    cerr << "usage: <-s> " << argv[0] << " <ppm> ..." << endl;
    cerr << "-s : swap" << endl;
    return -1;
  }

  while( (c = getopt(argc,argv,"sSn")) != -1){
    switch( c ){
      case 'n':
	forceNormalize=true;
	break;       
      case 's':
	signedPixels=true;
	break;
      case 'S':
	swapPixels=true;
	break;
    }
  }
  
  for(int i=optind; i<argc ; i++ ){
    PPM::Format format;
    int cpp,bpc;
    int off;
    unsigned char* pixels=0;
    try 
    {
      pixels = PPM::load(argv[i],&format,&cpp,&bpc,&srcSizeX,&srcSizeY,
			 signedPixels,forceNormalize,swapPixels);
    } 
    catch( exception& e )
    {
      cerr << "PPM::load() failed" << endl;
      cerr << "FILE:["<<argv[i]<<"]"<<endl;
    }

    if( pixels ){
      bytesPerPixel = cpp * (bpc/8);
      componentsPerPixel = cpp;
      bitsPerComponent = bpc;
      inputPixels.push_back(pixels);
      inputFilenames.push_back(argv[i]);
    }
  }

  curPixels = inputPixels.begin();
  curName = inputFilenames.begin();

  if( curPixels == inputPixels.end() ){
    cerr << "No images loaded!" << endl;
    return 0;
  }
    
  dstSizeX = srcSizeX;
  dstSizeY = srcSizeY;
  if( dstSizeX > maxWinX ) dstSizeX = maxWinX;
  if( dstSizeY > maxWinY ) dstSizeY = maxWinY;

  int dstSize=dstSizeX*dstSizeY*bytesPerPixel;
  VAR(dstSize);
  extraction = new unsigned char[dstSize];
  assert(extraction);
  
  fillArray(*curPixels,extraction,srcOffsetX,srcOffsetY,dstSizeX,dstSizeY);

  OpenEnv2D* env = new OpenEnv2D("pix");
  env->setWinSize(dstSizeX,dstSizeY);
  env->setMouseDownFunc(startMove, RIGHT_BUTTON);
  env->setMouseProcessFunc(trackMove, RIGHT_BUTTON);
  env->setRenderFunc(draw);
  env->setMaxFrames(inputPixels.size());
  env->mainLoop();
}
