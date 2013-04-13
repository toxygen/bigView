#include <iostream>
#include <unistd.h>
#include <OpenEnv2D.h>
#include <ppm.h>
#include <glerr.h>
#include <gelTexture.h>

//#define DEBUG 1
#include "debug.h"

using namespace std;

const int maxWinX = 1280;
const int maxWinY = 1024;

OpenEnv2D* env=0;
int frustumRows=-1,frustumCols=-1,itsRow=0,itsCol=0;

PPM::Format format;
int cpp,bpc;
int off,srcSizeX,srcSizeY,dstSizeX,dstSizeY;
bool signedPixels=false,forceNormalize=false,swapPixels=false;
int bytesPerPixel,componentsPerPixel,bitsPerComponent;
unsigned char* pixels=0;
unsigned int texID=0;
float maxS=0,maxT=0;
int sourceFormat=0, sourceType=0;
int texWidth=0,texHeight=0;
bool needsTextureInit=true;

vector<string> inputFilenames;
vector<unsigned char*> inputPixels;
vector<unsigned char*>::iterator curPixels;
vector<string>::iterator curName;


void loadTexture(unsigned char* tex)
{
  int levelOfDetail=0;
  int border=0;

  FANCYMESG("loadTexture");
  VARHEX(tex);

  env->makeCurrent();

  if( needsTextureInit ){
    needsTextureInit=false;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 
    checkError("glPixelStorei");
    glGenTextures(1,&texID);
    checkError("glGenTextures");
    VAR(texID);
  }

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D,texID);
  checkError("glBindTexture");

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
  checkError("glTexEnvf");

  VAR(levelOfDetail);
  VAR(sourceFormat);
  VAR(texWidth);
  VAR(texHeight);
  VAR(border);
  VAR(sourceFormat);
  VAR(sourceType);

  glTexImage2D(GL_TEXTURE_2D, levelOfDetail, sourceFormat, 
	       texWidth, texHeight, border, 
	       sourceFormat, sourceType, 
	       tex);
  checkError("glTexImage2D");
}

void draw(envBase*,int* flags, void*)
{
  int index=flags[FRAME]%inputPixels.size();
  float aspect = (float)srcSizeX/srcSizeY;
  glPushAttrib(GL_DEPTH_BUFFER_BIT|GL_LIGHTING_BIT|GL_TEXTURE_BIT);

  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);

  loadTexture(inputPixels[index]);

  glBindTexture(GL_TEXTURE_2D,texID);
  checkError("glBindTexture");
  glBegin(GL_QUADS);
  glTexCoord2f(0, 0);       glVertex2f(0, 0);
  glTexCoord2f(maxS, 0);    glVertex2f(aspect, 0);
  glTexCoord2f(maxS, maxT); glVertex2f(aspect, 1);
  glTexCoord2f(0, maxT);    glVertex2f(0, 1);
  glEnd();
  glDisable(GL_TEXTURE_2D);

  glPopAttrib();
}

void
tokenize(string input, vector<string>& tokens, string sep)
{
  string cur = input;
  int done=0;
  tokens.clear();
  while( ! done ){
    int start = cur.find_first_not_of(sep);
    int end = cur.find_first_of(sep,start+1);
    if( start == -1 || end == -1 ){
      if( start != -1 )
	tokens.push_back( string( cur, start ) );
      return;
    }
    tokens.push_back( string( cur, start, end-start ) );
    cur = string(cur, end+1);
  }
}

int main(int argc, char** argv)
{
  vector<string> tokens;
  bool firsttime=true;
  int c;

  char* ostr="r:";
  while( (c = getopt(argc,argv,ostr)) != -1)
  {
    switch( c ){

      case 'r':
        tokenize(optarg, tokens, string(","));
        frustumRows=atoi(tokens[0].c_str());
        frustumCols=atoi(tokens[1].c_str());
        itsRow=atoi(tokens[2].c_str());
        itsCol=atoi(tokens[3].c_str());
        VAR(frustumRows);
        VAR(frustumCols);
        VAR(itsRow);
        VAR(itsCol);
        break;
    }
  }

  for(int i=optind; i<argc ; i++ ){
    PPM::Format format;
    int cpp,bpc,off,texSize;
    unsigned char* pixels = 
      PPM::load(argv[i],&format,&cpp,&bpc,&srcSizeX,&srcSizeY,
		signedPixels,forceNormalize,swapPixels);
    if( pixels == NULL ){
      cerr << "error loading file:" << argv[i] << endl;
    } else {
      if( firsttime ){
	firsttime=false;
	bytesPerPixel = cpp * (bpc/8);
	componentsPerPixel = cpp;
	bitsPerComponent = bpc;
	float expWidth = log((float)srcSizeX)/log((float)2.0);
	float expHeight = log((float)srcSizeY)/log((float)2.0);
	int expiWidth = (int)floor(expWidth);
	int expiHeight = (int)floor(expHeight);
	texWidth = (int)pow(2.0,expiWidth);
	texHeight = (int)pow(2.0,expiHeight);
	if( texWidth < srcSizeX ) texWidth *= 2;
	if( texHeight < srcSizeY ) texHeight *= 2;
	maxS = (float)srcSizeX/texWidth;
	maxT = (float)srcSizeY/texHeight;
	texSize=texWidth*texHeight*bytesPerPixel;

	switch(bitsPerComponent){
	  case 8:
	    sourceType = GL_UNSIGNED_BYTE;
	    break;
	  case 16:
	    sourceType = GL_UNSIGNED_SHORT;
	    break;
	    default:assert(0);	    
	}
	switch(componentsPerPixel){
	  case 1:
	    sourceFormat = GL_LUMINANCE;
	    break;
	  case 3:
	    sourceFormat = GL_RGB;
	    break;
	    default:assert(0);
	}
	VAR(sourceFormat);
	VAR(sourceType);
	VAR(bytesPerPixel);
	VAR(componentsPerPixel);
	VAR(bitsPerComponent);
	VAR(texSize);
	VAR2(texWidth,texHeight);
	VAR2(maxS,maxT);
      }

      unsigned char* tex = new unsigned char[texSize];
      bzero(tex,texSize);  
      int size = bytesPerPixel;   
      for(int row = 0 ; row < srcSizeY ; row++ ){
	int srcIndex = row*srcSizeX;
	int dstIndex = row*texWidth;
	memcpy(&tex[dstIndex*size],&pixels[srcIndex*size],srcSizeX*size);
      }
      inputPixels.push_back(tex);
      inputFilenames.push_back(argv[i]);
      delete [] pixels;
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

  env = new OpenEnv2D("pix");

  if( frustumRows != -1 ){
    env->setSector(itsCol,itsRow,frustumCols,frustumRows);
  }
  env->setRenderFunc(draw);
  env->setWinSize(dstSizeX,dstSizeY);
  env->mainLoop();
  
}
