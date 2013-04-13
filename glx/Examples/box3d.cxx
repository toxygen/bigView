#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <X11/keysym.h>
#include <math.h>
#include <values.h>
#include <GLX.h>
#include <glxTrackball.h>

#include "debug.h"

using namespace std;

glx* env = 0;
vector<float*> boxes;

void drawBbox(float* lo, float* hi)
{
  glColor3f(1,1,1);
  glBegin(GL_LINES);
  glVertex3d(lo[0],lo[1],lo[2]);
  glVertex3d(hi[0],lo[1],lo[2]);

  glVertex3d(lo[0],hi[1],lo[2]);
  glVertex3d(hi[0],hi[1],lo[2]);

  glVertex3d(lo[0],hi[1],hi[2]);
  glVertex3d(hi[0],hi[1],hi[2]);

  glVertex3d(lo[0],lo[1],hi[2]);
  glVertex3d(hi[0],lo[1],hi[2]);
  //////////////
  glVertex3d(lo[0],lo[1],lo[2]);
  glVertex3d(lo[0],hi[1],lo[2]);

  glVertex3d(hi[0],lo[1],lo[2]);
  glVertex3d(hi[0],hi[1],lo[2]);

  glVertex3d(hi[0],lo[1],hi[2]);
  glVertex3d(hi[0],hi[1],hi[2]);

  glVertex3d(lo[0],lo[1],hi[2]);
  glVertex3d(lo[0],hi[1],hi[2]);
  //////////////
  glVertex3d(lo[0],lo[1],lo[2]);
  glVertex3d(lo[0],lo[1],hi[2]);

  glVertex3d(hi[0],lo[1],lo[2]);
  glVertex3d(hi[0],lo[1],hi[2]);

  glVertex3d(hi[0],hi[1],lo[2]);
  glVertex3d(hi[0],hi[1],hi[2]);

  glVertex3d(lo[0],hi[1],lo[2]);
  glVertex3d(lo[0],hi[1],hi[2]);
  glEnd();
}

void draw(glx* env, void* user)
{
  glDisable(GL_LIGHTING);
  vector<float*>::iterator i=boxes.begin();
  while( i!=boxes.end() ){
    float* lo = *i;
    ++i;
    float* hi = *i;
    ++i;
    drawBbox(lo,hi);
  }
}
float near=0.1;
float far=100.0;
void initGL(glx* env,void* user)
{
  Glx::Trackball* trackball = new Glx::Trackball(env);
  env->addDrawFunc(draw);
  trackball->itsNear=near;
  trackball->itsFar=far;
}

int
main(int argc, char** argv)
{
  int c;

  if( argc==1){
    cerr<<"usage: boxes3d [options] <file>\n";
    cerr<<"=== [options] ==="<<endl
	<<"-n <float> : near clipping plane"<<endl
	<<"-f <float> : far clipping plane"
	<< endl;
    return 1;
  }

  while( (c = getopt(argc,argv,"n:f:")) != -1){
    switch( c ){
      case 'n':
	near=atof(optarg);
	break;
      case 'f':
	far=atof(optarg);
	break;
    }
  }

  ifstream fin(argv[optind]);
  while( fin ){
    char ch;
    float* lo = new float[3];
    float* hi = new float[3];

    fin>>ch>>lo[0]>>ch>>lo[1]>>ch>>lo[2]>>ch>>ch
       >>ch>>hi[0]>>ch>>hi[1]>>ch>>hi[2]>>ch;
    _VAR3(lo[0],lo[1],lo[2]);
    _VAR3(hi[0],hi[1],hi[2]);
    if( fin ) {      
      boxes.push_back( lo );
      boxes.push_back( hi );
    }
  }
  fin.close();
  env = new glx(initGL);
  env->mainLoop();
}
