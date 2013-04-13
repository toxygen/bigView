
#include <GLX.h>
#include <Draggers/Slider.h>
#include <Draggers/Multislider.h>
#include <glxTrackpad.h>
#include <vector>

using namespace std;

vector<Glx::Slider*> sliders;
float xScale=1.,yScale=1.;
const double LO=0.9,HI=1.2;

void xDrag(void* user, float t)
{
  xScale *= t;
  cout << "X["<<xScale<<"]"<<endl;
}

void yDrag(void* user, float t)
{
  yScale *= t;
  cout << "Y["<<yScale<<"]"<<endl;
}

void reset(void* user, float t)
{
  cout << "reset"<<endl;
  Glx::Slider* slider = static_cast<Glx::Slider*>(user);
  slider->setRange(LO,1,HI);
}

int
main(int argc, char** argv)
{
  glx* env = new glx();

  Glx::Slider* xSlider = new Glx::Slider(env,Glx::Slider::X,60,30);
  xSlider->setRange(LO,1,HI);
  xSlider->setCallback(xDrag,xSlider);
  xSlider->setMouseupCallback(reset,xSlider);

  Glx::Slider* ySlider = new Glx::Slider(env,Glx::Slider::Y,30,60);
  ySlider->setRange(LO,1,HI);
  ySlider->setCallback(yDrag,ySlider);
  ySlider->setMouseupCallback(reset,ySlider);

  sliders.push_back(xSlider);
  sliders.push_back(ySlider);

  Glx::Trackpad* tp = new Glx::Trackpad(env);
  env->mainLoop();
}
