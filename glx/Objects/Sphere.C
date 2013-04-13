#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Intrinsic.h>
#include <X11/X.h>
#include <X11/Xlib.h> 
#include <values.h>
#include <Objects/Sphere.h>
#include <math.h>
#include <assert.h>

/* BLUTE: SPHERE STUFF ... */
#ifndef I
#define I 0
#define J 1
#define K 2
#endif

#define LEG ((float)0.85065081)
#define ALT ((float)0.5257311)
#define Z_TRANS ((float)0.42532541)

//#define DEBUG 1
#ifdef DEBUG 
#define WF 1
#else
#define WF 0
#endif

Glx::Sphere::Sphere(void) : 
  itsComplexity(3),
  itsRadius(1.0),
  wireFrameMode(WF),
  needsVertexList(1),
  itsGlyphID(-1),
  itsWireGlyphID(-1),
  itsWireframeFlag(false)
{
  checkNeedObj();
  setCenter(0.0,0.0,0.0);
}

Glx::Sphere::Sphere(float xyz[3], float r) :
  itsComplexity(3),
  itsRadius(r),
  wireFrameMode(WF),
  needsVertexList(1),
  itsGlyphID(-1),
  itsWireGlyphID(-1),
  itsWireframeFlag(false)
{
  checkNeedObj();
  setCenter(xyz[0], xyz[1],xyz[2]);
}

Glx::Sphere::Sphere(float x, float y, float z, float r) :
  itsComplexity(3),
  itsRadius(r),
  wireFrameMode(WF),
  needsVertexList(1),
  itsGlyphID(-1),
  itsWireGlyphID(-1),
  itsWireframeFlag(false)
{
  checkNeedObj();
  setCenter(x,y,z);
}

void
Glx::Sphere::checkNeedObj(void)
{
  if( itsGlyphID == -1 ){
    if( needsVertexList )
      genVertList();
    itsGlyphID = glGenLists(1);
    if( itsGlyphID == 0 ){
      itsGlyphID = -1;
      return;
    }
    glNewList(itsGlyphID, GL_COMPILE);
    icos(itsComplexity);
    glEndList();
  }
  if( itsWireGlyphID == -1 ){
    itsWireGlyphID = glGenLists(1);
    if( itsWireGlyphID == 0 ){
      itsWireGlyphID = -1;
      return;
    }
    glNewList(itsWireGlyphID, GL_COMPILE);
    wireSphere(20,0);
    glEndList();
  }
}

void
Glx::Sphere::setCenter(float x, float y, float z)
{
  itsCenter.set(x,y,z);
}

void
Glx::Sphere::setCenter(float xyz[3])
{
  itsCenter.set(xyz);
}

void
Glx::Sphere::setCenter(Glx::Vector& v)
{
  itsCenter.set(v);
}

void
Glx::Sphere::setRadius(float r)
{
  itsRadius = r;
}

void
Glx::Sphere::set(float xyz[3], float r)
{
  itsCenter.set(xyz);
  itsRadius = r;
}

void
Glx::Sphere::setComplexity(int c)
{
  itsComplexity = c;
  glNewList(itsGlyphID, GL_COMPILE);
  icos(itsComplexity);
  glEndList();
}

void
Glx::Sphere::setCenterCrosshairs(int v)
{
  glNewList(itsWireGlyphID, GL_COMPILE);  
  wireSphere(20,v);
  glEndList();
}

void Glx::Sphere::draw(glx*,void*)
{
  double mat[16];
  checkNeedObj();
  glPushAttrib(GL_LIGHTING|GL_TRANSFORM_BIT);
  if( itsWireframeFlag )
    glDisable(GL_LIGHTING);
  else {
    // make a diff where you enable these, 
    // app may have other ideas
    //glEnable(GL_LIGHTING);
    //glEnable(GL_LIGHT0);
    //glEnable(GL_LIGHT1);
    glEnable(GL_NORMALIZE);
  }
  itsQuat.buildMatrix(mat);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glTranslatef(itsCenter[0],itsCenter[1],itsCenter[2]);
  glMultMatrixd(mat);
  glScalef(itsRadius,itsRadius,itsRadius);
  if( itsWireframeFlag ){
    glCallList( itsWireGlyphID );
  } else {
    glCallList( itsGlyphID );
  }
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glPopAttrib();
}

