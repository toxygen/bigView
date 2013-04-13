// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Class Glx::CardDragger
///////////////////////////////////////////////////////////////////////////////

#include <Draggers/Card.h>

//#define DEBUG 1
#include "debug.h"
using namespace std;

#define PICK_DIST 0.01
static float M_SQRT1_3=0.57735026919f; // 1/sqrt(3)
static float SELECTION_ANGLE=0.17364817767f; // Radians(10 deg)
static const int NUM_AXES = 27;

static Glx::Vector canonicalAxes[NUM_AXES] = {
  // special, used when normal not close to anything
  // i.e. nothing gets drawn
  Glx::Vector(0,0,0), 

  Glx::Vector(1,0,0),
  Glx::Vector(0,1,0),
  Glx::Vector(0,0,1),

  Glx::Vector(-1,0,0),
  Glx::Vector(0,-1,0),
  Glx::Vector(0,0,-1),

  // X PLANE
  Glx::Vector(0, M_SQRT1_2, M_SQRT1_2), //  Y  Z
  Glx::Vector(0,-M_SQRT1_2, M_SQRT1_2), // -Y  Z
  Glx::Vector(0, M_SQRT1_2,-M_SQRT1_2), //  Y -Z
  Glx::Vector(0,-M_SQRT1_2,-M_SQRT1_2), // -Y -Z

  // Y PLANE
  Glx::Vector( M_SQRT1_2,0, M_SQRT1_2), //  X  Z
  Glx::Vector(-M_SQRT1_2,0, M_SQRT1_2), // -X  Z
  Glx::Vector( M_SQRT1_2,0,-M_SQRT1_2), //  X -Z
  Glx::Vector(-M_SQRT1_2,0,-M_SQRT1_2), // -X -Z

  // Z PLANE
  Glx::Vector( M_SQRT1_2, M_SQRT1_2, 0), //  X  Y
  Glx::Vector(-M_SQRT1_2, M_SQRT1_2, 0), // -X  Y
  Glx::Vector( M_SQRT1_2,-M_SQRT1_2, 0), //  X -Y
  Glx::Vector(-M_SQRT1_2,-M_SQRT1_2, 0), // -X -Y

  Glx::Vector(  M_SQRT1_3,  M_SQRT1_3,  M_SQRT1_3), //  X  Y  Z
  Glx::Vector(  M_SQRT1_3,  M_SQRT1_3, -M_SQRT1_3), //  X  Y -Z
  Glx::Vector( -M_SQRT1_3, -M_SQRT1_3,  M_SQRT1_3), // -X -Y  Z
  Glx::Vector( -M_SQRT1_3, -M_SQRT1_3, -M_SQRT1_3), // -X -Y -Z

  Glx::Vector(  M_SQRT1_3, -M_SQRT1_3,  M_SQRT1_3), //  X -Y  Z
  Glx::Vector(  M_SQRT1_3, -M_SQRT1_3, -M_SQRT1_3), //  X -Y -Z
  Glx::Vector( -M_SQRT1_3,  M_SQRT1_3,  M_SQRT1_3), // -X  Y  Z
  Glx::Vector( -M_SQRT1_3,  M_SQRT1_3, -M_SQRT1_3)  // -X  Y -Z
};

Glx::CardDragger::CardDragger(glx* env) : 
  Draggable3D(env),
  needsInit(true),
  lastX(0),lastY(0),
  itsWidth(1.0),itsHeight(1.0),
  iHat(0,1,0),
  jHat(0,0,1),
  needsCoord(true),
  itsHandleScale(0.2)
{
  itsSVDragger = new SVDragger(env,false);
}

Glx::CardDragger::~CardDragger(void)
{ 
  delete itsSVDragger;
}

