//////////////////////////////////////////////////////////////////////////
///////////////////////////////// ppm.C //////////////////////////////////
//////////////////////////////////////////////////////////////////////////

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

static unsigned char* 
readBinaryPixels(ifstream&,int,int,int,int,bool,bool,bool);

static unsigned char* 
readPlainPixels(ifstream&,int,int,int,int,bool);

static void eatSpace(ifstream&);
static int pow2(int);
static void swapShorts(void*, int);
static void normalize(void*, int);

static void eatSpace(int fin);
static char getOneChar(int fin);
static void ungetOneChar(int fin);
static int  getOneInt(int fin);
static void getline(int fin, char* lineBuf, int len);

bool PPM::verbose=false;

// PPM::ppmHeader()
//////////////////////////////////////////////////////////////////////////
// PURPOSE: scans the header of a pnm file returning info if possible
//  INPUTS: string filename: pathname of file
//           Format* format: format determined by scan
//                 int* cpp: components per pixel: 1=grey,3=rgb,
//                 int* bpc: bits per component: 8=bytes,16=shorts
//               int* sizeX: X dim of image
//               int* sizeY: Y dim of image
//   streampos* imageOffset: start of image data
// OUTPUTS: 
// RETURNS:
//   NOTES:
//////////////////////////////////////////////////////////////////////////
int 
PPM::ppmHeader(string filename, PPM::Format* format, 
	       int* cpp, int* bpc,
	       int* sizeX, int* sizeY,
	       int* imageOffset)
{
  const int BSIZE=2048;
  std::ostringstream err;
  char magic[3],hdr[BSIZE],line[BSIZE],junk;
  int res,max;

  *format=PPM::UNKNOWN;
  *cpp=0;
  *bpc=0;
  *sizeX=0;
  *sizeY=0;

  int fin = open(filename.c_str(),O_RDONLY|O_BINARY|O_LARGEFILE);
  if( fin == -1 ){
    perror("open");
    err << "PPM::ppmHeader("<<filename<<") : open() failed";
    throw std::logic_error(err.str());
  }

  res = read(fin, hdr, 2048);
  if( res <=0 ){
    perror("read");
    err << "PPM::ppmHeader("<<filename<<") : read() failed";
    throw std::logic_error(err.str());
  }
  close(fin);
  istringstream istr(hdr);
  istr>>magic[0]>>magic[1];
  magic[2]='\0';

  if( verbose )
    cout << "===== " << filename << " =====" << endl;
  
  if( ! strcmp(magic,"P6") ){
    *format=PPM::BINARY_PPM;
    *cpp=3;
    if( verbose )
      cout << "Format: Binary PPM" << endl;
  } else if( !strcmp(magic,"P3")){
    *format=PPM::PLAIN_PPM;
    *cpp=3;
    if( verbose )
      cout << "Format: Plain PPM" << endl;
  } else if( !strcmp(magic,"P5")){
    *format=PPM::BINARY_PGM;
    *cpp=1;
    if( verbose )
      cout << "Format: Binary PGM" << endl;
  } else if( !strcmp(magic,"P2")){
    *format=PPM::PLAIN_PGM;
    *cpp=1;
    if( verbose )
    cout << "Format: Binary PGM" << endl;
  }

  if( *cpp==0 || *format==PPM::UNKNOWN ){
    err << "PPM::ppmHeader("<<filename<<") : Unknown format";
    throw std::logic_error(err.str());
  }

  // skip ws after magic
  junk=istr.peek();
  while( isspace(junk) ){
    istr.ignore(1);
    junk=istr.peek();
    if( istr.tellg()>=BSIZE )
      throw std::runtime_error("header overflow");
  }

  while( junk == '#' ){    
    istr.getline(line,2048);
    if( istr.tellg()>=BSIZE )
      throw std::runtime_error("header overflow");
    if( verbose )
      cout << "Comment:"<<line<<":"<<endl;
    junk=istr.peek();
    while( isspace(junk) ){
      istr.ignore(1);
      junk=istr.peek();
      if( istr.tellg()>=BSIZE )
	throw std::runtime_error("header overflow");
    }
  } 

  istr>>*sizeX>>*sizeY>>max;
  istr.ignore(1);
  if( istr.tellg()>=BSIZE )
    throw std::runtime_error("header overflow");

  for(int i=0 ; i<=16 && *bpc==0 ; i++){
    int mask=pow2(i)-1;
    if( mask==max )
      *bpc=i;
  }
  if( *bpc==0 ){
    cerr << "Unknown pixel depth" << endl;
    close(fin);
    throw std::runtime_error("cannot parse header");
  }  

  *imageOffset = istr.tellg();
  int bytesPerPixel = *cpp * (*bpc/8);
  
  if( verbose ){
    cout << "Max Pixel Value = " << max << endl;
    cout << "          Width = " << *sizeX << endl;
    cout << "         Height = " << *sizeY << endl;
    cout << "     Components = " << *cpp << "[1=grey,3=rgb]"<<endl;
    cout << "      Bit Depth = " << *bpc << " bits" << endl;
    cout << "     Pixel Size = " << bytesPerPixel << " bytes" << endl;
    cout << "         Offset = " << *imageOffset << " bytes" << endl;
  }
  close(fin);
  return 1;
}

