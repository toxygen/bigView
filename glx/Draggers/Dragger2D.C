// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Class Glx::Draggable2D
///////////////////////////////////////////////////////////////////////////////

#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Intrinsic.h>
#include <X11/X.h>
#include <X11/Xlib.h> 
#include <values.h>
#include <Draggers/Dragger2D.h>

using namespace std;

void Glx::Draggable2D::setPosition(float x, float y, int callUser)
{
  itsPos.set(x,y,0);
  if( callUser )
    callDrag();
}

void Glx::Draggable2D::setPosition(float xy[2], int callUser)
{
  itsPos.set(xy[0],xy[1],0);
  if( callUser )
    callDrag();
}

void Glx::Draggable2D::getPosition(float xy[2])
{
  xy[0] = itsPos[0];
  xy[1] = itsPos[1];
}