void 
Glx::CardDragger::handleMouseUp(glx* env,void* user)
{
  FANCYMESG("Glx::CardDragger::handleMouseUp");

  float snapMag = 
    snapPlane.a*snapPlane.a + 
    snapPlane.b*snapPlane.b +
    snapPlane.c*snapPlane.c;
  
  if( snapMag > 1.0e-6 ){
    double mat[16];
    Glx::Vector N;
    Glx::Vector xVec(1,0,0);
    Glx::Vector nVec(snapPlane.a,snapPlane.b,snapPlane.c);
    Glx::Quat quat(xVec,nVec);
    itsSVDragger->setOrientation(quat);
    itsSVDragger->getVector(N);

    quat.buildMatrix(mat);
    // the 2nd col of m is the unit Y vector in the transformed space 
    // the 3rd col of m is the unit Z vector in the transformed space
    iHat.set(mat[1*4+0],mat[1*4+1],mat[1*4+2]);
    jHat.set(mat[2*4+0],mat[2*4+1],mat[2*4+2]);

    VAR(nVec);
    VAR(quat);
    VAR(N);
    VAR(mat);

  } else {
    itsSVDragger->itsSphere->handleMouseUp(env,user);
    itsSVDragger->itsVector->handleMouseUp(env,user);
  }
  updateProj(env,itsSVDragger->itsSphere->getVP());

  doneWithDrag();
}

void 
Glx::CardDragger::setPosition(Glx::Vector& p, bool call)
{
  itsSVDragger->setPosition(p);
  Glx::Draggable::setPosition(p);
  viewHasChanged(itsEnv);
  callFuncs(itsUpdateFuncs);
  if( call ) callFuncs(itsDragFuncs);
}
void 
Glx::CardDragger::setPosition(float p[3], bool call)
{
  itsSVDragger->setPosition(p);
  Glx::Draggable::setPosition(p);
  viewHasChanged(itsEnv);
  callFuncs(itsUpdateFuncs);
  if( call ) callFuncs(itsDragFuncs);
}
void 
Glx::CardDragger::setPosition(float p0, float p1, float p2, bool call)
{
  itsSVDragger->setPosition(p0,p1,p2);
  Glx::Draggable::setPosition(p0,p1,p2);
  viewHasChanged(itsEnv);
  callFuncs(itsUpdateFuncs);
  if( call ) callFuncs(itsDragFuncs);
}
void 
Glx::CardDragger::setScale(float s)
{
  itsSVDragger->setScale(s);
  Glx::Draggable::setScale(s);
  callFuncs(itsUpdateFuncs);
}
void 
Glx::CardDragger::getOrientation(Glx::Quat& orientationQuat) const
{
  itsSVDragger->getOrientation(orientationQuat);
}
void 
Glx::CardDragger::setOrientation(const Glx::Quat& orientationQuat, bool call)
{
  Glx::Quat quat;
  double m[16];
  itsSVDragger->setOrientation(orientationQuat);
  itsSVDragger->getOrientation(quat);
  quat.buildMatrix(m);
  iHat.set(m[1*4+0],m[1*4+1],m[1*4+2]);
  jHat.set(m[2*4+0],m[2*4+1],m[2*4+2]);
  viewHasChanged(itsEnv);
  callFuncs(itsUpdateFuncs);
  if( call ) callFuncs(itsDragFuncs);
}
void  
Glx::CardDragger::getVector(Glx::Vector& v)
{
  itsSVDragger->getVector(v);
}
void 
Glx::CardDragger::setVector(Glx::Vector& v, bool call)
{
  Glx::Quat quat;
  double m[16];
  itsSVDragger->setVector(v);
  itsSVDragger->getOrientation(quat);
  quat.buildMatrix(m);
  iHat.set(m[1*4+0],m[1*4+1],m[1*4+2]);
  jHat.set(m[2*4+0],m[2*4+1],m[2*4+2]);
  viewHasChanged(itsEnv);
  callFuncs(itsUpdateFuncs);
  if(call) callFuncs(itsDragFuncs);
}
void 
Glx::CardDragger::setVector(float v[3], bool call)
{
  Glx::Quat quat;
  double m[16];
  itsSVDragger->setVector(v);
  itsSVDragger->getOrientation(quat);
  quat.buildMatrix(m);
  iHat.set(m[1*4+0],m[1*4+1],m[1*4+2]);
  jHat.set(m[2*4+0],m[2*4+1],m[2*4+2]);
  viewHasChanged(itsEnv);
  callFuncs(itsUpdateFuncs);
  if(call) callFuncs(itsDragFuncs);
}
void 
Glx::CardDragger::setVector(float v0, float v1, float v2, bool call)
{
  Glx::Quat quat;
  double m[16];
  itsSVDragger->setVector(v0,v1,v2);
  itsSVDragger->getOrientation(quat);
  quat.buildMatrix(m);
  iHat.set(m[1*4+0],m[1*4+1],m[1*4+2]);
  jHat.set(m[2*4+0],m[2*4+1],m[2*4+2]);
  viewHasChanged(itsEnv);
  callFuncs(itsUpdateFuncs);
  if(call) callFuncs(itsDragFuncs);
}

