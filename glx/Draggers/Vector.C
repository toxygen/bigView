// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Class Glx::VectorDragger
///////////////////////////////////////////////////////////////////////////////

#include <GL/gl.h>
#include <Draggers/Vector.h>
#include <GLX.h>
#include <values.h> // for MAXFLOAT
using namespace std;

//#define DEBUG 1
#include "debug.h"

Glx::VectorDragger::VectorDragger(glx* env,bool registerDragger) : 
  Draggable3D(env,registerDragger),
  needsInit(true),
  itsVector(1,0,0),
  lastY(0),c(0),
  itsTubeVecProjectionRadius(0),
  itsTubeVecRadius(0.075f),
  itsNeedOffset(false)
{
  itsArrow = new Glx::Arrow();
}

Glx::VectorDragger::~VectorDragger(void)
{
  delete itsArrow;
}

void 
Glx::VectorDragger::draw(glx* env,void *)
{
  float defaultMaterial[4] = {1.0,1.0,1.0,1.0};
  if( itsVisibleFlag == 0)     
    return;

  glPushAttrib(GL_ENABLE_BIT);

  if(needsInit){
    viewHasChanged(env);
    needsInit=false;
  }

  if( itsCurDragHandle == VECTOR ){
    glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    glColor3f(1.0,0.0,0.0);
    itsArrow->draw(env,NULL);
    glDisable(GL_COLOR_MATERIAL);
    glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,defaultMaterial);	
    env->showMessage("mouse=translate along arrow vector");
  } else {
    glMaterialfv(GL_FRONT,GL_DIFFUSE,defaultMaterial);
    itsArrow->draw(env,NULL);
  }

  if( itsCurDragHandle != UNSELECTED ){
    env->showMessage("shift-mouse=scale");
  }
  glPopAttrib();
}

float Glx::VectorDragger::getScale(void)
{
  return itsArrow->getScale();
}

void  Glx::VectorDragger::setScale(float scale)
{
  itsArrow->setScale(scale);
  viewHasChanged(itsEnv);
  callFuncs(itsUpdateFuncs);
}

void 
Glx::VectorDragger::setPosition(Glx::Vector& p, bool call)
{
  Glx::Draggable::setPosition(p);
  itsArrow->setPos(p);
  viewHasChanged(itsEnv);
  callFuncs(itsUpdateFuncs);
  if(call) callFuncs(itsDragFuncs);
}
void 
Glx::VectorDragger::setPosition(float p[3], bool call)
{
  Glx::Draggable::setPosition(p);
  itsArrow->setPos(p);
  viewHasChanged(itsEnv);
  callFuncs(itsUpdateFuncs);
  if(call) callFuncs(itsDragFuncs);
}
void 
Glx::VectorDragger::setPosition(float p0, float p1, float p2, bool call)
{
  Glx::Draggable::setPosition(p0,p1,p2);
  itsArrow->setPos(p0,p1,p2);
  viewHasChanged(itsEnv);
  callFuncs(itsUpdateFuncs);
  if(call) callFuncs(itsDragFuncs);
}
void 
Glx::VectorDragger::setVector(Glx::Vector& v, bool call)
{
  itsArrow->setVec(v);
  viewHasChanged(itsEnv);
  callFuncs(itsUpdateFuncs);
  if(call) callFuncs(itsDragFuncs);
}
void 
Glx::VectorDragger::setVector(float v[3], bool call)
{
  itsArrow->setVec(v);
  viewHasChanged(itsEnv);
  callFuncs(itsUpdateFuncs);
  if(call) callFuncs(itsDragFuncs);
}
void 
Glx::VectorDragger::setVector(float v0, float v1, float v2, bool call)
{
  itsArrow->setVec(v0,v1,v2);
  viewHasChanged(itsEnv);
  callFuncs(itsUpdateFuncs);
  if(call) callFuncs(itsDragFuncs);
}

void  Glx::VectorDragger::getVector(Glx::Vector& v)
{
  itsArrow->getVec(v);
}

void 
Glx::VectorDragger::setOrientation(const Glx::Quat& quat, bool call)
{
  itsArrow->setOrientation(quat);
  viewHasChanged(itsEnv); 
  callFuncs(itsUpdateFuncs);
  if(call) callFuncs(itsDragFuncs);
}
void 
Glx::VectorDragger::getOrientation(Glx::Quat& orientationQuat)
{
  itsArrow->getOrientation(orientationQuat); 
}

