// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Class Glx::Check
///////////////////////////////////////////////////////////////////////////////

#include <Draggers/Check.h>
#include <GLX.h>
#include <math.h>
#include <values.h>
#include <sstream>

using namespace std;

//#define DEBUG 1
#include <debug.h>

#define CLAMP(_val,_min,_max){ \
  if(_val<_min)_val=_min; \
  if(_val>_max)_val=_max; \
}
#define LIMIT(_val,_max){if(_val>_max)_val=_max;}

Glx::Check::Check(glx* env,int x, int y, int w, int h, string label) : 
  Glx::Draggable2D(env),
  itsX(x),itsY(y),itsW(w),itsH(h),itsOffX(0),itsOffY(0),
  itsCurValue(false),
  itsState(Glx::Check::NORMAL),
  itsUserFunc(0),
  itsUserData(0),
  itsLabel(label),
  readyForChange(true)
{
  env->addMouseDownFunc(Glx::Check::mouseDown,glx::LEFT,this);
}

void 
Glx::Check::mouseDown(glx* env,int x,int y,void* user)
{
  Glx::Check* _this = static_cast<Glx::Check*>(user);
  if( _this->idlePick(env,x,y,NULL)!=Glx::Draggable::UNSELECTED ){
    _this->itsCurValue = ! _this->itsCurValue;
    if( _this->itsUserFunc != 0)
      _this->itsUserFunc(_this->itsUserData,_this->itsCurValue);
  }
}

void 
Glx::Check::draw(glx* env, void *)
{
  FANCYMESG("Glx::Check::draw");
  VAR(itsVisibleFlag);
  VAR(itsCurDragHandle);
  VAR(itsState);
  VAR(itsLabel);
  VAR(itsCurValue);
  int envW = env->winWidth();
  int envH = env->winHeight();
  if( itsVisibleFlag == 0)     
    return;

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
    case CHECK_HANDLE:
      glColor3f(1,0,0);
      env->showMessage("  Left mouse = check/uncheck");
      break;
  }

  setColor(CHECK_HANDLE);

  if( itsState == Glx::Check::MOVING ){    
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

  // check box

  if( itsLabel.length() )
    env->showMessage(itsX,itsY+itsW+10,itsLabel.c_str());
  setColor(CHECK_HANDLE);
  glBegin(GL_LINE_LOOP);
  glVertex2i(itsX,itsY);
  glVertex2i(itsX+itsW,itsY);
  glVertex2i(itsX+itsW,itsY+itsW);
  glVertex2i(itsX,itsY+itsW);
  glEnd();

  if( itsCurValue ){
    setColor(CHECK_HANDLE);
    glBegin(GL_LINES);
    glVertex2i(itsX,itsY);
    glVertex2i(itsX+itsW,itsY+itsW);
    glVertex2i(itsX+itsW,itsY);
    glVertex2i(itsX,itsY+itsW);
    glEnd();    
  }

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
}

void Glx::Check::handleMouseUp(glx*,void*)
{
  unselect();
  itsState = Glx::Check::NORMAL;
  readyForChange=true;
}

int Glx::Check::idlePick(glx *env, int x, int y, void*)
{
  int envH = env->winHeight();
  y=envH-y;

  itsSelectionDist = MAXFLOAT;

  if( x < itsX || x > itsX+itsW || 
      y < itsY || y > itsY+itsW )
  {
    unselect();
    return Glx::Draggable::UNSELECTED;
  }
  
  itsSelectionDist=0;
  return Glx::Check::CHECK_HANDLE;
}

void Glx::Check::handleDrag(glx* env, int x, int y, void*)
{
  float t,value;
  int envH = env->winHeight();
  y=envH-y;

  switch( env->buttonpressed ){
    case glx::LEFT: 
      break;
    case glx::RIGHT:
      itsState = Glx::Check::MOVING;
      itsX = x - itsOffX;
      itsY = y - itsOffY;
      break;
    default:
      break;
  }
}

void Glx::Check::setColor(int handle)
{
  if( itsCurDragHandle == handle )
    glColor3f(1.0f,0.0f,0.0f);
  else
    glColor3f(0.0f,1.0f,0.0f);
}
