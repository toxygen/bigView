// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Class Glx::Multislider
///////////////////////////////////////////////////////////////////////////////

#include <Draggers/Multislider.h>
#include <GLX.h>
#include <math.h>
#include <values.h>
#include <sstream>

//#define DEBUG 1
#include "debug.h"

using namespace std;
using namespace Glx;

#define CLAMP(_val,_min,_max){ \
  if(_val<_min)_val=_min; \
  if(_val>_max)_val=_max; \
}
#define LIMIT(_val,_max){if(_val>_max)_val=_max;}

void Multislider::defaultUserFunc(void*, std::vector<float>&){}

Multislider::Multislider(glx* env, int handles,
			 Multislider::Axis axis,
			 int x, int y, int w, string label) : 
  Draggable2D(env),
  itsAxis(axis),
  itsNumHandles(handles),
  itsX(x),itsY(y),itsW(w),
  itsMinRange(0.0),
  itsMaxRange(1.0),
  itsUserFunc(Multislider::defaultUserFunc),
  itsUserData(0),
  itsState(Multislider::NORMAL),
  itsShowValueFlag(1),
  itsSnapToIntFlag(false),
  itsLabel(label),
  userFilter(0)
{
  itsValues.resize(itsNumHandles);
  thumbs.resize(itsNumHandles);

  for(int i=0;i<itsNumHandles;++i){
    itsValues[i] = (float)i/(itsNumHandles-1.);
    thumbs[i] = new Glx::Multislider::Thumb;
    thumbs[i]->itsParent=this;
    thumbs[i]->itsID=i;
    thumbs[i]->itsValue=&itsValues[i];
  }

  updateFuncs();
}

Multislider::~Multislider(void)
{
  for(int i=0;i<itsNumHandles;++i)
    delete thumbs[i];
}

void Multislider::setUserFilter( Multislider::Filter f, void* d )
{
  userFilter=f;
  userFilterData=d;
}

void Multislider::setRange(float lo, float hi,bool callUser)
{
  FANCYMESG("Multislider::setRange");
  itsMinRange=lo;
  itsMaxRange=hi;
  for(int i=0;i<itsNumHandles;++i){
    itsValues[i] = lo + (float)i/(itsNumHandles-1.) * (hi-lo);
    VAR2(i, itsValues[i]);
    thumbs[i] = new Glx::Multislider::Thumb;
    thumbs[i]->itsParent=this;
    thumbs[i]->itsID=i;
    thumbs[i]->itsValue=&itsValues[i];
  }

  updateFuncs();
  if( callUser )
    itsUserFunc(itsUserData,itsValues);
  updateFuncs(); // in case user rejects the values/ranges
}

void 
Multislider::set(std::vector<float>& vals, bool callUser)
{
  FANCYMESG("Multislider::set(vector<float>&,bool)");
  if( vals.size() != itsValues.size() ){
    cerr << "Multislider::set(): wrong vector size()" << endl;
    cerr << "Multislider::set(): expected : " << itsValues.size()<<endl;
    cerr << "Multislider::set():      got : " << vals.size()<<endl;
    return;
  }
  itsValues = vals;
  updateFuncs();
  if( callUser )
    itsUserFunc(itsUserData,itsValues);
  updateFuncs(); // in case user rejects the values/ranges
}

void Multislider::get(std::vector<float>& vals)
{
  FANCYMESG("Multislider::get");
  vals=itsValues;
}

void Multislider::set(int index, float value, bool callUser)
{
  FANCYMESG("Multislider::set(int,float,bool)");
  if( index<0 || index>= itsValues.size() ){
    cerr << "Multislider::set(): index out of bounds" << endl;
    cerr << "Multislider::set(): expected : [0.."<<itsValues.size()<<"]"<<endl;
    cerr << "Multislider::set():      got : " << index<<endl;
    return;
  }
  itsValues[index]=value;
  if( callUser )
    itsUserFunc(itsUserData,itsValues);
  updateFuncs();
}

float Multislider::get(int index)
{
  FANCYMESG("Multislider::get(int)");
  if( index<0 || index>= itsValues.size() ){
    cerr << "Multislider::set(): index out of bounds" << endl;
    cerr << "Multislider::set(): expected : [0.."<<itsValues.size()<<"]"<<endl;
    cerr << "Multislider::set():      got : " << index<<endl;
    return 0;
  }
  return itsValues[index];
}