void 
Glx::CardDragger::handleDrag(glx* env, int x, int y, void* user)
{
  double xf,yf;
  double projWidth   = env->proj(glx::PROJW);
  double projHeight  = env->proj(glx::PROJH);
  double nearDist    = env->proj(glx::NEAR);
  double aspect      = env->aspect();
  env->pixelToWinCoords(x,y,&xf,&yf);
  xf *= aspect;

  FANCYMESG("Glx::CardDragger::handleDrag");

  Glx::Vector mousePos(xf*projWidth/aspect,yf*projHeight,-nearDist);

  switch( env->buttonpressed ){    
    case glx::MIDDLE: // normal handle
      itsCurDragHandle = Glx::CardDragger::NORMAL;
      itsSVDragger->select(Glx::SVDragger::NORMAL); 
      break;
    case glx::RIGHT: 
      // funnel events to EnvCardDragger::PLANE for right button
      // this makes sure constrain movement to the plane
      itsSVDragger->unselect();
      itsCurDragHandle = Glx::CardDragger::PLANE;
      break;
  }

  VAR(itsCurDragHandle);

  switch( itsCurDragHandle ){
    case Glx::CardDragger::PLANE:
      {
	Glx::Vector planePoint,planeNormal,eye;
	double* inv;
	itsSVDragger->itsSphere->getEye(eye);
	itsSVDragger->itsSphere->getInv(inv);
	Glx::xformVec(mousePos,inv,mousePos);
	Glx::Vector mouseProjectionVector = mousePos - eye;
	
	itsSVDragger->getPosition(planePoint);
	itsSVDragger->getVector(planeNormal);
	if( dot(planeNormal,mouseProjectionVector) == 0.0 )
	  return;
	float d = -(planeNormal[0]*planePoint[0] +
		    planeNormal[1]*planePoint[1] +
		    planeNormal[2]*planePoint[2]);
	float numer = d + dot(planeNormal,eye);
	float denom = dot(planeNormal,mouseProjectionVector);
	float t = -numer/denom;
	Glx::Vector newCorner = eye + mouseProjectionVector*t;
	itsSVDragger->setPosition(newCorner); 
	callDrag();
      }
      break;
    case Glx::CardDragger::SPHERE:
      {
	Glx::Quat quat;
	double m[16];
	itsSVDragger->select(Glx::SVDragger::SPHERE); 
	itsSVDragger->itsSphere->handleDrag(env,x,y,user);
	itsSVDragger->getOrientation(quat);
	quat.buildMatrix(m);
	// the 2nd col of m is the unit Y vector in the transformed space 
	// the 3rd col of m is the unit Z vector in the transformed space
	iHat.set(m[1*4+0],m[1*4+1],m[1*4+2]);
	jHat.set(m[2*4+0],m[2*4+1],m[2*4+2]);
      }
    break;
    case Glx::CardDragger::NORMAL: 
      {
	Glx::Vector pos;
	itsSVDragger->select(Glx::SVDragger::NORMAL); 
	itsSVDragger->itsVector->handleDrag(env,x,y,user);
	itsSVDragger->getPosition(pos);
	Glx::Draggable::setPosition(pos);
      }
    break;
    case Glx::CardDragger::CORNER:
      {
	double* inv;
	Glx::Vector planePoint,planeNormal,newCorner,v,eye;
	itsSVDragger->itsSphere->getEye(eye);
	itsSVDragger->itsSphere->getInv(inv);
	Glx::xformVec(mousePos,inv,mousePos);
	Glx::Vector mouseProjectionVector = mousePos - eye;
	itsSVDragger->getPosition(planePoint);
	itsSVDragger->getVector(planeNormal);
	if( dot(planeNormal,mouseProjectionVector) == 0.0 )
	  return;
	// find the const part of the formula:
	//
	// ax + by + cz + d = 0 => d = -(ax + by + cz)
	// 
	double d = -(planeNormal[0]*planePoint[0] +
		     planeNormal[1]*planePoint[1] +
		     planeNormal[2]*planePoint[2]);
	double denom = dot(planeNormal,mouseProjectionVector);
	double numer = d + dot(planeNormal,eye);
	double t = -numer/denom;
	newCorner = eye + mouseProjectionVector*t;
	v = newCorner-planePoint;
	itsWidth = dot(v,iHat);
	itsHeight = dot(v,jHat);
	double m = fabs(itsWidth)>fabs(itsHeight) ? 
	  fabs(itsWidth) : fabs(itsHeight);
	itsHandleScale = 0.2 * m;
      }
    break;
  }

  // set the snap plane if needed
  Glx::Vector canon; // by default canon=<0,0,0> i.e. none selected
  Glx::Vector planePoint,planeNormal;
  itsSVDragger->getVector(planeNormal);
  itsSVDragger->getPosition(planePoint);
  double mag = planeNormal.magnitude();
  for(int i=1; i<NUM_AXES ; i++){
    double num = dot( planeNormal, canonicalAxes[i] );
    double theta = acos(num/mag);
    if( theta < SELECTION_ANGLE ){
      //cout << "theta < SELECTION_ANGLE" <<endl;
      //cout << theta << " < " << SELECTION_ANGLE <<endl;
      //cout << "canonicalAxes["<<i<<"] selected:"<<canonicalAxes[i]<<endl;
      canon=canonicalAxes[i];
    }
  }

  snapPlane.a = canon[0];
  snapPlane.b = canon[1];
  snapPlane.c = canon[2];
  snapPlane.x = planePoint[0];
  snapPlane.y = planePoint[1];
  snapPlane.z = planePoint[2];

  callDrag();
}

