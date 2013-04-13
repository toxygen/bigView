// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Class Glx::Crosshair2D
///////////////////////////////////////////////////////////////////////////////

#include <Draggers/Crosshair2D.h>
#include <GLX.h>
#include "debug.h"

using namespace std;

#define PICK_DIST 0.01

float Glx::Crosshair2D::itsScale = 10.0f;

Glx::Crosshair2D::Crosshair2D(glx* env, int x, int y) : 
  Glx::Draggable2D(env)
{
  itsPos[0]=x;
  itsPos[1]=y;
}

Glx::Crosshair2D::~Crosshair2D(void) 
{
}

void Glx::Crosshair2D::draw(glx* env, void *)
{
  if( itsVisibleFlag == 0) return;

  int envW = env->winWidth();
  int envH = env->winHeight();
  float aspect=1.0;//env->aspect();

  glPushAttrib(GL_LIGHTING);
  glDisable(GL_LIGHTING);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0,envW,0,envH,-1,1);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  if( itsCurDragHandle == CROSSHAIR2D_HANDLE )
    glColor3f(1.0f,0.0f,0.0f);
  else
    glColor3f(0.0f,1.0f,0.0f);
  glBegin(GL_LINES);
  glVertex2f(itsPos[0]-itsScale/aspect,itsPos[1]);
  glVertex2f(itsPos[0]+itsScale/aspect,itsPos[1]);
  glVertex2f(itsPos[0],itsPos[1]-itsScale);
  glVertex2f(itsPos[0],itsPos[1]+itsScale);
  glEnd();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
}

void Glx::Crosshair2D::handleMouseUp(glx*,void*)
{
  doneWithDrag();
}

int Glx::Crosshair2D::idlePick(glx *env, int x, int y, void*)
{
  if( itsVisibleFlag == 0) 
    return Glx::Draggable::UNSELECTED;

  int envH = env->winHeight();
  y=envH-y;

  float selectDist = itsScale;
  float dx = fabs(x-itsPos[0]);
  float dy = fabs(y-itsPos[1]);

  itsSelectionDist = MAXFLOAT;

  if( dx<3 && dy<selectDist)
  {
    itsSelectionDist = dx<dy ? dx : dy;
    return Glx::Crosshair2D::CROSSHAIR2D_HANDLE;
  } 
  else if( dy<3 && dx<selectDist)
  {
    itsSelectionDist = dx<dy ? dx : dy;
    return Glx::Crosshair2D::CROSSHAIR2D_HANDLE;
  } 
  
  unselect();
  return Glx::Draggable::UNSELECTED;
}

void Glx::Crosshair2D::handleDrag(glx* env, int x, int y, void*)
{
  static int lastX = 0;
  static int lastY = 0;

  if( itsVisibleFlag == 0)     
    return;

  int envH = env->winHeight();
  y=envH-y;

  if( lastX==0 && lastY==0 ){
    lastX = x;
    lastY = y;
    return;
  }

  XKeyEvent *kep = (XKeyEvent *)(env->event);
  int shiftPressed = kep->state & ShiftMask;

  // SHIFTKEY down = scale dragger
  if( shiftPressed ){
    int dy = y-lastY;
    float scale = itsScale;
    if( dy > 0 ){
      scale /= (float)1.1;
    } else {
      scale *= (float)1.1;
    }
    itsScale = scale;
    
    callFuncs(itsUpdateFuncs);
    
    lastX = x;
    lastY = y;
    return;
  }
  
  itsPos[0] = x;
  itsPos[1] = y;

  if(itsUserFunc)
    itsUserFunc(this,(int)itsPos[0],(int)itsPos[1],itsUserData);
}

//////////////////////////////////////////////////////////////////////////
/////////////////////////// class Crosshair2Df ///////////////////////////
//////////////////////////////////////////////////////////////////////////

float Glx::Crosshair2Df::itsScale = 1.0f;

Glx::Crosshair2Df::Crosshair2Df(glx* env, float x, float y) : 
  Glx::Draggable2D(env)
{
  itsPosf[0]=x;
  itsPosf[1]=y;
  limited[Crosshair2Df::X]=limited[Crosshair2Df::Y]=false;
  constrained[Crosshair2Df::X]=constrained[Crosshair2Df::Y]=false;  
}

Glx::Crosshair2Df::~Crosshair2Df(void) 
{
}

