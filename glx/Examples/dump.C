#include <iostream>
#include <fstream>
#include <assert.h>
#include <GLX.h>
#include <glxTrackball.h>
#include <Objects/Sphere.h>
using namespace std;

#include "debug.h"

enum {S=1,C=2,ALL=3};
int mode=(S | C);
Glx::Sphere* s=0;
unsigned char* pix=0;
float* depth=0;
unsigned int PW=0,PH=0;

#define TST(var,flag) (var & (flag))
#define SET(var,flag) {var |= (flag);}
#define CLR(var,flag) {var &= ((ALL)^(flag));}
#define TGL(var,flag) {if(TST(var,flag)){CLR(var,flag)}else{SET(var,flag)}}

void setcube(float v[8][3],float gmin[3], float gmax[3])
{
  /* Setup cube vertex data. */
  v[0][0] = v[1][0] = v[2][0] = v[3][0] = gmin[0];
  v[4][0] = v[5][0] = v[6][0] = v[7][0] = gmax[0];
  v[0][1] = v[1][1] = v[4][1] = v[5][1] = gmin[1];
  v[2][1] = v[3][1] = v[6][1] = v[7][1] = gmax[1];
  v[0][2] = v[3][2] = v[4][2] = v[7][2] = gmin[2];
  v[1][2] = v[2][2] = v[5][2] = v[6][2] = gmax[2];
}

void drawCube(float gmin[3], float gmax[3])
{
  float n[6][3]={{-1,0,0},{0,1,0},{1,0,0},{0,-1,0},{0,0,1},{0,0,-1}};
  int f[6][4]={{0,1,2,3},{3,2,6,7},{7,6,5,4},{4,5,1,0},{5,6,2,1},{7,4,0,3}};
  float v[8][3];

  setcube(v,gmin,gmax);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glBegin(GL_QUADS);
  for(int i = 0; i < 6; i++) {
    glNormal3fv(&n[i][0]);
    glVertex3fv(&v[f[i][0]][0]);
    glVertex3fv(&v[f[i][1]][0]);
    glVertex3fv(&v[f[i][2]][0]);
    glVertex3fv(&v[f[i][3]][0]);
  }
  glEnd();
}

void draw(glx* env, void*)
{
  float gmin[3]={0,0,0}, gmax[3]={1,1,1};
  if( mode & C ) drawCube(gmin,gmax);
  if( mode & S ) s->draw(env,NULL);  
}

void dump(glx* env)
{
  int w = env->winWidth();
  int h = env->winHeight();
  int d = 4;
  int size = w * h * d; // RGBA
  int dims[3]={w,h,d};

  if( PW!=w || PH!=h || pix==0 || depth==0 ){
    delete [] pix;
    pix = new unsigned char[w*h*d];
    assert(pix);
    depth = new float[w*h];
    PW=w;PH=h;
  }
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_ROW_LENGTH,w); 
  glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,pix);
  glReadPixels(0,0,w,h,GL_DEPTH_COMPONENT,GL_FLOAT,depth);

  std::string fname = env->nextFilename("dump","rgba");
  cout << "=== " << fname << " ===" << endl;
  cout << w << " x " << h << " x " << d << " = " <<size<<endl;

  ofstream fout(fname.c_str());
  assert( fout );
  fout.write((char*)&dims[0],3*sizeof(int));
  fout.write((char*)pix,w*h*d*sizeof(unsigned char));
  fout.close();

  std::string zname = env->nextFilename("dump","z");
  cout << "=== " << zname << " ===" << endl;

  fout.open(zname.c_str());
  assert( fout );
  fout.write((char*)&dims[0],2*sizeof(int));
  fout.write((char*)depth,w*h*sizeof(float));
  fout.close();
}

void processKey(glx* env,XEvent *event,void*)
{
  KeySym ks = XLookupKeysym((XKeyEvent*)event,0);
  XKeyEvent *kep = (XKeyEvent *)(event);
  int ctlDown    = kep->state & ControlMask;
  int shiftDown  = kep->state & ShiftMask;
  int altDown    = kep->state & Mod1Mask;
  bool changed=false;

  switch( ks ){

    case XK_d:
      dump(env);
      break;

    case XK_x:
      TGL(mode,S);
      changed=true;
      break;
    case XK_c:
      TGL(mode,C);
      changed=true;
      break;
  }
  if( changed )
    env->wakeup();
}

void initGL(glx* env, void* user)
{
  env->addDrawFunc(draw);
  env->addEventFunc(processKey);
  Glx::Trackball* tb = new Glx::Trackball(env);
  s = new Glx::Sphere;
}

int main(int argc, char** argv)
{
  glx* env = new glx(initGL);
  env->mainLoop();
  return 1;
}