void 
Glx::CardDragger::viewHasChanged(glx* env)
{
  FANCYMESG("Glx::CardDragger::viewHasChanged(glx*)");
  itsSVDragger->itsSphere->viewHasChanged(env);
  itsSVDragger->itsVector->viewHasChanged(env);
  updateProj(env,itsSVDragger->itsSphere->getVP());
}

void 
Glx::CardDragger::viewHasChanged(glx* env, 
				 const double viewAndProjMatrix[16], 
				 const double invMatrix[16])
{
  FANCYMESG("Glx::CardDragger::viewHasChanged(glx*,double*,double*)");
  updateProj(env,viewAndProjMatrix);
}

void 
Glx::CardDragger::updateProj(glx* env, const double vp[16])
{
  Glx::Vector cntr,corner,tmp;
  itsSVDragger->getPosition(cntr);
  double aspect = env->aspect();

  FANCYMESG("Glx::CardDragger::updateProj(glx*,double*)");
  VAR(vp);

  corner = cntr + iHat*itsWidth + jHat*itsHeight;

  for(int i = 0 ; i < 2 ; i++ ){
    switch( i ) {
      case 0:
	tmp = corner - iHat*itsHandleScale;
	Glx::xformVec(tmp,vp,v1[i]);
	tmp = corner + iHat*itsHandleScale;
	Glx::xformVec(tmp,vp,v2[i]);
	break;
      case 1:
	tmp = corner - jHat*itsHandleScale;
	Glx::xformVec(tmp,vp,v1[i]);
	tmp = corner + jHat*itsHandleScale;
	Glx::xformVec(tmp,vp,v2[i]);
	break;	
    }

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
    
    n[i][0] = v2[i][1]-v1[i][1];
    n[i][1] = v1[i][0]-v2[i][0];
    
    // normalize N
    double squaredMag = n[i][0]*n[i][0] + n[i][1]*n[i][1];
    double mag = sqrt(squaredMag);
    if( mag != 0.0 ){
      n[i][0] /= mag;
      n[i][1] /= mag;
    }
    
    ////////////////////////////////////////////
    // c = -N . V1
    ////////////////////////////////////////////
    c[i] = -( n[i][0]*v1[i][0] + n[i][1]*v1[i][1] );
  }

  // now update the corner projection points
  for(int i = 0 ; i < 4 ; i++ ){
    Glx::Vector tmp;
    getCorner(i, tmp);
    Glx::xformVec(tmp,vp,cProj[i]);
  }
}

