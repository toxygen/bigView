//////////////////////////////////////////////////////////////////////////
/////////////////////////////// genPaged.C ///////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <fcntl.h>
#include <math.h>
#include <map>
#include <GL/glu.h>
#include <bigFile.h>
#include <assert.h>

//#define DEBUG 1
#include "debug.h"
#include "GLX.h"
#include "defs.h"
#include "ostr.h"
#include "ppm.h"
#include "PageReader.h"

using namespace std;

const int pageDim = Raw::PAGE_SIZE;
int pageSize = pageDim*pageDim;
unsigned char* page=0;
int sourceOffset=0;
int sourceWidth=0;
int sourceHeight=0;
int sourceFormat=GL_RGB;
int sourceType=GL_UNSIGNED_BYTE;
int dstOffset=0;
bool signedPixels=false;
map<string,Raw::Coord> coordMap;


int
pow2(int exp)
{
  int result = 1;
  while( exp-- > 0 )
    result *= 2;
  return result;
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


int 
maxLOD(int width, int height)
{
  int rows = (int)(height/Raw::PAGE_SIZE) + 1;
  int cols = (int)(width/Raw::PAGE_SIZE) + 1;
  int lod=0;

  while( (rows>1 || cols>1)){
    height /= 2;
    width /= 2;
    rows = (int)(height/Raw::PAGE_SIZE) + 1;
    cols = (int)(width/Raw::PAGE_SIZE) + 1;
    ++lod;
  }
  return lod;
}

void
mipmapSize(int lod,
	   int imageWidth, int imageHeight,
	   int& width, int& height)
{
  height = imageHeight>>lod;
  width = imageWidth>>lod;
}

void
pageCounts(int lod,
	   int imageWidth, int imageHeight,
	   int& pageCols, int& pageRows)
{
  pageRows = (imageHeight>>Raw::PAGE_SIZE_EXP>>lod) + 1;
  pageCols = (imageWidth>>Raw::PAGE_SIZE_EXP>>lod) + 1;
}

void 
toCoord(string& str,int& row, int& col)
{
  row=col=0;
  int l = str.length();
  for(int i=1 ; i<l ; ++i ){
    switch( str[i] ){
      case '1': 
	col += pow2(l-i-1); 
	break;
      case '2': 
	row += pow2(l-i-1); 
	break;
      case '3': 
	col += pow2(l-i-1); 
	row += pow2(l-i-1); 
	break;
    }
  }
}

void buildCoordmap(string str, int level, int maxlevel)
{
  if( ++level > maxlevel ){
    int row,col;
    toCoord(str,row,col);
    coordMap[str] = Raw::Coord(col,row);
    return;
  }
  buildCoordmap(str+"0",level,maxlevel);
  buildCoordmap(str+"1",level,maxlevel);
  buildCoordmap(str+"2",level,maxlevel);
  buildCoordmap(str+"3",level,maxlevel);
}

void storePage(unsigned char* dst, unsigned char* src,
	       int colStart, int rowStart)
{
  int depth = calcSourceDepth(sourceFormat,sourceType);
  for(int row=0 ; row<pageDim ; ++row){
    int srcIndex = row*pageDim;
    int dstIndex = (rowStart+row)*2*pageDim + colStart;
    memcpy(&dst[dstIndex*depth],&src[srcIndex*depth],pageDim*depth);
  }
}

void
load(int& fin,
     int row, int col,
     int srcWidth, int srcHeight, 
     int pageDim, unsigned char* page)
{
  BigFile::Offset _row = (BigFile::Offset)row;
  BigFile::Offset _col = (BigFile::Offset)col;
  BigFile::Offset _srcWidth = (BigFile::Offset)srcWidth;
  BigFile::Offset _srcHeight = (BigFile::Offset)srcHeight;
  BigFile::Offset _pageDim = (BigFile::Offset)pageDim;
  BigFile::Offset _sourceDepth = 
    (BigFile::Offset)calcSourceDepth(sourceFormat,sourceType);

  BigFile::Offset xPixelOffset = _col * _pageDim;
  BigFile::Offset yPixelOffset = _srcHeight-((_row+1)*_pageDim);

  // NOTE: offset may be negative here
  BigFile::Offset offset = (yPixelOffset*_srcWidth+xPixelOffset)*_sourceDepth;

  BigFile::Offset readSize = (xPixelOffset+_pageDim>_srcWidth ) ? 
    (_srcWidth-xPixelOffset) : _pageDim;

  BigFile::Offset seekoff = sourceOffset+offset;
  if( offset > 0 ){
    if( ! BigFile::seek(fin,seekoff) ){
      perror(" load(): seek failed");
      cerr << "wanted: " << seekoff << endl;
      cerr << "  line: " << __LINE__ << endl;
      exit(0);
    } 
  }

  BigFile::Offset srcRowSize = _srcWidth * _sourceDepth;
  BigFile::Offset dstRowSize = readSize * _sourceDepth;
  bzero(page,pageSize);

  for(BigFile::Offset dstrow=0; dstrow<_pageDim; ++dstrow ){

    if( offset < 0 ){
      offset += srcRowSize;
      continue;
    } 

    if( ! BigFile::seek(fin,sourceOffset+offset) ){
      perror(" load(): seek failed");
      cerr << "wanted: " << sourceOffset+offset << endl;
      cerr << "  line: " << __LINE__ << endl;
      exit(0);
    }

    BigFile::Offset dstIndex = (_pageDim-1-dstrow) * _pageDim;

    int res = ::read(fin, &page[dstIndex*_sourceDepth], dstRowSize);
    if( res != dstRowSize ){
      perror(" load(): read failed");      
      cerr << "wanted: " << dstRowSize << endl;
      cerr << "   got: " << res << endl;
      cerr << "  line: " << __LINE__ << endl;
      exit(0);
    }
    offset += srcRowSize;
  }
}

void scaleDownGreyUnsigned(unsigned char* fourPages, unsigned char* pixels)
{
  unsigned short* src = (unsigned short*)fourPages;
  unsigned short* dst = (unsigned short*)pixels;
  int depth = calcSourceDepth(sourceFormat,sourceType);
  long grey;
  int srcDim = pageDim<<1;
  int srcDim2 = srcDim<<1;
  int rpagedim=0;
  int srcR = 0;
  int rsrcdim = 0;
  for(int r=0;r<pageDim; ++r){
    for(int c=0;c<pageDim;++c){
      int dstIndex = rpagedim+c;
      int srcC = c<<1;
      int srcIndex = rsrcdim+srcC;
      int s0r = srcIndex;
      int s1r = srcIndex+1;
      int s2r = srcIndex+srcDim;
      int s3r = srcIndex+srcDim+1;
      switch( depth ){
	case 1:
	  grey = fourPages[s0r]+fourPages[s1r]+fourPages[s2r]+fourPages[s3r];
	  pixels[dstIndex]=(unsigned char)(grey>>2);
	  break;
	case 2:
	  grey = src[s0r]+src[s1r]+src[s2r]+src[s3r];
	  dst[dstIndex]=(unsigned short)(grey>>2);
	  break;
      }
    }
    rpagedim += pageDim;
    srcR += 2;
    rsrcdim += srcDim2;
  }
}

void scaleDownGrey(unsigned char* fourPages, unsigned char* pixels)
{
  short* src = (short*)fourPages;
  short* dst = (short*)pixels;
  int depth = calcSourceDepth(sourceFormat,sourceType);
  int grey;
  int srcDim = 2*pageDim;
  for(int r=0;r<pageDim; ++r){
    for(int c=0;c<pageDim;++c){
      int dstIndex = r*pageDim+c;
      int srcR = r*2;
      int srcC = c*2;
      int srcIndex = (srcR)*srcDim+srcC;
      int s0r = srcIndex;
      int s1r = srcIndex+1;
      int s2r = srcIndex+srcDim;
      int s3r = srcIndex+srcDim+1;
      switch( depth ){
	case 1:
	  grey = fourPages[s0r]+fourPages[s1r]+fourPages[s2r]+fourPages[s3r];
	  pixels[dstIndex]=grey/4;
	  break;
	case 2:
	  grey = src[s0r]+src[s1r]+src[s2r]+src[s3r];
	  dst[dstIndex]=grey/4;
	  break;
      }
    }
  }
}

void scaleDownRGB(unsigned char* fourPages, unsigned char* pixels)
{
  // BLUTE: this assumes 24bit rgb, 
  // i.e. 1 byte [unsigned char] per component
  int rgb[3];
  int srcDim = 2*pageDim;
  int rowDim = 3*srcDim;
  int pageDim3 = 3*pageDim;
  int srcDim6 = 6*srcDim;
  int rowSrc = 0;
  int rpagedim = 0;
  int rpagedim3 = 0;
  for(int r=0;r<pageDim; ++r){
    for(int c=0;c<pageDim;++c){
      int dstIndex = rpagedim3+3*c;
      int s0r = rowSrc+(c*6);
      int s2r = s0r+rowDim;
      for(int i=0;i<3;i++)
	rgb[i] = 
	  fourPages[s0r + i] +
	  fourPages[s0r + i + 3] +
	  fourPages[s2r + i] +
	  fourPages[s2r + i + 3];
      pixels[dstIndex+0]=rgb[0]>>2;
      pixels[dstIndex+1]=rgb[1]>>2;
      pixels[dstIndex+2]=rgb[2]>>2;
    }
    rpagedim += pageDim;
    rpagedim3 += pageDim3;
    rowSrc += srcDim6;
  }
}

bool needstorage=true;
unsigned char** pix0=0;
unsigned char** pix1=0;
unsigned char** pix2=0;
unsigned char** pix3=0;
unsigned char** fourPages=0;

void
allocBuffers(int maxlevel)
{
  pix0 = new unsigned char*[maxlevel];
  pix1 = new unsigned char*[maxlevel];
  pix2 = new unsigned char*[maxlevel];
  pix3 = new unsigned char*[maxlevel];
  fourPages = new unsigned char*[maxlevel];
  for(int i=0; i<maxlevel ; ++i){
    pix0[i]=new unsigned char[pageSize];
    pix1[i]=new unsigned char[pageSize];
    pix2[i]=new unsigned char[pageSize];
    pix3[i]=new unsigned char[pageSize];
    fourPages[i]=new unsigned char[4*pageSize];
  }
}
string lvlStr(int lvl){
  string res;
  while(lvl--) res += " ";
  return res;
}
void buildMipmaps(int& fin, int& fout,
		  string str, int cols, int rows, 
		  int level, int maxlevel, string& res,
		  unsigned char* pixels)
{  
  string s0 = str+"0";
  string s1 = str+"1";
  string s2 = str+"2";
  string s3 = str+"3";
  string r0,r1,r2,r3;

  FANCYMESG("buildMipmaps");
  MESGVAR(lvlStr(level),str);
  MESGVAR(lvlStr(level),cols);
  MESGVAR(lvlStr(level),rows);
  MESGVAR(lvlStr(level),level);
  MESGVAR(lvlStr(level),maxlevel);
  MESGVAR(lvlStr(level),res);

  if( ++level > maxlevel ){
    Raw::Coord c = coordMap[str];
    if( c.first>=cols || c.second>=rows ){
      MESGVAR(lvlStr(level)+"over",c); 
      return;
    }
    res = "["+str+"]";
    load(fin,c.second,c.first,sourceWidth,sourceHeight,pageDim,pixels);

    BigFile::Offset offset = 
      (BigFile::Offset)c.second * (BigFile::Offset)cols + 
      (BigFile::Offset)c.first;

    BigFile::Offset thisOffset = 
      (BigFile::Offset)dstOffset+offset*(BigFile::Offset)pageSize;

    if( ! BigFile::seek(fout,thisOffset) ){
      perror("seek failed [before write]");
      cerr << "wanted:" << dstOffset+(offset*pageSize) << endl;
      exit(0);
    }
    if( write(fout,pixels,pageSize) != pageSize ){
      perror("write failed");
      exit(0);
    }
    MESGVAR(lvlStr(level)+"wrote",c); 
    return;
  }
  buildMipmaps(fin,fout,s0,cols,rows,level,maxlevel,r0,pix0[level]);
  buildMipmaps(fin,fout,s1,cols,rows,level,maxlevel,r1,pix1[level]);
  buildMipmaps(fin,fout,s2,cols,rows,level,maxlevel,r2,pix2[level]);
  buildMipmaps(fin,fout,s3,cols,rows,level,maxlevel,r3,pix3[level]);
  res = r0+r1+r2+r3;
  if(res.length()==0)
    return;

  bzero(fourPages[level],4*pageSize);
  if(r0.length()) storePage(fourPages[level],pix0[level],0,0);
  if(r1.length()) storePage(fourPages[level],pix1[level],pageDim,0);
  if(r2.length()) storePage(fourPages[level],pix2[level],0,pageDim);
  if(r3.length()) storePage(fourPages[level],pix3[level],pageDim,pageDim);

  assert(fourPages[level]);
  assert(pixels);

  switch(sourceFormat){
    case GL_RGB:
      scaleDownRGB(fourPages[level], pixels);
      break;
    case GL_LUMINANCE:
      if( signedPixels )
	scaleDownGrey(fourPages[level], pixels);
      else
	scaleDownGreyUnsigned(fourPages[level], pixels);
      break;
  }

  int outRow,outCol,lvlcols,lvlrows;
  pageCounts(maxlevel-level+1,sourceWidth,sourceHeight,lvlcols,lvlrows);
  toCoord(str,outRow,outCol);
  if( outCol>lvlcols ) outCol=lvlcols;
  if( outRow>lvlrows ) outRow=lvlrows;

  BigFile::Offset offset=0;
  int max=maxlevel;
  for(int l=0;l<max-level+1;l++){
    int rows,cols;
    pageCounts(l,sourceWidth,sourceHeight,cols,rows);
    offset = offset + (BigFile::Offset)rows*(BigFile::Offset)cols;
  }
  offset += (BigFile::Offset)outRow * (BigFile::Offset)lvlcols + 
    (BigFile::Offset)outCol;
  BigFile::Offset seekoff = (BigFile::Offset)dstOffset + 
    offset*(BigFile::Offset)pageSize;
  if( ! BigFile::seek(fout,seekoff) ){
    perror("seek failed [before write]");
    cerr << "wanted:" << dstOffset+(offset*pageSize) << endl;
    exit(0);
  }
  if( write(fout,pixels,pageSize) != pageSize ){
    perror("write failed");
    exit(0);
  }
}

void buildPagedFile(int& fin, int& fout, int cols, int rows)
{
  int largest = rows>cols ? rows : cols;
  int level=0;
  bool found=false;

  for(level=0; level<10 && found==false ;++level)
  {
    int p = pow2(level);
    if( p>largest )
      found=true;
  }
  --level;
  cout << "Levels = " << level << endl;
  buildCoordmap("0",0,level);
  string res;
  unsigned char* resultPage=new unsigned char[pageSize];
  allocBuffers(level+1);
  buildMipmaps(fin,fout,"0",cols,rows,0,level,res,resultPage);
}

bool compare(string& s1,string& s2)
{
  int l1 = s1.length();
  int l2 = s2.length();
  if( l1 != l2 ) return true;
  for(int i=0 ; i < l1 ; i++){
    if( s1[i]!=s2[i] )
      return true;
  }
  return false;
}

int fw[8]={0,0,0,0,0,0,0,0};

void adjust(ostringstream& ostr, int index, int& value)
{
  int w = (int)floor(log10((double)value))+1;
  if(w>fw[index]) fw[index]=w;
  ostr.width(fw[index]);
}

void adjust(ostringstream& ostr, int index, BigFile::Offset& value)
{
  int w = (int)floor(log10((double)value))+1;
  if(w>fw[index]) fw[index]=w;
  ostr.width(fw[index]);
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

main(int argc, char** argv)
{
  int c;
  int bitsPerComponent=8;

  if( argc==1 ){
    cerr << "usage: toPaged [options] -h<int> -w<int> <raw image>"<<endl;
    cerr << "usage: toPaged [options] <ppm image>"<<endl;
    cerr << "----------------------------------------------------"<<endl;
    cerr << "Options:"<<endl;
    cerr << " -s                 : signed pixels" << endl;
    cerr << " -f<OpenGL keyword> : source format [GL_RGB...]"<<endl;
    cerr << " -t<OpenGL keyword> : source type [GL_BYTE...]"<<endl;
    return 0;
  }

  while( (c = getopt(argc,argv,"sh:w:f:t:")) != -1) 
  {
    switch( c ){
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
    } catch( exception& e ){
      cerr<<"Error opening file:"<<argv[optind]<<endl;
      return 1;
    }

    sourceWidth=sizeX;
    sourceHeight=sizeY;
    sourceOffset=off;
    sourceFormat = (cpp==1)?GL_LUMINANCE:GL_RGB;
    bitsPerComponent = bpc; 

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
  else
  {
    switch( sourceType ){
      case GL_UNSIGNED_BYTE:
	bitsPerComponent=8;
	signedPixels=false;
	break;
      case GL_BYTE:
	bitsPerComponent=8;
	signedPixels=true;
	break;
      case GL_SHORT:
	bitsPerComponent=16;
	signedPixels=true;
	break;
      case GL_UNSIGNED_SHORT:
	bitsPerComponent=16;
	signedPixels=false;
	break;
      default:
	cerr << "Unsupported input format!" << endl;
	return 0;
    }
  }
  int sourceFile = open(argv[optind],O_RDONLY|O_BINARY|O_LARGEFILE);
  if( sourceFile == -1 ){
    perror("open");
    cerr << "usage: toPaged -h<int> -w<int> <raw image>"<<endl;
    cerr << "usage: toPaged <ppm image>"<<endl;
    return 0;
  }
  string filename = argv[optind];
  filename += ".paged";

  int numRows=(int)(sourceHeight/pageDim) + 1;
  int numCols=(int)(sourceWidth/pageDim) + 1;
  int maxlod = maxLOD(sourceWidth,sourceHeight);

  ostringstream ostr;
  int lastHeaderSize=0;
  int headerSize=0;
  string hdr,lasthdr;
  bool nochange=false;
  int srcDepth = calcSourceDepth(sourceFormat,sourceType);
  int maxValue = pow2(bitsPerComponent)-1;

  pageSize *= srcDepth;

  do {
    int offset=0;
    BigFile::Offset totalBytes=lastHeaderSize;
    switch(sourceFormat ){
      case GL_RGB:
	ostr << "P6" << endl;
	break;
      case GL_LUMINANCE:
	ostr << "P5" << endl;
    }
    ostr << "# Paged format with mipmaps: "
	 << pageDim << "x" << pageDim << "x" << srcDepth << " = "
	 << pageSize << " bytes per page" << endl;
    for(int i=0; i<=maxlod ; ++i ){
      int width,height;
      int rows,cols;

      mipmapSize(i,sourceWidth,sourceHeight,width,height);
      pageCounts(i,sourceWidth,sourceHeight,cols,rows);
      
      int pageCount=rows*cols;

      ostr << "# Level ";
      adjust(ostr,0,i);
      ostr << i;
      ostr << " : [";
      adjust(ostr,1,width);
      ostr << width ;
      ostr << "," ;
      adjust(ostr,2,height);
      ostr << height ;
      ostr << "] => ";
      ostr << "[" ;
      adjust(ostr,3,cols);
      ostr << cols ;
      ostr << "," ;
      adjust(ostr,4,rows);
      ostr << rows ;
      ostr << "] = ";
      adjust(ostr,5,pageCount);
      ostr << pageCount;
      ostr << " pages starting at page ";
      adjust(ostr,6,offset);
      ostr << offset;
      ostr << " @ ";
      adjust(ostr,7,totalBytes);
      ostr << totalBytes ;
      ostr << " bytes" ;
      ostr <<endl;
      offset += pageCount;
      totalBytes = (BigFile::Offset)headerSize + 
	(BigFile::Offset)(offset)*pageSize;
    }
    BigFile::Offset pageBytes=(BigFile::Offset)offset*pageSize;
    ostr << "# ========================================" << endl;
    ostr << "# Total pages = " << offset << endl;
    ostr << "# Total bytes = header + pages" << endl; 
    ostr << "#             = "<< headerSize << " + " << pageBytes<<" = "
	 << totalBytes<<endl;
    ostr << sourceWidth << " " << sourceHeight << " " << maxValue << endl;
    lasthdr=hdr;
    hdr = ostr.str();
    ostr.str("");
    lastHeaderSize=headerSize;
    headerSize = hdr.length();
    nochange = ! compare(hdr,lasthdr);
  } while( (lastHeaderSize != headerSize) || ! nochange );

  int destFile = open(filename.c_str(),
		      O_WRONLY|O_BINARY|O_LARGEFILE|O_TRUNC|O_CREAT,0666);
  if( destFile == -1 ){
    perror("open");
    cerr << "usage: toPaged -h<int> -w<int> <raw image>"<<endl;
    cerr << "usage: toPaged <ppm image>"<<endl;
    return 0;
  }
  int res = write(destFile,hdr.c_str(),hdr.length());
  assert(res==hdr.length());
  dstOffset=headerSize;
  cout << hdr << endl;
  buildPagedFile(sourceFile,destFile,numCols,numRows);
}