void Multislider::draw(glx* env,void* user)
{
  if( itsVisibleFlag == 0)     
    return;

  int envW = env->winWidth();
  int envH = env->winHeight();

  glPushAttrib(GL_LIGHTING);
  glDisable(GL_LIGHTING);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0,envW,0,envH,-1,1);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  if( itsCurDragHandle==itsNumHandles ){
    // i.e. the box
    glColor3f(1.0f,0.0f,0.0f);
    env->showMessage("  Left mouse = move slider");
    env->showMessage("Middle mouse = resize slider");
  } else if(itsCurDragHandle != UNSELECTED ){
    glColor3f(1.0f,0.0f,0.0f);
    env->showMessage("  Left mouse = adjust value");
  }

  setColor(itsNumHandles); // i.e. the box

  if( itsState == Multislider::MOVING ){    
    glBegin(GL_LINES);

    // h line
    glVertex2i(itsX+itsOffX-20,itsY+itsOffY);
    glVertex2i(itsX+itsOffX+20,itsY+itsOffY); 

    // h left arrowhead
    glVertex2i(itsX+itsOffX-16,itsY+itsOffY+4);
    glVertex2i(itsX+itsOffX-20,itsY+itsOffY); 
    glVertex2i(itsX+itsOffX-20,itsY+itsOffY); 
    glVertex2i(itsX+itsOffX-16,itsY+itsOffY-4);

    // h right arrowhead
    glVertex2i(itsX+itsOffX+16,itsY+itsOffY+4);
    glVertex2i(itsX+itsOffX+20,itsY+itsOffY); 
    glVertex2i(itsX+itsOffX+20,itsY+itsOffY); 
    glVertex2i(itsX+itsOffX+16,itsY+itsOffY-4);

    // v line
    glVertex2i(itsX+itsOffX,itsY+itsOffY-20);
    glVertex2i(itsX+itsOffX,itsY+itsOffY+20);

    // v top arrowhead
    glVertex2i(itsX+itsOffX-4,itsY+itsOffY+16);
    glVertex2i(itsX+itsOffX,  itsY+itsOffY+20);
    glVertex2i(itsX+itsOffX,  itsY+itsOffY+20);
    glVertex2i(itsX+itsOffX+4,itsY+itsOffY+16);

    // v bottom arrowhead
    glVertex2i(itsX+itsOffX-4,itsY+itsOffY-16);
    glVertex2i(itsX+itsOffX,  itsY+itsOffY-20);
    glVertex2i(itsX+itsOffX,  itsY+itsOffY-20);
    glVertex2i(itsX+itsOffX+4,itsY+itsOffY-16);

    glEnd();
  } 
  else if( itsState == Multislider::RESIZING )
  { 
    glBegin(GL_LINES);
    switch( itsAxis ){
      case Multislider::X:   	
	// h line
	glVertex2i(itsX+itsW-20,itsY+Multislider::THICKNESS/2);
	glVertex2i(itsX+itsW+20,itsY+Multislider::THICKNESS/2); 
	
	// h left arrowhead
	glVertex2i(itsX+itsW-16,itsY+Multislider::THICKNESS/2+4);
	glVertex2i(itsX+itsW-20,itsY+Multislider::THICKNESS/2);
	glVertex2i(itsX+itsW-20,itsY+Multislider::THICKNESS/2); 
	glVertex2i(itsX+itsW-16,itsY+Multislider::THICKNESS/2-4);
	
	// h right arrowhead
	glVertex2i(itsX+itsW+16,itsY+Multislider::THICKNESS/2+4);
	glVertex2i(itsX+itsW+20,itsY+Multislider::THICKNESS/2); 
	glVertex2i(itsX+itsW+20,itsY+Multislider::THICKNESS/2); 
	glVertex2i(itsX+itsW+16,itsY+Multislider::THICKNESS/2-4);
	break;
      case Multislider::Y:
	// v line
	glVertex2i(itsX+Multislider::THICKNESS/2,itsY+itsW-20);
	glVertex2i(itsX+Multislider::THICKNESS/2,itsY+itsW+20);
	
	// v top arrowhead
	glVertex2i(itsX+Multislider::THICKNESS/2-4,itsY+itsW+16);
	glVertex2i(itsX+Multislider::THICKNESS/2,  itsY+itsW+20);
	glVertex2i(itsX+Multislider::THICKNESS/2,  itsY+itsW+20);
	glVertex2i(itsX+Multislider::THICKNESS/2+4,itsY+itsW+16);
	
	// v bottom arrowhead
	glVertex2i(itsX+Multislider::THICKNESS/2-4,itsY+itsW-16);
	glVertex2i(itsX+Multislider::THICKNESS/2,  itsY+itsW-20);
	glVertex2i(itsX+Multislider::THICKNESS/2,  itsY+itsW-20);
	glVertex2i(itsX+Multislider::THICKNESS/2+4,itsY+itsW-16);
	break;
    }
    glEnd();
  }

  // slider box

  switch( itsAxis ){
    case Multislider::X:
      if( itsLabel.length() )
	env->showMessage(itsX+itsW+5,itsY,itsLabel.c_str());
      setColor(itsNumHandles);
      glBegin(GL_LINE_LOOP);
      glVertex2i(itsX,     itsY);
      glVertex2i(itsX,     itsY+Multislider::THICKNESS);
      glVertex2i(itsX+itsW,itsY+Multislider::THICKNESS);
      glVertex2i(itsX+itsW,itsY);
      glEnd();
      break;
    case Multislider::Y:
      if( itsLabel.length() )
	env->showMessage(itsX,itsY+itsW+10,itsLabel.c_str());
      setColor(itsNumHandles);
      glBegin(GL_LINE_LOOP);
      glVertex2i(itsX,                     itsY);
      glVertex2i(itsX+Multislider::THICKNESS,itsY);
      glVertex2i(itsX+Multislider::THICKNESS,itsY+itsW);
      glVertex2i(itsX,                     itsY+itsW);
      glEnd();
      break;
  }

  for(int i=0;i<itsNumHandles;++i){
    setColor(i);
    thumbs[i]->draw(env,user);
  }

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
}

