#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <GLX.h>
#include <glxTrackpad.h>
#include <X11/keysym.h>

#include "debug.h"
using namespace std;

static const float EPS = 1.0e-6;
const int ZONES=35;
int zones[ZONES] = {
  545360,969570,519552,263340,969570,519552,263340,
  969570,519552,263340,969570,519552,263340,969570,
  519552,263340,969570,519552,263340,1298160,1250080,
  692208,692208,692208,692208,692208,692208,692208,
  692208,692208,692208,692208,692208,692208,1122741};
enum Algo{FIRST_FIT,BEST_FIT,NUM_ALGO};
std::string algoNames[NUM_ALGO] = {"First fit","Best fit"};

Algo curAlgo = BEST_FIT;
int imin=0xffffffff,imax=0;
vector<vector<int> > nodes;
Glx::Trackpad* trackpad;

const int MAXCELLS = 2000000;
int selected=-1;

//#define DEBUG 1
#ifdef DEBUG 
#define MESG(m) cout<<"MESG:"<<m<<endl;
#define VAR(v) cout<<#v<<"="<<v<<endl;
#define MESGVAR(m,v) cout<<"MESG:"<<m<<" "<<#v<<" = "<<v<<endl;
#else
#define MESG(m)
#define VAR(v)
#define MESGVAR(m,v)
#endif

ostream& operator<<(ostream& ostr , vector<int>& v)
{
  vector<int>::iterator iter = v.begin();
  for( ; iter != v.end() ; ++iter ){
    ostr << *iter << " ";
  }
  return ostr;
}

ostream& operator<<(ostream& ostr , vector<vector<int> >& v)
{
  vector<vector<int> >::iterator iter = v.begin();
  int node=0;
  for( ; iter != v.end() ; ++iter ){
    vector<int>& vec = *iter;
    ostr << "["<<node++<<"]: " << *iter << endl;    
  }
  return ostr;
}

//////////////////////////////////////////////////////////////////////////
/////////////////////////////// FIRST FIT  ///////////////////////////////
//////////////////////////////////////////////////////////////////////////
// The First Fit algorithm places a new object in the leftmost bin
// that still has room.

int accumulate(vector<int>& node)
{
  FANCYMESG("accumulate");
  int sum=0;
  for(vector<int>::iterator iter=node.begin();iter!=node.end();++iter){
    VAR(*iter)
    sum += zones[ *iter ];
  }
  return sum;
}

