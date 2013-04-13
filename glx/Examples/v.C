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
#include <glxVector.h>

using namespace std;

Glx::Vector cntr;
Glx::Vector pnts[4];
Glx::Vector vecs[4];
Glx::Vector nrms[4];
Glx::Vector aves[3];

void
draw(glx* env,void*)
{  
  Glx::Vector cntrs[4];
  Glx::Vector arrows[4];
  for(int i=0;i<4;++i){
    glColor3f(1,0,0);
    glBegin(GL_LINE_LOOP);
    glVertex3fv(cntr);    
    glVertex3fv(pnts[i]);
    glVertex3fv(pnts[(i+1)%4]);
    glEnd();
    cntrs[i] = (1./3.)*(cntr+pnts[i]+pnts[(i+1)%4]);
    arrows[i] = cntrs[i] + nrms[i];
  }
  glBegin(GL_LINES);
  for(int i=0;i<4;++i){
    glColor3f(0,1,0);
    glVertex3fv(cntrs[i]);
    glVertex3fv(arrows[i]);
  }
  glColor3f(0,0,1);
  glVertex3fv(cntr);
  glVertex3fv(cntr+aves[0]);

  glColor3f(1,0,1);
  glVertex3fv(cntr);
  glVertex3fv(cntr+aves[1]);

  glColor3f(1,1,0);
  glVertex3fv(cntr);
  glVertex3fv(cntr+aves[2]);
  
  glEnd();

  env->setMessageColor(0,0,1);
  env->showMessage("ave 0+2");

  env->setMessageColor(1,0,1);
  env->showMessage("ave 1+3");

  env->setMessageColor(1,1,0);
  env->showMessage("ave 0+1+2+3");
}

void gen(void)
{
  cntr[2]=drand48() * 0.1;

  for(int i=0;i<4;++i){
    double r=drand48();
    pnts[i][0] = r * cos((double)i * M_PI/2.);
    pnts[i][1] = r * sin((double)i * M_PI/2.);
    pnts[i][2] = 0.;
    vecs[i] = pnts[i]-cntr;
  }

  for(int i=0;i<5;++i){
    nrms[i]=Glx::cross( vecs[i],vecs[(i+1) % 4] );
    nrms[i].normalize();
  }

  aves[0] = 0.5 * (nrms[0]+nrms[2]);
  aves[1] = 0.5 * (nrms[1]+nrms[3]);
  aves[2] = 0.25 * (nrms[0]+nrms[1]+nrms[3]+nrms[2]);

  aves[0].normalize();
  aves[1].normalize();
  aves[2].normalize();
}

void processKey(glx* env,XEvent *event,void*)
{
  gen();
  env->wakeup();
}

void initGL(glx* env,void* user)
{
  Glx::Trackball* tb = new Glx::Trackball(env);
  env->addDrawFunc(draw);
  env->addEventFunc(processKey);
 
}

int
main(int argc, char** argv)
{
  srand48( time(0) );

  gen();

  glx* g = new glx(initGL);  
  g->addDrawFunc(draw);
  g->mainLoop();

  return 0;
}
