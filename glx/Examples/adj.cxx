#include <iostream>
#include "adj.h"

static int SADJ=40;
std::vector<Glx::AdjParam*> Glx::AdjParam::Params;

void Glx::AdjParam::update(void* user, float val)
{
  Glx::AdjParam* _this = static_cast<Glx::AdjParam*>(user);
  _this->tref=val;
  _this->env->wakeup();
}

Glx::AdjParam::AdjParam(glx* e, std::string lbl, 
			double& t, double l, double h) : 
  env(e),tref(t)
{
  Params.push_back( this );
  int off=SADJ+Params.size() * SADJ;
  adj = new Glx::Slider(env,Glx::Slider::Y,off,30);
  adj->setLabel(lbl);
  adj->setRange(l,t,h);
  adj->setCallback(Glx::AdjParam::update,this);
}
