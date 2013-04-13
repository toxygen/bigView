#include <iostream>
#include <math.h>
#include <GLX.h>
#include <glxTrackpad.h>
#include <Draggers/Plotter.h>

std::vector<float> data;

std::vector<float> dataA;
std::vector<float> dataB;
std::vector<std::vector<float>*> data2;
std::vector<std::string*> data2Names;

void
draw(glx*,void*)
{
}

void initGL(glx* env,void* user)
{
  env->addDrawFunc(draw);
  Glx::Trackpad* pd = new Glx::Trackpad(env);
  Glx::LinePlotter* plotter = 
    new Glx::LinePlotter(env,data,60,300);
  Glx::MultiPlotter* mplotter = 
    new Glx::MultiPlotter(env,data2,data2Names,60,60);  
}

int
main(int argc, char** argv)
{  
  int N=200;
  float min=-3.,max=-min,dx=(float)(max-min)/N;
  
  float u=0.; //    mu: mean
  float t=1.; // theta: variance

  // u,t = [0,1] = standard normal distribution

  for(float x=min; x<=max;x+=dx){
    
    // standard gaussian:
    //
    //         [ -(x-u)^2 ]
    //       e^[ -------- ]
    //         [   2t^2   ]
    // f(x) = --------------
    //          t*sqrt(2PI)
    //
    
    float y = exp( (-(x-u)*(x-u))/(2.*t*t) ) / (t*sqrt(2.*M_PI));    
    data.push_back(y);
  }

  for(int i=0; i<36;++i){
    double r = i*10 * (M_PI/180.);
    dataA.push_back(sin(r));
    dataB.push_back(cos(r));
  }
  data2.push_back(&dataA);
  data2.push_back(&dataB);
  data2Names.push_back(new std::string("sin"));
  data2Names.push_back(new std::string("cos"));

  glx* g = new glx(initGL);
  g->mainLoop();
}

