// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Class Glx::Slider
///////////////////////////////////////////////////////////////////////////////

#include <Draggers/Slider.h>
#include <GLX.h>
#include <math.h>
#include <values.h>
#include <sstream>

using namespace std;

#define DEBUG 1
#include <debug.h>

#define CLAMP(_val,_min,_max){ \
  if(_val<_min)_val=_min; \
  if(_val>_max)_val=_max; \
}
#define LIMIT(_val,_max){if(_val>_max)_val=_max;}

Glx::Slider::Slider(glx* env, Glx::Slider::Axis axis,
		    int x, int y, int w, string label) : 
  Glx::Draggable2D(env),
  itsAxis(axis),
  itsX(x),itsY(y),itsW(w),
  itsMinRange(0.0),
  itsMaxRange(1.0),
  itsCurValue(0.5),
  itsUserFunc(0),
  itsUserData(0),
  itsLastCurValue(0),
  itsCurScale(0),
  itsState(Glx::Slider::NORMAL),
  itsShowValueFlag(1),
  itsLabel(label),
  itsSnapToIntFlag(false),
  itsMouseupUserFunc(0),
  itsMouseupUserData(0)
{
}

void 
Glx::Slider::draw(glx* env, void *)
{
  int envW = env->winWidth();
  int envH = env->winHeight();
  if( itsVisibleFlag == 0)     
    return;

  if( itsNeedsInitFlag ){
    updateFuncs();
    itsNeedsInitFlag=false;
  }
  glPushAttrib(GL_LIGHTING);
  glDisable(GL_LIGHTING);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0,envW,0,envH,-1,1);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  switch( itsCurDragHandle ){
    case SLIDER_HANDLE:
      glColor3f(1.0f,0.0f,0.0f);
      env->showMessage("  Left mouse = move slider");
      env->showMessage("Middle mouse = resize slider");
      break;
    case THUMB_HANDLE:
      glColor3f(1.0f,0.0f,0.0f);
      env->showMessage("  Left mouse = adjust value");
      break;
  }

  setColor(SLIDER_HANDLE);

  if( itsState == Glx::Slider::MOVING ){    
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
  else if( itsState == Glx::Slider::RESIZING )
  { 
    glBegin(GL_LINES);
    switch( itsAxis ){
      case Glx::Slider::X:   	
	// h line
	glVertex2i(itsX+itsW-20,itsY+Glx::Slider::THICKNESS/2);
	glVertex2i(itsX+itsW+20,itsY+Glx::Slider::THICKNESS/2); 
	
	// h left arrowhead
	glVertex2i(itsX+itsW-16,itsY+Glx::Slider::THICKNESS/2+4);
	glVertex2i(itsX+itsW-20,itsY+Glx::Slider::THICKNESS/2);
	glVertex2i(itsX+itsW-20,itsY+Glx::Slider::THICKNESS/2); 
	glVertex2i(itsX+itsW-16,itsY+Glx::Slider::THICKNESS/2-4);
	
	// h right arrowhead
	glVertex2i(itsX+itsW+16,itsY+Glx::Slider::THICKNESS/2+4);
	glVertex2i(itsX+itsW+20,itsY+Glx::Slider::THICKNESS/2); 
	glVertex2i(itsX+itsW+20,itsY+Glx::Slider::THICKNESS/2); 
	glVertex2i(itsX+itsW+16,itsY+Glx::Slider::THICKNESS/2-4);
	break;
      case Glx::Slider::Y:
	// v line
	glVertex2i(itsX+Glx::Slider::THICKNESS/2,itsY+itsW-20);
	glVertex2i(itsX+Glx::Slider::THICKNESS/2,itsY+itsW+20);
	
	// v top arrowhead
	glVertex2i(itsX+Glx::Slider::THICKNESS/2-4,itsY+itsW+16);
	glVertex2i(itsX+Glx::Slider::THICKNESS/2,  itsY+itsW+20);
	glVertex2i(itsX+Glx::Slider::THICKNESS/2,  itsY+itsW+20);
	glVertex2i(itsX+Glx::Slider::THICKNESS/2+4,itsY+itsW+16);
	
	// v bottom arrowhead
	glVertex2i(itsX+Glx::Slider::THICKNESS/2-4,itsY+itsW-16);
	glVertex2i(itsX+Glx::Slider::THICKNESS/2,  itsY+itsW-20);
	glVertex2i(itsX+Glx::Slider::THICKNESS/2,  itsY+itsW-20);
	glVertex2i(itsX+Glx::Slider::THICKNESS/2+4,itsY+itsW-16);
	break;
    }
    glEnd();
  }

  // slider box

  switch( itsAxis ){
    case Glx::Slider::X:
      if( itsLabel.length() )
	env->showMessage(itsX+itsW+5,itsY,itsLabel.c_str());
      setColor(SLIDER_HANDLE);
      glBegin(GL_LINE_LOOP);
      glVertex2i(itsX,     itsY);
      glVertex2i(itsX,     itsY+Glx::Slider::THICKNESS);
      glVertex2i(itsX+itsW,itsY+Glx::Slider::THICKNESS);
      glVertex2i(itsX+itsW,itsY);
      glEnd();
      break;
    case Glx::Slider::Y:
      if( itsLabel.length() )
	env->showMessage(itsX,itsY+itsW+10,itsLabel.c_str());
      setColor(SLIDER_HANDLE);
      glBegin(GL_LINE_LOOP);
      glVertex2i(itsX,                     itsY);
      glVertex2i(itsX+Glx::Slider::THICKNESS,itsY);
      glVertex2i(itsX+Glx::Slider::THICKNESS,itsY+itsW);
      glVertex2i(itsX,                     itsY+itsW);
      glEnd();
      break;
  }

  setColor(THUMB_HANDLE);

  // slider 'thumb'
  float t = (itsCurValue-itsMinRange)/(itsMaxRange-itsMinRange);
  int off = (int)(t * itsW);
  CLAMP(off,0,itsW);

  switch( itsAxis ){
    case Glx::Slider::X:
      glBegin(GL_LINES);
      glVertex2i(itsX+off,itsY);
      glVertex2i(itsX+off,itsY+Glx::Slider::THICKNESS);
      glEnd();
      glBegin(GL_TRIANGLES);
      glVertex2i(itsX+off-Glx::Slider::THUMB_WIDTH,
		 itsY-Glx::Slider::THUMB_WIDTH);
      glVertex2i(itsX+off,   itsY);
      glVertex2i(itsX+off+Glx::Slider::THUMB_WIDTH,
		 itsY-Glx::Slider::THUMB_WIDTH);
      glEnd();
      break;
    case Glx::Slider::Y:
      glBegin(GL_LINES);
      glVertex2i(itsX,itsY+off);
      glVertex2i(itsX+Glx::Slider::THICKNESS,itsY+off);
      glEnd();
      glBegin(GL_TRIANGLES);
      glVertex2i(itsX-Glx::Slider::THUMB_WIDTH,
		 itsY+off-Glx::Slider::THUMB_WIDTH);
      glVertex2i(itsX,   itsY+off);
      glVertex2i(itsX-Glx::Slider::THUMB_WIDTH,
		 itsY+off+Glx::Slider::THUMB_WIDTH);
      glEnd();
      break;
  }
  
  if( itsShowValueFlag ){
    ostringstream ostr;
    ostr << itsCurValue;
    env->showMessage(itsX, itsY-Glx::Slider::THICKNESS, ostr.str());
  }

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
}