void Multislider::handleMouseUp(glx*,void*)
{
  unselect();
  itsState = Multislider::NORMAL;
}

void 
Multislider::handleDrag(glx* env, int x, int y, void*)
{
  int envH = env->winHeight();
  float t,value;
  y=envH-y;

  switch( env->buttonpressed ){
    case glx::LEFT: 
      if( itsCurDragHandle==itsNumHandles )
      {
	if( itsState == Multislider::NORMAL ){
	  itsOffX = x-itsX;
	  itsOffY = y-itsY;
	}
	itsState = Multislider::MOVING;
	itsX = x - itsOffX;
	itsY = y - itsOffY;
      }
      else
      {
	itsState = Multislider::NORMAL;
	switch( itsAxis ){
	  case Multislider::X:
	    t = (float)(x - itsX)/(float)itsW;
	    break;
	  case Multislider::Y:
	    t = (float)(y - itsY)/(float)itsW;
	    break;
	}
	CLAMP(t,0,1);
	value = itsMinRange + t*(itsMaxRange-itsMinRange);

	int loh = itsCurDragHandle-1;
	int hih = itsCurDragHandle+1;

	if( loh>=0 ) 
	  if( value<itsValues[loh]) value = itsValues[loh];
	if( hih<itsNumHandles ) 
	  if( value>itsValues[hih]) value = itsValues[hih];

	if( itsSnapToIntFlag ) 
	  value=(int)(0.5f+value);
	if(value == itsValues[itsCurDragHandle] )
	  return;
	itsValues[itsCurDragHandle] = value;

	if( itsUserFunc != 0)
	  itsUserFunc(itsUserData,itsValues);
      } 
      break;
    case glx::MIDDLE:
      itsState = Multislider::RESIZING;
      switch( itsAxis ){
	case Multislider::X:
	  itsW = x-itsX;
	  break;
	case Multislider::Y:
	  itsW = y-itsY;
	  break;
      }
      break;
  }
  updateFuncs();
}

