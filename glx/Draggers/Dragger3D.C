// Emacs mode -*-c++-*- //
//////////////////////////////////////////////////////////////////////
// Glx::Draggable3D
//////////////////////////////////////////////////////////////////////

#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Intrinsic.h>
#include <X11/X.h>
#include <X11/Xlib.h> 
#include <values.h>
#include <Draggers/Dragger3D.h>

using namespace std;

Glx::Draggable3D::Draggable3D(glx* env, bool registerDragger) : 
  Glx::Draggable(env,registerDragger)
{
}

void 
Glx::Draggable3D::calcNormal(Glx::Vector& yLeft,
			     Glx::Vector& yRight,
			     double invMatrix[16],
			     Glx::Vector& eyePos, 
			     Glx::Vector& res)
{
  Glx::Vector left,right,lLeg,rLeg;

  Glx::xformVec(yLeft,invMatrix,left);
  Glx::xformVec(yRight,invMatrix,right);

  lLeg[0] = left[0]-eyePos[0];
  lLeg[1] = left[1]-eyePos[1];
  lLeg[2] = left[2]-eyePos[2];

  rLeg[0] = right[0]-eyePos[0];
  rLeg[1] = right[1]-eyePos[1];
  rLeg[2] = right[2]-eyePos[2];

  res[0] =   lLeg[1]*rLeg[2] - lLeg[2]*rLeg[1];
  res[1] = -(lLeg[0]*rLeg[2] - lLeg[2]*rLeg[0]);
  res[2] =   lLeg[0]*rLeg[1] - lLeg[1]*rLeg[0];
}

