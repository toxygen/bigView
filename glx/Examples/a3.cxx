#include <GLX.h>
#include <algorithm>
#include <iterator>
#include <glxColormap.h>
#include <glxTrackball.h>
#include <glxVector.h>
#include <assert.h>
#include "debug.h"
using namespace std;

#define CLAMP(_val,_min,_max){ \
  if(_val<_min)_val=_min; \
  if(_val>_max)_val=_max; \
}

const int I=64;
const int J=64;
const int K=64;

struct Pnt {
  float pos[3];
  float scalar;
  float d;
};

vector<Pnt*> Pnts;
class PntCompare {
public:
  bool operator()(const Pnt*& a1, const Pnt*& a2){
    return a1->d < a2->d;
  }
  bool operator()(Pnt*& a1, Pnt*& a2){
    return a1->d < a2->d;
  }
  bool operator()(Pnt* const& a1, Pnt* const& a2){
    return a1->d < a2->d;
  }
};

double eye[4]={0};
double sMin=MAXFLOAT,sMax=-MAXFLOAT;
Glx::Colormap* cmap=NULL;

void getEyePosition(Glx::Trackball* tb)
{
  double invMatrix[16];
  double preeye[4] = {0,0,0,1};

  Glx::inv4x4(tb->view, invMatrix);
  Glx::xformVec(preeye,invMatrix,eye);
}

void distFromEye(Pnt*& p)
{
  double dx = p->pos[0]-eye[0];
  double dy = p->pos[1]-eye[1];
  double dz = p->pos[2]-eye[2];
  p->d = sqrt(dx*dx+dy*dy+dz*dz);
}
bool usecmap=true;
void
draw(glx* env, void* user)
{  
  Glx::Trackball* tb = static_cast<Glx::Trackball*>(user);
  getEyePosition(tb);
  for_each(Pnts.begin(),Pnts.end(),distFromEye);
  std::sort(Pnts.begin(),Pnts.end(),PntCompare());
  vector<Pnt*>::iterator pIter = Pnts.begin();
  glEnable(GL_BLEND);  
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_POINT_SMOOTH);
  glPointSize(3);
  glBegin(GL_POINTS);
  glColor4f(1,1,1,1);
  for( ; pIter != Pnts.end() ; ++ pIter ){
    float rgba[4];
    float t = ((*pIter)->scalar - sMin )/(sMax-sMin);
    CLAMP(t,0,1);
    assert(0<=t && t<=1);
    Glx::Colormap::genFast(t,&rgba[0]);
    rgba[3] = t;
    if(usecmap)
    {
      //glTexCoord1f(t);
      cmap->scalar(t);
    }
    else
      glColor4fv(rgba);

    glVertex3fv( (*pIter)->pos );
  }
  glEnd();
}

float func( float p[3] )
{
  float fx = sin(10.0*p[0]);
  float fy = p[1] * p[1];
  float fz = cos(20 * p[2] * p[2]) + sin(1.7*p[2] + p[2]);
  return fx * fy + fz;
}

void predraw(glx *env, void* user)
{
  Glx::Colormap* cmap = static_cast<Glx::Colormap*>(user);
  if( usecmap ) cmap->preRender();
}
void postdraw(glx *env, void* user)
{
  Glx::Colormap* cmap = static_cast<Glx::Colormap*>(user);
  if( usecmap ) cmap->postRender();
}

void
init(glx* env, void*)
{
  for(int i=0;i<I;++i){
    float fi = -0.5 + ((float)i/(I-1));
    for(int j=0;j<J;++j){
      float fj = -0.5 + ((float)j/(J-1));
      for(int k=0;k<K;++k){
	float fk = -0.5 + ((float)k/(K-1));
	Pnt* p = new Pnt;
	p->pos[0]=fi;
	p->pos[1]=fj;
	p->pos[2]=fk;
	p->scalar = func(p->pos);
	if( p->scalar<sMin)sMin=p->scalar;
	if( p->scalar>sMax)sMax=p->scalar;
	Pnts.push_back(p);
      }
    }
  }
  _VAR2(sMin,sMax);
  Glx::Trackball* tb = new Glx::Trackball(env);
  cmap = new Glx::Colormap("junk.cmapraw");

  env->addPreDrawFunc(predraw,cmap);
  env->addDrawFunc(draw, tb );
  env->addPostDrawFunc(postdraw,cmap);
}

int
main(int argc, char** argv)
{  
  glx* env = new glx(init);
  env->mainLoop();
  return 0;
}

