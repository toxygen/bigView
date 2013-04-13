#include <iostream>
#include <GLX.h>
#include <glxTrackball.h>
#include <glxColormap.h>
#include <pal_editor.h>

using namespace std;

bool show=true;
string message = "hit 'm' to toggle message";

struct Tools {
  Glx::Trackball* tb;
  Glx::Colormap* cmap;
};

void draw(glx* env, void*)
{
  if(show) env->showMessage(message);
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

    case XK_m:
      show=!show;
      changed=true;
      break;
    case XK_q:
      exit(0);
      break;
  }
  if( changed )
    env->wakeup();
}

void initGL(glx* env, void* user)
{
  Tools* t = static_cast<Tools*>(user);

  env->addDrawFunc(draw);
  env->addEventFunc(processKey);
  t->tb = new Glx::Trackball(env);
  t->cmap = new Glx::Colormap;
}

void update(double pal[256][4],void*)
{
}

int main(int argc, char** argv)
{
  Tools tools;

  glx* env = new glx(initGL,&tools);
  Pal::paltool(env->getApp(), env->getDisplay());
  Pal::addUserFunc(update);

  env->mainLoop();
  return 1;
}
