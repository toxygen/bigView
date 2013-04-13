#include <iostream>
#include <sstream>
#include <GLX.h>
#include <glxTrackball.h>
using namespace std;

int n=0;
void draw(glx* env, void*)
{
  ostringstream ostr;
  ostr<<"Frame:["<<n<<"]";
  env->showMessage(ostr.str());
  if( n % 10==0 ){
    env->wakeup();
    env->wakeup();
    env->wakeup();
  }
  ++n;
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
