#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Intrinsic.h>
#include <X11/X.h>
#include <X11/Xlib.h> 
#include <math.h>
#include <iostream>
#include <assert.h>
#include <GLX.h>
#include <Objects/Arrow.h>

using namespace std;

int Glx::Arrow::itsGlyphID[2] = {-1,-1};
static Glx::Vector xVec(1,0,0);

Glx::Arrow::Arrow(void)
{  
  setDefaults();
  checkNeedObj();
}

Glx::Arrow::Arrow(float xyz[3])
{
  Glx::Vector v(xyz);
  setDefaults();
  checkNeedObj();
  itsQuat = Glx::Quat(xVec, v);
  itsQuat.buildMatrix(itsCurMat);
}

Glx::Arrow::Arrow(float x, float y, float z)
{
  Glx::Vector v(x,y,z);
  setDefaults();
  checkNeedObj();
  itsQuat = Glx::Quat(xVec, v);
  itsQuat.buildMatrix(itsCurMat);
}

Glx::Arrow::Arrow(Glx::Vector& v)
{
  setDefaults();
  checkNeedObj();
  itsQuat = Glx::Quat(xVec, v);
  itsQuat.buildMatrix(itsCurMat);
}

void
Glx::Arrow::checkNeedObj(void)
{
  if( itsGlyphID[TUBEARROW] == -1 ){
    itsGlyphID[TUBEARROW] = glGenLists(1);
    if( itsGlyphID[TUBEARROW] == 0 ){
      itsGlyphID[TUBEARROW] = -1;
      return;
    }
    glNewList(itsGlyphID[TUBEARROW], GL_COMPILE);
    buildTubeArrow();
    glEndList();
  }
  if( itsGlyphID[WIREARROW] == -1 ){
    itsGlyphID[WIREARROW] = glGenLists(1);
    if( itsGlyphID[WIREARROW] == 0 ){
      itsGlyphID[WIREARROW] = -1;
      return;
    }
    glNewList(itsGlyphID[WIREARROW], GL_COMPILE);
    buildWireArrow();
    glEndList();
  }
}

void Glx::Arrow::setScale(const float s)
{
  itsScale = s;
}

void Glx::Arrow::setPos(const float x, const float y, const float z)
{
  itsPosition.set(x,y,z);
}

void Glx::Arrow::setVec(const float x, const float y, const float z)
{
  Glx::Vector v(x,y,z);
  itsQuat = Glx::Quat(xVec, v);
  itsQuat.buildMatrix(itsCurMat);
}

void Glx::Arrow::setPos(const float xyz[3])
{
  itsPosition.set(xyz);
}

void Glx::Arrow::setVec(const float xyz[3])
{
  Glx::Vector v(xyz);
  itsQuat = Glx::Quat(xVec, v);
  itsQuat.buildMatrix(itsCurMat);
}

void Glx::Arrow::setPos(const Glx::Vector& v)
{
  itsPosition = v;
}

void Glx::Arrow::setVec(const Glx::Vector& v)
{
  itsQuat = Glx::Quat(xVec,const_cast<Glx::Vector&>(v));
  itsQuat.buildMatrix(itsCurMat);
}

/*---------------------------------------------------------------------- */

float Glx::Arrow::getScale(void)
{
  return itsScale;
}

void Glx::Arrow::getPos(float vec[3])
{
  itsPosition.copy(vec);
}

void Glx::Arrow::getPos(Glx::Vector& v)
{
  v = itsPosition;
}

void Glx::Arrow::getVec(float vec[3])
{
  Glx::Vector v;
  Glx::xformVec(xVec,itsCurMat,v);
  v.copy(vec);
}

void Glx::Arrow::getVec(Glx::Vector& v)
{
  Glx::xformVec(xVec,itsCurMat,v);
}

void 
Glx::Arrow::draw(glx*,void*)
{
  checkNeedObj();
  
  glPushAttrib(GL_ENABLE_BIT|GL_LIGHTING);  
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  if( itsWireframeFlag ){
    if( itsGlyphID[WIREARROW] == -1 ){
      glPopAttrib();
      return;
    }
    glDisable(GL_LIGHTING);
  } else if(itsGlyphID[TUBEARROW] == -1 ){
    glPopAttrib();
    return;
  }
  glEnable(GL_NORMALIZE);
  itsQuat.buildMatrix(itsCurMat);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glTranslatef(itsPosition[0],itsPosition[1],itsPosition[2]);
  glMultMatrixd(itsCurMat);
  glScalef(itsScale,itsScale,itsScale);
  if( itsWireframeFlag )
    glCallList( itsGlyphID[WIREARROW] );
  else
    glCallList( itsGlyphID[TUBEARROW] );
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glPopAttrib();
}

/*---------------------------------------------------------------------- */

