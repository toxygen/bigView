#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <values.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>
#include <assert.h>
#include "debug.h"
#include <GLX.h>
#include <glxTrackball.h>
#include <Draggers/Slider.h>

using namespace std;
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
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glBegin(GL_QUADS);
  for(int i = 0; i < 6; i++) {
    glNormal3fv(&n[i][0]);
    glVertex3fv(&v[f[i][0]][0]);
    glVertex3fv(&v[f[i][1]][0]);
    glVertex3fv(&v[f[i][2]][0]);
    glVertex3fv(&v[f[i][3]][0]);
  }
  glEnd();
}

double clipplaneLo[4]={-1,0,0,0.3};
double clipplaneHi[4]={ 1,0,0,0.3};

void draw(glx* env, void*)
{
  float gmin[3]={-0.5,-0.5,-0.5};
  float gmax[3]={0.5,0.5,0.5};

  glClipPlane(GL_CLIP_PLANE0, clipplaneLo);
  glClipPlane(GL_CLIP_PLANE1, clipplaneHi);
  glEnable(GL_CLIP_PLANE0);
  glEnable(GL_CLIP_PLANE1);

  drawCube(gmin,gmax);

  glDisable(GL_CLIP_PLANE0);
  glDisable(GL_CLIP_PLANE1);

  ostringstream ostr;
  ostr << "lo: " 
       <<clipplaneLo[0]<<","
       <<clipplaneLo[1]<<","
       <<clipplaneLo[2]<<","
       <<clipplaneLo[3];
  string mesg = ostr.str();
  env->showMessage(mesg);

  ostr.str("");
  ostr << "hi: " 
       <<clipplaneHi[0]<<","
       <<clipplaneHi[1]<<","
       <<clipplaneHi[2]<<","
       <<clipplaneHi[3];
  mesg = ostr.str();
  env->showMessage(mesg);

}


void setLo(void* user, float t)
{
  glx* env = static_cast<glx*>(user);
  clipplaneLo[3]=t;
  env->wakeup();
}
void setHi(void* user, float t)
{
  glx* env = static_cast<glx*>(user);
  clipplaneHi[3]=t;
  env->wakeup();
}

void initGL(glx* env, void* user)
{
  Glx::Trackball* tb = new Glx::Trackball(env);
  env->addDrawFunc(draw);
  Glx::Slider* loSlider = new Glx::Slider(env,Glx::Slider::Y,30,20);
  Glx::Slider* hiSlider = new Glx::Slider(env,Glx::Slider::Y,60,20);
  loSlider->setRange(-1,0.3,1);
  hiSlider->setRange(-1,0.3,1);
  loSlider->setCallback(setLo,env);
  hiSlider->setCallback(setHi,env);
}

int
main(int argc, char** argv)
{
  glx* env = new glx(initGL);
  env->mainLoop();

  return 0;
}
