#include <iostream>
#include <GLX.h>
#include <glxTrackball.h>
#include <X11/keysym.h>

using namespace std;

const int N=3;
int borders[N]={glx::NoBorder,glx::SlimBorder,glx::FullBorder};
int bi=0;

void draw(glx* env, void*)
{
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
    case XK_Up:
      ++bi;
      bi = bi % N;
      changed=true;
      break;
    case XK_Down:
      --bi;
      if( bi<0 ) bi=N-1;
      changed=true;
      break;
  }
  if( changed )
    env->setBorder(borders[bi]);
}

void initGL(glx* env, void* user)
{
  env->addDrawFunc(draw);
  Glx::Trackball* tb = new Glx::Trackball(env);
  env->addEventFunc(processKey);
}

int main(int argc, char** argv)
{
  int c;
  
  while( (c = getopt(argc,argv,"nsf")) != -1){
    switch( c ){
      case 'n':bi=0;break;
      case 's':bi=1;break;
      case 'f':bi=2;break;
    }
  }
  glx* env = new glx(initGL,NULL,30,30,700,500,borders[bi]);
  env->mainLoop();
  return 1;
}