void 
Glx::VectorDragger::handleMouseUp(glx* env,void* user)
{
  FANCYMESG("Glx::VectorDragger::handleMouseUp");
  itsNeedOffset=true;
  viewHasChanged(env);
  doneWithDrag();
}

void 
Glx::VectorDragger::handleDrag(glx* env,int x,int y,void*)
{
  Glx::Vector yLeft,yRight,planeNormal;
  double xf,yf;
  double nearDist    = env->proj(glx::NEAR);
  double projWidth   = env->proj(glx::PROJW);
  double projHeight  = env->proj(glx::PROJH);
  double aspect      = env->aspect();
  double EPS         = 1.0e-6;

  if( itsVisibleFlag == 0) 
    return;

  env->pixelToWinCoords(x,y,&xf,&yf);
  xf *= aspect;

  // SHIFTKEY down = scale dragger
  XKeyEvent *kep = (XKeyEvent *)(env->event);
  if(  kep->state & ShiftMask ){
    float dy = yf-lastY;
    float scale = getScale();
    if( dy > 0.0 ){
      scale /= (float)1.1;
    } else {
      scale *= (float)1.1;
    }
    setScale(scale);
    lastY = yf;
    return;
  }

  float dx = v2[0]-v1[0];
  float dy = v2[1]-v1[1];
  if( fabs(dy) > fabs(dx) ){
    yLeft.set(-projWidth, yf*projHeight, -nearDist);
    yRight.set(projWidth, yf*projHeight, -nearDist);
  } else {
    yLeft.set(xf*projWidth/aspect,  projHeight, -nearDist);
    yRight.set(xf*projWidth/aspect, -projHeight, -nearDist);	
  }
  calcNormal(yLeft,yRight,itsCurInverse,itsCurEyePos,planeNormal);
  planeNormal.normalize();

  Glx::Vector vec = itsVector;
  float nx=planeNormal[0];
  float ny=planeNormal[1];
  float nz=planeNormal[2];
  float vx=vec[0];
  float vy=vec[1];
  float vz=vec[2];
  float x0=itsCurEyePos[0];
  float y0=itsCurEyePos[1];
  float z0=itsCurEyePos[2];
  float px=itsPos[0];
  float py=itsPos[1];
  float pz=itsPos[2];
  float num = -(nx*px) - ny*py - nz*pz + nx*x0 + ny*y0 + nz*z0;
  float denom = nx*vx + ny*vy + nz*vz;
  if( fabs(denom)<EPS || fabs(num)<EPS )
    return;
  float t = num/denom;
  if( itsNeedOffset ){
    itsOffset = t * vec;
    itsNeedOffset=false;
  } else {
    Glx::Vector delta = t * vec - itsOffset;
    itsPos += delta;
  }
  
  itsArrow->setPos(itsPos);
  itsArrow->getVec(itsVector); 
  callDrag();
}

int  
Glx::VectorDragger::idlePick(glx* env,int x,int y,void*)
{  
  double xf,yf;
  int candidate = Glx::Draggable::UNSELECTED;
  double worldDist=MAXFLOAT;
  double vecProjDist,vecWorldDist;
  double t;
  Glx::Vector vecIntersection,eyeToVec;

  env->pixelToWinCoords(x,y,&xf,&yf);

  // calc the 2D distance from the vector
  // essentially here considered as a 'fat' line
  vecProjDist = calcDistFromVector(xf,yf,&t);

  // elect this as a candidate if we are within
  // the projection of the vector
  if( vecProjDist < itsTubeVecProjectionRadius ){
    vecIntersection = point1 + (point2-point1)*t;
    eyeToVec = vecIntersection - itsCurEyePos;
    vecWorldDist = eyeToVec.magnitude();
    if( candidate == UNSELECTED ){
      candidate = VECTOR;
      worldDist = vecWorldDist;
    } 
  }  
  
  if( candidate == UNSELECTED )
    unselect();

  itsSelectionDist = worldDist;
  return candidate;
}