void 
Glx::Sphere::sph_interp(Glx::Vector& v1, Glx::Vector& v2, 
			float t, Glx::Vector& res)
{
  double cur[16], inv[16];
  float alpha = acos( dot(v1, v2)/(v1.magnitude()*v2.magnitude()) );
  float delta = alpha*t;
  Glx::Vector i,j,k,tmp, _v1, _v2;

  i.set(v1[0],v1[1],v1[2]);
  i.normalize();

  /* calc the vector ortho to these two, the 'K' vector */
  k = cross(i, v2);

  /* calc a new 'J' vector bases using I and K*/
  j = cross(k,i);
  j.normalize();

  buildMat(cur,i,j,k);
  
  _v1[0] = projection(v1, i);
  _v1[1] = projection(v1, j);
  _v1[2] = projection(v1, k);

  if( Glx::inv4x4(cur, inv) ){
    rot_z(_v1, delta, _v2 );
    Glx::xformVec(_v2, cur, res);
  } else {
    std::cerr << "! INV failed!" << std::endl;
  }
}

void 
Glx::Sphere::sendTri(Glx::Vector& a, Glx::Vector& b, Glx::Vector& c, int lvl)
{
  Glx::Vector ab,bc,ca;

  if( lvl > 0 ){
    --lvl;
    sph_interp( a, b, 0.5, ab );
    sph_interp( b, c, 0.5, bc );
    sph_interp( c, a, 0.5, ca );    
    ab.normalize();
    bc.normalize();
    ca.normalize();
    sendTri( a, ab, ca, lvl);
    sendTri( b, bc, ab, lvl);
    sendTri( c, ca, bc, lvl);
    sendTri(ab, bc, ca, lvl);
  } else {
    if( wireFrameMode ){
      glBegin(GL_LINE_LOOP);
      glVertex3fv(a);
      glVertex3fv(b);
      glVertex3fv(c);
      glEnd();
    } else {
      glBegin(GL_TRIANGLES);
      glNormal3fv(a);
      glVertex3fv(a);
      glNormal3fv(b);
      glVertex3fv(b);
      glNormal3fv(c);
      glVertex3fv(c);
      glEnd();
    }
  }
}

void 
Glx::Sphere::wireSphere(int numSteps, int addCenter)
{
  int i;
  float alpha,delta,x,y,z;
  delta = (float)(2.0*M_PI)/(float)numSteps;

  glBegin(GL_LINE_LOOP);
  for(i=0;i<numSteps;i++){
    alpha = delta*(float)i;
    x = cos(alpha); y = sin(alpha); z = 0.0;
    glVertex3f(x,y,z);
  }
  glEnd();

  glBegin(GL_LINE_LOOP);
  for(i=0;i<numSteps;i++){
    alpha = delta*(float)i;
    y = cos(alpha); z = sin(alpha); x = 0.0;
    glVertex3f(x,y,z);
  }
  glEnd();

  glBegin(GL_LINE_LOOP);
  for(i=0;i<numSteps;i++){
    alpha = delta*(float)i;
    z = cos(alpha); x = sin(alpha); y = 0.0;
    glVertex3f(x,y,z);
  }
  glEnd();

  if( addCenter ){    
    glBegin(GL_LINES);
    glVertex3f(-1,0,0);
    glVertex3f( 1,0,0);
    glVertex3f(0,-1,0);
    glVertex3f(0, 1,0);
    glVertex3f(0,0,-1);
    glVertex3f(0,0, 1);
    glEnd();
  }
}

void 
Glx::Sphere::icos(int lvl)
{
  sendTri(vertex_list[ 0],vertex_list[ 1],vertex_list[ 2],lvl);
  sendTri(vertex_list[ 0],vertex_list[ 2],vertex_list[ 3],lvl);
  sendTri(vertex_list[ 0],vertex_list[ 3],vertex_list[ 4],lvl);
  sendTri(vertex_list[ 0],vertex_list[ 4],vertex_list[ 5],lvl);
  sendTri(vertex_list[ 0],vertex_list[ 5],vertex_list[ 1],lvl);

  sendTri(vertex_list[ 1],vertex_list[ 8],vertex_list[ 9],lvl);
  sendTri(vertex_list[ 2],vertex_list[ 9],vertex_list[10],lvl);
  sendTri(vertex_list[ 3],vertex_list[10],vertex_list[ 6],lvl);
  sendTri(vertex_list[ 4],vertex_list[ 6],vertex_list[ 7],lvl);
  sendTri(vertex_list[ 5],vertex_list[ 7],vertex_list[ 8],lvl);

  sendTri(vertex_list[ 6],vertex_list[ 4],vertex_list[ 3],lvl);
  sendTri(vertex_list[ 7],vertex_list[ 5],vertex_list[ 4],lvl);
  sendTri(vertex_list[ 8],vertex_list[ 1],vertex_list[ 5],lvl);
  sendTri(vertex_list[ 9],vertex_list[ 2],vertex_list[ 1],lvl);
  sendTri(vertex_list[10],vertex_list[ 3],vertex_list[ 2],lvl);

  sendTri(vertex_list[11],vertex_list[ 6],vertex_list[10],lvl);
  sendTri(vertex_list[11],vertex_list[ 7],vertex_list[ 6],lvl);
  sendTri(vertex_list[11],vertex_list[ 8],vertex_list[ 7],lvl);
  sendTri(vertex_list[11],vertex_list[ 9],vertex_list[ 8],lvl);
  sendTri(vertex_list[11],vertex_list[10],vertex_list[ 9],lvl);
#if 0
  std::cout << "static float gel_icos[12][3] = {" << std::endl;
  for(int i = 0 ; i < 12 ; i++){
    std::cout << "  {"
	      <<vertex_list[i][0]<<","
	      <<vertex_list[i][1]<<","
	      <<vertex_list[i][2]<<"}," << std::endl;
  }
  std::cout << "};" << std::endl;
#endif

}


