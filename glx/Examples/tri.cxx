
//
// code reads and displays a CART3D .tri file
//

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
#include "debug.h"
#include <GLX.h>
#include <glxTrackball.h>
using namespace std;

const int DIM=3;

double gmin[3]={MAXFLOAT,MAXFLOAT,MAXFLOAT};
double gmax[3]={-MAXFLOAT,-MAXFLOAT,-MAXFLOAT};
vector<double> nodes;
vector<int> indices;
int nnodes,ntris;

void draw(glx* env, void*)
{
  MESGVAR("draw",indices.size());
  vector<int>::iterator iter = indices.begin();
  while( iter != indices.end() ){
    int i0=*iter-1;++iter;
    int i1=*iter-1;++iter;
    int i2=*iter-1;++iter;
    VAR3(i0,i1,i2);
    glColor3f(1,1,0);
    glBegin(GL_LINE_LOOP);
    glVertex3dv( &nodes[i0*DIM] );
    glVertex3dv( &nodes[i1*DIM] );
    glVertex3dv( &nodes[i2*DIM] );
    glEnd();
  }

}

void initGL(glx* env, void* user)
{
  env->addDrawFunc(draw);
  Glx::Trackball* tb = new Glx::Trackball(env);
  tb->viewAll(gmin,gmax);
}

int
main(int argc, char** argv)
{
  ifstream fin(argv[1]);
  if( ! fin ){
    cerr << "usage: tri [.tri file]"<<endl;
    return 1;
  }
  fin>>nnodes>>ntris;

  VAR2(nnodes,ntris);

  for(int i=0;i<nnodes;++i){
    double x,y,z;
    fin>>x>>y>>z;
    nodes.push_back(x);
    nodes.push_back(y);
    nodes.push_back(z);
    if( x<gmin[0] )gmin[0]=x;
    if( x>gmax[0] )gmax[0]=x;
    if( y<gmin[1] )gmin[1]=y;
    if( y>gmax[1] )gmax[1]=y;
    if( z<gmin[2] )gmin[2]=z;
    if( z>gmax[2] )gmax[2]=z;
    VAR3(x,y,z);
    //cout << x << " " << y << " " << z << endl;
  }
  for(int i=0;i<ntris;++i){
    int u,v,w;
    fin>>u>>v>>w;
    indices.push_back(u);
    indices.push_back(v);
    indices.push_back(w);    
  }
  
  fin.close();
  VAR3V(gmin);
  VAR3V(gmax);

  glx* env = new glx(initGL);
  env->mainLoop();

  return 0;
}