double 
Glx::VectorDragger::calcDistFromVector(double x, double y, double* t)
{
  double EPS = 1.0e-6;
  double q = c + (n[0]*x + n[1]*y);
  double denom, dist;

  Q[0] = x - q*n[0];
  Q[1] = y - q*n[1];

  // solve for t, i.e. the normalized position of
  // Q on the line between v1->v2. 
  // if 0<= t <= 1, then the Q is actaully on the line segment
  // otherwise, Q is not on the line segment, and should not
  // be considered for picking
  
  denom = v2[0]-v1[0];

  // is X not availible for calculation of t?
  if( fabs(denom)<EPS ){
    
    // ok, try to use Y
    denom = v2[1]-v1[1];
    if( fabs(denom)<EPS ){
      
      // ok, do a point distance
      dist = sqrt( (x-v1[0])*(x-v1[0]) + (y-v1[1])*(y-v1[1]) );
      *t = 0.0; // hmmm???
      return dist;	
    }
    
    // ok, Y worked
    *t = (Q[1] - v1[1])/denom;
    
    // calc t, return MAXFLOAT if t is out of bounds
    if( *t < 0.0 || *t > 1.0 ){
      return MAXFLOAT;
    }
    // t is good, calc the distance between Q and the incoming point
    dist = sqrt( (x-Q[0])*(x-Q[0]) + (y-Q[1])*(y-Q[1]) );
    return dist;
  }
  
  // ok, X worked
  *t = (Q[0] - v1[0])/denom;
  
  // calc t, return MAXFLOAT if t is out of bounds
  if( *t < 0.0 || *t > 1.0 ){
    return MAXFLOAT;
  }
  
  // t is good, calc the distance between Q and the incoming point
  dist = sqrt( (x-Q[0])*(x-Q[0]) + (y-Q[1])*(y-Q[1]) );
  return dist;
}

void 
Glx::VectorDragger::viewHasChanged(glx* env)
{
  FANCYMESG("Glx::VectorDragger::viewHasChanged(glx*)");
  double viewMatrix[16],projMatrix[16], vp[16];
  if( ! env->makeCurrent() )
    return;

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  env->applyProjections();
  glGetDoublev(GL_MODELVIEW_MATRIX,viewMatrix);
  glGetDoublev(GL_PROJECTION_MATRIX,projMatrix);
  glPopMatrix();

  // combine the view and projection matrices
  Glx::mult4x4( viewMatrix, projMatrix, vp );

  // get the inverse
  Glx::inv4x4(viewMatrix, itsCurInverse);
  viewHasChanged(env,vp,itsCurInverse);
  VAR(vp);
  VAR(itsCurInverse);
}

void 
Glx::VectorDragger::viewHasChanged(glx* env, 
				   const double viewAndProjMatrix[16], 
				   const double invMatrix[16])
{
  FANCYMESG("Glx::VectorDragger::viewHasChanged(glx*,double*,double*)");
  double radius      = itsTubeVecRadius;
  double nearDist    = env->proj(glx::NEAR);
  double projWidth   = env->proj(glx::PROJW);
  double projHeight  = env->proj(glx::PROJH);
  double aspect      = env->aspect();
  double EPS         = 1.0e-6;
  double eyeDistToProjPlane = nearDist;

  updateProj(env,viewAndProjMatrix);

  Glx::Vector eye;
  Glx::xformVec(eye,invMatrix,itsCurEyePos);

  Glx::Vector eyeToV1 = point1 - itsCurEyePos;
  double eyeDistToV1 = eyeToV1.magnitude();

  itsTubeVecProjectionRadius = (radius * eyeDistToProjPlane)/eyeDistToV1;
  itsTubeVecProjectionRadius /= projHeight;
  itsNeedOffset=true;
}

void 
Glx::VectorDragger::updateProj(glx* env, const double viewAndProjMatrix[16])
{
  float aspect = env->aspect();
  const float EPS = 1.0e-6;
  itsArrow->getVec(itsVector);
  point1 = itsPos;
  point2 = point1 + itsVector*getScale();

  Glx::xformVec(point1,viewAndProjMatrix,v1);
  Glx::xformVec(point2,viewAndProjMatrix,v2);
  
  ////////////////////////////////////////////
  // 1. This is the vector from v1 -> v2
  // 
  // [ v2[X]-v1[X],v2[Y]-v1[Y] ] 
  //
  // 2. rotate this 90 deg CCW, by multiplying 
  //    it by a rotation matrix:
  //
  // [ v2[X]-v1[X],v2[Y]-v1[Y] ] [ 0 -1 ] = N 
  //                             [ 1  0 ]
  ////////////////////////////////////////////
  
  n[0] = v2[1]-v1[1];
  n[1] = v1[0]-v2[0];
  
  // normalize N
  float squaredMag = n[0]*n[0] + n[1]*n[1];
  float mag = sqrt(squaredMag);
  if( fabs(mag)>EPS ){
    n[0] /= mag;
    n[1] /= mag;
  }
  
  ////////////////////////////////////////////
  // c = -N . V1
  ////////////////////////////////////////////
  c = -( n[0]*v1[0] + n[1]*v1[1] );
}
