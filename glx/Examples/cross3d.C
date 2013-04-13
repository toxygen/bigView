#include <iostream>
#include <vector>
#include <math.h>
#include <GLX.h>
#include <glxTrackball.h>
#include <Draggers/Crosshair3D.h>
using namespace std;

vector<Glx::Crosshair3D*> draggers;
#define MAX_DRAGGERS 12

void
draw(glx*,void*)
{
}

void genDraggers(glx* env)
{
  srand48( getpid() );
  int numDraggers = (int)((double)MAX_DRAGGERS*drand48());
  float MIN=-10.;

  for(int i = 0 ; i < numDraggers ; i++ ){
    
    float x=MIN + drand48() * 2. * -MIN;
    float y=MIN + drand48() * 2. * -MIN;
    float z=MIN + drand48() * 2. * -MIN;
    Glx::Crosshair3D* c3d = new Glx::Crosshair3D(env);
    c3d->setPosition(x,y,z);
    c3d->setScale((float)20./sqrt((double)MAX_DRAGGERS));
    draggers.push_back(c3d);
  }
}


void initGL(glx* env,void* user)
{
  env->addDrawFunc(draw);
  Glx::Trackball* pd = new Glx::Trackball(env);
  genDraggers(env);
  //Glx::Crosshair3D* c3d = new Glx::Crosshair3D(env);
}

int
main(int argc, char** argv)
{
  glx* g = new glx(initGL);
  g->mainLoop();
}