void 
Glx::CardDragger::draw(glx* env,void *user)
{
  if( itsVisibleFlag == 0)     
    return;

  if(needsInit){
    itsSVDragger->viewHasChanged(env);
    viewHasChanged(env);
    needsInit=false;
  }

  Glx::Vector cntr;
  itsSVDragger->getPosition(cntr);
  //itsSVDragger->itsSphere->draw(env,user);
  //itsSVDragger->itsVector->draw(env,user);

  Glx::Vector corner = cntr + iHat*itsWidth + jHat*itsHeight;

  switch( itsCurDragHandle ){
    case Glx::CardDragger::SPHERE:
    case Glx::CardDragger::NORMAL:
    case Glx::CardDragger::CORNER:
      break;
  }

  //////////////////////////
  // draw the corner handle
  //////////////////////////
  glLineWidth(1);
  if( itsCurDragHandle==Glx::CardDragger::CORNER ){
    glLineWidth(4);    
    glColor3f(1,0,0);
  } else
    glColor3f(0,1,0);
  glBegin(GL_LINES);
  glVertex3fv(corner - iHat*itsHandleScale);
  glVertex3fv(corner + iHat*itsHandleScale);
  glVertex3fv(corner - jHat*itsHandleScale);
  glVertex3fv(corner + jHat*itsHandleScale);
  glEnd();

  /////////////////
  // draw the card
  /////////////////

  glLineWidth(1);
  if( itsCurDragHandle==Glx::CardDragger::SPHERE ||
      itsCurDragHandle==Glx::CardDragger::NORMAL )
  {
    glLineWidth(4);    
    glColor3f(1,0,0);
  } else
    glColor3f(0,1,0);
  
  glBegin(GL_LINES);
  Glx::Vector i = iHat * itsWidth;
  Glx::Vector j = jHat * itsHeight;

  glVertex3fv(cntr + i + j);
  glVertex3fv(cntr + i - j);

  glVertex3fv(cntr + i - j);
  glVertex3fv(cntr - i - j);

  glVertex3fv(cntr - i - j);
  glVertex3fv(cntr - i + j);

  glVertex3fv(cntr - i + j);
  glVertex3fv(cntr + i + j);
  glEnd();

  // draw the plane normal

  glColor3f(1,0,0);

  Glx::Vector c[3],l,r,n;
  double lm,rm,max,nm;
  getCorner(0, c[0]);
  getCorner(1, c[1]);
  getCorner(2, c[2]);
  l = c[1]-c[0];
  r = c[2]-c[0];
  lm = l.magnitude();
  rm = r.magnitude();
  max = lm>rm ? lm : rm;
  n.set(snapPlane.a,snapPlane.b,snapPlane.c);
  n.normalize();
  n *= max;
  glBegin(GL_LINES);
  glVertex3f(snapPlane.x,snapPlane.y,snapPlane.z);
  glVertex3f(snapPlane.x+n[0],
	     snapPlane.y+n[1],
	     snapPlane.z+n[2]);
  glEnd();

  float snapMag = 
    snapPlane.a*snapPlane.a + 
    snapPlane.b*snapPlane.b +
    snapPlane.c*snapPlane.c;
  
  if( snapMag > 1.0e-6 ){
    Glx::Vector planeNormal;
    itsSVDragger->getVector(planeNormal);
    Glx::Vector diff(planeNormal[0]-snapPlane.a,
		     planeNormal[1]-snapPlane.b,
		     planeNormal[2]-snapPlane.c);    
    if( diff.magnitude() > 1.0e-6 ){
      char message[128];
      env->setMessageColor(1,1,1);
      sprintf(message,"Snap to vector <%2.2f,%2.2f,%2.2f>",
	      snapPlane.a,snapPlane.b,snapPlane.c);
      env->showMessage(message);
    }
  }

  GLboolean saveBlendEnabled;
  GLint saveBlendSrc,saveBlendDst;
  int lw;

  glGetIntegerv(GL_LINE_WIDTH,&lw);
  glGetBooleanv(GL_BLEND,&saveBlendEnabled);
  glGetIntegerv(GL_BLEND_SRC,&saveBlendSrc);
  glGetIntegerv(GL_BLEND_DST,&saveBlendDst);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColor4f(1,1,1,0.05);
  glBegin(GL_QUADS);
  glVertex3fv(cntr + i + j);
  glVertex3fv(cntr + i - j);
  glVertex3fv(cntr + i - j);
  glVertex3fv(cntr - i - j);
  glVertex3fv(cntr - i - j);
  glVertex3fv(cntr - i + j);
  glVertex3fv(cntr - i + j);
  glVertex3fv(cntr + i + j);
  glEnd();
  
  if( saveBlendEnabled )
    glEnable(GL_BLEND);
  else
    glDisable(GL_BLEND);
  glBlendFunc(saveBlendSrc,saveBlendDst);
  glLineWidth(lw);

#if DEBUG
  // draw projection points
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glPointSize(4);
  glBegin(GL_POINTS);
  glVertex3fv(cProj[0]);
  glVertex3fv(cProj[1]);
  glVertex3fv(cProj[2]);
  glVertex3fv(cProj[3]);
  glVertex3fv(v1[0]);
  glVertex3fv(v1[1]);
  glVertex3fv(v2[0]);
  glVertex3fv(v2[1]);
  glEnd();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
#endif
}

