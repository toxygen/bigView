//////////////////////////////////////////////////////////////////////////
/////////////////////////////// glxQuat.C ////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "glxQuat.h"
#include <math.h>
#include <values.h>
#include <assert.h>

using namespace std;

static double EPS = 1.0e-6;

// STATIC FUNCTIONS

static int   signsEqual(float x, float y);
static float vmag( const float v[3]);
static void  vnorm(float v[3]);
static float vdot(const float v1[3], const float v2[3]);
static void  vcross(const float v1[3], const float v2[3], float res[3]);
static void  print_4x4_mat( float mat[4][4] );

Glx::Quat::Quat(void)
{
  reset();
}

Glx::Quat::Quat(const float radians, const float axis[3])
{
  set(radians,axis);
}

Glx::Quat::Quat(const Glx::Quat& quat)
{
  v[0] = quat.v[0];
  v[1] = quat.v[1];
  v[2] = quat.v[2];
  v[3] = quat.v[3];
}

Glx::Quat::Quat(float qv0, float qv1, float qv2, float qv3 )
{
  v[0] = qv0;
  v[1] = qv1;
  v[2] = qv2;
  v[3] = qv3;
}

Glx::Quat::Quat(const float v1[3], const float v2[3])
{
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
      reset();
      return;
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
      set(M_PI,norm);
      normalize();
      return;
    }

    // otherwise, these vectors are contained in the XY plane,
    // so we can use the Z vector as our axis

    norm[0] = 0.0f;
    norm[1] = 0.0f;
    norm[2] = 1.0f;
    set(M_PI,norm);
    normalize();
    return;
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
    reset();
    return;
  }

  // acos: [-1..1] -> [0 to pi radians]
  // BLUTE: why does mapV1toV2(X,Y) fail?
  theta = acos( vdot(v1,v2)/(m1*m2) );

  /* 3) Build a quaternion from [axis] and [theta] */
  set(-theta,norm);
  normalize();
}


void Glx::Quat::set(const float radians, const float axis[3])
{
  v[0] = axis[0] * sin(radians/(float)2.0);
  v[1] = axis[1] * sin(radians/(float)2.0);
  v[2] = axis[2] * sin(radians/(float)2.0);
  v[3] = cos(radians/(float)2.0);
}

void Glx::Quat::get(float& radians, float axis[3])
{
  radians = 2 * acos(v[3]);
  axis[0] = v[0]/sin(radians/(float)2.0);
  axis[1] = v[1]/sin(radians/(float)2.0);
  axis[2] = v[2]/sin(radians/(float)2.0);
}

void Glx::Quat::reset(void)
{
  v[0]=v[1]=v[2]=0.0f;
  v[3] = (float)1.0;
}

const float Glx::Quat::magnitude(void) const
{
  return sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] + v[3]*v[3]);
}

void Glx::Quat::normalize(void)
{
  float mag = magnitude();
  if ( fabs(mag) < EPS ) {
    v[0]=v[1]=v[2]=0.0f;
    v[3] = 1.0f;
  } else {
    v[0] /= mag;
    v[1] /= mag;
    v[2] /= mag;
    v[3] /= mag;
  }
}

Glx::Quat& 
Glx::Quat::operator=(const Glx::Quat& quat)
{
  v[0] = quat.v[0];
  v[1] = quat.v[1];
  v[2] = quat.v[2];
  v[3] = quat.v[3];
  return *this;
}

Glx::Quat& 
Glx::Quat::operator+=(const Glx::Quat& quat)
{
  v[0] += quat.v[0];
  v[1] += quat.v[1];
  v[2] += quat.v[2];
  v[3] += quat.v[3];
  normalize();
  return *this;
}

Glx::Quat& 
Glx::Quat::operator*=(const float scalar)
{
  v[0] *= scalar;
  v[1] *= scalar;
  v[2] *= scalar;
  v[3] *= scalar;
  return *this;
}

void 
Glx::Quat::objectSpaceRotate(const Glx::Quat& quat)
{
  Glx::Quat res = *this * quat;
  *this = res;
}

void 
Glx::Quat::worldSpaceRotate(const Glx::Quat& quat)
{
  Glx::Quat res = quat * *this;
  *this = res;
}

float& 
Glx::Quat::operator[](const int index)
{
  return v[index];
}

