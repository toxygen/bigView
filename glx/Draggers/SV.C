// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Class Glx::SVDragger
///////////////////////////////////////////////////////////////////////////////

#include <Draggers/SV.h>
#include <assert.h>

//#define DEBUG 1
#include "debug.h"

using namespace std;

Glx::SVDragger::SVDragger(glx* env,bool registerDragger) : 
  Draggable3D(env,false)
{
  itsSphere = new Glx::SphereDragger(env,registerDragger);
  itsVector = new Glx::VectorDragger(env,registerDragger);

  itsSphere->addDragFunc(Glx::SVDragger::dragSphere,this);
  itsVector->addDragFunc(Glx::SVDragger::dragVector,this);

  double itsTubeVecRadius = 0.075f;
  itsSphere->setScale(itsTubeVecRadius*2.0);
}

void Glx::SVDragger::dragSphere(Draggable*,void* user)
{
  FANCYMESG("Glx::SVDragger::dragSphere");
  Glx::SVDragger* _this = static_cast<Glx::SVDragger*>(user);
  Glx::Vector vec,pos;
  Glx::Quat q;
  _this->itsSphere->getOrientation(q);
  _this->itsVector->setOrientation(q);
  _this->itsSphere->getPosition(pos);
  _this->itsVector->setPosition(pos);
  _this->itsPos = pos;
  _this->itsVector->getVector(vec);
  VAR3(q,vec,pos);
}

void Glx::SVDragger::dragVector(Draggable*,void* user)
{
  FANCYMESG("Glx::SVDragger::dragVector");
  Glx::SVDragger* _this = static_cast<Glx::SVDragger*>(user);
  Glx::Vector vec,pos;
  _this->itsVector->getPosition(pos);
  _this->itsVector->getVector(vec);
  _this->itsSphere->setPosition(pos);
  _this->itsPos = pos;
  VAR2(vec,pos);
}

Glx::SVDragger::~SVDragger(void)
{ 
  delete itsSphere;
  delete itsVector;
}

void 
Glx::SVDragger::setPosition(Glx::Vector& p, bool call)
{
  itsVector->setPosition(p);
  itsSphere->setPosition(p);
  Glx::Draggable::setPosition(p);
  callFuncs(itsUpdateFuncs);
  if( call ) callFuncs(itsDragFuncs);
}
void 
Glx::SVDragger::setPosition(float p[3], bool call)
{
  itsVector->setPosition(p);
  itsSphere->setPosition(p);
  Glx::Draggable::setPosition(p);
  callFuncs(itsUpdateFuncs);
  if( call ) callFuncs(itsDragFuncs);
}
void 
Glx::SVDragger::setPosition(float p0, float p1, float p2, bool call)
{
  itsVector->setPosition(p0,p1,p2);
  itsSphere->setPosition(p0,p1,p2);
  Glx::Draggable::setPosition(p0,p1,p2);
  callFuncs(itsUpdateFuncs);
  if( call ) callFuncs(itsDragFuncs);
}

void 
Glx::SVDragger::setScale(float s)
{
  double itsTubeVecRadius = s * 0.075f;
  itsVector->setScale(s);
  itsSphere->setScale(itsTubeVecRadius*2.0);
  Glx::Draggable::setScale(s);
  callFuncs(itsUpdateFuncs);
}

void 
Glx::SVDragger::getOrientation(Glx::Quat& orientationQuat)
{
  itsVector->getOrientation(orientationQuat);
}

void 
Glx::SVDragger::setOrientation(const Glx::Quat& orientationQuat, bool call)
{
  itsVector->setOrientation(orientationQuat);
  itsSphere->setOrientation(orientationQuat);
  callFuncs(itsUpdateFuncs);
  if( call ) callFuncs(itsDragFuncs);
}

void 
Glx::SVDragger::setVector(Glx::Vector& v, bool call)
{
  Glx::Quat q;
  itsVector->setVector(v);
  itsVector->getOrientation(q);
  itsSphere->setOrientation(q);
  callFuncs(itsUpdateFuncs);
  if(call) callFuncs(itsDragFuncs);
}
void 
Glx::SVDragger::setVector(float v[3], bool call)
{
  Glx::Quat q;
  itsVector->setVector(v);
  itsVector->getOrientation(q);
  itsSphere->setOrientation(q);
  callFuncs(itsUpdateFuncs);
  if(call) callFuncs(itsDragFuncs);
}
void 
Glx::SVDragger::setVector(float v0, float v1, float v2, bool call)
{
  Glx::Quat q;
  itsVector->setVector(v0,v1,v2);
  itsVector->getOrientation(q);
  itsSphere->setOrientation(q);
  callFuncs(itsUpdateFuncs);
  if(call) callFuncs(itsDragFuncs);
}
void  
Glx::SVDragger::getVector(Glx::Vector& v)
{
  itsVector->getVector(v);
}

void 
Glx::SVDragger::handleMouseUp(glx* env,void*)
{
}

void 
Glx::SVDragger::viewHasChanged(glx* env)
{
  FANCYMESG("Glx::SVDragger::viewHasChanged");

  itsSphere->viewHasChanged(env);
  itsVector->viewHasChanged(env);  
}

int  
Glx::SVDragger::idlePick(glx* env,int x,int y,void* user)
{
  int candidate = UNSELECTED;
  float worldDist=MAXFLOAT;
	
  candidate = itsSphere->idlePick(env,x,y,user);
  if( candidate != UNSELECTED){
    worldDist = itsSphere->getSelectionDist();
    itsVector->unselect();
  } else {
    candidate = itsVector->idlePick(env,x,y,user);
    if( candidate != UNSELECTED) {
      itsSphere->unselect();
      candidate = NORMAL;
      worldDist = itsVector->getSelectionDist();
    }
  }
	
  if( candidate == UNSELECTED )
    unselect();
  else
    itsSelectionDist = worldDist;

  return candidate;
}