// PPM::load()
//////////////////////////////////////////////////////////////////////////
// PURPOSE: loads a pnm file returning pixels if possible
//  INPUTS: string filename: pathname of file
//           Format* format: format determined by scan
//                 int* cpp: components per pixel: 1=grey,3=rgb,
//                 int* bpc: bits per component: 8=bytes,16=shorts
//               int* sizeX: X dim of image
//               int* sizeY: Y dim of image
// OUTPUTS: 
// RETURNS:
//   NOTES: throws if fail cannot be opened 
//////////////////////////////////////////////////////////////////////////

unsigned char*  
PPM::load(std::string filename, PPM::Format* format, 
	  int* cpp, int* bpc, int* sizeX, int* sizeY)
{
  return PPM::load(filename,format,cpp,bpc,sizeX,sizeY,false,false,false);
}

// PPM::load()
//////////////////////////////////////////////////////////////////////////
// PURPOSE: loads a pnm file returning pixels if possible
//          this version is for 16bit signed/unsigned shorts
//          like the MOLA elevation maps
//  INPUTS: string filename: pathname of file
//           Format* format: format determined by scan
//                 int* cpp: components per pixel: 1=grey,3=rgb,
//                 int* bpc: bits per component: 8=bytes,16=shorts
//               int* sizeX: X dim of image
//               int* sizeY: Y dim of image
//        bool signedPixels:  
//      bool forceNormalize:
//          bool swapPixels: 
// OUTPUTS: 
// RETURNS:
//   NOTES: throws if fail cannot be opened 
//////////////////////////////////////////////////////////////////////////

unsigned char* 
PPM::load(std::string filename, PPM::Format* format, 
	  int* cpp, int* bpc,
	  int* sizeX, int* sizeY,
	  bool signedPixels,
	  bool forceNormalize,
	  bool swapPixels)
{
  unsigned char* pixels=0;
  int off;
  try {
    PPM::ppmHeader(filename,format,cpp,bpc,sizeX,sizeY,&off);
  } catch( exception& e ){
    cerr<<"PPM::load() caught exception:["<<e.what()<<"]\n";
    throw;
  }
  
  int bytesPerPixel = *cpp * (*bpc/8);
  int componentsPerPixel = *cpp;
  int bitsPerComponent = *bpc;

  ifstream fin(filename.c_str());
  if( ! fin ){
    perror("open");
    return NULL;
  }
  fin.seekg(off, ios::beg);
  MESGVAR("FILEPOS",fin.tellg());
  switch( *format ){
    case BINARY_PGM:
    case BINARY_PPM:
      pixels = readBinaryPixels(fin,*sizeX,*sizeY,*cpp,*bpc,
				signedPixels,forceNormalize,swapPixels);
      break;
    case PLAIN_PGM:
    case PLAIN_PPM:
      pixels = readPlainPixels(fin,*sizeX,*sizeY,*cpp,*bpc,signedPixels);
      break;
    default:
      cerr << "PPM::load() : Unknown format" << endl;
      return NULL;
  }
  fin.close();
  return pixels;
}