int Glx::Multislider::idlePick(glx *env, int x, int y, void* user)
{
  int envH = env->winHeight();
  int thumbPos,candidate=-1;
  float delta,t;
  itsSelectionDist = MAXFLOAT;

  itsLastX=x;
  itsLastY=envH-y;
  
  for(int i=0;i<itsNumHandles;++i){
    int c = thumbs[i]->idlePick(env,itsLastX,itsLastY,user);
    if( c != UNSELECTED ){
      itsSelectionDist=0;
      return i;
    }
  }

  // unslected if completely outside the box and thumbs area
  switch( itsAxis ){
    case Multislider::X:
      if( itsLastX < itsX || 
	  itsLastX > itsX+itsW || 
	  itsLastY < itsY || 
	  itsLastY > itsY+Multislider::THICKNESS )
      {
	unselect();
	return Draggable::UNSELECTED;
      } 
      
      for(int i=0;i<itsNumHandles;++i){
	t = (itsValues[i]-itsMinRange)/(itsMaxRange-itsMinRange);
	thumbPos = (int)(t * itsW);
	CLAMP(thumbPos,0,itsW);
	delta = itsLastX-(itsX+thumbPos);
	if( fabs(delta)<itsSelectionDist ){
	  itsSelectionDist=fabs(delta);
	  candidate=i;
	}
      }
      if( itsSelectionDist<Multislider::THUMB_WIDTH && candidate != -1 ){
	itsSelectionDist=0;
	return candidate;
      }
      break;
    case Multislider::Y:
      if( itsLastX < itsX || 
	  itsLastX > itsX+Multislider::THICKNESS || 
	  itsLastY < itsY || 
	  itsLastY > itsY+itsW )
      {
	unselect();
	return Draggable::UNSELECTED;
      } 
      for(int i=0;i<itsNumHandles;++i){
	t = (itsValues[i]-itsMinRange)/(itsMaxRange-itsMinRange);
	thumbPos = (int)(t * itsW);
	CLAMP(thumbPos,0,itsW);
	delta = itsLastY-(itsY+thumbPos);
	if( fabs(delta)<itsSelectionDist ){
	  itsSelectionDist=fabs(delta);
	  candidate=i;
	}
      }
      if( itsSelectionDist<Multislider::THUMB_WIDTH && candidate != -1 ){
	itsSelectionDist=0;
	return candidate;
      }
      break;
  }
  
  itsSelectionDist=0;
  return itsNumHandles; // i.e. the box
}

void 
Multislider::updateFuncs(void)
{
  for(int i=0;i<itsNumHandles;++i)
    thumbs[i]->updateFuncs();
}

void 
Multislider::setColor(int handle)
{
  if( itsCurDragHandle == handle )
    glColor3f(1.0f,0.0f,0.0f);
  else
    glColor3f(0.0f,1.0f,0.0f);
}

void Multislider::viewHasChanged(glx*)
{
  for(int i=0;i<itsNumHandles;++i)
    thumbs[i]->updateFuncs();  
}

//////////////////////////////////////////////////////////////////////////
//////////////////////// class Multislider::Thumb ////////////////////////
//////////////////////////////////////////////////////////////////////////

int Multislider::Thumb::idlePick(glx *env, int x, int y, void*)
{
  FANCYMESG("Multislider::Thumb::idlePick");
  VAR2(x,y);

  float range = itsParent->itsMaxRange - itsParent->itsMinRange;
  float t = (*itsValue-itsParent->itsMinRange)/range;
  VAR2(*itsValue,t);
  int off = (int)(t * itsParent->itsW);
  CLAMP(off,0,itsParent->itsW);
  VAR(off);

  bool in=true;
  float t0,t1,t2;
  switch( itsParent->itsAxis ){
    case Multislider::X:
      t0 = funcs[0].eval(itsParent->itsX+off-10,itsParent->itsY-10);
      t1 = funcs[1].eval(itsParent->itsX+off,   itsParent->itsY);
      t2 = funcs[2].eval(itsParent->itsX+off+10,itsParent->itsY-10);
      break;
    case Multislider::Y:
      t0 = funcs[0].eval(itsParent->itsX-10,itsParent->itsY+off-10);
      t1 = funcs[1].eval(itsParent->itsX,   itsParent->itsY+off);
      t2 = funcs[2].eval(itsParent->itsX-10,itsParent->itsY+off+10);
      break;
  }
  VAR3(t0,t1,t2);

  float p0 = funcs[0].eval(x,y);
  float p1 = funcs[1].eval(x,y);
  float p2 = funcs[2].eval(x,y);
  VAR3(p0,p1,p2);

  if( p0<0 && t0>0 ) in=false;
  if( p0>0 && t0<0 ) in=false;
      
  if( p1<0 && t1>0 ) in=false;
  if( p1>0 && t1<0 ) in=false;
      
  if( p2<0 && t2>0 ) in=false;
  if( p2>0 && t2<0 ) in=false;
  return in ? itsID : UNSELECTED;
}

