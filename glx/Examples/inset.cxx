#include <iostream>
#include <GLX.h>
#include <glxTrackball.h>
#include <Objects/Sphere.h>

using namespace std;

bool useScissors=false;
int inset=100;

void draw(glx* env, void* user)
{
  Glx::Sphere* s=static_cast<Glx::Sphere*>(user);

  if( useScissors ){
    s->setWireframe(true);
    s->draw(env,NULL);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    s->setWireframe(false);
    s->draw(env,NULL);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_LIGHTING);
  } else {
    glViewport(0, 0, env->winWidth(), env->winHeight());
    s->setWireframe(true);
    s->draw(env,NULL);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);    
    s->setWireframe(false);
    glViewport(inset, inset, 
	       env->winWidth()-2*inset, env->winHeight()-2*inset);
    s->draw(env,NULL);
    glDisable(GL_LIGHTING);   
  }  
}

void initGL(glx* env, void* user)
{
  Glx::Sphere* s=new Glx::Sphere;
  glScissor(inset,inset,
	    env->winWidth()-2*inset,
	    env->winHeight()-2*inset);
  env->addDrawFunc(draw,s);
  Glx::Trackball* tb = new Glx::Trackball(env);
}

int main(int argc, char** argv)
{
  glx* env = new glx(initGL);
  env->mainLoop();
  return 1;
}
