#include <iostream>
#include <GLX.h>
#include <glxTrackpad.h>
using namespace std;

int N=20;
float *pntx=0,*pnty=0,*dx=0,*dy=0;

#if 0
float pntx[5]={.0,.1,.2,.3,.4};
float pnty[5]={.0,.1,.15,.1,.0};
float dx[5],dy[5];
#endif

void drawCross(int i){
  float D=0.025;
  glColor3f(0,1,0);
  glBegin(GL_LINES);
  glVertex2f(pntx[i]-D,pnty[i]);
  glVertex2f(pntx[i]+D,pnty[i]);
  glVertex2f(pntx[i],pnty[i]-D);
  glVertex2f(pntx[i],pnty[i]+D);
  glEnd();
}

void drawTangent(int i){
  glColor3f(1,0,0);
  glBegin(GL_LINES);
  glVertex2f(pntx[i]-dx[i],pnty[i]-dy[i]);
  glVertex2f(pntx[i]+dx[i],pnty[i]+dy[i]);
  glEnd();
}


void draw(glx* env, void*)
{
  for(int i=0;i<N;++i)
    drawCross(i);

  for(int i=0;i<N;++i)
    drawTangent(i);
}

void tangent(int N, float *x, float* y, float* vx, float* vy, int i)
{
  Glx::Vector qm1,qp0,qp1,qp2;

  if(i==0)
  {
    qp1.set(x[i+1]-x[i+0],y[i+1]-y[i+0],0);
    qp2.set(x[i+2]-x[i+1],y[i+2]-y[i+1],0);
    qp0.set(2.*qp1[0]-qp2[0],2.*qp1[1]-qp2[1],0);
    qm1.set(2.*qp0[0]-qp1[0],2.*qp0[1]-qp1[1],0);
  } 
  else if(i==1 )
  {
    qp0.set(x[i+0]-x[i-1],y[i+0]-y[i-1],0);
    qp1.set(x[i+1]-x[i+0],y[i+1]-y[i+0],0);
    qp2.set(x[i+2]-x[i+1],y[i+2]-y[i+1],0);
    qm1.set(2.*qp0[0]-qp1[0],2.*qp0[1]-qp1[1],0);    
  }
  else if(i==N-2)
  {
    qm1.set(x[i-1]-x[i-2],y[i-1]-y[i-2],0);
    qp0.set(x[i+0]-x[i-1],y[i+0]-y[i-1],0);
    qp1.set(x[i+1]-x[i+0],y[i+1]-y[i+0],0);
    qp2.set(2.*qp1[0]-qp0[0],2.*qp1[1]-qp0[1],2.*qp1[2]-qp0[2]);
  }
  else if(i==N-1)
  {
    qm1.set(x[i-1]-x[i-2],y[i-1]-y[i-2],0);
    qp0.set(x[i+0]-x[i-1],y[i+0]-y[i-1],0);
    qp1.set(2.*qp0[0]-qm1[0],2.*qp0[1]-qm1[1],2.*qp0[2]-qm1[2]);
    qp2.set(2.*qp1[0]-qp0[0],2.*qp1[1]-qp0[1],2.*qp1[2]-qp0[2]);
  } 
  else 
  {
    qm1.set(x[i-1]-x[i-2],y[i-1]-y[i-2],0);
    qp0.set(x[i+0]-x[i-1],y[i+0]-y[i-1],0);
    qp1.set(x[i+1]-x[i+0],y[i+1]-y[i+0],0);
    qp2.set(x[i+2]-x[i+1],y[i+2]-y[i+1],0);    
  }

  Glx::Vector nvec = Glx::cross(qm1,qp0);
  Glx::Vector dvec = Glx::cross(qp1,qp2);
  double n = nvec.mag();
  double d = nvec.mag() + dvec.mag();
  double a = (d>0) ? (n/d) : 1.;
  Glx::Vector v = (1.-a)*qp0 + a * qp1;
  vx[i]=v[0];
  vy[i]=v[1];
}

void initGL(glx* env, void* user)
{
  env->addDrawFunc(draw);
  Glx::Trackpad* tb = new Glx::Trackpad(env);
}

int main(int argc, char** argv)
{
  pntx=new float[N];
  pnty=new float[N];
  dx=new float[N];
  dy=new float[N];

  for(int i=0;i<N;++i){
    pntx[i]=(float)i/N;
    pnty[i]=sin(pntx[i]) + 0.1 * cos(4*M_PI*pntx[i]);
  }

  for(int i=0;i<N;++i)
    tangent(N, pntx, pnty, dx, dy,i);

  glx* env = new glx(initGL);
  env->mainLoop();
  return 1;
}
