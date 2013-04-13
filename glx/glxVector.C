
#include "glxVector.h"
#include <math.h>
#include <values.h>

using namespace std;

static const float EPS = 1.0e-6;

Glx::Vector::Vector(void)
{
  reset();
}

Glx::Vector::Vector(const Glx::Vector& vec)
{
  v[0]=vec.v[0];
  v[1]=vec.v[1];
  v[2]=vec.v[2];
}

Glx::Vector::Vector(const float vec[3])
{
  v[0]=vec[0];
  v[1]=vec[1];
  v[2]=vec[2];
}

Glx::Vector::Vector(float v0, float v1, float v2)
{
  v[0]=v0;
  v[1]=v1;
  v[2]=v2;
}

void 
Glx::Vector::set(const float v0,const float v1, const float v2)
{
  v[0]=v0;
  v[1]=v1;
  v[2]=v2;
}

void 
Glx::Vector::set(const float xyz[3])
{
  v[0]=xyz[0];
  v[1]=xyz[1];
  v[2]=xyz[2];
}

void 
Glx::Vector::reset(void)
{
  v[0]=0.0;
  v[1]=0.0;
  v[2]=0.0;
}

void 
Glx::Vector::copy(float dest[3]) const
{
  dest[0]=v[0];
  dest[1]=v[1];
  dest[2]=v[2];
}

float 
Glx::Vector::magnitude(void) const
{
  return sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
}
float 
Glx::Vector::mag(void) const
{
  return sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
}

const float 
Glx::dot(const Glx::Vector& v1, const Glx::Vector& v2)
{
  return v1.v[0]*v2.v[0] + v1.v[1]*v2.v[1] + v1.v[2]*v2.v[2];
}

const Glx::Vector 
Glx::cross(const Glx::Vector& v1, const Glx::Vector& v2)
{
  return Glx::Vector(
	  (v1.v[1] * v2.v[2]) - (v1.v[2] * v2.v[1]),
	  (v1.v[2] * v2.v[0]) - (v1.v[0] * v2.v[2]),
	  (v1.v[0] * v2.v[1]) - (v1.v[1] * v2.v[0]));
}

void 
Glx::Vector::normalize(void)
{
  float mag = magnitude();
  if ( fabs(mag) < EPS ) {
    v[0] = v[1] = v[2] = 0.;
  } else {
    v[0] /= mag;
    v[1] /= mag;
    v[2] /= mag;
  }
}

float& 
Glx::Vector::operator[](const int index)
{
  return v[index];
}

const float& 
Glx::Vector::operator[](const int index) const
{
  return v[index];
}

Glx::Vector& 
Glx::Vector::operator=(const Glx::Vector& vec)
{
  v[0] = vec.v[0];
  v[1] = vec.v[1];
  v[2] = vec.v[2];
  return *this;
}

Glx::Vector::operator const float*(void) const
{
  return &v[0];
}

Glx::Vector::operator float*(void)
{
  return &v[0];
}

const Glx::Vector 
Glx::operator+(const Glx::Vector& v1, const Glx::Vector& v2)
{
  return Glx::Vector(v1.v[0]+v2.v[0],v1.v[1]+v2.v[1],v1.v[2]+v2.v[2]);
}

const Glx::Vector 
Glx::operator-(const Glx::Vector& v1, const Glx::Vector& v2)
{
  return Glx::Vector(v1.v[0]-v2.v[0],v1.v[1]-v2.v[1],v1.v[2]-v2.v[2]);
}

bool 
Glx::operator==(const Glx::Vector& a, const Glx::Vector& b)
{
  const float EPS=1.0e-6;
  for (int i = 0; i < 3; i++) {
    if(fabs(a[i]-b[i])>EPS )
      return false;
  }
  return true;
}

// operator<() must NOT return true if a==b

bool 
Glx::operator<(const Glx::Vector& a, const Glx::Vector& b)
{
  const float EPS=1.0e-6;
  bool ambiguous=true,res=false;
  for (int i = 0; i < 3 && ambiguous==true; i++) {
    if(fabs(a[i]-b[i])<EPS ) // ambiguous
      continue;
    ambiguous=false;
    res = (a[i] < b[i]) ? true : false;   
  }
  return res;
}

const Glx::Vector 
Glx::operator*(const Glx::Vector& v1, const float scalar)
{
  return Glx::Vector(v1.v[0]*scalar,
		   v1.v[1]*scalar,
		   v1.v[2]*scalar);
}

const Glx::Vector 
Glx::operator*(const float scalar, const Glx::Vector& v1)
{
  return Glx::Vector(v1.v[0]*scalar,
		   v1.v[1]*scalar,
		   v1.v[2]*scalar);
}

Glx::Vector& 
Glx::Vector::operator+=(const Glx::Vector& vec)
{
  v[0] += vec.v[0];
  v[1] += vec.v[1];
  v[2] += vec.v[2];
  return *this;
}

Glx::Vector& 
Glx::Vector::operator-=(const Glx::Vector& vec)
{
  v[0] -= vec.v[0];
  v[1] -= vec.v[1];
  v[2] -= vec.v[2];
  return *this;
}

Glx::Vector& 
Glx::Vector::operator*=(const float scalar)
{
  v[0] *= scalar;
  v[1] *= scalar;
  v[2] *= scalar;
  return *this;
}

ostream& 
Glx::operator<<(ostream& str, const Glx::Vector& vec)
{
  str << vec.v[0] << " " << vec.v[1] << " " << vec.v[2];
  return str;
}

istream& 
Glx::operator>>(std::istream& istr, Glx::Vector& vec)
{
  istr >> vec.v[0] >> vec.v[1] >> vec.v[2];
  return istr;  
}

