#include <iostream>
#include <math.h>
#include <values.h>
#include <assert.h>
#include "glxVector.h"
using namespace std;

#define EPS 1.0e-6

int
main(int argc, char** argv)
{
  double translate[16]={
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    1,2,3,1,
  };
  double scale[16]={
    2,0,0,0,
    0,2,0,0,
    0,0,2,0,
    0,0,0,1,
  };
  float zero[3]={0};
  float raw[3]={3,4,5};
  float raw2[3]={4,5,6};
  Glx::Vector axb(-1,2,-1);
  Glx::Vector bxa(1,-2,1);
  Glx::Vector aplusb(7,9,11);
  Glx::Vector aminusb(-1,-1,-1);
  Glx::Vector atranslated(4,6,8);
  Glx::Vector ascaled(6,8,10);
  Glx::Vector a; // Vector(void)
  Glx::Vector b(raw); // Vector(const float[3])
  Glx::Vector c(b); // Vector(const Glx::Vector&);
  Glx::Vector d(raw[0],raw[1],raw[2]); // Vector(float,float,float);

  a.normalize();
  b.normalize();
  
  assert(a.mag()==0);
  assert(b.mag()==1);

  b.reset();
  assert(b.mag()==0);

  a.set(raw[0],raw[1],raw[2]);
  b.set(raw2);
  assert(a[0]==raw[0]&&a[1]==raw[1]&&a[2]==raw[2]);
  assert(b[0]==raw2[0]&&b[1]==raw2[1]&&b[2]==raw2[2]);

  a.reset();
  a.set(raw);
  assert(a[0]==raw[0]&&a[1]==raw[1]&&a[2]==raw[2]);

  assert(Glx::dot(a,b)==62);
  assert( Glx::cross(a,b) == axb );
  assert( Glx::cross(b,a) == bxa );
  c = a;
  a += b;
  assert(a==aplusb);
  a -= b;
  assert( a==c );
  a *= M_PI;
  assert(fabs(a[0]-(3*M_PI))<EPS);
  assert(fabs(a[1]-(4*M_PI))<EPS);
  assert(fabs(a[2]-(5*M_PI))<EPS);

  a = c;
  assert( *a == 3.0f );

  c = a * M_PI;
  assert(fabs(c[0]-(3*M_PI))<EPS);
  assert(fabs(c[1]-(4*M_PI))<EPS);
  assert(fabs(c[2]-(5*M_PI))<EPS);

  c = M_PI * a;
  assert(fabs(c[0]-(3*M_PI))<EPS);
  assert(fabs(c[1]-(4*M_PI))<EPS);
  assert(fabs(c[2]-(5*M_PI))<EPS);
  
  assert( a+b == aplusb );
  assert( a-b == aminusb );

  xformVec(a,translate,b); 
  assert( b == atranslated );

  xformVec(a,scale,b); 
  assert( b == ascaled );

  a.copy(zero);
  assert( zero[0]==3 && zero[1]==4 && zero[2]==5);

  return 0;
}
