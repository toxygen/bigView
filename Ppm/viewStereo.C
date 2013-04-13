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

const int maxWinX = 1024;
const int maxWinY = 1024;
enum {LEFT,RIGHT,NUMCHANNELS};

string inputFilenames[NUMCHANNELS];
unsigned char* inputPixels=0;
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
int bytesPerPixel=0;


//////////////////////////////////////////////////////////////////////////
/////////////////////////// METHOD DECLARATIONS //////////////////////////
//////////////////////////////////////////////////////////////////////////

void draw(const envFlagsPtr flags, void* user);
void fillArray(unsigned char* srcBytes,unsigned char* dstBytes,
	       int srcX, int srcY,int dstWidth,int dstHeight);
void startMove(float x, float y, void*);
void trackMove(float x, float y, void* user);

//////////////////////////////////////////////////////////////////////////
/////////////////////////// METHOD DEFINITIONS ///////////////////////////
//////////////////////////////////////////////////////////////////////////

void 
draw(const envFlagsPtr flags, void* user)
{
  OpenEnv2D* env = static_cast<OpenEnv2D*>(user);
  GLenum componentType = GL_RGB;
  GLenum componentSize = GL_UNSIGNED_BYTE;

  if( extraction==0 ) return;
  fillArray(inputPixels,extraction,
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

  if( env->getWinWidth() != srcSizeX || env->getWinHeight() != srcSizeY ){
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
}

void 
fillArray(unsigned char* srcBytes,unsigned char* dstBytes,
	  int srcX, int srcY,
	  int dstWidth,int dstHeight)
{
  int srcBase = srcY * srcSizeX + srcX;
  int size = bytesPerPixel;
  for(int row = 0 ; row < dstHeight ; row++ ){
    int srcIndex = srcBase + row*srcSizeX;
    int dstIndex = row*dstWidth;
    memcpy(&dstBytes[dstIndex*size],&srcBytes[srcIndex*size],dstWidth*size);
  }  
}

void startMove(float x, float y, void*)
{
  startX = x;
  startY = y;
}

void trackMove(float x, float y, void* user)
{
  OpenEnv2D* env = static_cast<OpenEnv2D*>(user);
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

  env->wakeup();
}

int main(int argc, char** argv)
{
  PPM::Format format;
  int cpp,bpc;
  int sizeX[NUMCHANNELS],sizeY[NUMCHANNELS];
  unsigned char* pixels[NUMCHANNELS]={0};
  bool signedPixels=false,forceNormalize=false,swapPixels=false;
  int numPixels=0;

  if( argc == 1 ){
    cerr << "usage: " << argv[0] << " <left ppm> <right ppm>" << endl;
    return -1;
  }

  pixels[LEFT] = 
    PPM::load(argv[1],&format,&cpp,&bpc,
	      &sizeX[LEFT],&sizeY[LEFT],
	      signedPixels,forceNormalize,swapPixels);
  if( pixels[LEFT] == NULL ){
    cerr << "error loading file:" << argv[1] << endl;
    return -1;
  } else if( bpc != 8 || cpp != 1){
    cerr << "greyscale images only" << endl;
    return -1;
  }

  pixels[RIGHT] = 
    PPM::load(argv[2],&format,&cpp,&bpc,
	      &sizeX[RIGHT],&sizeY[RIGHT],
	      signedPixels,forceNormalize,swapPixels);
  if( pixels[RIGHT] == NULL ){
    cerr << "error loading file:" << argv[2] << endl;
    return -1;
  } else if( bpc != 8 || cpp != 1){
    cerr << "greyscale images only" << endl;
    return -1;
  }
  
  if( (sizeX[LEFT] != sizeX[RIGHT]) || (sizeY[LEFT] != sizeY[RIGHT]) ){
    cerr << "images must be the same size" << endl;
    return -1;
    
  }
  
  numPixels = sizeX[LEFT] * sizeY[LEFT];
  bytesPerPixel = 3;
  
  int size = bytesPerPixel * numPixels;
  inputPixels = new unsigned char[size];
  bzero(inputPixels,size);

  int lOff=0;  // left=red
  int rOff=2; // right=blue
  for(int i=0;i<numPixels;++i){
    int index=i*bytesPerPixel;
    inputPixels[index+0]=pixels[ LEFT][i];
    inputPixels[index+1]=pixels[RIGHT][i];
    inputPixels[index+2]=pixels[RIGHT][i];
  }

  dstSizeX = srcSizeX = sizeX[LEFT];
  dstSizeY = srcSizeY = sizeY[LEFT];
  if( dstSizeX > maxWinX ) dstSizeX = maxWinX;
  if( dstSizeY > maxWinY ) dstSizeY = maxWinY;

  int dstSize=dstSizeX*dstSizeY*bytesPerPixel;
  VAR(dstSize);
  extraction = new unsigned char[dstSize];
  assert(extraction);

  OpenEnv2D* env = new OpenEnv2D("pix");
  env->setWinSize(dstSizeX,dstSizeY);
  env->setMouseDownFunc(startMove, RIGHT_BUTTON,env);
  env->setMouseProcessFunc(trackMove, RIGHT_BUTTON, env);
  env->setRenderFunc(draw,env);
  env->mainLoop();
}
