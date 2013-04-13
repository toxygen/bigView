#ifndef _ADJ_H_
#define _ADJ_H_

#include <vector>
#include <string>
#include <GLX.h>
#include <Draggers/Slider.h>

namespace Glx {
  class AdjParam {
    
  public:
    AdjParam(glx*, std::string, double&, double, double);
    
  protected:
    static std::vector<AdjParam*> Params;
    static void update(void*, float);

    glx* env;
    Glx::Slider* adj;
    double& tref;
  };
  
};

#endif