void 
Glx::xformVec(const Glx::Vector& v, const double m[16], Glx::Vector& res)
{
  Glx::Vector r;
  double r3;
  r[0] = v[0]*m[ 0]+v[1]*m[ 4]+v[2]*m[ 8]+m[12];
  r[1] = v[0]*m[ 1]+v[1]*m[ 5]+v[2]*m[ 9]+m[13];
  r[2] = v[0]*m[ 2]+v[1]*m[ 6]+v[2]*m[10]+m[14];
  r3   = v[0]*m[ 3]+v[1]*m[ 7]+v[2]*m[11]+m[15];
  if( fabs(r3) > EPS ){
    r[0] /= r3;
    r[1] /= r3;
    r[2] /= r3;
  }
  res = r;
}

void 
Glx::xformVec(const double v[4], const double m[16], double res[4])
{
  res[0] = v[0]*m[ 0]+v[1]*m[ 4]+v[2]*m[ 8]+m[12];
  res[1] = v[0]*m[ 1]+v[1]*m[ 5]+v[2]*m[ 9]+m[13];
  res[2] = v[0]*m[ 2]+v[1]*m[ 6]+v[2]*m[10]+m[14];
  res[3] = v[0]*m[ 3]+v[1]*m[ 7]+v[2]*m[11]+m[15];
  if( fabs(res[3]) > EPS ){
    res[0] /= res[3];
    res[1] /= res[3];
    res[2] /= res[3];
  }
}

void Glx::mult4x4(const double m[16], const double n[16], double res[16])
{
  res[ 0]=m[ 0]*n[0]+m[ 1]*n[4]+m[ 2]*n[ 8]+m[ 3]*n[12];
  res[ 1]=m[ 0]*n[1]+m[ 1]*n[5]+m[ 2]*n[ 9]+m[ 3]*n[13];
  res[ 2]=m[ 0]*n[2]+m[ 1]*n[6]+m[ 2]*n[10]+m[ 3]*n[14];
  res[ 3]=m[ 0]*n[3]+m[ 1]*n[7]+m[ 2]*n[11]+m[ 3]*n[15];

  res[ 4]=m[ 4]*n[0]+m[ 5]*n[4]+m[ 6]*n[ 8]+m[ 7]*n[12];
  res[ 5]=m[ 4]*n[1]+m[ 5]*n[5]+m[ 6]*n[ 9]+m[ 7]*n[13];
  res[ 6]=m[ 4]*n[2]+m[ 5]*n[6]+m[ 6]*n[10]+m[ 7]*n[14];
  res[ 7]=m[ 4]*n[3]+m[ 5]*n[7]+m[ 6]*n[11]+m[ 7]*n[15];

  res[ 8]=m[ 8]*n[0]+m[ 9]*n[4]+m[10]*n[ 8]+m[11]*n[12];
  res[ 9]=m[ 8]*n[1]+m[ 9]*n[5]+m[10]*n[ 9]+m[11]*n[13];
  res[10]=m[ 8]*n[2]+m[ 9]*n[6]+m[10]*n[10]+m[11]*n[14];
  res[11]=m[ 8]*n[3]+m[ 9]*n[7]+m[10]*n[11]+m[11]*n[15];

  res[12]=m[12]*n[0]+m[13]*n[4]+m[14]*n[ 8]+m[15]*n[12];
  res[13]=m[12]*n[1]+m[13]*n[5]+m[14]*n[ 9]+m[15]*n[13];
  res[14]=m[12]*n[2]+m[13]*n[6]+m[14]*n[10]+m[15]*n[14];
  res[15]=m[12]*n[3]+m[13]*n[7]+m[14]*n[11]+m[15]*n[15];
}

int 
Glx::inv4x4(const double m[16], double res[16])
{  
  double EPS = 1.0e-12;
  double a = m[0];
  double b = m[1];
  double c = m[2];
  double d = m[4];
  double e = m[5];
  double f = m[6];
  double g = m[8];
  double h = m[9];
  double i = m[10];
  double j = m[12];
  double k = m[13];
  double l = m[14];
  double denom = -c*e*g + b*f*g + c*d*h - a*f*h - b*d*i + a*e*i;
  if( fabs(denom) < EPS )
    return 0;
  res[0] = ( -(f*h)+(e*i) )/denom;
  res[1] = (  (c*h)-(b*i) )/denom;
  res[2] = ( -(c*e)+(b*f) )/denom;
  res[3] = 0.;
  res[4] = (  (f*g)-(d*i) )/denom;
  res[5] = ( -(c*g)+(a*i) )/denom;
  res[6] = (  (c*d)-(a*f) )/denom;
  res[7] = 0.;
  res[8] = ( -(e*g)+(d*h) )/denom;
  res[9] = (  (b*g)-(a*h) )/denom;
  res[10] = ( -(b*d)+(a*e) )/denom;
  res[11] = 0.;
  res[12] = (  (f*h*j)-(e*i*j)-(f*g*k)+(d*i*k)+(e*g*l)-(d*h*l) )/denom;
  res[13] = ( -(c*h*j)+(b*i*j)+(c*g*k)-(a*i*k)-(b*g*l)+(a*h*l) )/denom;
  res[14] = (  (c*e*j)-(b*f*j)-(c*d*k)+(a*f*k)+(b*d*l)-(a*e*l) )/denom;
  res[15] = 1.;
  return 1;
}

std::ostream& Glx::operator<<(std::ostream& ostr,const double mat[16])
{
  int i, j;  
  ostr << endl;
  for(i=0; i<4; i++){
    for(j=0; j< 4; j++){
      ostr.precision(5);
      ostr.width(10);
      ostr.fill(' ');
      ostr.setf(ios::right);
      ostr << mat[i*4+j] << "\t" ;
    }
    ostr << endl;
  }
  return ostr;
}
