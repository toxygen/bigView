
#include <GLX.h>
#include <Draggers/Slider.h>
#include <Draggers/Multislider.h>
#include <glxTrackpad.h>
#include <vector>

using namespace std;

vector<Glx::Slider*> sliders;

void
sliderListener(void* user, float t)
{
  cout << "["<<t<<"]"<<endl;
  Glx::Slider* src = static_cast<Glx::Slider*>(user);
  vector<Glx::Slider*>::iterator i=sliders.begin();
  for( ; i != sliders.end() ; i++){
    Glx::Slider* s = *i;
    if( s != src )
      s->setCur(t);
  }
}

void
rangeSliderListener(void* user, std::vector<float>& t)
{
  for(int i=0;i<t.size();++i)
    cout << "["<<i<<"] : " << t[i]<<endl;
}

int
main(int argc, char** argv)
{
  glx* env = new glx();

  Glx::Slider* xSlider = new Glx::Slider(env,Glx::Slider::X,60,30);
  xSlider->setRange(0,50,100);
  xSlider->setCallback(sliderListener,xSlider);

  Glx::Slider* ySlider = new Glx::Slider(env,Glx::Slider::Y,30,60);
  ySlider->setRange(0,50,100);
  ySlider->setCallback(sliderListener,ySlider);

  Glx::Multislider* rangeSlider = 
    new Glx::Multislider(env,5,Glx::Multislider::X,300,30);

  rangeSlider->setRange(0.,100.);
  rangeSlider->setCallback(rangeSliderListener,rangeSlider);
  rangeSlider->setSnapToInt(true);

  sliders.push_back(xSlider);
  sliders.push_back(ySlider);

  Glx::Trackpad* tp = new Glx::Trackpad(env);
  env->mainLoop();
}
