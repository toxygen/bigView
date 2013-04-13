#include <GLX.h>
#include <glxTrackpad.h>
#include <sstream>
#include <GL/glu.h>

#include "debug.h"

using namespace std;

static const float EPS = 1.0e-6;

void
pixelToWorldCoords(glx* env, int x, int y, float* xf, float* yf)
{
  ostringstream ostr;
  int winh = env->winHeight();
  double px,py,pz;
  int viewport[4];
  double modelMatrix[16],projMatrix[16],inv[16];
  glGetDoublev(GL_MODELVIEW_MATRIX,(double *)modelMatrix);
  glGetDoublev(GL_PROJECTION_MATRIX,(double *)projMatrix);
  glGetIntegerv(GL_VIEWPORT, viewport);
  env->unproject(x,winh-y,0,modelMatrix,projMatrix,viewport,&px,&py,&pz);
  *xf = px;
  *yf = py;
}

int N=0,skipped=0,replaced=0;
vector<float> pntx,pnty;

void draw(glx* env, void* user)
{
  int* mousePos = static_cast<int*>(user);
  float worldX,worldY;
  float wx,wy;
  ostringstream ostr;
  ostr << "Pixel["<<mousePos[0]<<","<<mousePos[1]<<"]"<<endl;
  env->showMessage(ostr.str().c_str());
  pixelToWorldCoords(env,mousePos[0],mousePos[1],&worldX,&worldY);
  ostr.str("");
  ostr << "World["<<worldX<<","<<worldY<<"]"<<endl;
  env->showMessage(ostr.str().c_str());

  if(N>0){
    ostr.str("");
    ostr << "       N="<<N;
    env->showMessage(ostr.str().c_str());
    ostr.str("");
    ostr << " skipped="<<skipped<<" [" << (int)(100.*(float)skipped/N)<<"%]";
    env->showMessage(ostr.str().c_str());
    ostr.str("");
    ostr << "replaced="<<replaced<<" [" << (int)(100.*(float)replaced/N)<<"%]";
    env->showMessage(ostr.str().c_str());
  }

  int envW = env->winWidth();
  int envH = env->winHeight();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0,envW,0,envH,-1,1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  
  glBegin(GL_LINE_STRIP);
  for(int i=0;i<pntx.size();++i)
    glVertex2f(pntx[i],envH-pnty[i]);
  glEnd();
}

void mouseIdle(glx* env, int x, int y, void* user)
{
  int* mousePos = static_cast<int*>(user);
  mousePos[0]=x;
  mousePos[1]=y;
  env->wakeup();
}
void reset(void)
{
  N=0;
  skipped=0;
  replaced=0;
}

void mouseDown(glx* env, int x, int y, void* user)
{
  _FANCYMESG("==================");
  reset();
  pntx.clear();
  pnty.clear();
  pntx.push_back(x);
  pnty.push_back(y);
  env->wakeup();
}
void mouseProcess(glx* env, int x, int y, void* user)
{
  if( x==pntx[N] && y==pnty[N] ){
    ++skipped;
    return;
  }
  if(N>1){
    float pdx=(float)pntx[N]-pntx[N-1];
    float pdy=(float)pnty[N]-pnty[N-1];
    float cdx=(float)x-pntx[N];
    float cdy=(float)y-pnty[N];
    if( fabs(pdx-cdx)<EPS && fabs(pdy-cdy)<EPS ){
      pntx[N]=x;
      pnty[N]=y;
      ++replaced;
      return;
    }
  }
  pntx.push_back(x);
  pnty.push_back(y);
  ++N;
  env->wakeup();
}
void mouseUp(glx* env, void* user)
{
  pntx.push_back(pntx[0]);
  pnty.push_back(pnty[0]);
  env->wakeup();
}

int
main(int,char**)
{
  int mousePos[2];
  glx* env = new glx();
  Glx::Trackpad* tp = new Glx::Trackpad(env);
  env->addMouseIdleFunc(mouseIdle,&mousePos[0]);
  env->addMouseDownFunc(mouseDown);
  env->addMouseProcessFunc(mouseProcess);
  env->addMouseUpFunc(mouseUp);

  env->addDrawFunc(draw,&mousePos[0]);
  env->mainLoop();
  return 0;
}
