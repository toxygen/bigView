#include <iostream>
#include <math.h>
#include <GLX.h>
#include <glxTrackball.h>
#include <Draggers/Vector.h>
#include <Draggers/SphereDragger.h>
#include <Draggers/SV.h>
#include <Draggers/Card.h>
#include <Objects/Sphere.h>

void
draw(glx* env,void* user)
{
}

void initGL(glx* env,void* user)
{  
  Glx::Trackball* tb = new Glx::Trackball(env);
  /*
  Glx::VectorDragger* vd = new Glx::VectorDragger(env);
  vd->setPosition(0,1,0);

  Glx::SphereDragger* sd = new Glx::SphereDragger(env);
  sd->setScale(0.1);
  sd->setPosition(1,0,0);

  Glx::SVDragger* svd = new Glx::SVDragger(env);
  svd->setPosition(1,1,0);
  */

  Glx::CardDragger* cd = new Glx::CardDragger(env);
  env->addDrawFunc(draw);
}

int
main(int argc, char** argv)
{
  glx* g = new glx(initGL);
  g->mainLoop();
}

