// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Class Glx::Crosshair3D
///////////////////////////////////////////////////////////////////////////////

#include <GL/gl.h>
#include <Draggers/Crosshair3D.h>
#include <GLX.h>
#include <values.h> // for MAXFLOAT
using namespace std;

//#define DEBUG 1
#include "debug.h"

Glx::Vector intersection; 
int CAN[3],TRI[3];

Glx::Crosshair3D::Crosshair3D(glx* env,bool registerDragger) : 
  Draggable3D(env,registerDragger),
  needsInit(true),
  itsNeedOffset(true),
  lastY(0)
{
}

Glx::Crosshair3D::~Crosshair3D(void)
{
}

void Glx::Crosshair3D::draw(glx*,void *)
{
  float d = itsScale;
  int lw;
  glGetIntegerv(GL_LINE_WIDTH,&lw);
  GLboolean saveBlendEnabled;
  GLint saveBlendSrc,saveBlendDst;

  glGetBooleanv(GL_BLEND,&saveBlendEnabled);
  glGetIntegerv(GL_BLEND_SRC,&saveBlendSrc);
  glGetIntegerv(GL_BLEND_DST,&saveBlendDst);

  glLineWidth(1);
  if( itsCurDragHandle==ZPLANE )
    glLineWidth(4);
  
  glColor3f(0,0,0.5);
  glBegin(GL_LINE_LOOP);
  glVertex3f(itsPos[0]-d,itsPos[1]-d,itsPos[2]);
  glVertex3f(itsPos[0]+d,itsPos[1]-d,itsPos[2]);
  glVertex3f(itsPos[0]+d,itsPos[1]+d,itsPos[2]);
  glVertex3f(itsPos[0]-d,itsPos[1]+d,itsPos[2]);
  glEnd();

  glLineWidth(1);
  if( itsCurDragHandle==YPLANE )
    glLineWidth(4);

  glColor3f(0,0.5,0);
  glBegin(GL_LINE_LOOP);
  glVertex3f(itsPos[0]-d,itsPos[1],itsPos[2]-d);
  glVertex3f(itsPos[0]+d,itsPos[1],itsPos[2]-d);
  glVertex3f(itsPos[0]+d,itsPos[1],itsPos[2]+d);
  glVertex3f(itsPos[0]-d,itsPos[1],itsPos[2]+d);
  glEnd();    

  glLineWidth(1);
  if( itsCurDragHandle==XPLANE )
    glLineWidth(4);

  glColor3f(0.5,0,0);
  glBegin(GL_LINE_LOOP);
  glVertex3f(itsPos[0],itsPos[1]-d,itsPos[2]-d);
  glVertex3f(itsPos[0],itsPos[1]+d,itsPos[2]-d);
  glVertex3f(itsPos[0],itsPos[1]+d,itsPos[2]+d);
  glVertex3f(itsPos[0],itsPos[1]-d,itsPos[2]+d);
  glEnd();

  if( itsCurDragHandle != UNSELECTED ){
    // show projection to planes
    glLineWidth(1);
    glBegin(GL_LINES);
    
    // X 
    glColor3f(0.3,0,0);
    glVertex3f(itsPos[0],itsPos[1],itsPos[2]);
    glVertex3f(0,        itsPos[1],itsPos[2]);
    glVertex3f(itsPos[0],itsPos[1],0);
    glVertex3f(0 ,       itsPos[1],0);
    glVertex3f(itsPos[0],0,itsPos[2]);
    glVertex3f(0,        0,itsPos[2]);
    glVertex3f(itsPos[0],0,0);
    glVertex3f(0,        0,0);

    /*
      glVertex3f(0,itsPos[1]+itsScale/10.,itsPos[2]);
      glVertex3f(0,itsPos[1]-itsScale/10.,itsPos[2]);
      glVertex3f(0,itsPos[1],itsPos[2]+itsScale/10.);
      glVertex3f(0,itsPos[1],itsPos[2]-itsScale/10.);
    */
    
    // Y
    glColor3f(0,0.3,0);
    glVertex3f(itsPos[0],itsPos[1],itsPos[2]);
    glVertex3f(itsPos[0],0,itsPos[2]);
    
    glVertex3f(0,itsPos[1],itsPos[2]);
    glVertex3f(0,0,itsPos[2]);
    
    glVertex3f(itsPos[0],itsPos[1],0);
    glVertex3f(itsPos[0],0,0);
    
    glVertex3f(0, itsPos[1],0);
    glVertex3f(0, 0,        0);
    
    /*
      glVertex3f(itsPos[0]+itsScale/10.,0,itsPos[2]);
      glVertex3f(itsPos[0]-itsScale/10.,0,itsPos[2]);
      glVertex3f(itsPos[0],0,itsPos[2]+itsScale/10.);
      glVertex3f(itsPos[0],0,itsPos[2]-itsScale/10.);
    */
    
    // Z
    glColor3f(0,0,0.3);
    glVertex3f(itsPos[0],itsPos[1],itsPos[2]);
    glVertex3f(itsPos[0],itsPos[1],0);
    
    glVertex3f(0,itsPos[1],itsPos[2]);
    glVertex3f(0,itsPos[1],0);
    
    glVertex3f(itsPos[0],0,itsPos[2]);
    glVertex3f(itsPos[0],0,0);

    
    glVertex3f(0, 0, itsPos[2]);
    glVertex3f(0, 0,        0);
    /*
      glVertex3f(itsPos[0]+itsScale/10.,itsPos[1],0);
      glVertex3f(itsPos[0]-itsScale/10.,itsPos[1],0);
      glVertex3f(itsPos[0],itsPos[1]+itsScale/10.,0);
      glVertex3f(itsPos[0],itsPos[1]-itsScale/10.,0);
    */
    glEnd();
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glColor4f(1,1,1,0.05);
  glBegin(GL_QUADS);
  glVertex3f(itsPos[0]-d,itsPos[1]-d,itsPos[2]);
  glVertex3f(itsPos[0]+d,itsPos[1]-d,itsPos[2]);
  glVertex3f(itsPos[0]+d,itsPos[1]+d,itsPos[2]);
  glVertex3f(itsPos[0]-d,itsPos[1]+d,itsPos[2]);
  glVertex3f(itsPos[0]-d,itsPos[1],itsPos[2]-d);
  glVertex3f(itsPos[0]+d,itsPos[1],itsPos[2]-d);
  glVertex3f(itsPos[0]+d,itsPos[1],itsPos[2]+d);
  glVertex3f(itsPos[0]-d,itsPos[1],itsPos[2]+d);
  glVertex3f(itsPos[0],itsPos[1]-d,itsPos[2]-d);
  glVertex3f(itsPos[0],itsPos[1]+d,itsPos[2]-d);
  glVertex3f(itsPos[0],itsPos[1]+d,itsPos[2]+d);
  glVertex3f(itsPos[0],itsPos[1]-d,itsPos[2]+d);
  glEnd();

  if( saveBlendEnabled )
    glEnable(GL_BLEND);
  else
    glDisable(GL_BLEND);
  glBlendFunc(saveBlendSrc,saveBlendDst);


  glLineWidth(lw);

#if DEBUG

  glPointSize(4);
  glColor3f(1,1,0);
  glBegin(GL_POINTS);
  glVertex3fv(intersection);
  glEnd();

  // draw projection points
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
 
  for(int i=0;i<3;++i){
    if( CAN[i] != Glx::Draggable::UNSELECTED ){
      Glx::Vector T[3];
      switch( TRI[i] ){
	case 1:
	  T[0]=cproj[i][0];
	  T[1]=cproj[i][1];
	  T[2]=cproj[i][2];
	  break;
	case 2:
	  T[0]=cproj[i][0];
	  T[1]=cproj[i][2];
	  T[2]=cproj[i][3];	
	  break;
      }
      glLineWidth(4);
      glColor3f(1,1,1);
      glBegin(GL_LINE_LOOP);
      glVertex3fv(T[0]);
      glVertex3fv(T[1]);
      glVertex3fv(T[2]);
      glEnd();
    }
    glLineWidth(1);
  }

  glColor3f(0.5,0.5,0.5);
  glBegin(GL_POINTS);
  for(int dim=0;dim<3;++dim){
    for(int index=0;index<4;++index){
      glVertex3fv(cproj[dim][index]);
    }
  }
  glEnd();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
#endif
}

void 
Glx::Crosshair3D::handleMouseUp(glx* env,void*)
{
  FANCYMESG("Glx::Crosshair3D::handleMouseUp");
  itsNeedOffset=true;
  viewHasChanged(env);
  doneWithDrag();
}

void 
Glx::Crosshair3D::handleDrag(glx* env,int x,int y,void*)
{
  FANCYMESG("Glx::Crosshair3D::handleDrag");
  VAR(itsCurDragHandle);
  double nearDist    = env->proj(glx::NEAR);
  double projWidth   = env->proj(glx::PROJW);
  double projHeight  = env->proj(glx::PROJH);
  double aspect      = env->aspect();
  double EPS         = 1.0e-6;
  
  double xf,yf;

  if( itsVisibleFlag == 0) 
    return;
  if( itsCurDragHandle == UNSELECTED )
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

  Glx::Vector eye,eyepos,projvec;
  Glx::Vector mousePos(xf*projWidth/aspect,yf*projHeight,-nearDist);
  Glx::Vector mousePosXformed;
  int N,U,V;

  switch( itsCurDragHandle ){
    case 0:N=0;U=1;V=2;break;
    case 1:N=1;U=0;V=2;break;
    case 2:N=2;U=0;V=1;break;
    default: break;
  }
  Glx::xformVec(eye,itsCurInverse,eyepos);
  Glx::xformVec(mousePos,itsCurInverse,mousePosXformed);
  projvec[0] = mousePosXformed[0] - eyepos[0];
  projvec[1] = mousePosXformed[1] - eyepos[1];
  projvec[2] = mousePosXformed[2] - eyepos[2];    
  double t = (itsPos[N] - eyepos[N])/projvec[N];

  if( itsNeedOffset ){
    Glx::Vector p;
    p[U] = eyepos[U] + projvec[U]*t;
    p[V] = eyepos[V] + projvec[V]*t;
    p[N] = itsPos[N];
    itsOffset = p - itsPos;
    itsNeedOffset=false;
  } else {
    itsPos[U] = eyepos[U] + projvec[U]*t - itsOffset[U];
    itsPos[V] = eyepos[V] + projvec[V]*t - itsOffset[V];
  }
  callDrag();
}

int 
Glx::Crosshair3D::sign(double f)
{
  return (f < 0.0f) ? -1 : 1;
}

int Glx::Crosshair3D::testPlane(glx* env, int dim, 
				Glx::Vector& eyepos,
				Glx::Vector& projvec,
				double xPos, double yPos, double& dist)
{
  float s1,s2,s3;
  int candidate = Glx::Draggable::UNSELECTED;
  Glx::Vector leg1,leg2,leg3;
  double EPS = 1.0e-6;

  dist = MAXFLOAT;

  // + 1     0
  // ^   pos 
  // | 2     3
  //   ----->+

  // 1st triangle
  leg1 = cproj[dim][1] - cproj[dim][0];
  s1 = (xPos-cproj[dim][0][0])*leg1[1] - (yPos-cproj[dim][0][1])*leg1[0];

  leg2 = cproj[dim][2] - cproj[dim][1];
  s2 = (xPos-cproj[dim][1][0])*leg2[1] - (yPos-cproj[dim][1][1])*leg2[0];

  leg3 = cproj[dim][0] - cproj[dim][2];
  s3 = (xPos-cproj[dim][2][0])*leg3[1] - (yPos-cproj[dim][2][1])*leg3[0];

  // mouse point is in the triangle if all the signs are the same
  if( sign(s1) == sign(s2) && sign(s2)==sign(s3) ){
    int N,U,V;
    TRI[dim]=1;
    switch( dim ){
      case 0:N=0;U=1;V=2;break;
      case 1:N=1;U=0;V=2;break;
      case 2:N=2;U=0;V=1;break;
      default: break;
    }
    if( fabs(projvec[N])<EPS ){
      _MESG("projvec==0");
      dist=0.;
      return dim;
    }
    double t = (itsPos[N] - eyepos[N])/projvec[N];
    intersection[U] = eyepos[U] + projvec[U]*t;
    intersection[V] = eyepos[V] + projvec[V]*t;
    intersection[N] = itsPos[N];
    Glx::Vector d = intersection-eyepos;
    dist = sqrt(d[0]*d[0] + d[1]*d[1] + d[2]*d[2]);
    return dim; 
  }
  
  // 2nd triangle
  leg1 = cproj[dim][2] - cproj[dim][0];
  s1 = (xPos-cproj[dim][0][0])*leg1[1] - (yPos-cproj[dim][0][1])*leg1[0];

  leg2 = cproj[dim][3] - cproj[dim][2];
  s2 = (xPos-cproj[dim][2][0])*leg2[1] - (yPos-cproj[dim][2][1])*leg2[0];

  leg3 = cproj[dim][0] - cproj[dim][3];
  s3 = (xPos-cproj[dim][3][0])*leg3[1] - (yPos-cproj[dim][3][1])*leg3[0];

  // mouse point is in the triangle if all the signs are the same
  if( sign(s1) == sign(s2) && sign(s2)==sign(s3) ){
    int N,U,V;
    TRI[dim]=2;
    switch( dim ){
      case 0:N=0;U=1;V=2;break;
      case 1:N=1;U=0;V=2;break;
      case 2:N=2;U=0;V=1;break;
      default: break;
    }
    if( fabs(projvec[N])<EPS ){
      _MESG("projvec==0");
      dist=0.;
      return dim;
    }
    double t = (itsPos[N] - eyepos[N])/projvec[N];
    intersection[U] = eyepos[U] + projvec[U]*t;
    intersection[V] = eyepos[V] + projvec[V]*t; 
    intersection[N] = itsPos[N];   
    Glx::Vector d = intersection-eyepos;
    dist = sqrt(d[0]*d[0] + d[1]*d[1] + d[2]*d[2]);
    return dim; 
  }
  return candidate;
}

int  
Glx::Crosshair3D::idlePick(glx* env,int x,int y,void*)
{
  int candidate = Glx::Draggable::UNSELECTED;
  double worldDist=MAXFLOAT,xPos,yPos;
  env->pixelToWinCoords(x,y,&xPos,&yPos);
  Glx::Vector eye;
  Glx::Vector mousePos(xPos*env->proj(glx::PROJW)/env->aspect(),
		       yPos*env->proj(glx::PROJH),
		       -env->proj(glx::NEAR));

  Glx::xformVec(eye,itsCurInverse,eye);
  Glx::xformVec(mousePos,itsCurInverse,mousePos); 
  Glx::Vector projvec = mousePos - eye;

  for(int dim=0;dim<3 ;++dim ){
    double dist;
    int can = testPlane(env,dim,eye,projvec,xPos,yPos,dist);
    CAN[dim] = can;    
    VAR3(can,dist,TRI);
    if( dist<worldDist ){
      worldDist=dist;
      candidate=can;	
    }
  }
  MESGVAR("idlePick",candidate);
  if( candidate == UNSELECTED )
    unselect();
#ifdef DEBUG
  else
    env->wakeup();
  
#endif
  itsSelectionDist=worldDist;
  return candidate;
}

void 
Glx::Crosshair3D::viewHasChanged(glx* env)
{
  FANCYMESG("Glx::Crosshair3D::viewHasChanged(glx*)");
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
}

void 
Glx::Crosshair3D::viewHasChanged(glx* env,
				 const double viewAndProj[16], 
				 const double inv[16])
{
  FANCYMESG("viewHasChanged");
  updateProj(env,viewAndProj);
}

void 
Glx::Crosshair3D::updateProj(glx* env, const double vp[16])
{
  FANCYMESG("updateProj");
  float d = itsScale;
  for(int dim=0;dim<3;++dim){
    for(int index=0;index<4;++index){
      Glx::Vector corner;
      getCorner(dim,index,corner);
      Glx::xformVec(corner,vp,cproj[dim][index]);
    }
  }

}

void
Glx::Crosshair3D::getCorner(int dim, int index, Glx::Vector& res) const
{  
  float d = itsScale;
  res=itsPos;

  switch( dim ){
    case 0:
      switch( index ){
	case 0: res += Glx::Vector(0, d, d);break;
	case 1: res += Glx::Vector(0,-d, d);break;
	case 2: res += Glx::Vector(0,-d,-d);break;
	case 3: res += Glx::Vector(0, d,-d);break;
      }
      break;
    case 1:
      switch( index ){
	case 0: res += Glx::Vector( d,0, d);break;
	case 1: res += Glx::Vector(-d,0, d);break;
	case 2: res += Glx::Vector(-d,0,-d);break;
	case 3: res += Glx::Vector( d,0,-d);break;
      }
      break;
    case 2:
      switch( index ){
	case 0: res += Glx::Vector( d, d,0);break;
	case 1: res += Glx::Vector(-d, d,0);break;
	case 2: res += Glx::Vector(-d,-d,0);break;
	case 3: res += Glx::Vector( d,-d,0);break;
      }
      break;
  }
}
