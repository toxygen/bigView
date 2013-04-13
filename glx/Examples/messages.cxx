#include <iostream>
#include <GLX.h>
#include <glxTrackball.h>
#include <str.h>
using namespace std;

std::vector<std::string> lines;

void draw(glx* env, void*)
{
  env->showTitle(lines);
}

void initGL(glx* env, void* user)
{
  std::string face("Times New Roman");
  std::string mesg("Dude!:You should've:got a:Dell!");

  str::tokenize(mesg,lines,":");
  env->setMessageColor(1,1,0);
  env->setMessageFont(face,36.);
  env->addDrawFunc(draw);
  Glx::Trackball* tb = new Glx::Trackball(env);
}

int main(int argc, char** argv)
{
  glx* env = new glx(initGL);
  env->mainLoop();
  return 1;
}
