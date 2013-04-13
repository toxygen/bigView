//////////////////////////////////////////////////////////////////////////
////////////////////////////// PageReader.C //////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <exception>
#include <stdio.h> // for perror()
#include <GL/gl.h> // FOR GL_LUMINANCE, etc
#include <strings.h> // for bzero
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>     
#include <unistd.h>
#include <assert.h>
#include <bigFile.h>
#include "PageReader.h"
#include "ppm.h"
#include "ostr.h"

using namespace std;

//#define DEBUG 1
#include "debug.h"

// PageReader::PageReader()
//////////////////////////////////////////////////////////////////////////
// PURPOSE: CTOR for class PageReader
//  INPUTS:
// OUTPUTS:
// RETURNS:
//   NOTES:
//////////////////////////////////////////////////////////////////////////

Raw::PageReader::PageReader(ThreadedQueue<Page>& workQueue,
			    string filename,
			    int pageSize, bool paged, 
			    int sourceWidth, int sourceHeight,
			    int sourceFormat, int sourceType,
			    int offset) :
  itsWorkQueue(workQueue),
  itsFilename(filename),
  itsPagedFlag(paged),
  itsOffset(offset),
  itsThreadID(0),
  itsReadyFlag(false),
  itsDoneFlag(false),
  itsSourceWidth(sourceWidth),
  itsSourceHeight(sourceHeight),
  itsSourceFormat(sourceFormat),
  itsSourceType(sourceType),
  itsPageDim(pageSize),
  itsMaxLOD(0),
  itsPageDimExp(0)
{
  FANCYMESG("Raw::PageReader::PageReader()");

  if( itsSourceWidth==0 && itsSourceHeight==0 ){
    MESG("Raw::PageReader::PageReader() : trying to read ppm header");
    PPM::Format format;
    int cpp,bpc;
    int sizeX,sizeY;
    int off;
    try {
      PPM::ppmHeader(itsFilename.c_str(),&format,&cpp,&bpc,&sizeX,&sizeY,&off);
    } catch( exception& e ){throw;}

    itsSourceWidth=sizeX;
    itsSourceHeight=sizeY;
    switch( cpp ){
      case 1:
	MESG("itsSourceFormat=GL_LUMINANCE");
	itsSourceFormat=GL_LUMINANCE;
	break;
      case 3:
	MESG("itsSourceFormat=GL_RGB");
	itsSourceFormat=GL_RGB;
	break;
    }
    switch( bpc ){
      case 8:
	MESG("itsSourceType=GL_UNSIGNED_BYTE");
	itsSourceType = GL_UNSIGNED_BYTE;
	break;
      case 16:
	MESG("itsSourceType=GL_UNSIGNED_SHORT");
	itsSourceType = GL_UNSIGNED_SHORT;
	break;
    }
    itsOffset=off;
    VAR(itsSourceWidth);
    VAR(itsSourceHeight);
  }

  itsPageRows = (int)(itsSourceHeight/itsPageDim) + 1;
  itsPageCols = (int)(itsSourceWidth/itsPageDim) + 1;
  VAR(itsPageRows);
  VAR(itsPageCols);

  int largestDim = itsPageRows>itsPageCols ? itsPageRows : itsPageCols;
  bool found=false;
  while( ! found ){
    int p = pow2(itsMaxLOD);
    if( p>largestDim )
      found=true;
    else
      ++itsMaxLOD;
  }

  itsPageDimExp=0;
  found=false;
  while( ! found ){
    long p = pow2(itsPageDimExp);
    if( p==itsPageDim )
      found=true;
    else
      ++itsPageDimExp;
  }

  itsTotalPages=0;
  for(int i=0; i<=itsMaxLOD ; ++i ){
    int rows,cols;
    Raw::PageReader::pageCounts(i,PAGE_SIZE,itsSourceWidth,itsSourceHeight,
				cols,rows);
    itsTotalPages += rows*cols;
  }
  VAR(itsTotalPages);

  if( pthread_mutex_init(&itsDoneLock,0) ) 
    throw;

  try {
    openFile();
    startReaderThread();
  }
  catch(exception e){
    cerr << "Exception: " << e.what() << endl;
    throw;
  }
}