// readBinaryPixels()
//////////////////////////////////////////////////////////////////////////
// PURPOSE: reads pixels from a ppm assumed to be in binary [raw] format
//  INPUTS: ifstream& fin: source file
//              int width: width of image
//             int height: height of image
//                int cpp: componentsPerPixel ; 1=luminosity,3=rgb
//                int bpc: bitsPerComponent   ; 8=[unsigned] char
//      bool signedPixels:  
//    bool forceNormalize:
//        bool swapPixels: 
// OUTPUTS:
// RETURNS:
//   NOTES: assumes the images is stored such that the rows
//          near the beginning of the file are at the top of the image
//          i.e. if you were to read the file directly into memory
//          and then do a glDrawPixels, the image would be upsidedown.
//          Thus this method inverts the image from it's layout
//          in the file.
//////////////////////////////////////////////////////////////////////////

static unsigned char*
readBinaryPixels(ifstream& fin, int width, int height,
		 int cpp, int bpc, 
		 bool signedPixels,
		 bool forceNormalize,
		 bool swapPixels)
{
  FANCYMESG("readBinaryPixels");
  VAR(width);
  VAR(height);
  VAR(cpp);
  VAR(bpc);
  VAR(signedPixels);
  VAR(forceNormalize);
  VAR(swapPixels);

  int bytesPerPixel = cpp * (bpc/8);
  VAR(bytesPerPixel);
  u_int64_t size = (u_int64_t)width*height*bytesPerPixel;
  VAR(size);
  unsigned char* pix=new unsigned char[size];
  assert( pix );
  for(int r=height-1; r>=0 ; r-- ){
    u_int64_t offset = (u_int64_t)r*width*bytesPerPixel;
    fin.read((char*)&pix[offset],width*bytesPerPixel);
#if 0
    cout << (int)pix[offset+0] << " "
	 << (int)pix[offset+1] << " "
	 << (int)pix[offset+2] << endl;
#endif

  }
  if( swapPixels ){
    u_int64_t num = (u_int64_t)width*height;
    VAR(num);
    swapShorts((unsigned short*)pix, num);
  }
  if( signedPixels && forceNormalize){
    u_int64_t num = (u_int64_t)width*height;
    VAR(num);
    normalize((unsigned short*)pix, num);
  }
  return pix;
}

// PPM::readPlainPixels()
//////////////////////////////////////////////////////////////////////////
// PURPOSE: reads pixels from a ppm assumed to be in ascii [plain] format
//  INPUTS: ifstream& fin: source file
//              int width: width of image
//             int height: height of image
//                int cpp: componentsPerPixel ; 1=luminosity,3=rgb
//                int bpc: bitsPerComponent   ; 8=[unsigned] char
// OUTPUTS:
// RETURNS:
//   NOTES:
//////////////////////////////////////////////////////////////////////////

static unsigned char*
readPlainPixels(ifstream& fin, int width, int height,
		int cpp, int bpc, bool signedPixels)
{
  FANCYMESG("readPlainPixels");
  VAR(width);
  VAR(height);
  VAR(cpp);
  VAR(bpc);
  VAR(signedPixels);

  int bytesPerPixel = cpp * (bpc/8);
  int size = width*height*bytesPerPixel;
  VAR(size);
  unsigned char* pix=new unsigned char[size];
  assert( pix );
  VAR(bytesPerPixel);

  int buf;
  char* scp = (char*)pix;
  unsigned char* ucp = (unsigned char*)pix;
  short* ssp = (short*)pix;
  unsigned short* usp = (unsigned short*)pix;

  for(int row =height-1; row >=0 ; --row ){
    int offset = row * width;
    for(int col=0 ; col<width ; ++col ){
      int index = (offset+col)*cpp;	
      for(int c=0;c<cpp;c++){
	fin >> buf;
	switch( bpc ){
	  case 8:
	    if(signedPixels)
	      scp[index+c] = (char)buf;
	    else 
	      ucp[index+c] = (unsigned char)buf;
	    break;
	  case 16:
	    if(signedPixels)
	      ssp[index+c] = (short)buf;
	    else 
	      usp[index+c] = (unsigned short)buf;
	    break;
	  default:
	    assert(0);
	}
      }
    }
    // read a row
  }
  return pix;
}

