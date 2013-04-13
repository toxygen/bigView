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
vector<float*> vertandvec;

double lo[3]={0,-0.484082,0};
double hi[3]={48,20.3377,48};
bool dumpAndExit=false;
string viewfile;
int curT=0;

enum {Backward=-1,Forward=1};
int timedir=Forward;
bool animating=false;
float scale=0.1;

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

void inctime(int inc)
{
  curT += inc;
  if(curT<0) 
    curT+=vertandvec.size();
  else if( curT >= vertandvec.size()) 
    curT=0;
}

void draw(glx* env, void*)
{
  _FANCYVAR(curT);
  glEnable(GL_COLOR_MATERIAL);

  float* buf = vertandvec[curT];

  glBegin(GL_LINES);
  for(int i=0;i<nPoints;++i){
    float* pp = &buf[i*6];
    float* vp = &buf[i*6+3];
    if( vp[1]<0 ) glColor3f(1,0,0);
    else glColor3f(0,1,0);
    glVertex3fv( pp );
    glVertex3f( pp[0]+vp[0]*scale,pp[1]+vp[1]*scale,pp[2]+vp[2]*scale );
  }
  glEnd();
  if(env->getAnimation()) {
    inctime(timedir);
    env->wakeup();
  }
    
}

void processKey(glx* env,XEvent *event,void*)
{
  KeySym ks = XLookupKeysym((XKeyEvent*)event,0);
  XKeyEvent *kep = (XKeyEvent *)(event);
  int shiftdown  = kep->state & ShiftMask;
  
  switch( ks ){

    case XK_Up:
      scale *= 1.5;
      break;
    case XK_Down:
      scale /= 1.5;
      break;
    case XK_P:
      timedir=Backward;
      inctime( timedir );
      break;
      
    case XK_p: 
      timedir=Forward;
      inctime( timedir );
      break;
  }
  env->wakeup();
}

void initGL(glx* env, void* user)
{
  env->addDrawFunc(draw,user);
  Glx::Trackball* tb = new Glx::Trackball(env);
  tb->viewAll(lo,hi);
  if( viewfile.length() )
    tb->loadView(viewfile);

  if( dumpAndExit )
    env->addPostDrawFunc(save);
  env->setSize(1024,1024);
  env->addEventFunc(processKey);


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
  int c,dim=6; // [x,y,z,vx,vy,vz]

  if( argc==1 ){
    cerr<<"usage: vec3d <binary [t=0]> <binary [t=1]> ..." << endl;
    cerr<<"binary format [floats]: x y z vx vy vz ... "<<endl;
    return 0;
  }

  struct stat statbuf;

  while( (c = getopt(argc,argv,"dv:")) != -1){
    switch( c ){
      case 'v':
	viewfile=optarg;
	break;
      case 'd':
        dumpAndExit=true;
        break;
    }
  }

  off_t bytes,nFloats;
  for(int i=optind;i<argc;++i){
    _FANCYVAR(argv[i]);
    int res = stat(argv[i], &statbuf);
    off_t bytes = statbuf.st_size;
    off_t nFloats = bytes/sizeof(float);
    nPoints = nFloats/dim; 
    float* buf=new float[nFloats];
    int fd=open(argv[i],O_RDONLY);
    assert( read(fd,buf,bytes) == bytes );
    vertandvec.push_back( buf );
    close(fd);
  }

  //swapWords( (unsigned int*)points,nFloats);

  out = argv[optind] + string(".ppm");

  glx* env = new glx(initGL);
  env->mainLoop();

  return 0;
}
