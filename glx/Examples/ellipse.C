#include <iostream>
#include <GLX.h>
#include <glxTrackball.h>
#include <Draggers/Slider.h>
using namespace std;

float r=1.;
float a=1.;
float b=1.3;

float X[2]={1,0};
float Y[2]={0,1};
float T=0.;

/*
  [X Y][ cos t sin t] = [X' Y']
       [-sin t cos t]

  for T=Pi:

  [X Y][ 0  1] = [X' Y']
       [-1  0]

       X => [ 0, 1], Y => [-1, 0]
       
 */

void draw(glx* env, void*)
{
  const int N=720;
  const float dx = (float)1./N;

  float m[2][2]={
    {cos(T),sin(T)},
    {-sin(T),cos(T)}    
  };
  
  float x[2] = {X[0]*m[0][0] + X[1]*m[1][0],X[0]*m[0][1] + X[1]*m[1][1]};
  float y[2] = {Y[0]*m[0][0] + Y[1]*m[1][0],Y[0]*m[0][1] + Y[1]*m[1][1]};

  glBegin(GL_LINE_LOOP);
  for(int i=0;i<N;++i){
    float fi = (float)i/(N-1);    
    float theta = 2. * M_PI * fi;
    float xf = a * cos(theta);
    float yf = b * sin(theta);
    glVertex2f(xf*x[0]+yf*y[0],xf*x[1]+yf*y[1]);
  }
  glEnd();
}

void aChanged(void* user, float val){a=val;}
void bChanged(void* user, float val){b=val;}
void tChanged(void* user, float val){T=val;}

void initGL(glx* env, void* user)
{
  env->addDrawFunc(draw);
  Glx::Trackball* tb = new Glx::Trackball(env);
  Glx::Slider* as = new Glx::Slider(env,Glx::Slider::Y,30,60);
  as->setRange(0,1,2);
  as->setCallback(aChanged,0);
  Glx::Slider* bs = new Glx::Slider(env,Glx::Slider::Y,60,60);
  bs->setRange(0,1,2);
  bs->setCallback(bChanged,0);
  Glx::Slider* ts = new Glx::Slider(env,Glx::Slider::Y,90,60);
  ts->setRange(-M_PI,0,M_PI);
  ts->setCallback(tChanged,0);
}

int main(int argc, char** argv)
{
  glx* env = new glx(initGL);
  env->mainLoop();
  return 1;
}