void Glx::Slider::handleMouseUp(glx*,void*)
{
  unselect();
  itsLastCurValue = itsCurValue;
  itsState = Glx::Slider::NORMAL;
  if( itsMouseupUserFunc != 0)
    itsMouseupUserFunc(itsMouseupUserData,itsCurValue);
}

bool 
Glx::Slider::pointInTri(float x, float y)
{
  float t = (itsCurValue-itsMinRange)/(itsMaxRange-itsMinRange);
  int off = (int)(t * itsW);
  CLAMP(off,0,itsW);
  bool in=true;
  float t0,t1,t2;
  switch( itsAxis ){
    case Glx::Slider::X:
      t0 = funcs[0].eval(itsX+off-10,itsY-10);
      t1 = funcs[1].eval(itsX+off,   itsY);
      t2 = funcs[2].eval(itsX+off+10,itsY-10);
      break;
    case Glx::Slider::Y:
      t0 = funcs[0].eval(itsX-10,itsY+off-10);
      t1 = funcs[1].eval(itsX,   itsY+off);
      t2 = funcs[2].eval(itsX-10,itsY+off+10);
      break;
  }
  float p0 = funcs[0].eval(x,y);
  float p1 = funcs[1].eval(x,y);
  float p2 = funcs[2].eval(x,y);
  if( p0<0 && t0>0 ) in=false;
  if( p0>0 && t0<0 ) in=false;

  if( p1<0 && t1>0 ) in=false;
  if( p1>0 && t1<0 ) in=false;

  if( p2<0 && t2>0 ) in=false;
  if( p2>0 && t2<0 ) in=false;

  return in;
}

