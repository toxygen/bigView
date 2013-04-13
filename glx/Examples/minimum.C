#include <iostream>
#include <GLX.h>
#include <glxTrackball.h>
using namespace std;

void draw(glx* env, void*)
{
}

void initGL(glx* env, void* user)
{
  env->addDrawFunc(draw);
  Glx::Trackball* tb = new Glx::Trackball(env);
}

int main(int argc, char** argv)
{
  glx* env = new glx(initGL);
  env->mainLoop();
  return 1;
}
