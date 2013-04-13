// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Class Glx::Draggable
///////////////////////////////////////////////////////////////////////////////

#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Intrinsic.h>
#include <X11/X.h>
#include <X11/Xlib.h> 
#include <values.h>
#include <GLX.h>
#include <Draggers/Dragger.h>
#include "debug.h"

using namespace std;

Glx::Draggable::Draggable(glx* env, bool registerDragger) : itsEnv(env)
{
  setDefaults();
  if( registerDragger )
    itsEnv->registerDragger(this);
}

Glx::Draggable::~Draggable(void)
{
  itsEnv->unregisterDragger(this);
}

void 
Glx::Draggable::addDragFunc(Glx::DraggerCallback func, void* user)
{
  itsDragFuncs.push_back(Glx::DragPair(func,user) );
}

void 
Glx::Draggable::addUpFunc(Glx::DraggerCallback func, void* user)
{
  itsUpFuncs.push_back(Glx::DragPair(func,user) );
}

void 
Glx::Draggable::addUpdateFunc(Glx::DraggerCallback func, void* user)
{
  itsUpdateFuncs.push_back(Glx::DragPair(func,user) );
}

void Glx::Draggable::setCurDragHandle(int handle)
{
  if( itsCurDragHandle==handle ) return;
  itsCurDragHandle=handle;
  callFuncs(itsUpdateFuncs); // appearance probably changed
}

void Glx::Draggable::setScale(float s)
{
  if( itsScale==s ) return;
  itsScale = s;
  callFuncs(itsUpdateFuncs); // appearance probably changed
}

void  
Glx::Draggable::setVisibility(bool enabled)
{
  if( itsVisibleFlag==enabled ) return;
  itsVisibleFlag=enabled;
  callFuncs(itsUpdateFuncs); // appearance probably changed
}

void 
Glx::Draggable::setPosition(Glx::Vector& p, bool call)
{
  itsPos=p;
  if( call ){
    callFuncs(itsUpdateFuncs);
    callFuncs(itsDragFuncs);
  }
}

void 
Glx::Draggable::setPosition(float p[3], bool call)
{
  itsPos.set(p);
  if( call ){
    callFuncs(itsUpdateFuncs);
    callFuncs(itsDragFuncs);
  }
}

void 
Glx::Draggable::setPosition(float p0, float p1, float p2, bool call)
{
  itsPos.set(p0,p1,p2);
  if( call ){
    callFuncs(itsUpdateFuncs);
    callFuncs(itsDragFuncs);
  }
}

int 
Glx::Draggable::is3D(void)
{
  return 0;
}

void 
Glx::Draggable::callDrag(void)
{
  callFuncs(itsDragFuncs);
}

void 
Glx::Draggable::doneWithDrag(void)
{
  callFuncs(itsUpFuncs);
  setCurDragHandle(Glx::Draggable::UNSELECTED);    
}

void 
Glx::Draggable::viewHasChanged(glx* env)
{
}

void 
Glx::Draggable::select (int dragHandle)
{
  if( itsCurDragHandle != dragHandle ){   
    itsCurDragHandle = dragHandle;
    callFuncs(itsUpdateFuncs); // appearance probably changed
    //itsEnv->wakeup(); BLUTE: redundant?
  }
}

void 
Glx::Draggable::unselect(void)
{
  if( itsCurDragHandle != UNSELECTED ){ 
    itsCurDragHandle = UNSELECTED;	
    callFuncs(itsUpdateFuncs); // appearance probably changed
    //itsEnv->wakeup();
  }
}

void
Glx::Draggable::callFuncs(vector< Glx::DragPair >& funcs)
{
  vector<Glx::DragPair>::iterator iter = funcs.begin();
  for( ; iter != funcs.end() ; ++iter ){
    Glx::DragPair p = *iter;
    p.first(this,p.second);
  }
}

bool Glx::Draggable::interested(int btn)
{
  if( btn<0 || btn>=glx::NUM_BUTTON ) return false;
  return itsMouseFlags[btn];
}

void 
Glx::Draggable::setDefaults(void)
{
  itsNeedsInitFlag=true;
  itsVisibleFlag = 1;
  itsCurDragHandle = -1;
  itsSelectionDist = MAXFLOAT;
  itsScale = 1.0;
  for(int i=0;i<=glx::NUM_BUTTON;++i)
    itsMouseFlags.push_back(1);
}