int Glx::Slider::idlePick(glx *env, int x, int y, void*)
{
  int envH = env->winHeight();
  float delta,t = (itsCurValue-itsMinRange)/(itsMaxRange-itsMinRange);
  int thumbPos = (int)(t * itsW);
  CLAMP(thumbPos,0,itsW);

  itsLastX=x;
  itsLastY=envH-y;

  itsSelectionDist = MAXFLOAT;

  if( pointInTri(itsLastX,itsLastY) ){
    itsSelectionDist=0;
    return Glx::Slider::THUMB_HANDLE;
  }

  switch( itsAxis ){
    case Glx::Slider::X:
      if( itsLastX < itsX || 
	  itsLastX > itsX+itsW || 
	  itsLastY < itsY || 
	  itsLastY > itsY+Glx::Slider::THICKNESS )
      {
	unselect();
	return Glx::Draggable::UNSELECTED;
      }
      // ok, we're in the box, see if we're close to the thumb
      delta = itsLastX-(itsX+thumbPos);
      if( fabs(delta)<Glx::Slider::THUMB_WIDTH ){
	itsSelectionDist=0;
	return Glx::Slider::THUMB_HANDLE;
      }
      break;
    case Glx::Slider::Y:
      if( itsLastX < itsX || 
	  itsLastX > itsX+Glx::Slider::THICKNESS || 
	  itsLastY < itsY || 
	  itsLastY > itsY+itsW )
      {
	unselect();
	return Glx::Draggable::UNSELECTED;
      }
      // ok, we're in the box, see if we're close to the thumb
      delta = itsLastY-(itsY+thumbPos);
      if( fabs(delta)<Glx::Slider::THUMB_WIDTH ){
	itsSelectionDist=0;
	return Glx::Slider::THUMB_HANDLE;
      }
      break;
  }
  
  itsSelectionDist=0;
  return Glx::Slider::SLIDER_HANDLE;
}

void Glx::Slider::keyEvent(int key, bool ctl, bool shift, bool alt)
{
  switch( key ){
    case XK_Up:
    case XK_Right:
      itsCurValue += 1.;
      break;
    case XK_Down:
    case XK_Left:
      itsCurValue -= 1.;
      break;
  }
  CLAMP(itsCurValue,itsMinRange,itsMaxRange); 
  if( itsUserFunc != 0)
    itsUserFunc(itsUserData,itsCurValue);
  updateFuncs();
  itsEnv->wakeup();
}

void Glx::Slider::handleDrag(glx* env, int x, int y, void*)
{
  float t,value;
  int envH = env->winHeight();
  y=envH-y;

  switch( env->buttonpressed ){
    case glx::LEFT: 
      if( itsCurDragHandle==Glx::Slider::THUMB_HANDLE )
      {
	itsState = Glx::Slider::NORMAL;
	switch( itsAxis ){
	  case Glx::Slider::X:
	    t = (float)(x - itsX)/(float)itsW;
	    break;
	  case Glx::Slider::Y:
	    t = (float)(y - itsY)/(float)itsW;
	    break;
	}
	CLAMP(t,0,1);
	value = itsMinRange + t*(itsMaxRange-itsMinRange); 
	CLAMP(value,itsMinRange,itsMaxRange); 
	if( itsSnapToIntFlag ) 
	  value = (int)(0.5f + value);	
	if(value == itsCurValue )
	  return;
	itsCurValue = value;
	if( itsUserFunc != 0)
	  itsUserFunc(itsUserData,itsCurValue);
      } 
      else if( itsCurDragHandle==Glx::Slider::SLIDER_HANDLE )
      {
	if( itsState == Glx::Slider::NORMAL ){
	  itsOffX = x-itsX;
	  itsOffY = y-itsY;
	}
	itsState = Glx::Slider::MOVING;
	itsX = x - itsOffX;
	itsY = y - itsOffY;
      }
      break;
    case glx::MIDDLE:
      itsState = Glx::Slider::RESIZING;
      switch( itsAxis ){
	case Glx::Slider::X:
	  itsW = x-itsX;
	  break;
	case Glx::Slider::Y:
	  itsW = y-itsY;
	  break;
      }
      break;
  }
  updateFuncs();
}

void Glx::Slider::updateFuncs(void)
{
  float t = (itsCurValue-itsMinRange)/(itsMaxRange-itsMinRange);
  int off = (int)(t * itsW);
  if( off < 0 ) off = 0;
  if( off > itsW ) off = itsW;
  float p0[2];
  float p1[2];
  float p2[2];

  switch( itsAxis ){
    case Glx::Slider::X:
      p0[0]=itsX+off-10;
      p0[1]=itsY-10;

      p1[0]=itsX+off;
      p1[1]=itsY;

      p2[0]=itsX+off+10;
      p2[1]=itsY-10;
      break;
    case Glx::Slider::Y:
      p0[0]=itsX-10;
      p0[1]=itsY+off-10;

      p1[0]=itsX;
      p1[1]=itsY+off;

      p2[0]=itsX-10;
      p2[1]=itsY+off+10;
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

void Glx::Slider::setColor(int handle)
{
  if( itsCurDragHandle == handle )
    glColor3f(1.0f,0.0f,0.0f);
  else
    glColor3f(0.0f,1.0f,0.0f);
}
