#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iterator>
#include <GLX.h>
#include <glxTrackpad.h>
#include <Draggers/WorldPalette.h>
#include "debug.h"

using namespace std;

int yLen=3;
int xLen=3;
int COLPARAM,ROWPARAM;

double H=0.1;
double W=0.16;
double F=W/50.;
int xoff=0,yoff=0,dr=1,dc=1;

int nparams;
int nvars;
vector<int> psize;
vector<int> pcoord;
vector<string> params;
vector<string> depvars;

vector< vector<string> > parvals;

void drawQuad(int i,int j, double z=0.)
{
  glBegin(GL_LINE_LOOP);
  glVertex3d(i*W+F,j*H+F,z);
  glVertex3d(i*W+W-F,j*H+F,z);
  glVertex3d(i*W+W-F,j*H+H-F,z);
  glVertex3d(i*W+F,j*H+H-F,z);
  glEnd();
}

void draw(glx* env, void*)
{
  glColor3f(0.2,0.2,0.5);
  for(int i=0;i<psize[COLPARAM];++i){
    for(int j=0;j<psize[ROWPARAM];++j){
      drawQuad(i,j);
    }
  }
  
  glColor3f(1,1,1);
  for(int c=0;c<xLen;++c){
    for(int r=0;r<yLen;++r){
      drawQuad(xoff+c*dc,yoff+r*dr,0.1);
    }
  }
}

void drag(Glx::WorldPalette* wp, double x, double y, double w, double h, void*)
{
  int c = (int)round(w/W);
  int r = (int)round(h/H);
  if( r<yLen ) r=yLen;
  if( c<xLen ) c=xLen;
  dr = r/yLen;
  dc = c/xLen;
  xoff = (int)round(x/W);
  yoff = (int)round(y/H);

  for(int c=0;c<xLen;++c){
    int xpos = xoff+c*dc;
    if( xpos<0||xpos>=psize[COLPARAM] ) continue;
    for(int r=0;r<yLen;++r){
      int ypos = yoff+r*dr;
      if( ypos<0||ypos>=psize[ROWPARAM] ) continue;
      printf("LOAD %03dx%03d ",c,r);
      for(int p=0;p<pcoord.size();++p){
	if( p==COLPARAM )
	  printf("%d ",xoff+c*dc);
	else if(p==ROWPARAM )
	  printf("%d ",yoff+r*dr);
	else
	  printf("%d ",pcoord[p]);
      }
      printf("\n");
    }
  }
  
}

void initGL(glx* env, void* user)
{
  double lo[2]={0.,0.},hi[2]={psize[COLPARAM]*W,psize[ROWPARAM]*H};
  env->addDrawFunc(draw);
  Glx::Trackpad* tb = new Glx::Trackpad(env);
  tb->itsTranslationSensitivity=1.;
  tb->viewAll(lo,hi);
  Glx::WorldPalette* wp = new Glx::WorldPalette(env,0,0,W*xLen,H*yLen);  
  wp->setSnap(W,H);
  wp->setBold(true);
  wp->setDragCallback(drag);
}

int main(int argc, char** argv)
{
  if( argc!=2 ){
    cerr << "usage: pan MxN" << endl;
    return 0;
  }
  sscanf(argv[1],"%dx%d",&xLen,&yLen);

  _VAR2(xLen,yLen);

  H=1./yLen;
  W=1./xLen;
  F=W/50.;

  cin >> nparams;
  VAR(nparams);

  psize.resize(nparams);
  pcoord.resize(nparams);
  pcoord.assign(nparams,0);
  params.resize(nparams);
  parvals.resize(nparams);

  for (int i=0;i<nparams;++i){ 
    char buf[80];   
    cin.get(buf,32,'"');
    cin.clear();
    cin.get();
    cin.get(buf,32,'"');
    cin.get();
    params[i]=buf;
    cin >> psize[i];
    VAR2(params[i],psize[i]);
    vector<string>& pv = parvals[i];
    for (int j=0;j<psize[i];++j){
      string p;
      cin>>p;
      pv.push_back( p );
      VAR(p);
    }
  }
  cin >> nvars;
  VAR(nvars);
  depvars.resize(nvars);
  for (int i=0;i<nvars;++i){
    char buf[80];
    cin.get(buf,32,'"');
    cin.clear();
    cin.get();
    cin.get(buf,32,'"');
    cin.get();
    depvars[i] = buf;
    VAR(depvars[i]);
  }
#ifdef DEBUG 
  cout << "====PARAMS["<<nparams<<"]===="<<endl;
  for(int i=0;i<params.size();++i){
    cout << "["<<i<<"]: "<<params[i]<<endl;
  }
  for(int i=0;i<params.size();++i){
    vector<string>& pv = parvals[i];
    cout << "\t==== [ "<<params[i]<<" ] ====" << endl;
    for(int j=0;j<pv.size();++j){
      cout << "\t\t["<<j<<"]: "<<pv[j]<<endl;
    }    
  }
#endif

  ROWPARAM=0;
  COLPARAM=1;

  glx* env = new glx(initGL);
  env->showAxis(false);
  env->mainLoop();
  return 1;
}