Raw::PageReader::~PageReader(void)
{
  FANCYMESG("Raw::PageReader::~PageReader()");
  void* result;
  closeFile();
  stopReaderThread(); 
  pthread_join(itsThreadID,&result);
  pthread_mutex_destroy(&itsDoneLock);
}

void 
Raw::PageReader::addListener(Raw::PageReader::ListenerFunc func, void* user)
{
  FANCYMESG("Raw::PageReader::addListener");
  Raw::PageReader::ListenerPair lp;
  lp.first = func;
  lp.second = user;
  itsListeners.push_back( lp );
}

void
Raw::PageReader::callListeners(Page* page)
{
  std::vector<Raw::PageReader::ListenerPair>::iterator iter = 
    itsListeners.begin();
  for( ; iter != itsListeners.end() ; ++iter ){
    ListenerPair lp = *iter;
    lp.first(this,page,lp.second);
  }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////// PROECTED METHODS ////////////////////////////
//////////////////////////////////////////////////////////////////////////


// Raw::PageReader::startReaderThread()
//////////////////////////////////////////////////////////////////////////
// PURPOSE: init the reader thread
//  INPUTS:
// OUTPUTS:
// RETURNS:
//   NOTES: tries to open the file then start the thread, otherwise throws
//////////////////////////////////////////////////////////////////////////

void 
Raw::PageReader::startReaderThread(void)
{
  FANCYMESG("Raw::PageReader::startReaderThread()");
  int res = pthread_create(&itsThreadID,0,Raw::PageReader::work,this);
  if( res != 0 ){
    cerr << "startReaderThread: pthread_create() failed ["<<res<<"]!" << endl;
    throw exception();
  }
}

// Raw::PageReader::stopReaderThread(void)
//////////////////////////////////////////////////////////////////////////
// PURPOSE: tells the reader thread to stop
//  INPUTS:
// OUTPUTS:
// RETURNS:
//   NOTES: may need to wake queue lock
//////////////////////////////////////////////////////////////////////////

void Raw::PageReader::stopReaderThread(void)
{
  FANCYMESG("Raw::PageReader::stopReaderThread()");
  pthread_mutex_lock(&itsDoneLock);
  itsDoneFlag = 1;
  itsWorkQueue.die();
  pthread_mutex_unlock(&itsDoneLock);
}

// Raw::PageReader::work()
//////////////////////////////////////////////////////////////////////////
// PURPOSE: Main block for the reader thread. Reader thread is simple:
//        : while not done, take coords from queue and load them
//  INPUTS: 
// OUTPUTS:
// RETURNS:
//   NOTES:
//////////////////////////////////////////////////////////////////////////

void *
Raw::PageReader::work(void * arg)
{
  FANCYMESG("Raw::PageReader::work()");
  PageReader* _this = static_cast<PageReader*>(arg);
  unsigned char* pixels=0;

  _this->itsReadyFlag=true;

  while( ! _this->isReaderDone() ){
    Page* p = _this->itsWorkQueue.next();
    if(VERBOSE){
      _MESGVAR("PageReader: page request",p->itsCoord);
    }
    if( _this->isReaderDone() )
      return 0;
    assert(p);
    if( p->itsPixels == 0 ){
      if(VERBOSE){
	_MESG("PageReader: needs to be loaded");
      }
      if( _this->itsPagedFlag )
	pixels = _this->load(*p);
      else 
	pixels = _this->loadUnpaged(p->itsCoord); 
      p->itsPixels = pixels;

      // clear the old texture id
      p->itsID=0; 

      if(VERBOSE){
	_MESG("PageReader: loaded!");
      }
    } else {
      if(VERBOSE){
	_MESG("PageReader: page already loaded!");
      }
    }
    _this->callListeners(p); // pass off p to client
  }
  return 0;
}

// Raw::PageReader::load()
//////////////////////////////////////////////////////////////////////////
// PURPOSE: Reads a 'page' from ppm/raw image files in paged format. 
//         
//  INPUTS: Page& page: a page in the image
// OUTPUTS:
// RETURNS: pointer to page buffer
//   NOTES: This is called by readerThread as taken from the queue.
//        : The page is read in one fell swoop into a page buffer
//        : swapping bytes if needed.
//////////////////////////////////////////////////////////////////////////

unsigned char*
Raw::PageReader::load(Page& page)
{
  FANCYMESG("Raw::PageReader::load");
  VAR( page.itsCoord );

  int lod=page.itsLevelOfDetail;
  int row=page.itsCoord.second;
  int col=page.itsCoord.first;
  int sourceDepth;
  int bpc;

  switch( itsSourceType ){
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

  switch( itsSourceFormat ){
    case GL_RGB:
      sourceDepth=3*bpc;
      break;
    case GL_BGR:
      sourceDepth=3*bpc;
      break;
    case GL_RGBA:
      sourceDepth=4*bpc;
      break;
    case GL_BGRA:
      sourceDepth=4*bpc;
      break;
    case GL_LUMINANCE:
      sourceDepth=1*bpc;
      break;
    default: assert(0);
  }

  VAR(sourceDepth);

  int pageSize = itsPageDim*itsPageDim*sourceDepth;
  int lvlCols,lvlRows;
  pageCounts(lod,itsPageDim,itsSourceWidth,itsSourceHeight,lvlCols,lvlRows);

  BigFile::Offset offset=0;
  for(int i=0; i<lod ; ++i ){
    int width,height;
    int rows,cols;
    mipmapSize(i,itsSourceWidth,itsSourceHeight,width,height);
    pageCounts(i,itsPageDim,itsSourceWidth,itsSourceHeight,cols,rows);
    offset += rows*cols;
  }
  offset += row*lvlCols + col;
  assert(offset<itsTotalPages);
#ifdef DEBUG 
  cout << lod<< " : [" << col << "," << row << "]: at page# " << offset <<endl;
#endif

  offset = offset * pageSize;
  if( ! BigFile::seek(itsFile,itsOffset+offset) ){
    perror(" load(): seek failed");
    cerr << "wanted: " << itsOffset+offset << endl;
    cerr << "  line: " << __LINE__ << endl;
    return 0;
  }
  unsigned char* pixels = new unsigned char[pageSize];

  int res = ::read(itsFile, pixels, pageSize);

  if( res != pageSize ){
    perror(" load(): read failed");
    cerr << "before: " << itsOffset+offset << endl;
    cerr << "wanted: " << pageSize << endl;
    cerr << "  line: " << __LINE__ << endl;
    delete [] pixels;
    return 0;
  }
  // use to simulate slow reads :)
  //usleep(100000);
  page.itsPixels=pixels;
  return pixels;
}

unsigned char*
Raw::PageReader::loadUnpaged(Coord& coord)
{  
  FANCYMESG("Raw::PageReader::loadUnpaged");
  VAR( coord );

  int row=coord.second;
  int col=coord.first;
  int sourceDepth;
  int bpc;

  switch( itsSourceType ){
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

  switch( itsSourceFormat ){
    case GL_RGB:
      sourceDepth=3*bpc;
      break;
    case GL_BGR:
      sourceDepth=3*bpc;
      break;
    case GL_RGBA:
      sourceDepth=4*bpc;
      break;
    case GL_BGRA:
      sourceDepth=4*bpc;
      break;
    case GL_LUMINANCE:
      sourceDepth=1*bpc;
      break;
    default: assert(0);
  }

  BigFile::Offset xPixelOffset = col * itsPageDim;
  BigFile::Offset yPixelOffset = itsSourceHeight-(row+1)*itsPageDim;

  // NOTE: offset may be negative here
  BigFile::Offset offset = 
    (yPixelOffset * itsSourceWidth + xPixelOffset) * sourceDepth;

  VAR(xPixelOffset);
  VAR(yPixelOffset);
  VAR(offset);

  int readSize = (xPixelOffset+itsPageDim>itsSourceWidth ) ? 
    (itsSourceWidth-xPixelOffset) : itsPageDim;
  VAR(readSize);

  if( offset > 0 ){

    if( ! BigFile::seek(itsFile,itsOffset+offset) ){
      perror(" load(): seek failed");
      cerr << "wanted: " << itsOffset+offset << endl;
      cerr << "  line: " << __LINE__ << endl;
      return 0;
    }
  }

  int pageSize = itsPageDim*itsPageDim*sourceDepth;
  unsigned char* page = new unsigned char[pageSize];
  bzero(page,pageSize);
  bool done=false;

  int srcRowSize = itsSourceWidth * sourceDepth;
  int dstRowSize = readSize * sourceDepth;

  for(int row=0; row<itsPageDim && ! done; ++row ){
    if( offset < 0 ){
      offset += srcRowSize;
      continue;
    }

    if( ! BigFile::seek(itsFile,itsOffset+offset) ){
      perror(" load(): seek failed");
      cerr << "wanted: " << itsOffset+offset << endl;
      cerr << "  line: " << __LINE__ << endl;
      delete [] page;
      return 0;
    }

    int dstIndex = (itsPageDim-1-row) * itsPageDim;

    int res = ::read(itsFile, &page[dstIndex*sourceDepth], dstRowSize);

    if( res != dstRowSize ){
      perror(" load(): read failed");
      cerr << "wanted: " << dstRowSize << endl;
      cerr << "  line: " << __LINE__ << endl;
      delete [] page;
      return 0;
    }

    offset += srcRowSize;

  }
  return page;
}

// Raw::PageReader::isReaderDone()
//////////////////////////////////////////////////////////////////////////
// PURPOSE: called by readerThread to keep going
//  INPUTS:
// OUTPUTS:
// RETURNS:
//   NOTES:
//////////////////////////////////////////////////////////////////////////

bool Raw::PageReader::isReaderDone(void)
{
  bool res=false;
  pthread_mutex_lock(&itsDoneLock);
  res = itsDoneFlag;
  pthread_mutex_unlock(&itsDoneLock);
  return res;
}

// Raw::PageReader::openFile()
//////////////////////////////////////////////////////////////////////////
// PURPOSE: tries to open the paged file
//  INPUTS:
// OUTPUTS:
// RETURNS:
//   NOTES: tries to open the file otherwise throws
//////////////////////////////////////////////////////////////////////////

void
Raw::PageReader::openFile(void)
{
  FANCYMESG("Raw::PageReader::openFile()");
  itsFile = ::open(itsFilename.c_str(),O_RDONLY|O_BINARY|O_LARGEFILE);
  if( itsFile == -1 ){
    cerr << "Raw::PageReader::openFile: open("
	 <<itsFilename<<") failed!" <<endl;
    perror("open");
    throw exception();
  }
}

// Raw::PageReader::closeFile()
//////////////////////////////////////////////////////////////////////////
// PURPOSE: tries to close the paged file
//  INPUTS:
// OUTPUTS:
// RETURNS:
//   NOTES:
//////////////////////////////////////////////////////////////////////////

void
Raw::PageReader::closeFile(void)
{
  FANCYMESG("Raw::PageReader::closeFile()");
  ::close(itsFile);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////// PAGE UTILITITES /////////////////////////////
//////////////////////////////////////////////////////////////////////////

void
Raw::PageReader::mipmapSize(int lod,
			    int imageWidth, int imageHeight,
			    int& width, int& height)
{
  height = imageHeight>>lod;
  width = imageWidth>>lod;
}

void
Raw::PageReader::pageCounts(int lod, int pageDim,
			    int imageWidth, int imageHeight,
			    int& pageCols, int& pageRows)
{
  int exp=0;
  bool found=false;
  while( ! found ){
    int p = pow2(exp);
    if( p==pageDim )
      found=true;
    else
      ++exp;
  }
  pageRows = (imageHeight>>exp>>lod) + 1;
  pageCols = (imageWidth>>exp>>lod) + 1;
}

int 
Raw::PageReader::maxLOD(int pageSize, int imageWidth, int imageHeight)
{
  int rows = (int)(imageHeight/pageSize) + 1;
  int cols = (int)(imageWidth/pageSize) + 1;
  int lod=0;
  
  while( (rows>1 || cols>1) ){
    imageHeight /= 2;
    imageWidth /= 2;
    rows = (int)(imageHeight/pageSize) + 1;
    cols = (int)(imageWidth/pageSize) + 1;
    ++lod;
  }
  return lod;
}

int
Raw::PageReader::pow2(int exp)
{
  int result = 1;
  while( exp-- > 0 )
    result *= 2;
  return result;
}