void Multislider::Thumb::draw(glx* env,void *)
{
  FANCYMESG("Multislider::Thumb::draw");
  int displace=(itsID & 1)? Multislider::THICKNESS+5 : 
    (-Multislider::THICKNESS-5);
  float range = itsParent->itsMaxRange - itsParent->itsMinRange;
  float t = (*itsValue-itsParent->itsMinRange)/range;  
  int off = (int)(t * itsParent->itsW);
  CLAMP(off,0,itsParent->itsW);
  VAR2(t,off);

  switch( itsParent->itsAxis ){
    case Multislider::X:
      glBegin(GL_LINES);
      glVertex2i(itsParent->itsX+off,itsParent->itsY);
      glVertex2i(itsParent->itsX+off,
		 itsParent->itsY+Multislider::THICKNESS);
      glEnd();
      glBegin(GL_TRIANGLES);
      glVertex2i(itsParent->itsX+off-Multislider::THUMB_WIDTH,
		 itsParent->itsY-Multislider::THUMB_WIDTH);
      glVertex2i(itsParent->itsX+off, itsParent->itsY);
      glVertex2i(itsParent->itsX+off+Multislider::THUMB_WIDTH,
		 itsParent->itsY-Multislider::THUMB_WIDTH);
      glEnd();
      if( itsParent->itsShowValueFlag ){
	ostringstream ostr;
	if( itsParent->userFilter )
	  ostr << itsParent->userFilter(itsParent->userFilterData,*itsValue);
	else
	  ostr << *itsValue;
	env->showMessage(itsParent->itsX+off, 
			 itsParent->itsY + displace, 
			 ostr.str());
      }
      break;
    case Multislider::Y:
      glBegin(GL_LINES);
      glVertex2i(itsParent->itsX,itsParent->itsY+off);
      glVertex2i(itsParent->itsX+Multislider::THICKNESS,
		 itsParent->itsY+off);
      glEnd();
      glBegin(GL_TRIANGLES);
      glVertex2i(itsParent->itsX-Multislider::THUMB_WIDTH,
		 itsParent->itsY+off-Multislider::THUMB_WIDTH);
      glVertex2i(itsParent->itsX,   itsParent->itsY+off);
      glVertex2i(itsParent->itsX-Multislider::THUMB_WIDTH,
		 itsParent->itsY+off+Multislider::THUMB_WIDTH);
      glEnd();
      if( itsParent->itsShowValueFlag ){
	ostringstream ostr;
	if( itsParent->userFilter )
	  ostr << itsParent->userFilter(itsParent->userFilterData,*itsValue);
	else
	  ostr << *itsValue;
	env->showMessage(itsParent->itsX + Multislider::THICKNESS-5, 
			 itsParent->itsY+off, ostr.str());
      }
      break;
  }
}

void Multislider::Thumb::updateFuncs(void)
{
  float range = itsParent->itsMaxRange - itsParent->itsMinRange;
  float t = (*itsValue-itsParent->itsMinRange)/range;
  int off = (int)(t * itsParent->itsW);
  CLAMP(off,0,itsParent->itsW);
  float p0[2];
  float p1[2];
  float p2[2];

  switch( itsParent->itsAxis ){
    case Multislider::X:
      p0[0]=itsParent->itsX+off-10;
      p0[1]=itsParent->itsY-10;

      p1[0]=itsParent->itsX+off;
      p1[1]=itsParent->itsY;

      p2[0]=itsParent->itsX+off+10;
      p2[1]=itsParent->itsY-10;

      break;
    case Multislider::Y:
      p0[0]=itsParent->itsX-10;
      p0[1]=itsParent->itsY+off-10;

      p1[0]=itsParent->itsX;
      p1[1]=itsParent->itsY+off;

      p2[0]=itsParent->itsX-10;
      p2[1]=itsParent->itsY+off+10;

      break;
  }
  funcs[2].a = p1[0] - p0[0];
  funcs[2].b = p1[1] - p0[1];
  funcs[2].c = p1[0] * p0[1];
  funcs[2].d = p0[0] * p1[1];

  funcs[0].a = p2[0] - p1[0];
  funcs[0].b = p2[1] - p1[1];
  funcs[0].c = p2[0] * p1[1];
  funcs[0].d = p1[0] * p2[1];  

  funcs[1].a = p0[0] - p2[0];
  funcs[1].b = p0[1] - p2[1];
  funcs[1].c = p0[0] * p2[1];
  funcs[1].d = p2[0] * p0[1];
}
