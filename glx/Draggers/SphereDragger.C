// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Class Glx::SphereDragger
///////////////////////////////////////////////////////////////////////////////

#include <GL/gl.h>
#include <GLX.h>
#include <Draggers/SphereDragger.h>
#include <glxTrackball.h>
#include <values.h> // for MAXFLOAT

//#define DEBUG 1
#include "debug.h"

#define R2D(r) (double)(r*57.29577951308232087679815481411)

using namespace std;

static float defaultMaterial[4] = {1,1,1,1};

Glx::SphereDragger::SphereDragger(glx* env, bool registerDragger) : 
  Draggable3D(env,registerDragger),
  needsInit(true),
  itsWireMode(false),
  itsEnableRotationFlag(true),
  needsCoord(true)
{
  itsSphere = new Glx::Sphere;
  itsSphere->setCenterCrosshairs(1);
}

Glx::SphereDragger::~SphereDragger(void)
{ 
  delete itsSphere;
}

void Glx::SphereDragger::setPosition(Glx::Vector& p, bool call)
{
  Glx::Draggable::setPosition(p);
  itsSphere->setCenter(p);
  viewHasChanged(itsEnv);
  callFuncs(itsUpdateFuncs);
  if( call ) callFuncs(itsDragFuncs);
}

void Glx::SphereDragger::setPosition(float p[3], bool call)
{
  Glx::Draggable::setPosition(p);
  itsSphere->setCenter(p); 
  viewHasChanged(itsEnv);
  callFuncs(itsUpdateFuncs);
  if( call ) callFuncs(itsDragFuncs);
}

void Glx::SphereDragger::setPosition(float p0, float p1, float p2, bool call)
{
  Glx::Draggable::setPosition(p0,p1,p2);
  itsSphere->setCenter(p0,p1,p2); 
  viewHasChanged(itsEnv);
  callFuncs(itsUpdateFuncs);
  if( call ) callFuncs(itsDragFuncs);
}

float Glx::SphereDragger::getScale(void)
{
  return itsSphere->getRadius();
}

void Glx::SphereDragger::setScale(float scale)
{
  itsSphere->setRadius(scale);
  viewHasChanged(itsEnv);
  callFuncs(itsUpdateFuncs);
}

void 
Glx::SphereDragger::getOrientation(Glx::Quat& orientationQuat)
{
  itsSphere->getOrientation(orientationQuat);
}

void 
Glx::SphereDragger::setOrientation(const Glx::Quat& orientationQuat, bool call)
{
  itsSphere->setOrientation(orientationQuat);
  viewHasChanged(itsEnv);
  callFuncs(itsUpdateFuncs);
  if( call ) callFuncs(itsDragFuncs);
}