int 
Glx::CardDragger::sign(double f)
{
  return (f < 0.0f) ? -1 : 1;
}

int  
Glx::CardDragger::idlePick(glx* env,int x,int y,void* user)
{
  Glx::Vector corner,point1,point2;
  Glx::Vector vecIntersection,eyeToVec,eye;
  double worldDist=MAXFLOAT,xPos,yPos;
  int candidate = UNSELECTED;
  double t;

  FANCYMESG("Glx::CardDragger::idlePick()");
  env->pixelToWinCoords(x,y,&xPos,&yPos);
  itsSVDragger->itsSphere->getEye(eye);

  // see if the corner is selected
  for( int i=0 ; i < 2 ; i++ ){
    
    double dist = calcDist(i,xPos,yPos,&t);

    if( dist < PICK_DIST ){
      getCorner(0,corner);
      candidate = Glx::CardDragger::CORNER;
      switch( i ){
	case 0:
	  point1 = corner - iHat*0.5;
	  point2 = corner + iHat*0.5;
	  break;
	case 1:
	  point1 = corner - jHat*0.5;
	  point2 = corner + jHat*0.5;
	  break;
      }

      vecIntersection = point1 + (point2-point1)*t;
      eyeToVec = vecIntersection - eye;
      worldDist = eyeToVec.magnitude();
    }
  }

  if( candidate != UNSELECTED ){
    itsSelectionDist = worldDist;
    return candidate;
  }

  MESG("still no candidate");

  // ok, now just see if the mouse is in the projection of the card
  float s1,s2,s3;
  Glx::Vector leg1,leg2,leg3;

  // divide the card into two triangles,
  // test if the mouse is in the projection of one of the triangles

  // 1st triangle
  leg1 = cProj[1] - cProj[0];
  s1 = (xPos-cProj[0][0])*leg1[1] - (yPos-cProj[0][1])*leg1[0];

  leg2 = cProj[2] - cProj[1];
  s2 = (xPos-cProj[1][0])*leg2[1] - (yPos-cProj[1][1])*leg2[0];

  leg3 = cProj[0] - cProj[2];
  s3 = (xPos-cProj[2][0])*leg3[1] - (yPos-cProj[2][1])*leg3[0];

  // mouse point is in the triangle if all the signs are the same
  if( sign(s1) == sign(s2) && sign(s2)==sign(s3) ){
    MESG("tri #1");
    itsSelectionDist=0.0f;
    return Glx::CardDragger::SPHERE; 
  }
  
  // 2nd triangle
  leg1 = cProj[2] - cProj[0];
  s1 = (xPos-cProj[0][0])*leg1[1] - (yPos-cProj[0][1])*leg1[0];

  leg2 = cProj[3] - cProj[2];
  s2 = (xPos-cProj[2][0])*leg2[1] - (yPos-cProj[2][1])*leg2[0];

  leg3 = cProj[0] - cProj[3];
  s3 = (xPos-cProj[3][0])*leg3[1] - (yPos-cProj[3][1])*leg3[0];

  // mouse point is in the triangle if all the signs are the same
  if( sign(s1) == sign(s2) && sign(s2)==sign(s3) ){
    MESG("tri #2");
    itsSelectionDist=0.0f;
    return Glx::CardDragger::SPHERE; 
  }
  MESGVAR("STILL NO CANDIDATE?!?",candidate);
  if( candidate == UNSELECTED ){
    unselect();
    itsSelectionDist = MAXFLOAT;
  } else
    itsSelectionDist = worldDist;
  return candidate;
}

