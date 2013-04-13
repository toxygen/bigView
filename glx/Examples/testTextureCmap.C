#include <GLX.h>
#include <values.h>
#include <GEL.h>
#include <gelColormap.h>
#include <guiColormapEditor.h>
#include <glxTrackball.h>
#include <stdlib.h>
#include <math.h>

//#define DEBUG 1
#include "debug.h"

using namespace std;

glx* env=0;
const float MIN=-M_PI,MAX=M_PI,INC=0.1;
float sMin=-1,sMax=1;
Gel::Colormap* cmap=0;
ColormapEditor* cedit=0;

void
draw(glx*, void*)
{
  float x,y;
  glDisable(GL_LIGHTING);
  for(x = MIN; x <= MAX ; x += INC ){
    cmap->preRender();
    glBegin(GL_QUAD_STRIP);
    for(y = MIN; y <= MAX ; y += INC ){
      float z = sin(x*y);
      cmap->scalar(z);
      glVertex3f(x,y,z);

      z = sin((x+INC)*y);
      cmap->scalar(z);
      glVertex3f(x+INC,y,z);
    }    
    glEnd();
    cmap->postRender();
  }
}

void cmapChanged(ColormapEditor&,void* user)
{
  Gel::Colormap* cmap = static_cast<Gel::Colormap*>(user);
  env->makeCurrent();
  cmap->update();
  env->wakeup();
}

void
copyPixels(unsigned char* dst, unsigned char* src,
	   int dstW, int dstH,
	   int srcW, int srcH,
	   int dstX, int dstY,
	   int depth)
{
  int lastOffset = dstH*dstW*depth;
  int dstBase = dstY * dstW + dstX;
  for(int row = 0 ; row < srcH ; row++ ){
    int srcRow = row;
    int srcIndex = srcRow*srcW;
    int dstIndex = dstBase + row*dstW;
    memcpy(&dst[lastOffset-dstIndex*depth],&src[srcIndex*depth],srcW*depth);
  }
}

void 
dumpImage(glx* env, int w, int h, string fName, void* user)
{
  Glx::Trackball* trackball = static_cast<Glx::Trackball*>(user);
  int depth=3;
  int offscreen=w>h ? w : h;
  double near = 0.01;
  double far  = 100.0f;
  double fov = (float)M_PI/(float)6.0;
  double half_fov = fov/2.0;
  int winW = env->winWidth();
  int winH = env->winHeight();

  double gt = near * tan(half_fov);
  double gb = -gt;
  double gr = gt * env->aspect();
  double gl = -gr;

  int numSectors=offscreen/winW;
  int fullW = winW*numSectors;
  int fullH = winH*numSectors; 
  unsigned char* full=new unsigned char[fullW*fullH*depth];
  assert(full);
  unsigned char* pixels=new unsigned char[winW*winH*depth];
  assert(pixels);
  ofstream fout(fName.c_str());
  
  cout << "offscreen dim = " << fullW << " x " << fullH << endl;
  cout << " # of sectors = " << numSectors << endl;

  for(int i=0 ; i<numSectors*numSectors ; i++){
    sleep(1);
    int col = i % numSectors;
    int row = i / numSectors;
    int offX = col*winW;
    int offY = row*winH;    
    trackball->setSector(col,row,numSectors,numSectors);
    env->draw();
    env->makeCurrent();
    glReadBuffer(GL_FRONT);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0,0,winW,winH,GL_RGB,GL_UNSIGNED_BYTE,pixels);
    copyPixels(full,pixels,
	       fullW,fullH,
	       winW,winH,
	       offX,offY,
	       depth);
  }
  fout << "P6 " << fullW << " " << fullH << " 255" << endl;
  fout.write((const char*)full, fullW*fullH*depth);
  fout.close();

  delete [] full;
  delete [] pixels;
}

void keydown(glx* env,XEvent *event,void* user)
{
  KeySym ks;
  XComposeStatus status;
  char buffer[20];
  int buf_len = 20;
  XKeyEvent *kep = (XKeyEvent *)(event);
  XLookupString(kep, buffer, buf_len, &ks, &status);
  switch( ks ){
    case 'q':
      exit(0);
      break;
    case 'd':
      dumpImage(env,1400,1400,"big.ppm",user);
      break;
    default:
      break;      
  }
}

void init(glx* env,void*)
{
  env->makeCurrent();
  Glx::Trackball* trackball = new Glx::Trackball(env);
  cmap = new Gel::Colormap(256,&sMin,&sMax);
  env->addDrawFunc(draw);
  env->addEventFunc(keydown, trackball);

  cedit = new ColormapEditor(env->getApp(), env->getDisplay());
  cedit->addColormap("User",cmap);
  cedit->addListener(cmapChanged,cmap);
  cedit->editColormap("User");
}

int
main(int argc, char** argv)
{
  env = new glx(init);
  env->mainLoop();
}
