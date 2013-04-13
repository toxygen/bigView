#define GL_GLEXT_VERBOSE 1
#define GL_GLEXT_PROTOTYPES 1

#include <iostream>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GLX.h>
#include <glxTrackball.h>
#include <unistd.h>
#include <fcntl.h>
#include <values.h>
#include <assert.h>
#include "slicer.h"

#include "debug.h"
using namespace std;

GLuint itsID=0;
double EPS=1e-12;

void draw(glx* env, void* user)
{
  Glx::Trackball* tb = static_cast<Glx::Trackball*>(user);
  static GLfloat xequalzero[] = {1,0,0,0};
  static GLfloat yequalzero[] = {0,1,0,0};
  static GLfloat zequalzero[] = {0,0,1,0};
  double m[16],minv[16];
  glGetDoublev(GL_MODELVIEW_MATRIX,(double *)m);
  int N=60;

  float eye[3]={m[0*4+2],m[1*4+2],m[2*4+2]};
  float gmin[3]={0,0,0},gmax[3]={1,1,1};  
  float spacing = (float)1./N;
  float dist = 0.5;
  SlicerState slicer(eye, spacing, dist);
  SlicerState::Slice* slice;
  int n_slices = slicer.calc_slices(gmin, gmax, &slice);

  glPushAttrib(GL_LIGHTING_BIT|GL_TEXTURE_BIT);
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);
  glEnable(GL_TEXTURE_GEN_R);
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
  glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
  glTexGenfv(GL_S, GL_EYE_PLANE, xequalzero);
  glTexGenfv(GL_T, GL_EYE_PLANE, yequalzero);
  glTexGenfv(GL_R, GL_EYE_PLANE, zequalzero);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_TEXTURE_3D);
  glDisable(GL_LIGHTING);

  glColor3f(1,1,1);
  for (int s = 0; s < n_slices; s++) {
    glBegin(GL_POLYGON);
    int n_verts = slice[s].n_verts;
    float (*v3p)[3] = &slice[s].vert[0];
    for (int v = 0; v < n_verts; v++)
      glVertex3fv(*v3p++);
    glEnd();
  }
  glPopAttrib();
}

#define LIMIT(v,lo,hi) {v = (v<lo) ? lo : ((v>hi) ? hi : v);}
void 
genFast(float percent, float rgb[4])
{
  const float LVL_0 = 0;
  const float LVL_1 = 0.141176;
  const float LVL_2 = 0.282353;
  const float LVL_3 = 0.427451;
  const float LVL_4 = 0.568627;
  const float LVL_5 = 0.713726;
  const float LVL_6 = 0.854902;
  const float LVL_7 = 1;
  if( percent <= LVL_1 ){
    float t = (percent-LVL_0)/0.141176;
    if( t<0 ) t = 0.0;
    if( t>1 ) t = 1.0;
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 0 + t * 1;
  } else if( percent <= LVL_2 ){
    float t = (percent-LVL_1)/0.141176;
    if( t<0 ) t = 0.0;
    if( t>1 ) t = 1.0;
    rgb[0] = 0;
    rgb[1] = 0 + t * 1;
    rgb[2] = 1;
  } else if( percent <= LVL_3 ){
    float t = (percent-LVL_2)/0.145098;
    if( t<0 ) t = 0.0;
    if( t>1 ) t = 1.0;
    rgb[0] = 0;
    rgb[1] = 1;
    rgb[2] = 1 + t * -1;
  } else if( percent <= LVL_4 ){
    float t = (percent-LVL_3)/0.141176;
    if( t<0 ) t = 0.0;
    if( t>1 ) t = 1.0;
    rgb[0] = 0 + t * 1;
    rgb[1] = 1;
    rgb[2] = 0;
  } else if( percent <= LVL_5 ){
    float t = (percent-LVL_4)/0.145098;
    if( t<0 ) t = 0.0;
    if( t>1 ) t = 1.0;
    rgb[0] = 1;
    rgb[1] = 1 + t * -1;
    rgb[2] = 0;
  } else if( percent <= LVL_6 ){
    float t = (percent-LVL_5)/0.141176;
    if( t<0 ) t = 0.0;
    if( t>1 ) t = 1.0;
    rgb[0] = 1;
    rgb[1] = 0;
    rgb[2] = 0 + t * 1;
  } else {
    float t = (percent-LVL_6)/0.145098;
    if( t<0 ) t = 0.0;
    if( t>1 ) t = 1.0;
    rgb[0] = 1;
    rgb[1] = 0 + t * 1;
    rgb[2] = 1;
  } 
  rgb[3]= 0.025;
}

GLfloat *
loadtex3d(char* fname, int *texwid, int *texht, int *texdepth)
{
  float maxf=1.;
  int dim[4]={0};
  int fd = open(fname,O_RDONLY);
  if( fd == -1 ){
    perror("open");
    return 0;
  }
  if( read(fd,dim,4*sizeof(int)) != 4 * sizeof(int)){
    perror("read");
    return 0;
  }
  printf("dim: [%d,%d,%d]\n",dim[0],dim[1],dim[2]);
  const int N=1;
  const int D=dim[0]*dim[1]*dim[2];
  const int S = N * D;
  GLfloat *data = new GLfloat[ S ];
  GLfloat *udata = new GLfloat[ 4 * S ];
  assert( data );  
  assert( udata );  
  if( read(fd,data,S*sizeof(float)) != S*sizeof(float)){
    perror("read");
    return 0;
  }
  close(fd);

  GLfloat mn=MAXFLOAT,mx=-MAXFLOAT;
  for(int i=0;i<S;++i){
    if( data[i]<mn ) mn = data[i];
    if( data[i]>mx ) mx = data[i];
  }
  printf("min/max: [%f,%f]\n",mn,mx);

  for(int i=0;i<S;++i){
    int index = i*4;
    float v = (float)(data[i]-mn)/(mx-mn);
    float rgba[4]={0};
    genFast(v,rgba);

    udata[index+0] = rgba[0] * maxf;
    udata[index+1] = rgba[1] * maxf;
    udata[index+2] = rgba[2] * maxf;
    udata[index+3] = rgba[3] * maxf;
  }
  delete [] data;
  *texwid   = dim[0];
  *texht    = dim[1];
  *texdepth = dim[2];
  return udata;
}

void initGL(glx* env, void* user)
{  
  int w,h,d;
  char* fname = static_cast<char*>(user);
  GLfloat *udata = loadtex3d(fname,&w,&h,&d);
  env->showAxis(0);

  glGenTextures(1,&itsID);
  glBindTexture(GL_TEXTURE_3D,itsID);

  glTexParameterf(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R, GL_CLAMP);

  glTexParameterf(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);  
  glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glTexImage3D(GL_TEXTURE_3D,0,GL_RGBA,
	       w,h,d,0,GL_RGBA,GL_FLOAT,udata);

  Glx::Trackball* tb = new Glx::Trackball(env);
  double lo[3]={0,0,0},hi[3]={1,1,1};
  tb->viewAll(lo,hi);

  env->addDrawFunc(draw,tb);
  env->background(1,1,1);
}

int main(int argc, char** argv)
{
  if( argc==1 ){
    cerr << "usage: vol <[2^N x 2^N x 2^N] plot3d scalar file>\n"
	 << "     : vol /scratch/zingale/sampled/bubble_1.5e7_3d_4_0760f.bin\n"
	 << endl;
    return 0;
  }
  glx* env = new glx(initGL,argv[1]);
  env->mainLoop();
  return 1;
}
