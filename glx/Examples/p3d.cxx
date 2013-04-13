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

off_t nPoints;
float* points=0;

double lo[3]={0,-0.484082,0};
double hi[3]={48,20.3377,48};
bool dumpAndExit=false;

void 
setColor(float percent, float alpha)
{
  const float LVL_0 = 0;
  const float LVL_1 = 0.556863;
  const float LVL_2 = 0.929412;
  const float LVL_3 = 0.992157;
  float rgb[4]={0,0,0,alpha};

  if( percent <= LVL_1 ){
    float t = (percent-LVL_0)/0.556863;
    if( t<0 ) t = 0.0;
    if( t>1 ) t = 1.0;
    rgb[0] = 0 + t * 1;
    rgb[1] = 0 + t * 0.0901961;
    rgb[2] = 0;
  } else if( percent <= LVL_2 ){
    float t = (percent-LVL_1)/0.372549;
    if( t<0 ) t = 0.0;
    if( t>1 ) t = 1.0;
    rgb[0] = 1;
    rgb[1] = 0.0901961 + t * 0.831373;
    rgb[2] = 0;
  } else {
    float t = (percent-LVL_2)/0.0627451;
    if( t<0 ) t = 0.0;
    if( t>1 ) t = 1.0;
    rgb[0] = 1;
    rgb[1] = 0.921569 + t * 0.0784314;
    rgb[2] = 0 + t * 1;
  }
  glColor4fv(rgb);
}

string out;
static int HACK=0;
void
save(glx* env, void* user)
{
  if( ++HACK==3 ){
    env->dumpImage(out);
    exit(0);
  }
  else
    env->wakeup();
}

const float sqrttwopi=sqrt(2.*M_PI);

float gaussian(off_t x, off_t n)
{
  float t = (float)x/n;
  float e = (float)(t-0.5);
  float e2 = e*e/2.;
  float ex= exp(-e2)/sqrttwopi;
  VAR5(x,n,e,e2,ex);
  return ex;
}

void draw(glx* env, void*)
{
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBegin(GL_POINTS);
  for(int i=0;i<nPoints;++i){
    float alpha = gaussian(i,nPoints);
    float* p = &points[i*3];
    float depth=p[1]/hi[1];
    setColor(depth,alpha);
    glVertex3fv( p );
  }
  glEnd();
}

void initGL(glx* env, void* user)
{
  env->addDrawFunc(draw,user);
  Glx::Trackball* tb = new Glx::Trackball(env);
  tb->viewAll(lo,hi);
  tb->loadView("p3d.view");
  if( dumpAndExit )
    env->addPostDrawFunc(save);
  env->setSize(1024,1024);

  //tb->viewAll(lo,hi);
}

static void
swapWords(unsigned* buf, int numWords)
{
  register int i;
  register unsigned* src = buf;

  for(i = 0 ; i < numWords ; i++, src++ ){

    //assert(*src==0xffff0000);
      //cout << hex << *src << endl;

    *src = ((*src & 0x000000ff) << 24) |
      ((*src & 0x0000ff00) << 8) |
      ((*src & 0x00ff0000) >> 8) |
      ((*src & 0xff000000) >> 24);
  }
}

int
main(int argc, char** argv)
{
  int c,dim=3;
  struct stat statbuf;

  if( argc==1 ){
    cerr <<"usage: p3d <raw binary 3D points>" <<endl;
    return 0;
  }

  while( (c = getopt(argc,argv,"d")) != -1){
    switch( c ){
      case 'd':
        dumpAndExit=true;
        break;
    }
  }


  int res = stat(argv[optind], &statbuf);
  off_t bytes = statbuf.st_size;
  off_t nFloats = bytes/sizeof(float);
  nPoints = nFloats/dim;

  points = new float[nFloats];

  int fd=open(argv[optind],O_RDONLY);
  assert( read(fd,points,bytes) == bytes );
  close(fd);

  //swapWords( (unsigned int*)points,nFloats);
  cout << points[0] << " " << points[1]<<" " <<points[2]<<endl;

  out = argv[optind] + string(".ppm");

  glx* env = new glx(initGL);
  env->mainLoop();

  return 0;
}
