#include <iostream>
#include <math.h>
#include <GLX.h>
#include <glxTrackpad.h>
#include <Draggers/TFeditor.h>

void
draw(glx*,void*)
{
}

void initGL(glx* env,void* user)
{
  env->addDrawFunc(draw);
  Glx::Trackpad* pd = new Glx::Trackpad(env);
  Glx::TFeditor* editor = new Glx::TFeditor(env,10,10,200,100);
}

int
main(int argc, char** argv)
{
  glx* g = new glx(initGL);
  g->mainLoop();
}

