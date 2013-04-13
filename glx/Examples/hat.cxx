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

using namespace std;

double center=0.;
double scale=5.;

void draw(glx* env, void*)
{
  int N=100;
  int W=25;
  for(int i=-N;i<=N;++i){  
    glBegin(GL_LINE_STRIP);  
    double di=(double)i/W;
    for(int j=-N;j<=N;++j){
      double dj=(double)j/W;
      double d=di*di + dj*dj;
      double factor = (1.-d) * exp( -d/2. );
      glVertex3d(di,dj,factor);
    }
    glEnd();
  }
  for(int j=-N;j<=N;++j){
    double dj=(double)j/W;
    glBegin(GL_LINE_STRIP);
    for(int i=-N;i<=N;++i){    
      double di=(double)i/W;
      double d=di*di + dj*dj;
      double factor = (1.-d) * exp( -d/2. );
      glVertex3d(di,dj,factor);
    }
    glEnd();
  }
}

void initGL(glx* env, void* user)
{
  env->addDrawFunc(draw);
  Glx::Trackball* tb = new Glx::Trackball(env);
}


int
main(int argc, char** argv)
{
  glx* env = new glx(initGL);
  env->mainLoop();

  return 0;
}