void
Glx::CardDragger::getCorner(int index, Glx::Vector& res) const
{  
  Glx::Quat quat;
  Glx::Vector base,cntr;
  double mat[16];

  itsSVDragger->getPosition(cntr);
  itsSVDragger->getOrientation(quat);
  quat.buildMatrix(mat);

  switch( index ){
    case 0:
      base.set(0, itsWidth, itsHeight);
      break;
    case 1:
      base.set(0,-itsWidth, itsHeight);
      break;
    case 2:
      base.set(0,-itsWidth,-itsHeight);
      break;
    case 3:
      base.set(0, itsWidth,-itsHeight);
      break;
  }
  Glx::xformVec(base,mat,res);
  res += cntr;
}

double 
Glx::CardDragger::calcDist(int i, double x, double y, double* t)
{
#ifdef DEBUG_DRAGGER
  cout<<"=== EnvCardDragger::calcDist() ==="<<endl;
#endif
  float q = c[i] + (n[i][0]*x + n[i][1]*y);
  float denom, dist;
  
  Q[i][0] = x - q*n[i][0];
  Q[i][1] = y - q*n[i][1];
  
  // solve for t, i.e. the normalized position of
  // Q on the line between v1->v2. 
  // if 0<= t <= 1, then the Q is actaully on the line segment
  // otherwise, Q is not on the line segment, and should not
  // be considered for picking
  
  denom = v2[i][0]-v1[i][0];
  
  // is X not availible for calculation of t?
  if( denom == 0.0 ){
    
    // ok, try to use Y
    denom = v2[i][1]-v1[i][1];
    if( denom == 0.0 ){
      
      // ok, do a point distance
      dist = sqrt( (x-v1[i][0])*(x-v1[i][0]) + (y-v1[i][1])*(y-v1[i][1]) );
      *t = 0.0; // hmmm???
      return dist;	
    }
    
    // ok, Y worked
    *t = (Q[i][1] - v1[i][1])/denom;
    
    // calc t, return MAXFLOAT if t is out of bounds
    if( *t < 0.0 || *t > 1.0 ){
      return MAXFLOAT;
    }
    // t is good, calc the distance between Q and the incoming point
    dist = sqrt( (x-Q[i][0])*(x-Q[i][0]) + (y-Q[i][1])*(y-Q[i][1]) );
    return dist;
  }
  
  // ok, X worked
  *t = (Q[i][0] - v1[i][0])/denom;
  
  // calc t, return MAXFLOAT if t is out of bounds
  if( *t < 0.0 || *t > 1.0 ){
    return MAXFLOAT;
  }
  
  // t is good, calc the distance between Q and the incoming point
  dist = sqrt( (x-Q[i][0])*(x-Q[i][0]) + (y-Q[i][1])*(y-Q[i][1]) );
  return dist;
}
