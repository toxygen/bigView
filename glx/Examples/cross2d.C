#include <GLX.h>
#include <Draggers/Crosshair2D.h>
#include <sys/types.h>
#include <unistd.h>


glx* env;
#define MAX_DRAGGERS 250
const float CROSS = 0.25;
float lastDx,lastDy;

void drawWorld(glx*,void*)
{
}

void handleDrag(Glx::Crosshair2D*, int x, int y, void*)
{
  lastDx = x;
  lastDy = y;
}

void genDraggers(glx* env)
{
  srand48( getpid() );
  int numDraggers = rand() % MAX_DRAGGERS;

  for(int i = 0 ; i < numDraggers ; i++ ){
    float xy[2];
    xy[0] = drand48() * 300.0f;
    xy[1] = drand48() * 300.0f;
    Glx::Crosshair2D* theDragger = new Glx::Crosshair2D(env,xy[0],xy[1]);
    theDragger->setCallback(handleDrag,0);
  }
}

int
main(int argc, char** argv)
{  
  env = new glx();
  genDraggers(env);
  env->addDrawFunc(drawWorld);
  env->mainLoop();

  delete env;
  return 1;
}
