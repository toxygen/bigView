#include <iostream>
#include <GLX.h>
#include <glxTrackball.h>
using namespace std;

vector<string> extensions;

void draw(glx* env, void*)
{
}

void initGL(glx* env, void* user)
{
  env->addDrawFunc(draw);
  Glx::Trackball* tb = new Glx::Trackball(env);
  char* ext = (char*)glGetString(GL_EXTENSIONS);
  char* cp = ext;
  cout << ext << endl;
}

int main(int argc, char** argv)
{
  glx* env = new glx(initGL);
  env->mainLoop();
  return 1;
}