void Glx::Quat::buildMatrix(double mat[16]) const
{
  Glx::Quat q = *this;
  q.normalize();

  mat[ 0] = 1.0 - 2.0 * (q[1] * q[1] + q[2] * q[2]);
  mat[ 1] =       2.0 * (q[0] * q[1] - q[2] * q[3]);
  mat[ 2] =       2.0 * (q[2] * q[0] + q[1] * q[3]);
  
  mat[ 4] =       2.0 * (q[0] * q[1] + q[2] * q[3]);
  mat[ 5] = 1.0 - 2.0 * (q[2] * q[2] + q[0] * q[0]);
  mat[ 6] =       2.0 * (q[1] * q[2] - q[0] * q[3]);
  
  mat[ 8] =       2.0 * (q[2] * q[0] - q[1] * q[3]);
  mat[ 9] =       2.0 * (q[1] * q[2] + q[0] * q[3]);
  mat[10] = 1.0 - 2.0 * (q[1] * q[1] + q[0] * q[0]);
  
  mat[3]=mat[7]=mat[11]=mat[12]=mat[13]=mat[14]=0.0;
  mat[15] = 1.0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////// FRIEND FUNCTIONS ////////////////////////////
//////////////////////////////////////////////////////////////////////////

const Glx::Quat 
Glx::operator*(const Glx::Quat& q1, const float& scalar)
{
  return Glx::Quat(q1.v[0] * scalar,
		   q1.v[1] * scalar,
		   q1.v[2] * scalar,
		   q1.v[3] * scalar);
}

const Glx::Quat 
Glx::operator*(const Glx::Quat& q1, const Glx::Quat& q2)
{
  float v0,v1,v2,v3;

  v3 = q1.v[3]*q2.v[3] - q1.v[0]*q2.v[0] - q1.v[1]*q2.v[1] - q1.v[2]*q2.v[2];
  v0 = q1.v[3]*q2.v[0] + q1.v[0]*q2.v[3] + q1.v[1]*q2.v[2] - q1.v[2]*q2.v[1];
  v1 = q1.v[3]*q2.v[1] + q1.v[1]*q2.v[3] + q1.v[2]*q2.v[0] - q1.v[0]*q2.v[2];
  v2 = q1.v[3]*q2.v[2] + q1.v[2]*q2.v[3] + q1.v[0]*q2.v[1] - q1.v[1]*q2.v[0];

  float mag = sqrt( v0*v0 + v1*v1 + v2*v2 + v3*v3);
  if ( fabs(mag) < EPS ) {
    v0=v1=v2=0.0;
    v3 = 1.0;
  } else {
    v0 /= mag;
    v1 /= mag;
    v2 /= mag;
    v3 /= mag;
  }
  return Glx::Quat(v0,v1,v2,v3);
}

const Glx::Quat 
Glx::operator+(const Glx::Quat& q1, const Glx::Quat& q2)
{
  Glx::Quat t1, t2, t3, tf;

  //vcopy(q1,t1);
  t1.v[0] = q1.v[0];
  t1.v[1] = q1.v[1];
  t1.v[2] = q1.v[2];

  //vscale(t1,q2[3]);
  t1.v[0] *= q2.v[3];
  t1.v[1] *= q2.v[3];
  t1.v[2] *= q2.v[3];
  
  //vcopy(q2,t2);
  t2.v[0] = q2.v[0];
  t2.v[1] = q2.v[1];
  t2.v[2] = q2.v[2];

  //vscale(t2,q1[3]);
  t2.v[0] *= q1.v[3];
  t2.v[1] *= q1.v[3];
  t2.v[2] *= q1.v[3];
  
  //vcross(q2,q1,t3);
  vcross(q2.v,q1.v,t3.v);

  //vadd(t1,t2,tf);
  //vadd(t3,tf,tf);
  tf.v[0] = t1.v[0] + t2.v[0] + t3.v[0];
  tf.v[1] = t1.v[1] + t2.v[1] + t3.v[1];
  tf.v[2] = t1.v[2] + t2.v[2] + t3.v[2];

  //tf[3] = q1[3] * q2[3] - vdot(q1,q2);
  tf.v[3] = q1.v[3] * q2.v[3] - vdot(q1.v,q2.v);

  tf.normalize();
  return tf;
}

void Glx::slerp(const Glx::Quat& q1, 
		const Glx::Quat& q2, 
		float t, Glx::Quat& res)
{
  Glx::Quat _q1(q1);
  Glx::Quat _q2(q2);
  float rot1q[4];
  float omega, cosom, sinom;
  float scalerot0, scalerot1;
  int i;

  // Calculate the cosine
  cosom = dot(_q1,_q2);

  // adjust signs if necessary
  if ( cosom < 0.0 ) {
    cosom = -cosom;
    for ( int j = 0; j < 4; j++ )
      rot1q[j] = -_q2[j];
  } else  {
    for ( int j = 0; j < 4; j++ )
      rot1q[j] = _q2[j];
  }
  // calculate interpolating coeffs
  if ( (1.0 - cosom) > EPS ) { // standard case
    omega = acos(cosom);
    sinom = sin(omega);
    scalerot0 = sin(((float)1.0 - t) * omega) / sinom;
    scalerot1 = sin(t * omega) / sinom;
  } else { // rot0 and rot1 very close - just do linear interp.
    scalerot0 = (float)1.0 - t;
    scalerot1 = t;
  }
  for (i = 0; i < 4; i++) // build the new quarternion
    res[i] = scalerot0 * _q1[i] + scalerot1 * rot1q[i];
}

const float 
Glx::dot(const Glx::Quat& q1, const Glx::Quat& q2)
{
  return( q1.v[3]*q2.v[3] + vdot(q1.v,q2.v));
}

ostream& Glx::operator<<(ostream& str, const Glx::Quat& q)
{
  return str << q.v[3] << "[ " 
	     << q.v[0] << " "<< q.v[1] << " " << q.v[2] << "]";
}

istream& Glx::operator>>(std::istream& istr, Glx::Quat& q)
{
  char bl,br;
  istr >> q.v[3];
  istr >> bl; 
  istr >> q.v[0];
  istr >> q.v[1];
  istr >> q.v[2];
  istr >> br;
  q.normalize();  
  return istr;
}


void Glx::toR4(const double s3[4], double t, double r4[4])
{
  const double EPS = 1.0e-6;
  assert( fabs(t)> EPS );
  if( fabs(1.0 - s3[0])<EPS &&
      fabs(s3[1])<EPS &&
      fabs(s3[2])<EPS &&
      fabs(s3[3])<EPS )
  {
    r4[0]=r4[1]=r4[2]=1;
    r4[3]=0;
  } else {
    r4[0] = t * s3[1];
    r4[1] = t * s3[2];
    r4[2] = t * s3[3];
    r4[3] = t * (1.-s3[0]);
  }
}

void Glx::fromR4(const double r4[4], double s3[4])
{
  double rmag = 1./(r4[0]*r4[0] + r4[1]*r4[1] + r4[2]*r4[2] + r4[3]*r4[3]);  
  s3[0] = rmag * (r4[0]*r4[0] + r4[1]*r4[1] + r4[2]*r4[2] - r4[3]*r4[3]);
  s3[1] = rmag * 2 * r4[0] * r4[3];
  s3[2] = rmag * 2 * r4[1] * r4[3];
  s3[3] = rmag * 2 * r4[2] * r4[3];
}

bool 
Glx::operator==(const Glx::Quat& a, const Glx::Quat& b)
{
  const float EPS=1.0e-6;
  Glx::Quat acopy(a),bcopy(b);
  for (int i = 0; i < 3; i++) {
    if(fabs(acopy[i]-bcopy[i])>EPS )
      return false;
  }
  return true;
}

// operator<() must NOT return true if a==b

bool 
Glx::operator<(const Glx::Quat& a, const Glx::Quat& b)
{
  const float EPS=1.0e-6;
  bool ambiguous=true,res=false;
  Glx::Quat acopy(a),bcopy(b);
  for (int i = 0; i < 4 && ambiguous==true; i++) {
    if(fabs(acopy[i]-bcopy[i])<EPS ) // ambiguous
      continue;
    ambiguous=false;
    res = (acopy[i] < bcopy[i]) ? true : false;   
  }
  return res;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////// STATIC FUNCTIONS ////////////////////////////
//////////////////////////////////////////////////////////////////////////

static int signsEqual(float x, float y){
  if( x >= 0.0f && y >= 0.0f )
    return 1;
  if( x < 0.0f && y < 0.0f )
    return 1;
  return 0;
}

static float vmag( const float v[3])
{
  return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

static void vnorm(float v[3])
{
  float mag = vmag(v);
  if( fabs(mag)>EPS){
    v[0] /= mag;
    v[1] /= mag;
    v[2] /= mag;
  }
}

static void vcross(const float v1[3], const float v2[3], float res[3])
{
  res[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
  res[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
  res[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
}

static float vdot(const float v1[3], const float v2[3])
{
  return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

static void print_4x4_mat( float mat[4][4] )
{
  int i, j;
  
  cout << "===== MATRIX =====" << endl;
  for(i=0; i<4; i++){
    for(j=0; j< 4; j++){
      cout << mat[i][j] << "\t";
    }
    cout << endl;
  }
}
