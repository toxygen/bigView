#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
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
#include <GLX.h>
#include "debug.h"

using namespace std;
std::vector< unsigned char* > rgba;
std::vector< float* > depth;
int PW=0,PH=0;
bool needsize=true;

bool load(string name)
{
  int dims[3];
  string fname;
  fname = name + ".rgba";
  ifstream fin;

  cout << "=== " << fname << " ===" << endl;
  fin.open(fname.c_str());
  if( ! fin ){
    perror("open");
    return false;
  }
  
  if( ! fin.read((char*)&dims[0],3*sizeof(int)) ){
    perror("read");
    return false;
  }
  int size = dims[0] * dims[1] * dims[2]; // RGBA
  cout<<dims[0]<<"x"<<dims[1]<<"x"<<dims[2]<<"="<<size<<endl;

  unsigned char* pix = new unsigned char[size];
  assert(pix);

  if( ! fin.read((char*)pix,size*sizeof(unsigned char)) ){
    perror("read");
    return false;
  }
  fin.close();  
  rgba.push_back( pix );

  if( needsize ){
    PW=dims[0];
    PH=dims[1];
    needsize=false;
  } else {
    if( PW!=dims[0] || PH!=dims[1] )
      return false;
  }

  fname = name + ".z";
  cout << "=== " << fname << " ===" << endl;
  fin.open(fname.c_str());
  if( ! fin ){
    perror("open");
    return false;
  }

  if( ! fin.read((char*)&dims[0],2*sizeof(int)) ){
    perror("read");
    return false;
  }
  size = dims[0] * dims[1]; // Z COMPONENT
  cout<<dims[0]<<"x"<<dims[1]<<"="<<size<<endl;

  if( PW!=dims[0] || PH!=dims[1] ){
    return false;
  }

  float* z = new float[size];
  assert(z);

  if( ! fin.read((char*)z,size*sizeof(float)) ){
    perror("read");
    return false;
  }
  fin.close();  
  depth.push_back( z );

  return true;
}
/*
  stencil function and stencil operation

  stencil function compares a target stencil buffer value to a reference value

  stencil operation specifies what is done with the target stencil value 
                    when the stencil test passes or fails

*/

void draw(glx* env, void*)
{
  int w = env->winWidth();
  int h = env->winHeight();
  
  bool first=true;
  float depthbias=0.;

  glClearColor(0,0,0,0);
  glDepthFunc(GL_LESS);   
  //glPixelTransferf(GL_DEPTH_BIAS, 1.); 
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

  std::vector< unsigned char* >::iterator piter = rgba.begin();
  std::vector< float* >::iterator ziter = depth.begin();
  for ( ; piter != rgba.end() ; ++piter, ++ziter ){

    glClear( GL_STENCIL_BUFFER_BIT ); 
    glEnable(GL_STENCIL_TEST);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glStencilFunc(GL_ALWAYS, 1, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glPixelTransferf(GL_DEPTH_BIAS, depthbias);    
    glDrawPixels(w, h, GL_DEPTH_COMPONENT, GL_FLOAT, *ziter);

    glPixelTransferf(GL_DEPTH_BIAS, 0.f);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilFunc(GL_EQUAL, 1, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glDisable(GL_DEPTH_TEST);
    
    glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, *piter);  
    
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
  }
}

int id=0;
void postdraw(glx* env, void*)
{
  _FANCYMESG("postdraw");
  int w = env->winWidth();
  int h = env->winHeight();
  int size = w * h;
  unsigned char* pixels = new unsigned char[size];
  glReadBuffer(GL_BACK);
  glReadPixels(0,0,w,h,GL_STENCIL_INDEX,GL_UNSIGNED_BYTE,pixels);
  for(int i=0;i<w*h;++i)
    pixels[i] *= 255;
  char buf[80];
  sprintf(buf,"stencil.%02d.ppm",id++);  
  ofstream fout(buf);
  fout << "P5 " << w << " " << h << " 255" << endl;
  fout.write((const char*)pixels, w*h);
  fout.close();
}

void initGL(glx* env, void* user)
{
  env->addDrawFunc(draw);
  env->addPostDrawFunc(postdraw);
  env->setSize(PW,PH);
  GLint stencilSize = 0;
  glGetIntegerv(GL_STENCIL_BITS,&stencilSize);
  _VAR(stencilSize);
}

int
main(int argc, char** argv)
{
  if( argc==1 ){
    cerr << "usage: load [image] [image]..." << endl;
    return 0;
  }
  
  while( optind < argc ){
    if( ! load( argv[optind] ) ){
      cerr << "unable to load file: " << argv[optind] << endl;
    }
    ++optind;
  }

  glx* env = new glx(initGL);
  env->mainLoop();

  return 0;
}