bool firstFit(vector<vector<int> >& nodes, int index)
{
  FANCYMESG("firstFit");
  bool found=false;
  bool any=false;
  int cells = zones[index];

  for(int i=0; i<49 && !found ;++i){
    vector<int>& node = nodes[i];
    int sum = accumulate(node);
    any=true;
    if( sum+cells < MAXCELLS ){
      node.push_back(index);
      found=true;
    }
  }
  return found;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////// BEST FIT ////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// The Best Fit algorithm places a new object in the fullest bin that
// still has room.

int compare(const void *a, const void *b)
{
  int* aptr = (int*)a;
  int* bptr = (int*)b;
  return ( *aptr-*bptr );
}

bool bestFit(vector<vector<int> >& nodes, int index)
{
  FANCYMESG("bestFit");
  VAR(nodes.size());
  bool found=false;
  int cells = zones[index];
  int best=-1;
  int bestSize=0;
  
  for(int i=0; i<49 && !found ;++i){
    vector<int>& node = nodes[i];
    int sum = accumulate(node);
    if( sum+cells < MAXCELLS ){
      // still has room...
      if( sum > bestSize || best == -1 ){
	// fullest so far...
	best = i;
	bestSize=sum;
      }
    }
  }
  if( best != -1 ){
    vector<int>& node = nodes[best];
    node.push_back(index);
    found=true;
  }
  return found;
}

void runAlgo(Algo algo){
  FANCYMESG("runAlgo");
  VAR(algo);

  nodes.resize(49);
  for(int i=0;i<49;++i){
    nodes[i].clear();
  }
  VAR(nodes.size());
  int numFailed=0;
  int index=0;
  bool done=false;
  while( ! done ){
    
    bool placed;
    switch( algo ){
      case FIRST_FIT: 
	placed = firstFit(nodes,index++);
	break;
      case BEST_FIT: 
	placed = bestFit(nodes,index++);
	break;
    }
    
    if( ! placed ) 
      ++numFailed;
    else
      numFailed=0;
    if( numFailed==ZONES )
      done=true;
    if( index==ZONES ) index=0;
  }
  cout << "====================" << endl;
  for(int zone=0;zone<ZONES ; ++zone){
    cout << "ZONE["<<zone<<"] : ";
    for(int index=0;index<49;++index){
      vector<int>& node = nodes[index];
      vector<int>::iterator iter = find(node.begin(),node.end(),zone);
      if( iter != node.end() )
	cout << "n" << index << " ";
    }
    cout << endl;
  }
  cout << "====================" << endl;
  for(int index=0;index<49;++index){
    vector<int>& node = nodes[index];
    ostringstream ostr;
    cout << "n"<<index<<" : ";
    vector<int>::iterator iter = node.begin();
    for( ; iter != node.end(); ++iter ){
      cout << *iter <<" ";
    }
    double cells = accumulate(node);
    double gridBytesPerVertex = 12;
    double solnBytesPerVertex = 20;
    double timesteps=1200;
    double bytesPerCell = gridBytesPerVertex + solnBytesPerVertex;
    double totalBytes = bytesPerCell * cells * timesteps;
    double totalKBytes = (double)totalBytes/(double)1024;
    double totalMBytes = (double)totalKBytes/(double)1024;
    double totalGBytes = (double)totalMBytes/(double)1024;
    VAR(cells);
    VAR(bytesPerCell);
    VAR(totalBytes);
    VAR(totalKBytes);
    VAR(totalMBytes);
    VAR(totalGBytes);
    cout << ": ["<< accumulate(node)<<" cells = "<<totalGBytes<<" GB]"<<endl;
  }

}

void
genFast(float percent, float rgb[4])
{
  rgb[0]=percent;
  rgb[1]=percent;
  rgb[2]=1;
  rgb[3]=1.0f;
}

void draw(glx* env, void* user)
{
  float dx=(float)1./49.;
  for(int i=0;i<49;++i){
    float xlo=(float)i*dx;
    float xhi= xlo + (dx*0.9);
    float sum=0;
    vector<int>& node = nodes[i];
    vector<int>::iterator iter = node.begin();

    for( ; iter != node.end(); ++iter ){
      float rgb[3];
      int count=zones[*iter];
      float height = (float)count/(float)MAXCELLS;
      float t = (float)(count-imin)/(float)(imax-imin);
      genFast(t,rgb);
      glBegin(GL_QUADS);
      glColor3fv(rgb);
      glVertex2f(xlo,sum);
      glVertex2f(xlo,sum+height);
      glVertex2f(xhi,sum+height);
      glVertex2f(xhi,sum);
      glEnd();
      
      glLineWidth(1);
      glBegin(GL_LINE_LOOP);
      glColor3f(0,0,0);
      glVertex3f(xlo,sum,0.1);
      glVertex3f(xlo,sum+height,0.1);
      glVertex3f(xhi,sum+height,0.1);
      glVertex3f(xhi,sum,0.1);
      glEnd();
      
      if( i==selected ){
	glLineWidth(3);
	glBegin(GL_LINE_LOOP);
	glColor3f(0,0,0);
	glVertex3f(xlo,sum,0.15);
	glVertex3f(xlo,sum+height,0.15);
	glVertex3f(xhi,sum+height,0.15);
	glVertex3f(xhi,sum,0.15);
	glEnd();
	
      }
      sum += height;
    }
  }

  if( selected>=0 && selected<49 ){
    vector<int>& node = nodes[selected];
    ostringstream ostr;
    ostr << "node["<<selected<<"]: ";
    vector<int>::iterator iter = node.begin();
    for( ; iter != node.end(); ++iter ){
      ostr << *iter <<" ";
    }
    env->showMessage(ostr.str());
  }
  env->showMessage(algoNames[curAlgo]);
    
}

void
pixelToWorldCoords(glx* env,int x, int y, float* xf, float* yf)
{
  ostringstream ostr;
  int winh = env->winHeight();
  double px,py,pz;
  int viewport[4];
  double modelMatrix[16],projMatrix[16],inv[16];

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  env->applyProjections();
  glGetIntegerv(GL_VIEWPORT, viewport);
  glGetDoublev(GL_MODELVIEW_MATRIX,(double *)modelMatrix);
  glGetDoublev(GL_PROJECTION_MATRIX,(double *)projMatrix);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  env->unproject(x,winh-y,0,modelMatrix,projMatrix,viewport,&px,&py,&pz);
  *xf = px;
  *yf = py;
}

void idle(glx* env,int x,int y,void*)
{
  float fx,fy;
  pixelToWorldCoords(env,x,y,&fx,&fy);
  float dx=(float)1./49.;
  selected = (int)(fx/dx);
  env->wakeup();
}

void processKey(glx* env,XEvent *event,void*)
{
  FANCYMESG("processKey");
  KeySym ks = XLookupKeysym((XKeyEvent*)event,0);
  XKeyEvent *kep = (XKeyEvent *)(event);
  int ctlDown    = kep->state & ControlMask;
  int shiftDown  = kep->state & ShiftMask;
  int altDown    = kep->state & Mod1Mask;
  int algo=(int)curAlgo;
  switch( ks ){
    case XK_Up:
      algo = (algo+1) % NUM_ALGO;
      curAlgo = (Algo)algo;
      break;      
    case XK_Down:
      --algo;
      if( algo<0 ) algo=NUM_ALGO-1;
      curAlgo = (Algo)algo;
      break;      
  }
  MESGVAR("processKey",curAlgo);
  runAlgo(curAlgo);
  env->wakeup();
}

void initGL(glx* env,void*)
{
  for(int i=0;i<ZONES;++i){
    if(zones[i]<imin)imin=zones[i];
    if(zones[i]>imax)imax=zones[i];
  }
  runAlgo(curAlgo);
  trackpad = new Glx::Trackpad(env);
  trackpad->itsTranslationSensitivity=0.1;
  env->addDrawFunc(draw);
  env->addMouseIdleFunc(idle);
  env->addEventFunc(processKey);
}

int
main(int argc, char** argv)
{
  glx* env = new glx(initGL);
  env->mainLoop();
}