#define D2R(d) (float)(d* 0.01745329251994329576923690768)
void   
Glx::Arrow::buildTubeArrow(void)
{
  const int NUM_SIDES = 12;
  const float TUBE_RADIUS = (float)0.075;
  const float CONE_ANGLE  = D2R(20.0);
  const float CONE_RADIUS = 1.5*TUBE_RADIUS;
  int side;
  float y,z;
  float theta = 0.0;
  float dtheta = D2R( 360.0/(float)NUM_SIDES );
  float coneLength = CONE_RADIUS*(sin(D2R(90)-CONE_ANGLE)/sin(CONE_ANGLE));
  float tubeLength = 1.0 - coneLength;
  Glx::Vector v0,v1,v2,save,norm;
  Glx::Vector leg1,leg2;
  //float v0[3],v1[3],v2[3],save[3],norm[3];
  //float leg1[3],leg2[3];
  int nextSide;
  
  // make the tube
  glBegin(GL_TRIANGLE_STRIP);
  for(side = 0 ; side <= NUM_SIDES ; side++ ){
    if( side == 0 || side == NUM_SIDES ){
      y = TUBE_RADIUS;
      z = 0.0;
    } else {
      y = TUBE_RADIUS*cos(theta);
      z = TUBE_RADIUS*sin(theta);
    }
    // no X component in the normal vector
    // since this vector is along the X-axis
    // use the coords as the normal!
    norm[0] = 0.0;
    norm[1] = y;
    norm[2] = z;
    //normalize(norm);
    norm.normalize();
    glNormal3fv(norm);
    glVertex3f(0.0,       y,z);
    glNormal3fv(norm);
    glVertex3f(tubeLength,y,z);
    theta += dtheta;
  }
  glEnd();
  
  // Now, make the cap
  // v0 is the apex
  v0[0] = 1.0;
  v0[1] = 0.0;
  v0[2] = 0.0;

  // reset theta;
  theta = 0.0;

  glBegin(GL_TRIANGLE_FAN);

  for(side = 0 ; side < NUM_SIDES ; side++ ){

    // 'this' vert

    if( side == 0 ){
      v1[1] = CONE_RADIUS;
      v1[2] = 0.0;
    } else {
      v1[1] = CONE_RADIUS*cos(theta);
      v1[2] = CONE_RADIUS*sin(theta);
    }
    v1[0] = tubeLength;

    // 'next' vert

    // this will made next = 0 if side + 1 == NUM_SIDES
    nextSide = (side + 1) % NUM_SIDES;
    if( nextSide == 0 ){
      v2[1] = CONE_RADIUS;
      v2[2] = 0.0;
    } else {
      v2[1] = CONE_RADIUS*cos(theta+dtheta);
      v2[2] = CONE_RADIUS*sin(theta+dtheta);
    }
    v2[0] = tubeLength;
    
    leg1 = v1-v0;
    leg2 = v2-v0;
    norm = cross(leg1,leg2);
    norm.normalize();

    glNormal3fv(norm);

    if( side == 0 ){
      //send the apex
      glVertex3fv(v0);
      // save this first vert for sending last
      save[0]=v1[0];
      save[1]=v1[1];
      save[2]=v1[2];
    }

    glVertex3fv(v1);
    theta += dtheta;
  }
  glVertex3fv(save);
  glEnd();
}

void   
Glx::Arrow::buildWireArrow(void)
{
  const float TUBE_RADIUS = (float)0.075;
  const float CONE_ANGLE  = D2R(20.0);
  const float CONE_RADIUS = 1.5*TUBE_RADIUS;
  float coneLength = CONE_RADIUS*(sin(D2R(90)-CONE_ANGLE)/sin(CONE_ANGLE));
  float tubeLength = 1.0 - coneLength;
  
  // make the staff
  glBegin(GL_LINES);
  glVertex3f(0,0,0);
  glVertex3f(tubeLength,0,0);
  glEnd();
  
  // Now, make the arrowhead
  glBegin(GL_LINE_STRIP);
  glVertex3f(1,0,0);
  glVertex3f(tubeLength,CONE_RADIUS,0);
  glVertex3f(tubeLength,-CONE_RADIUS,0);
  glVertex3f(1,0,0);
  glEnd();
  glBegin(GL_LINE_STRIP);
  glVertex3f(1,0,0);
  glVertex3f(tubeLength,0,CONE_RADIUS);
  glVertex3f(tubeLength,0,-CONE_RADIUS);
  glVertex3f(1,0,0);
  glEnd();
}

void 
Glx::Arrow::setDefaults(void)
{
  itsScale = 1.0;
  itsQuat.buildMatrix(itsCurMat);
  itsWireframeFlag=false;
}

/************** END OF ARROW CODE ****************/