void 
Glx::SphereDragger::handleDrag(glx* env, int x, int y, void*)
{
  double xf,yf;
  double projWidth   = env->proj(glx::PROJW);
  double projHeight  = env->proj(glx::PROJH);
  double nearDist    = env->proj(glx::NEAR);
  double aspect      = env->aspect();
  double deltaY,itsZtrans,factor;
  Glx::Vector sphereCenter,deltaCenter,mouseProjectionVector;
  Glx::Vector eyeToSphereVec;
  Glx::Quat deltaQuat;
  env->pixelToWinCoords(x,y,&xf,&yf);
  xf *= aspect;

  FANCYMESG("Glx::SphereDragger::handleDrag");
  VAR(itsSphereProjection);

  if( needsCoord ){
    startX = xf - itsSphereProjection[0];
    startY = yf - itsSphereProjection[1];
    lastY = yf;
    needsCoord=false;
    return;
  }

  // SHIFTKEY down = scale dragger
  XKeyEvent *kep = (XKeyEvent *)(env->event);
  if(  kep->state & ShiftMask ){
    MESG("scaling");
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

  Glx::Vector mousePos(xf*projWidth/aspect,yf*projHeight,-nearDist);
  itsSphere->getCenter(sphereCenter);

  VAR2(sphereCenter,itsEye);

  switch( env->buttonpressed ){
    
    case glx::LEFT:
      {
	MESG("glx::LEFT");
	if( ! itsEnableRotationFlag )
	  break;
	
	// map so that the glyph's location on the screen is
	// the center of the mouse movements, otherwise
	// all mouse movements would be relative to the 
	// center of the window instead of the center of the 
	// glyph which is not intuitive. Also, scale the
	xf -= itsSphereProjection[0];
	yf -= itsSphereProjection[1];
	
	// from the mouse movement between the last time and
	// this one, determine the axis of rotation and the amount
	// of rotation implied.
	float a[3],radians;
	double saveX=itsCurInv[12],saveY=itsCurInv[13],saveZ=itsCurInv[14];
	Glx::trackball(deltaQuat,startX,startY,xf,yf);
	deltaQuat.get(radians,a);
	Glx::Vector screenVec(a);
	itsCurInv[12]=itsCurInv[13]=itsCurInv[14]=0.0;//no translations!!! 
	Glx::xformVec(screenVec,itsCurInv,screenVec);
	itsCurInv[12]=saveX;itsCurInv[13]=saveY;itsCurInv[14]=saveZ;
	screenVec.normalize();
	deltaQuat.set(radians,screenVec);
	deltaQuat.normalize();
	// world: cur = quat * cur
	itsSphere->setOrientation(itsSaveQuat * deltaQuat);
	callDrag();
      }
      break;

    case glx::MIDDLE: // Z Translation

      MESG("glx::MIDDLE");

      eyeToSphereVec = sphereCenter - itsEye;
      if( yf != lastY ){
	deltaY = yf - lastY;
	VAR3(lastY,yf,deltaY);
	itsZtrans = deltaY;
	deltaCenter = eyeToSphereVec * itsZtrans;
	sphereCenter += deltaCenter;
	itsSphere->setCenter(sphereCenter);
	itsPos = sphereCenter;
      } 
      VAR(sphereCenter);
      callDrag();
      break;

    case glx::RIGHT: // Screen XY translation
      MESG("glx::RIGHT");

      // calculate the vector that goes from the eye through
      // the point on the projection plane where the mouse is
      Glx::xformVec(mousePos,itsCurInv,mousePos);
      mouseProjectionVector = mousePos - itsEye;

      // the View vector has been scaled so that it's magnitude
      // represents the distance from the eye to the plane 
      // that is parallel to the projection plane and contains
      // the sphere center
      factor = itsViewVector.magnitude()/nearDist;

      sphereCenter = itsEye + mouseProjectionVector*factor;
      itsSphere->setCenter(sphereCenter);
      itsPos = sphereCenter;

      callDrag();
      break;
    default:
      break;
  }
  lastY = yf;
}

void 
Glx::SphereDragger::viewHasChanged(glx* env)
{  
  FANCYMESG("Glx::SphereDragger::viewHasChanged(glx*)");
  double viewMatrix[16],projMatrix[16];
  if( ! env->makeCurrent() )
    return;
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  env->applyProjections();
  glGetDoublev(GL_MODELVIEW_MATRIX,(double *)viewMatrix);
  glGetDoublev(GL_PROJECTION_MATRIX,(double *)projMatrix);
  glPopMatrix();

  // combine the view and projection matrices
  Glx::mult4x4( viewMatrix, projMatrix, itsVP );

  // get the inverse
  Glx::inv4x4(viewMatrix, itsCurInv);
  viewHasChanged(env,itsVP,itsCurInv);
}

void 
Glx::SphereDragger::viewHasChanged(glx* env,
				   const double viewAndProjMatrix[16],
				   const double invMatrix[16])
{
  FANCYMESG("Glx::SphereDragger::viewHasChanged(glx*,double*,double*)");
  Glx::Vector proj, cntr, sphereVec,viewVector;
  double aspect      = env->aspect();
  double nearDist    = env->proj(glx::NEAR);
  double projHeight  = env->proj(glx::PROJH);
  double eyeDistToProjPlane = nearDist;
  double eyeDistToSphere;
  double factor;

  Glx::Vector eye;
  Glx::Vector view(0,0,-1);
  Glx::xformVec(eye,invMatrix,itsEye);
  Glx::xformVec(view,invMatrix,itsViewVector);

  // get the center of this sphere
  itsSphere->getCenter(cntr);
  itsSphere->getOrientation(itsSaveQuat);

  // get the vector from the eye through 
  // the center of this sphere
  sphereVec = cntr - itsEye;
  eyeDistToSphere = sphereVec.magnitude();

  if( eyeDistToSphere != 0.0 ){
    double r = getScale();

    /////////////////////////////////////////////////////////////
    // calculate the distance away from the projected point
    // that the mouse can be without intersecting this sphere
    // use similar triangles. 
    //
    // radius of projection   dist to proj
    // -------------------- = --------------
    // radius of sphere       dist to sphere
    //
    //                        (radius of sphere) * (dist to proj)
    // radius of projection = -----------------------------------
    //                                    dist to sphere
    //
    /////////////////////////////////////////////////////////////
    
    itsSphereProjectionRadius = (r * eyeDistToProjPlane)/eyeDistToSphere;
    itsSphereProjectionRadius /= projHeight;

    Glx::xformVec(cntr,viewAndProjMatrix,itsSphereProjection);
    itsSphereEyeDistance = eyeDistToSphere;
    
    // now we want the vector that goes through the center of the 
    // projection and extends to the Z plane of the sphere  

    // now we want to project the sphere vector onto this vector
    // to get a vector the proper length, we are essentially scaling it
    factor = Glx::dot(itsViewVector,sphereVec) / 
      Glx::dot(itsViewVector,itsViewVector);
    VAR(factor);

    itsViewVector *= factor;
  }
}

void 
Glx::SphereDragger::handleMouseUp(glx* env,void*)
{
  FANCYMESG("Glx::SphereDragger::handleMouseUp");
  needsCoord=true;
  viewHasChanged(env,itsVP,itsCurInv);
  doneWithDrag();
}

void 
Glx::SphereDragger::draw(glx* env,void* user)
{
  char message[80];

  if( itsVisibleFlag == 0)     
    return;

  if(needsInit){
    viewHasChanged(env);
    needsInit=false;
  }

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  if( itsCurDragHandle == SPHERE ){
    glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);    
    glEnable(GL_COLOR_MATERIAL);
    glColor3f(1.0,0.0,0.0);
    itsSphere->draw(env,user);
    glDisable(GL_COLOR_MATERIAL);
    glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,defaultMaterial);
    if( itsEnableRotationFlag ){
      sprintf(message,"Leftmouse=rotate");
      env->showMessage(message);
    }
    sprintf(message,"middlemouse=push/pull");
    env->showMessage(message);
    sprintf(message,"rightmouse=screen translate");
    env->showMessage(message);
    sprintf(message,"shift-mouse=scale");
    env->showMessage(message);
  } else {
    glMaterialfv(GL_FRONT,GL_DIFFUSE,defaultMaterial);
    itsSphere->draw(env,user);
  }

  glPopMatrix();
}

int  
Glx::SphereDragger::idlePick(glx* env,int x,int y,void*)
{
  FANCYMESG("Glx::SphereDragger::idlePick");
  int candidate = Glx::Draggable::UNSELECTED;
  double winx,winy;
  double dx,dy,dist;
  double fx,fy,r;

  VAR(itsSphereProjection);

  // Sphere is behind the projection plane, do not select
  if( itsSphereProjection[2] > 1.0 ){
    itsCurDragHandle = UNSELECTED;
    itsSelectionDist = MAXFLOAT;
    return UNSELECTED;
  }

  env->pixelToWinCoords(x,y,&winx,&winy);

  fx = itsSphereProjection[0];
  fy = itsSphereProjection[1];
  r = itsSphereProjectionRadius;
  
  dx = winx - fx;
  dy = winy - fy;
  dist = sqrt(dx*dx + dy*dy);
  VAR3(winx,winy,r);
  VAR3(dx,dy,dist);

  candidate = ( dist < r ) ? (int)SPHERE : (int)UNSELECTED;

  if( candidate == SPHERE )
    itsSelectionDist = itsSphereEyeDistance;
  else
    unselect();
  return candidate;
}



