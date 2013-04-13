#include <iostream>
#include <glxVector.h>
#include <glxQuat.h>
using namespace std;

#include "debug.h"

int
main()
{
  Glx::Quat worldQuat(0.964564,-0.000737971,-0.26385, 0);
  Glx::Quat dq(1,-0.000500002,0,0);
  Glx::Quat c = worldQuat + dq;
  Glx::Quat d = worldQuat;
  d += dq;
  cout << worldQuat << endl;
  cout << dq << endl;
  cout << c << endl;
  cout << d << endl;
  
  double mat[16];
  Glx::Vector v;
  Glx::Vector xVec(1,0,0);
  Glx::Vector yVec(0.7,0.7,0);
  Glx::Quat n(xVec,yVec);
  n.buildMatrix(mat);
  Glx::xformVec(xVec,mat,v);
  for(int i=0;i<4;++i){
    for(int j=0;j<4;++j){
      cout << mat[i*4+j] << " ";
    }
    cout<<endl;
  }
  _VAR(n);
  _VAR4x4(mat);
  _VAR(v);
  cout << "\n\n\n\n";
#if 0
  Glx::Vector av(1,1,0),bv(1,1,1),cv(0,1,1);
  Glx::Quat q1(xVec,av);
  Glx::Quat q2(xVec,bv);
  Glx::Quat q3(xVec,cv);
  Glx::Quat res;
  for(int i=0;i<10;++i){
    double s3[4];
    float fi = (float)i/9.;
    slerp(q1, q2, fi, res);
    res.toS3(s3);
    cout << res << ":" 
	 << s3[0]<<","<<s3[1]<<","<<s3[2]<<","<<s3[3]<<endl;
    
  }
#endif

}
