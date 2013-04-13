
#include <fstream>
#include <assert.h>
#include <stdio.h>
#include <exception>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <bigFile.h>
#include "ppm.h"
using namespace std;

//#define DEBUG 1
#include "debug.h"

PPM::Image::Image(std::string filename, int flags) :
  itsFilename(filename),
  itsFormat(PPM::UNKNOWN),
  itsComponentsPerPixel(0),
  itsBitsPerComponent(0),
  itsWidth(0),
  itsHeight(0),
  itsImageOffset(-1),
  itsFile(-1),
  itsFlags(flags|O_BINARY|O_LARGEFILE)  
{
  if( flags==O_RDONLY ){
    try {
      PPM::ppmHeader(itsFilename,
		     &itsFormat,
		     &itsComponentsPerPixel,
		     &itsBitsPerComponent,
		     &itsWidth,&itsHeight,&itsImageOffset);
    } catch( exception& e ){
      cerr << "PPM::Image::Image() : PPM::ppmHeader failed\n";
    }
  } else {
    itsFlags |= O_TRUNC|O_CREAT;
  }
  try {
    openFile(); 
  } catch( exception& e ){
    cerr << "PPM::Image::Image() : error opening file\n";
  }
}

PPM::Image::~Image(void)
{
  closeFile();
}

// flags = O_RDONLY | O_WRONLY
void
PPM::Image::openFile(void)
{
  std::ostringstream err;

  itsFile = ::open(itsFilename.c_str(),itsFlags,0666);
  if( itsFile == -1 ){
    err << "PPM::Image::openFile: open("<<itsFilename<<") failed!";
    perror("open");
    std::runtime_error(err.str());
  }
}

void
PPM::Image::closeFile(void)
{
  ::close(itsFile);
}

void 
PPM::Image::setImageParams(int cpp, int bpc,int w, int h)
{
  ostringstream ostr;
  itsFormat=PPM::BINARY_PPM;
  itsComponentsPerPixel=cpp;
  assert( itsComponentsPerPixel==3 || itsComponentsPerPixel==1 );
  itsBitsPerComponent=bpc;
  assert( itsBitsPerComponent==8 || itsBitsPerComponent==16 );
  itsWidth=w;
  itsHeight=h;
  switch( itsComponentsPerPixel ){
    case 1:
      ostr << "P5 " << endl;      
      break;
    case 3:
      ostr << "P6 " << endl;
      break;
  }
  int maxValue = PPM::Image::pow2(itsBitsPerComponent)-1;
  int bytesPerComponent=itsBitsPerComponent/8;
  ostr << itsWidth << " " << itsHeight << " " << maxValue << endl;
  string hdr = ostr.str();
  itsImageOffset = hdr.length();  
  BigFile::seek(itsFile,0);
  int res = ::write(itsFile,hdr.c_str(),hdr.length());
  VAR(res);
  assert(res==hdr.length());

  BigFile::Offset lastByte = itsImageOffset +
    itsComponentsPerPixel * bytesPerComponent * itsWidth * itsHeight - 1;
  if( ! BigFile::seek(itsFile,lastByte) ){
    perror(" setImageParams(): seek failed");
    cerr << "wanted: " << lastByte << endl;
    cerr << "  line: " << __LINE__ << endl;
    return;
  }
  unsigned char byte=0;
  res = ::write(itsFile, &byte, sizeof(unsigned char));
  assert(res==sizeof(unsigned char));
}

