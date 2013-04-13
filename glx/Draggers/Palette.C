				   
#include <Draggers/Palette.h>
#include <GLX.h>

//#define DEBUG 1
#include "debug.h"

using namespace std;

Glx::Palette::Palette(glx* env, int x, int y, int w, int h) :  
  Glx::Draggable2D(env),
  itsOffX(0),itsOffY(0),
  itsX(x),itsY(y),itsW(w),itsH(h),
  itsState(Glx::Palette::NORMAL)
{
}

Glx::Palette::~Palette(void)
{
}

void 
Glx::Palette::draw(glx* env,void*)
{
  if( ! itsVisibleFlag )
    return;

  int lw;
  int envW = env->winWidth();
  int envH = env->winHeight();
  
  glGetIntegerv(GL_LINE_WIDTH,&lw);
  glPushAttrib(GL_DEPTH_BUFFER_BIT|GL_LIGHTING_BIT);
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_COLOR_MATERIAL);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0,envW,0,envH,-1,1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // draw the entire container
  if( itsCurDragHandle == Glx::Palette::MOVE )
    glColor3f(0.5f,0.0f,0.0f);
  else 
    glColor3f(0.0f,0.5f,0.0f);

  glLineWidth(1);
  glBegin(GL_LINE_LOOP);
  glVertex3f(itsX,     itsY,-0.1);
  glVertex3f(itsX,     itsY+itsH,-0.1);
  glVertex3f(itsX+itsW,itsY+itsH,-0.1);
  glVertex3f(itsX+itsW,itsY,-0.1);
  glEnd();

  // draw the resize handle
  if( itsCurDragHandle == Glx::Palette::RESIZE )
    glColor3f(0.5f,0.0f,0.0f);
  else 
    glColor3f(0.0f,0.5f,0.0f);

  glLineWidth(1);
  glBegin(GL_LINE_LOOP);
  glVertex3f(itsX+itsW-HANDLESIZE, itsY+itsH,-0.1);
  glVertex3f(itsX+itsW-HANDLESIZE, itsY+itsH-HANDLESIZE,-0.1);
  glVertex3f(itsX+itsW,            itsY+itsH-HANDLESIZE,-0.1);
  glVertex3f(itsX+itsW,            itsY+itsH,-0.1);
  glEnd();

  // Show Usage messages
  glColor3f(1,1,1);
  switch( itsCurDragHandle ){
    case Glx::Palette::MOVE:
      env->showMessage("Left mouse = move");
      break;
    case Glx::Palette::RESIZE:
      env->showMessage("Left mouse = resize");
      break;
  }

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
  glLineWidth(lw);
}

int 
Glx::Palette::idlePick(glx * env,int x, int y, void*)
{
  int envH = env->winHeight();
  y=envH-y;
  itsSelectionDist = MAXFLOAT;

  if( x >= (itsX+itsW-HANDLESIZE) && x <= (itsX+itsW) &&
      y >= (itsY+itsH-HANDLESIZE) && y <= (itsY+itsH) )
  {
    itsSelectionDist=0;
    return Glx::Palette::RESIZE;
  } 
  else if( x >= itsX && x <= (itsX+itsW) &&
	   y >= itsY && y <= (itsY+itsH) )
  {
    itsSelectionDist=0;
    return Glx::Palette::MOVE;
  }
  unselect();
  return UNSELECTED;
}

void 
Glx::Palette::handleMouseUp(glx *, void*)
{
  unselect();
  itsState = Glx::Palette::NORMAL;
}

void 
Glx::Palette::handleDrag(glx* env, int x, int y, void*)
{
  int envH = env->winHeight();
  float t;
  y=envH-y;
  
  switch( env->buttonpressed ){
    case glx::LEFT:
      if( itsCurDragHandle == Glx::Palette::MOVE ) 
      {
	if( itsState == Glx::Palette::NORMAL )
	{
	  itsOffX = x-itsX;
	  itsOffY = y-itsY;
	}
	itsState = Glx::Palette::MOVING;
	itsX = x - itsOffX;
	itsY = y - itsOffY;
      }
      else if( itsCurDragHandle == Glx::Palette::RESIZE )
      {
	if( itsState == Glx::Palette::NORMAL )
	{
	  itsOffX = itsX;
	  itsOffY = itsY;
	}
	itsState = Glx::Palette::MOVING;
	itsW = x - itsOffX;
	itsH = y - itsOffY;
      }
      break;
  }
}
