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

using namespace std;

glx* env = 0;
vector< float* > data;

static float d2r(double d){
  return d*(M_PI/180.0f);
}

void line(float lat1, float lng1, float lat2, float lng2)
{
  glBegin(GL_LINE_STRIP);
  for(int i=0;i<20 ;++i ){
    float t = (float)i/(float)19;    
    float lat = lat1 + t*(lat2-lat1);
    float lng = lng1 + t*(lng2-lng1);
    float x = cos(d2r(lat)) * cos(d2r(lng));
    float y = cos(d2r(lat)) * sin(d2r(lng));
    float z = sin(d2r(lat));
    glVertex3f(x,y,z);
  }
  glEnd();
}

void draw(glx* env, void* user)
{
  glDisable(GL_LIGHTING);
  glPointSize(2);

  vector<float*>::iterator i=data.begin();
  for( ; i!=data.end();++i){
    float* v = *i;
    line(v[0],v[1],v[0],v[3]);
    line(v[0],v[3],v[2],v[3]);
    line(v[2],v[3],v[2],v[1]);
    line(v[2],v[1],v[0],v[1]);
  }
}

void initGL(glx* env,void* user)
{
  env->addDrawFunc(draw);
}

int
main(int argc, char** argv)
{
  if( argc==1){
    cerr<<"usage: boxes <lat-long file>\n";
    return 1;
  }
  ifstream fin(argv[1]);
  while( fin ){
    float* fp = new float[4];
    for(int i=0; i<4 && fin ; ++i ){
      fin >> fp[i];
    }
    if( fin ) data.push_back( fp );
  }
  fin.close();
  env = new glx(initGL);
  Glx::Trackball* trackball = new Glx::Trackball(env);
  env->mainLoop();
}
