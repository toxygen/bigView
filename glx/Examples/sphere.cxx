#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <values.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>
#include <assert.h>

#include <GLX.h>
#include <Objects/Sphere.h>
#include <glxTrackball.h>
#include "debug.h"

Glx::Sphere* s;
using namespace std;
int comp=0;

void draw(glx* env, void*)
{
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  s->draw(env,NULL);
}
void keydown(glx* env,XEvent *event,void*)
{
  KeySym ks;
  XComposeStatus status;
  char buffer[20];
  int buf_len = 20;
  XKeyEvent *kep = (XKeyEvent *)(event);
  XLookupString(kep, buffer, buf_len, &ks, &status);
  switch( ks ){
    case XK_Up:
      comp++;
      break;
    case XK_Down:
      comp--;
      if(comp<0)comp=0;
      break;
  }
  s->setComplexity(comp);
  _VAR(comp);
  env->wakeup();
}

void initGL(glx* env, void* user)
{
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  s=new Glx::Sphere;
  //s->setWireframe(true);
  s->setComplexity(comp);
  Glx::Trackball* tb = new Glx::Trackball(env);
  env->addDrawFunc(draw);
  env->addEventFunc(keydown);
}

int
main(int argc, char** argv)
{
  glx* env = new glx(initGL);
  env->mainLoop();
  return 1;

}
