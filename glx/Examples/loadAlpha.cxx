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

bool load(string fname)
{
  int dims[3];

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
  int size = dims[0] * dims[1] * dims[2]; // RGBA: dims[X,Y,4]
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
  return true;
}

void draw(glx* env, void*)
{
  int w = env->winWidth();
  int h = env->winHeight();
  
  bool first=true;
  float depthbias=0.;

  glClearColor(0,0,0,0);
  glDepthFunc(GL_LESS);   
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glDisable(GL_LIGHTING);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

  std::vector< unsigned char* >::iterator piter = rgba.begin();
  std::vector< float* >::iterator ziter = depth.begin();
  for ( ; piter != rgba.end() && first; ++piter, ++ziter ){
    glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, *piter);  
    first=false;
  }
}

void initGL(glx* env, void* user)
{
  env->addDrawFunc(draw);
  env->setSize(PW,PH);
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
