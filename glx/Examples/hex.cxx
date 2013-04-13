#include <iostream>
#include <sstream>
#include <set>
#include <assert.h>
#include <GLX.h>
#include <glxTrackpad.h>
using namespace std;

//#define DEBUG 1
#include "debug.h"

//              ____
//             /    \
//        ____/  U   \____
// HEX:  /    \ 0,1  /    \
//      /  UL  \____/  UR  \
//      \ -1,1 /    \ 1,1  /
//       \____/      \____/
//       /    \ 0,0  /    \
//      /   L  \____/  R   \
//      \ -1,0 /    \ 1,0  /
//       \____/  D   \____/
//            \ 0,-1 /
//             \____/

class c2d {
public:

  enum {L,R,U,D,UL,UR,NPOS};

  c2d(void)
  {
    v[0]=v[1]=0;
  }

  c2d(int c, int r)
  {
    v[0]=c;
    v[1]=r;
  }

  c2d(const c2d& that) 
  {
    *this=that;
  }

  c2d(const c2d& that, int dir) 
  {
    neighbor(that.v, dir, this->v);
  }

  c2d neighbor(int dir)
  {
    c2d res;
    neighbor(this->v, dir, res.v);
    return res;    
  }

  c2d& operator=(const c2d& that) 
  {
    v[0]=that.v[0];
    v[1]=that.v[1];
    return *this;
  }

  int& operator[](int i)
  {
    assert(i>=0 && i<=1);
    return v[i];
  }

  friend bool operator==(const c2d& a, const c2d& b)
  {    
    return (a.v[0]==b.v[0] && a.v[1]==b.v[1]) ? true : false;
  }

  friend bool operator<(const c2d& a, const c2d& b)
  {
    if(a.v[0]<b.v[0]){
      return true;
    } else if(a.v[0]>b.v[0]){
      return false;
    } else if( a.v[1]<b.v[1]){
      return true;
    } else if( a.v[1]>b.v[1]){
      return false;
    } else {
      return false;
    }
  }

  friend ostream& operator<<(ostream& ostr, const c2d& c)
  {
    return ostr <<"["<<c.v[0]<<","<<c.v[1]<<"]";
  }

  static void neighbor(const int coord[2], int dir, int rel[2])
  {
    rel[0]=coord[0];
    rel[1]=coord[1];
    switch(dir){
      case c2d::L:
        rel[0]-=1;
	break;
      case c2d::R:
        rel[0]+=1;
	break;
      case c2d::U:
        rel[1]+=1;
	break;
      case c2d::D:
        rel[1]-=1;
	break;
      case c2d::UL:
        rel[0]-=1;
        rel[1]+=1;
	break;
      case c2d::UR:
        rel[0]+=1;
        rel[1]+=1;
	break;
    }
  }

  static float dist(c2d& a, c2d& b)
  {
    float dx=b.v[0]-a.v[0]; 
    float dy=b.v[1]-a.v[1]; 
    return sqrt( dx*dx + dy*dy );
  }

  int v[2];
};

//        ____
//       /    \
//      /   ___\  R
//      \      /
//       \____/
//        ____
//       /    \
//      /      \  
//      \   |  /  L
//       \__|_/
//
float R=0.57735026918963;
float SQRT_PI_OVER_3=0.86602540378444;
float L=SQRT_PI_OVER_3*R;
const int N=100;

const int HEXN=6;
float C[HEXN][2]={
  { 1.0, 0.000},
  { 0.5, SQRT_PI_OVER_3},
  {-0.5, SQRT_PI_OVER_3},
  {-1.0, 0.000},
  {-0.5,-SQRT_PI_OVER_3},
  { 0.5,-SQRT_PI_OVER_3}
};

bool picked=false;
int col=0,row=0;
float theta=8.5;
set<c2d> cellset;
enum {RED,GREEN,BLUE};

void cr2xy(int x, int y, float* fx, float* fy)
{
  * fx = x * 1.5 * R;
  * fy = SQRT_PI_OVER_3 * R * (y * 2. + (x%2));
}

void color(int c)
{
  switch(c){ 
    case RED: glColor3f(1,0,0);break;
    case GREEN: glColor3f(0,1,0);break;
    case BLUE: glColor3f(0,0,1);break;
  }
}