void Glx::Crosshair2Df::draw(glx* env, void *)
{
  if( itsVisibleFlag == 0) return;
  int envW = env->winWidth();
  int envH = env->winHeight();
  float aspect=1.;//env->aspect();

  glPushAttrib(GL_LIGHTING);
  glDisable(GL_LIGHTING);

  if( itsCurDragHandle == CROSSHAIR2D_HANDLE )
    glColor3f(1.0f,0.0f,0.0f);
  else
    glColor3f(0.0f,1.0f,0.0f);
  glBegin(GL_LINES);
  glVertex2f(itsPosf[0]-itsScale/aspect,itsPosf[1]);
  glVertex2f(itsPosf[0]+itsScale/aspect,itsPosf[1]);
  glVertex2f(itsPosf[0],itsPosf[1]-itsScale);
  glVertex2f(itsPosf[0],itsPosf[1]+itsScale);
  glEnd();

  glPopAttrib();
}

void Glx::Crosshair2Df::handleMouseUp(glx*,void*)
{
  doneWithDrag();
}

int Glx::Crosshair2Df::idlePick(glx *env, int x, int y, void*)
{
  _FANCYMESG("Glx::Crosshair2Df::idlePick");
  double worldX,worldY;

  if( itsVisibleFlag == 0) 
    return Glx::Draggable::UNSELECTED;

  int envH = env->winHeight();
  env->pixelToWorldCoords(x,envH-y,&worldX,&worldY);

  float selectDist = itsScale;
  float dx = fabs(worldX-itsPosf[0]);
  float dy = fabs(worldY-itsPosf[1]);

  itsSelectionDist = MAXFLOAT;

  if( dx<selectDist/10.0f && dy<selectDist)
  {
    itsSelectionDist = dx<dy ? dx : dy;
    return Glx::Crosshair2Df::CROSSHAIR2D_HANDLE;
  } 
  else if( dy<selectDist/10.0f && dx<selectDist)
  {
    itsSelectionDist = dx<dy ? dx : dy;
    return Glx::Crosshair2Df::CROSSHAIR2D_HANDLE;
  } 
  
  unselect();
  return Glx::Draggable::UNSELECTED;
}

void Glx::Crosshair2Df::handleDrag(glx* env, int x, int y, void*)
{
  _FANCYMESG("Glx::Crosshair2Df::handleDrag");

  double world[2];
  static int lastX = 0;
  static int lastY = 0;

  if( itsVisibleFlag == 0)     
    return;

  if( lastX==0 && lastY==0 ){
    lastX = x;
    lastY = y;
    return;
  }

  int envH = env->winHeight();
  env->pixelToWorldCoords(x,envH-y,&world[X],&world[Y]);

  XKeyEvent *kep = (XKeyEvent *)(env->event);
  int shiftPressed = kep->state & ShiftMask;

  // SHIFTKEY down = scale dragger
  if( shiftPressed ){
    int dy = lastY-y;
    float scale = itsScale;
    if( dy > 0 ){
      scale /= (float)1.1;
    } else {
      scale *= (float)1.1;
    }
    itsScale = scale;
    
    callFuncs(itsUpdateFuncs);
    
    lastX = x;
    lastY = y;
    return;
  }
  
  for(int i=X;i<=Y;++i){
    if( ! constrained[i] ) {
      if( limited[i] ){
	itsPosf[i] = world[i];
	if( itsPosf[i]<limits[i][LO] ) itsPosf[i]=limits[i][LO];
	else if( itsPosf[i]>limits[i][HI] ) itsPosf[i]=limits[i][HI];
      } else 
	itsPosf[i] = world[i];
    }
  }

  if(itsUserFunc){
    _MESGVAR2("Glx::Crosshair2Df::handleDrag",itsPosf[X],itsPosf[Y]);
    itsUserFunc(this,itsPosf[X],itsPosf[Y],itsUserData);
  }
}

void Glx::Crosshair2Df::constrain(Crosshair2Df::Axis axis, bool enabled)
{
  constrained[axis]=enabled;
}

void Glx::Crosshair2Df::limit(Crosshair2Df::Axis axis, 
			      double lo,double hi)
{
  limited[axis]=true;
  limits[axis][Crosshair2Df::LO]=lo;
  limits[axis][Crosshair2Df::HI]=hi;
}