static void 
normalize(void* buf, int numShorts)
{
  register int i;
  register short* src = (short*)buf;
  short mn=0x7fff,mx=-mn;
  for( i = 0 ; i < numShorts ; i++, src++ ){
    if( *src<mn ) mn=*src;
    if( *src>mx ) mx=*src;
  }
  src = (short*)buf;
  float width = (float)(mx-mn);
  for( i = 0 ; i < numShorts ; i++, src++ ){
    float t = (float)(*src-mn)/width;
    *src = (short)(t * (float)0x7fff);
  }
}

static void 
swapShorts(void* buf, int numShorts)
{
  register int i;
  register unsigned short* src = (unsigned short*)buf;
  for( i = 0 ; i < numShorts ; i++, src++ ){
    *src = ((*src & 0x00ff) << 8) | ((*src & 0xff00) >> 8);
  }
}

static void 
eatSpace(ifstream& fin)
{
  unsigned char ws=' ';
  while( isspace(ws) )
    fin >> ws;
  fin.putback(ws);
}

static int 
pow2(int exp)
{
  int result = 1;
  while( exp-- > 0 )
    result *= 2;
  return result;
}

static void getline(int fin, char* lineBuf, int len)
{
  bool done=false;
  int index=0;
  lineBuf[index]=' '; 
  while(! done && index<len ){
    lineBuf[index] = getOneChar(fin); 
    if( lineBuf[index]==10 ) {
      lineBuf[index]=0;
      done=true;
    }
    ++index;
  }
  lineBuf[index]=0;
}

static void 
eatSpace(int fin)
{
  unsigned char ws=' ';
  int gotSpace=0;
  while( isspace(ws) ){
    ws = getOneChar(fin);
    gotSpace=1;
  }
  if( gotSpace )
    ungetOneChar(fin);
}

static int getOneInt(int fin)
{
  int res=0;
  unsigned char ch='0';
  eatSpace(fin);
  while( ! isspace(ch) ){
    ch = getOneChar(fin);
    if( ! isspace(ch) ){
      res = res*10 + ch-'0';
    }
  }
  ungetOneChar(fin);
  return res;
}

static char getOneChar(int fin)
{
  char junk;
  int res = read(fin, &junk, sizeof(char));
  if( res != sizeof(char) ){
    std::ostringstream err;
    perror("read");
    err << "read() failed";
    throw std::logic_error(err.str());
  }
  return junk;
}

static void ungetOneChar(int fin)
{
  BigFile::Offset pos = BigFile::tell(fin);
  BigFile::seek(fin,pos-1);
}

////////////////////////////////////////////////////////////
// Utilities
////////////////////////////////////////////////////////////

void
PPM::swapWords(unsigned* buf, int numWords)
{
  register int i;
  register unsigned* src = buf;

  for(i = 0 ; i < numWords ; i++, src++ ){
    *src = ((*src & 0x000000ff) << 24) |
      ((*src & 0x0000ff00) << 8) |
      ((*src & 0x00ff0000) >> 8) |
      ((*src & 0xff000000) >> 24);
  }
}

void
PPM::swapShorts(unsigned short* buf, int numShorts)
{
  register int i;
  register unsigned short* src = buf;

  for(i = 0 ; i < numShorts ; i++, src++ ){
    *src = ((*src & 0x00ff) << 8) | ((*src & 0xff00) >> 8);
  }
}
