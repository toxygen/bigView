#include <iostream>
#include <sstream>
#include <set>
#include <glxVector.h>
#include <glxQuat.h>
#include "debug.h"

using namespace std;

float EPS = 1.0e-06;

void print_4x4_mat( double mat[16] )
{
  int i, j;
  
  for(i=0; i<4; i++){
    for(j=0; j< 4; j++){
      int index = i*4 + j;
      cout.width(12);
      cout << mat[index];
      cout << "\t" ;
    }
    cout << endl;
  }
}

void vcross(const float v1[3], const float v2[3], float res[3])
{
  res[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
  res[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
  res[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
}

float vmag( const float v[3])
{
  return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

void vnorm( float v[3])
{
  float mag = vmag(v);
  if( fabs(mag)>EPS){
    v[0] /= mag;
    v[1] /= mag;
    v[2] /= mag;
  }
}

float vdot(const float v1[3], const float v2[3])
{
  return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

static int signsEqual(float x, float y){
  if( x >= 0.0f && y >= 0.0f )
    return 1;
  if( x < 0.0f && y < 0.0f )
    return 1;
  return 0;
}

Glx::Quat 
mapV1toV2(float v1[3], float v2[3])
{
  Glx::Quat q;
  float norm[3];
  float xAxis[3]={1,0,0};
  float mag,theta;
  /* 
   * 1) Find the vector normal to the V1xV2 plane, by taking
   *    their cross product.
   */
  vcross(v1,v2,norm);
  mag = vmag(norm);

  // find out if v2 is a merely a scaled version of v1
  
  if( fabs(mag) < EPS ){
    // hmm, either these two vectors are parallel or anti-parallel
    if( signsEqual(v1[0],v2[0]) && 
	signsEqual(v1[0],v2[0]) &&
	signsEqual(v1[0],v2[0]) )
    {
      // ok, these are parallel, then use the identity
      return Glx::Quat();
    }
    
    // ok, the target vector is 180 degrees different than the
    // src vector. We need a vector perpendicular to both
    
    // so if we've got a vector not completely in the XY plane, 
    // we can drop a projection of one of the vectors and
    // easily determine it's 90 rotation, this will give
    // us a vector normal to v1 and v2. Yes, there are an 
    // infinite number of vectors orthogonal to these, we 
    // just want one.

    if( fabs(v1[2]) > EPS ){
      norm[0] = -v1[1];
      norm[1] = v1[0];
      norm[2] = 0.0f;
      q.set(M_PI,norm);
      q.normalize();
      return Glx::Quat(q);
    }
    
    // otherwise, these vectors are contained in the XY plane,
    // so we can use the Z vector as our axis 
    
    norm[0] = 0.0f;
    norm[1] = 0.0f;
    norm[2] = 1.0f;
    q.set(M_PI,norm);
    q.normalize();
    return Glx::Quat(q);
  }

  vnorm(norm);

  /* 2) Determine the angle thru which V1 must be rotated
   *    around [norm] to bring it [V1] into alignment with the 
   *    target vector [V2].
   *
   *               V1 dot V2
   * cos(theta) = -----------
   *              |V1| * |V2|
   *
   *                           [  V1 dot V2  ]
   *              theta = acos [-------------]
   *                           [ |V1| * |V2| ]
   */
  float m1 = vmag(v1);
  float m2 = vmag(v2);

  if( fabs(m1)<EPS || fabs(m2)<EPS ){
    return Glx::Quat();
  }

  // acos: [-1..1] -> [0 to pi radians]
  // BLUTE: why does mapV1toV2(X,Y) fail?
  theta = acos( vdot(v1,v2)/(m1*m2) );
  
  /* 3) Build a quaternion from [axis] and [theta] */
  q.set(theta,norm);
  q.normalize();
  return Glx::Quat(q);
}

struct Basis {
  Basis(Glx::Vector _x, Glx::Vector _y, Glx::Vector _z) : x(_x),y(_y),z(_z){}
  string toString(void){
    for(int i=0;i<3;++i){
      if(fabs(x[i])<1e-3)x[i]=0;
      if(fabs(y[i])<1e-3)y[i]=0;
      if(fabs(z[i])<1e-3)z[i]=0;
    }
    ostringstream ostr;
    ostr<<"["<<x<<"]["<<y<<"]["<<z<<"]";
    return ostr.str();
  }
  Glx::Vector x,y,z;
};

double diff(Glx::Vector a, Glx::Vector b)
{
  Glx::Vector d = a-b;
  return sqrt(Glx::dot(d,d));
}

int main(int argc, char** argv)
{
  float axes[3][3]={
    {1,0,0},
    {0,1,0},
    {0,0,1},
  };
  double m[16];
  Glx::Quat q;
  float xAxis[3]={1,0,0};
  float xMax[3]={1,0,0};
  float xMin[3]={-1,0,0};
  q = mapV1toV2(xAxis,xMax);

  q.buildMatrix(m);  
  print_4x4_mat(m);

  cout << q << endl;
  q = mapV1toV2(xAxis,xMin);
  q.buildMatrix(m);  
  print_4x4_mat(m);

  cout << q << endl;

  cout << "====================" <<endl;
  
  float target[3]={0.35,-0.24,0};
  double m2[16];
  Glx::Quat q2(xAxis,target);
  q2.buildMatrix(m2);

  set<string> baseSet;

  Glx::Quat viewQuat;
  Glx::Vector ix(1,0,0),iy(0,1,0),iz(0,0,1);
  Glx::Vector qx,qy,qz;

  viewQuat.buildMatrix(m2);
  xformVec(ix,m2,qx);
  xformVec(iy,m2,qy);
  xformVec(iz,m2,qz);
  Basis* basis = new Basis(qx,qy,qz);
  baseSet.insert(basis->toString());

  cout << viewQuat 
       << "\t\t:\t\t"
       << "["<<qx<<"] "
       << "["<<qy<<"] "
       << "["<<qz<<"] "
       << endl;

  for(int src=0;src<3;++src){
    for(int r1=1;r1<4;++r1){
      Glx::Quat q1;
      float rad1 = (float)r1 * (M_PI/2.0);
      q1.set(rad1,axes[src]);
      q1.normalize();
      
      for(int axis=0;axis<3;++axis){
	for(int r2=1;r2<4;++r2){
	  Glx::Quat q2;
	  float rad2 = (float)r2 * (M_PI/2.0);
	  q2.set(rad2,axes[axis]);
	  viewQuat = q1 * q2;
	  viewQuat.normalize();

	  for(int i=0;i<4;++i) 
	    if(fabs(viewQuat[i])<1e-3)viewQuat[i]=0;
	  FANCYVAR(viewQuat);

	  viewQuat.buildMatrix(m2);
	  VAR4x4(m2);
	  Glx::xformVec(ix,m2,qx);
	  Glx::xformVec(iy,m2,qy);
	  Glx::xformVec(iz,m2,qz);
	  for(int i=0;i<3;++i){
	    if(fabs(qx[i])<1e-3)qx[i]=0;
	    if(fabs(qy[i])<1e-3)qy[i]=0;
	    if(fabs(qz[i])<1e-3)qz[i]=0;
	  }
	  Basis* b = new Basis(qx,qy,qz);
	  VAR3(qx,qy,qz);

	  set<string>::iterator iter = baseSet.find(b->toString());
	  if( iter == baseSet.end() ){
	    MESG("\t\t====inserting===");
	    cout << viewQuat 
		 << "\t\t:\t\t"
		 << "["<<qx<<"] "
		 << "["<<qy<<"] "
		 << "["<<qz<<"] "
		 << endl;
	    baseSet.insert(b->toString());
	    VAR(baseSet.size());
	  } else {
	    MESG("====found===");
	    VAR( *iter );
	  }
	}
      }
    }
  }
}
