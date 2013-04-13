#include <iostream>
#include <sstream>
#include <GLX.h>
#include <glxTrackball.h>
using namespace std;

#include "debug.h"

bool show=true;
int level=0;
string message = "Level: up=increase, down=decrease";

void setcube(float v[8][3],float gmin[3], float gmax[3])
{
  /* Setup cube vertex data. */
  v[0][0] = v[1][0] = v[2][0] = v[3][0] = gmin[0];
  v[4][0] = v[5][0] = v[6][0] = v[7][0] = gmax[0];
  v[0][1] = v[1][1] = v[4][1] = v[5][1] = gmin[1];
  v[2][1] = v[3][1] = v[6][1] = v[7][1] = gmax[1];
  v[0][2] = v[3][2] = v[4][2] = v[7][2] = gmin[2];
  v[1][2] = v[2][2] = v[5][2] = v[6][2] = gmax[2];
}

void drawCube(float gmin[3], float gmax[3])
{
  float n[6][3]={{-1,0,0},{0,1,0},{1,0,0},{0,-1,0},{0,0,1},{0,0,-1}};
  int f[6][4]={{0,1,2,3},{3,2,6,7},{7,6,5,4},{4,5,1,0},{5,6,2,1},{7,4,0,3}};
  float v[8][3];

  setcube(v,gmin,gmax);
  //glEnable(GL_LIGHTING);
  //glEnable(GL_LIGHT0);
  //glBegin(GL_QUADS);
  for(int i = 0; i < 6; i++) {
    glBegin(GL_LINE_LOOP);
    glNormal3fv(&n[i][0]);
    glVertex3fv(&v[f[i][0]][0]);
    glVertex3fv(&v[f[i][1]][0]);
    glVertex3fv(&v[f[i][2]][0]);
    glVertex3fv(&v[f[i][3]][0]);
    glEnd();
  }
}

void loadCoords(int id, float glo[3],float ghi[3],float lo[3],float hi[3])
{
  switch( id ){
    case 0: 
      lo[0]=glo[0];
      lo[1]=glo[1];
      lo[2]=glo[2];
      hi[0]=0.5 * (glo[0]+ghi[0]);
      hi[1]=0.5 * (glo[1]+ghi[1]);
      hi[2]=0.5 * (glo[2]+ghi[2]);
      break;

    case 1:
      lo[0]=0.5 * (glo[0]+ghi[0]);
      lo[1]=glo[1];
      lo[2]=glo[2];
      hi[0]=ghi[0];
      hi[1]=0.5 * (glo[1]+ghi[1]);
      hi[2]=0.5 * (glo[2]+ghi[2]);
      break;

    case 2:
      lo[0]=glo[0];
      lo[1]=0.5 * (glo[1]+ghi[1]);
      lo[2]=glo[2];
      hi[0]=0.5 * (glo[0]+ghi[0]);
      hi[1]=ghi[1];
      hi[2]=0.5 * (glo[2]+ghi[2]);
      break;

    case 3:
      lo[0]=0.5 * (glo[0]+ghi[0]);
      lo[1]=0.5 * (glo[1]+ghi[1]);
      lo[2]=glo[2];
      hi[0]=ghi[0];
      hi[1]=ghi[1];
      hi[2]=0.5 * (glo[2]+ghi[2]);
      break;

    case 4: 
      lo[0]=glo[0];
      lo[1]=glo[1];
      lo[2]=0.5 * (glo[2]+ghi[2]);
      hi[0]=0.5 * (glo[0]+ghi[0]);
      hi[1]=0.5 * (glo[1]+ghi[1]);
      hi[2]=ghi[2];
      break;

    case 5:
      lo[0]=0.5 * (glo[0]+ghi[0]);
      lo[1]=glo[1];
      lo[2]=0.5 * (glo[2]+ghi[2]);
      hi[0]=ghi[0];
      hi[1]=0.5 * (glo[1]+ghi[1]);
      hi[2]=ghi[2];
      break;

    case 6:
      lo[0]=glo[0];
      lo[1]=0.5 * (glo[1]+ghi[1]);
      lo[2]=0.5 * (glo[2]+ghi[2]);
      hi[0]=0.5 * (glo[0]+ghi[0]);
      hi[1]=ghi[1];
      hi[2]=ghi[2];
      break;

    case 7:
      lo[0]=0.5 * (glo[0]+ghi[0]);
      lo[1]=0.5 * (glo[1]+ghi[1]);
      lo[2]=0.5 * (glo[2]+ghi[2]);
      hi[0]=ghi[0];
      hi[1]=ghi[1];
      hi[2]=ghi[2];
      break;
  }
}

int N=0;

void drawCube(int lvl, float gmin[3], float gmax[3])
{
  MESGVAR("drawCube [start]",lvl);
  if( lvl<=0 ){
    ++N;
    drawCube(gmin,gmax);
  } else {
    for(int i=0;i<8;++i){
      float lo[3], hi[3];
      loadCoords(i,gmin,gmax,lo,hi);
      drawCube(lvl-1,lo,hi);
    }
  }
  MESGVAR("drawCube [done]",lvl);
}

void draw(glx* env, void*)
{
  float hi=0.5;
  float gmin[3]={-hi,-hi,-hi}, gmax[3]={hi,hi,hi};
  N=0;
  drawCube(level,gmin,gmax);

  if(show) {
    ostringstream ostr;
    ostr << "level: " << level << ": subs: " << N;
    string levelmesg = ostr.str();
    env->showMessage(message);
    env->showMessage(levelmesg);
  }
  
}

void processKey(glx* env,XEvent *event,void*)
{
  KeySym ks = XLookupKeysym((XKeyEvent*)event,0);
  XKeyEvent *kep = (XKeyEvent *)(event);
  int ctlDown    = kep->state & ControlMask;
  int shiftDown  = kep->state & ShiftMask;
  int altDown    = kep->state & Mod1Mask;
  bool changed=false;


  switch( ks ){
    case XK_m:
      show=!show;
      changed=true;
      break;
    case XK_Up:
      ++level;
      changed=true;
      break;
    case XK_Down:
      --level;
      if( level<0 ){
	level=0;
      } else
	changed=true;
      break;
  }
  if( changed )
    env->wakeup();
}

void initGL(glx* env, void* user)
{
  env->addDrawFunc(draw);
  env->addEventFunc(processKey);
  Glx::Trackball* tb = new Glx::Trackball(env);
}

int main(int argc, char** argv)
{
  glx* env = new glx(initGL);
  env->mainLoop();
  return 1;
}