/*---------------------------------------------------------------------- */

void 
Glx::Sphere::rotate_vec2d(float vec[2], float alpha)
{
  float xtmp, ytmp;
  float cos_alpha = cos(alpha);
  float sin_alpha = sin(alpha);
  xtmp =  vec[0]*cos_alpha + vec[1]*sin_alpha;
  ytmp = -vec[0]*sin_alpha + vec[1]*cos_alpha;
  vec[0] = xtmp;
  vec[1] = ytmp;
}

/*---------------------------------------------------------------------- */
#define D2R(d) (float)(d* 0.01745329251994329576923690768)
void 
Glx::Sphere::genVertList(void)
{
  float a[2], b[2];
  int angle;
  int num_vertices = 0;

  needsVertexList = 0;

  /* topmost vertex */
  vertex_list[num_vertices][0] = 0.0;
  vertex_list[num_vertices][1] = 0.0;
  vertex_list[num_vertices][2] = ALT+Z_TRANS;
  vertex_list[num_vertices].normalize();
  ++num_vertices;
 
  /* calc verts of basic pentagon */
  a[0] = LEG;
  a[1] = 0.0;
  for(angle = 0; angle < 360 ; angle += 72 ){
    b[0] = a[0];
    b[1] = a[1];
    rotate_vec2d(b, D2R(angle) );
    vertex_list[num_vertices][0] = b[0];
    vertex_list[num_vertices][1] = b[1];
    vertex_list[num_vertices][2] = Z_TRANS;
    vertex_list[num_vertices].normalize();
    ++num_vertices;
  }
 
  a[0] = -LEG;
  a[1] = 0.0;
  for( angle = 0; angle < 360 ; angle += 72 ){
    b[0] = a[0];
    b[1] = a[1];
    rotate_vec2d(b, D2R(angle) );
    vertex_list[num_vertices][0] = b[0];
    vertex_list[num_vertices][1] = b[1];
    vertex_list[num_vertices][2] = -Z_TRANS;
    vertex_list[num_vertices].normalize();
    ++num_vertices;
  }
 
  /* bottommost vertex*/
  vertex_list[num_vertices][0] = 0.0;
  vertex_list[num_vertices][1] = 0.0;
  vertex_list[num_vertices][2] = -(ALT+Z_TRANS);
  vertex_list[num_vertices].normalize();
  ++num_vertices;
}


/*---------------------------------------------------------------------- */

void 
Glx::Sphere::getCenter(float xyz[3])
{
  xyz[0]=itsCenter[0];
  xyz[1]=itsCenter[1];
  xyz[2]=itsCenter[2];
}

/*---------------------------------------------------------------------- */

void  
Glx::Sphere::getCenter(Glx::Vector& v)
{
  v.set(itsCenter);
}

/*---------------------------------------------------------------------- */
  
void   
Glx::Sphere::rot_z(Glx::Vector& v, float alpha, Glx::Vector& res)
{
  res[0] =  v[0]*cos(alpha) - v[1]*sin(alpha);
  res[1] =  v[0]*sin(alpha) + v[1]*cos(alpha);
  res[2] =  v[2];
}  

/*---------------------------------------------------------------------- */

float
Glx::Sphere::projection(Glx::Vector& src, Glx::Vector& dst)
{
  return( dot(src, dst)/dot(dst,dst) );
}

/*---------------------------------------------------------------------- */

void   
Glx::Sphere::buildMat(double mat[16], Glx::Vector& i, 
		      Glx::Vector& j, Glx::Vector& k)
{
  mat[ 0] = i[0]; mat[ 1] = i[1]; mat[ 2] = i[2]; mat[ 3] = 0.0;
  mat[ 4] = j[0]; mat[ 5] = j[1]; mat[ 6] = j[2]; mat[ 7] = 0.0;
  mat[ 8] = k[0]; mat[ 9] = k[1]; mat[10] = k[2]; mat[11] = 0.0;
  mat[12] = 0.0;  mat[13] = 0.0;  mat[14] = 0.0;  mat[15] = 1.0;
}

/*---------------------------------------------------------------------- */

/************** END OF SPHERE CODE ****************/