void drawhex(int x, int y,int c)
{
  int lw;
  float fx,fy,z=0.;;

  cr2xy(x,y,&fx,&fy);
  float S=R*0.9;
  glGetIntegerv(GL_LINE_WIDTH,&lw);
  color(c);
  glBegin(GL_TRIANGLES);
  for(int i=0;i<=HEXN;++i){
    int j=i+1;
    if( j>=HEXN )j=0;
    glVertex3f(fx,fy,z);
    glVertex3f(fx+C[i][0]*S,fy+C[i][1]*S,z);
    glVertex3f(fx+C[j][0]*S,fy+C[j][1]*S,z);
  }  
  glEnd();
  z+=0.1;
  glLineWidth(2);
  glColor3f(0,0,0);
  glBegin(GL_LINE_LOOP);
  for(int i=0;i<HEXN;++i)
    glVertex3f(fx+C[i][0]*R,fy+C[i][1]*R,z);
  glEnd();
  glLineWidth(lw);
}
void clamp(int& val, int lo, int hi)
{
  if( val<lo ){
    val = lo;
  } else if (val>hi ){
    val = hi;
  }
}

void draw(glx* env, void*)
{
  int R=N,C=N;
  glPushAttrib(GL_DEPTH_BUFFER_BIT|GL_LIGHTING_BIT);
  glDisable(GL_LIGHTING);

  //glDisable(GL_DEPTH_TEST);
  for(int r=0;r<R;++r){
    for(int c=0;c<C;++c){
      int color=BLUE;
      if(picked && c==col && r==row)
	color=RED;
      drawhex(c,r,color);
    }
  }

  set<c2d>::iterator iter = cellset.begin();
  for( ; iter != cellset.end();++iter){
    c2d c=*iter;
    int x=c[0];
    int y=c[1];

    if(x<0)  x+=C;
    if(x>=C) x-=C;
    if(y<0)  y+=R;
    if(y>=R) y-=R; 

    if( x==col && y==row) continue;
    drawhex(x,y,GREEN);    
  }

  glPopAttrib();
  ostringstream ostr;
  ostr << "[C,R]=["<<col<<","<<row<<"]";
  env->showMessage(ostr.str().c_str());
}

void pixelToWorldCoords(glx* env,int x, int y, float* xf, float* yf)
{ 
  int winh = env->winHeight();
  double px,py,pz;
  int viewport[4];
  double modelMatrix[16],projMatrix[16],inv[16];

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  env->applyProjections();
  glGetIntegerv(GL_VIEWPORT, viewport);
  glGetDoublev(GL_MODELVIEW_MATRIX,(double *)modelMatrix);
  glGetDoublev(GL_PROJECTION_MATRIX,(double *)projMatrix);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  env->unproject(x,winh-y,0,modelMatrix,projMatrix,viewport,&px,&py,&pz);
  *xf = px;
  *yf = py;
}

void xy2cr(glx* env,int x, int y, int* c, int* r)
{
  float xf,yf;
  pixelToWorldCoords(env,x,y,&xf,&yf);

  int xlo = (int) floor(1. + (float)(xf-R)/(1.5*R));
  int xhi = xlo+1;

  int ylo = (int)floor(yf/(SQRT_PI_OVER_3 * R * 2.));
  int yhi = ylo+1;

  for(int ix=xlo;ix<=xhi;++ix){
    for(int iy=ylo;iy<=yhi;++iy){
      float xc,yc,dx,dy;
      cr2xy(ix,iy,&xc,&yc);
      dx=xf-xc;
      dy=yf-yc;
      if( sqrt(dx*dx + dy*dy) < L ){
	*c=ix;
	*r=iy;
	picked=true;
	return;
      }
    }    
  }
}


void
add(set<c2d>& work, c2d& c, c2d& cc)
{
  set<c2d>::iterator iter = work.find(c);
  if( iter!=work.end() ) return;

  float delta=c2d::dist(c,cc);

  if( delta > theta ) return;
  work.insert(c);

  c2d l(c,c2d::L);
  c2d r(c,c2d::R);
  c2d u(c,c2d::U);
  c2d d(c,c2d::D);
  c2d ul(c,c2d::UL);
  c2d ur(c,c2d::UR);

  add(work,l,cc);
  add(work,r,cc);
  add(work,u,cc);
  add(work,d,cc);
  add(work,ul,cc);
  add(work,ur,cc);
}

void idle(glx* env,int x,int y,void*)
{
  cellset.clear();
  picked=false;
  xy2cr(env,x,y,&col,&row);
  VAR2(col,row);

  if( picked ){
    c2d cc(col,row);
    add(cellset,cc,cc);
  }
  env->wakeup();
}

void initGL(glx* env, void* user)
{
  env->addDrawFunc(draw);
  env->showAxis(0);
  env->background(1);
  env->addMouseIdleFunc(idle);
  Glx::Trackpad* tb = new Glx::Trackpad(env);
}

int main(int argc, char** argv)
{
  glx* env = new glx(initGL);
  env->mainLoop();
  return 1;
}