unsigned char* 
PPM::Image::loadSubimage(int srcCol, int srcRow, // offset into source image
			 int dstWidth,int dstHeight) // size of read
{
  int bytesPerComponent=itsBitsPerComponent/8;
  int srcOffset=itsImageOffset;
  int srcWidth=itsWidth,srcHeight=itsHeight;
  int sourceDepth=itsComponentsPerPixel*bytesPerComponent;
  
  BigFile::Offset xPixelOffset = srcCol;
  BigFile::Offset yPixelOffset = srcHeight-(srcRow+1); // PPM format

  yPixelOffset -= dstHeight-1;
  
  // NOTE: offset may be negative here
  BigFile::Offset offset = (yPixelOffset*srcWidth+xPixelOffset)*sourceDepth;
  
  BigFile::Offset readSize = (xPixelOffset+dstWidth>srcWidth ) ? 
    (srcWidth-xPixelOffset) : dstWidth;
  
  if( offset > 0 ) 
    BigFile::seek(itsFile,srcOffset+offset);
  
  int dstSize = dstWidth*dstHeight*sourceDepth;
  unsigned char* page = new unsigned char[dstSize];
  bzero(page,dstSize);
  bool done=false;

  int srcRowSize = srcWidth * sourceDepth;
  int dstRowSize = readSize * sourceDepth;

  for(int row=0; row<dstHeight && ! done; ++row ){
    if( offset < 0 ){
      offset += srcRowSize;
      continue;
    }
    if( ! BigFile::seek(itsFile,srcOffset+offset) ){
      perror(" load(): seek failed");
      cerr << "wanted: " << srcOffset+offset << endl;
      cerr << "  line: " << __LINE__ << endl;
      delete [] page;
      return 0;
    }

    int dstIndex = (dstHeight-1-row) * dstWidth;
    int res = ::read(itsFile, &page[dstIndex*sourceDepth], dstRowSize);
    if( res != dstRowSize ){
      perror(" load(): read failed");
      cerr << "    at: " << srcOffset+offset << " bytes" <<endl;
      cerr << "wanted: " << dstRowSize << endl;
      cerr << "   got: " << res << endl;
      cerr << "   row: " << row << endl;
      delete [] page;
      return 0;
    }
    offset += srcRowSize;	    
  }
  return page;
}

void
PPM::Image::storeSubimage(int dstCol, int dstRow, // offset into dst image
			  int srcWidth, int srcHeight, // size of write
			  unsigned char* pixels) // pixels to write
{
  int bytesPerComponent=itsBitsPerComponent/8;
  int dstOffset=itsImageOffset;
  int dstWidth=itsWidth,dstHeight=itsHeight;
  int dstDepth=itsComponentsPerPixel*bytesPerComponent;

  FANCYMESG("storeSubimage");
  VAR(dstCol);
  VAR(dstRow);
  VAR(srcWidth);
  VAR(srcHeight);

  BigFile::Offset xPixelOffset = dstCol;
  BigFile::Offset yPixelOffset = dstHeight-dstRow-1; // PPM format
  
  // NOTE: offset may be negative here
  BigFile::Offset offset = (yPixelOffset*dstWidth+xPixelOffset)*dstDepth;
  
  if( offset > 0 ) 
    BigFile::seek(itsFile,dstOffset+offset);

  bool done=false;
  int dstRowSize = dstWidth * dstDepth;

  for(int row=0; row<srcHeight; ++row ){
    if( offset < 0 ){
      offset -= dstRowSize;
      continue;
    }
    if( ! BigFile::seek(itsFile,dstOffset+offset) ){
      perror(" storeSubimage(): seek failed");
      cerr << "wanted: " << dstOffset+offset << endl;
      cerr << "  line: " << __LINE__ << endl;
      return;
    }

    int srcIndex = row*srcWidth;
    int writeSize = (dstCol+srcWidth>itsWidth ) ? 
      (itsWidth-dstCol) : srcWidth;
    if( writeSize<0 ){
      MESG("writeSize<0");
      VAR(writeSize);
      VAR(dstCol);
      VAR(itsWidth);
      VAR(srcWidth);
      continue;
    }

    int res = ::write(itsFile, &pixels[srcIndex*dstDepth], writeSize*dstDepth);
    if( res != writeSize*dstDepth ){
      perror(" storeSubimage(): write failed");
      cerr << "      at: " << dstOffset+offset << " bytes" <<endl;
      cerr << "srcIndex: " << srcIndex <<endl;
      cerr << "dstDepth: " << dstDepth <<endl;
      cerr << "  wanted: " << writeSize << endl;
      cerr << "     got: " << res << endl;
      cerr << "     row: " << row << endl;
      return;
    }
    VAR((int)pixels[srcIndex*dstDepth]);
    VAR((int)pixels[srcIndex*dstDepth+1]);
    VAR((int)pixels[srcIndex*dstDepth+2]);
    offset -= dstRowSize;	    
  }
}

int
PPM::Image::pow2(int exp)
{
  int result = 1;
  while( exp-- > 0 )
    result *= 2;
  return result;
}


