#include <iostream>
#include <fstream>
#include <sstream>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <pthread.h>
#include <signal.h>
#include <assert.h>
//#define DEBUG 1
#include "pdebug.h"
#include "GLX.h"
#include "glxTrackball.h"
#include "DB.h"

using namespace std;

enum {WINX,WINY,WINW,WINH,WINCOORDS};
bool allDone=false;
pthread_mutex_t workLock = PTHREAD_MUTEX_INITIALIZER;

struct Hold {
  pthread_mutex_t *lock;
  Hold(pthread_mutex_t *l) : lock(l){pthread_mutex_lock(lock);}
  ~Hold(void){pthread_mutex_unlock(lock);}
};

struct ThreadData {
  ThreadData(int c[WINCOORDS]){setCoords(c);}
  void setCoords(int c[WINCOORDS]){memcpy(coords,c,WINCOORDS*sizeof(int));}
  int coords[WINCOORDS];
};

struct VisThread {
  VisThread(ThreadData* threadData) : 
    env(0),tid(0),id(++nThreads),redraws(0),lockcnt(0),toid(0)
  {
    setCoords(threadData->coords);
    assert( pthread_mutex_init(&objlock, NULL) ==0 );
  }
  ~VisThread(void){ 
    delete env;
    if( pthread_mutex_destroy(&objlock) != 0 )
      perror("pthread_mutex_destroy");
    XtRemoveTimeOut(toid);
  }
  
  void allocTimeout(void){
    toid=XtAppAddTimeOut(env->getApp(), 500, VisThread::timeout, this);
  }
  static void timeout(XtPointer user,XtIntervalId*){
    VisThread* _this = static_cast<VisThread*>(user);
    _this->toid=XtAppAddTimeOut(_this->env->getApp(), 500, 
				VisThread::timeout, _this);
  }
  
  void setCoords(int c[WINCOORDS]){memcpy(coords,c,WINCOORDS*sizeof(int));} 
  void lock(void){pthread_mutex_lock(&objlock);}
  void unlock(void){pthread_mutex_unlock(&objlock);}

  glx* env;
  pthread_t tid;
  int id;
  int coords[WINCOORDS];
  static int nThreads;
  int redraws;
  pthread_mutex_t objlock;
  int lockcnt;
  XtIntervalId toid;
private:
  VisThread(VisThread&){}
  VisThread& operator=(VisThread&){}
};

DB::Mgr mgr(".mt");
int VisThread::nThreads=0;
vector<VisThread*> threads;

ostream& operator<<(ostream& ostr, vector<VisThread*>& v)
{
  ostr << "\n=== [" << v.size() << " elements ] ===" << endl;
  for(vector<VisThread*>::iterator i=v.begin();i!=v.end();++i){
    ostr << hex << (void*)*i << dec << endl;
  }
  return ostr;
}

void setDone(void)
{
  FANCYMESG("setDone [start]");
  Hold h(&workLock);
  allDone=true;
  vector<VisThread*>::iterator i=threads.begin();
  for(;i!=threads.end();++i){
    VisThread* vt = *i;
    vt->env->setDone();
  } 
  FANCYMESG("setDone [ end ]");
}

void 
tokenize(string input, vector<string>& tokens, string sep=" \t\n")
{
  string cur = input;
  int done=0;
  tokens.erase(tokens.begin(),tokens.end());
  while( ! done ){
    int start = cur.find_first_not_of(sep);
    int end = cur.find_first_of(sep,start+1);
    if( start == -1 || end == -1 ){
      if( start != -1 )
        tokens.push_back( string( cur, start ) );
      return;
    }
    tokens.push_back( string( cur, start, end-start ) );
    cur = string(cur, end+1);
  }
}

void 
getCoords(Display *dpy, Window win,
	  int* x,int* y, int* w, int* h,int lvl=0)
{
  Window root,par,*children;
  int _x,_y;
  unsigned int nChildren,_w,_h,b,d;
  XQueryTree(dpy,win,&root,&par,&children,&nChildren);
  XGetGeometry(dpy,win,&root,&_x,&_y,&_w,&_h,&b,&d);
  if(lvl==0) {*w=_w;*h=_h;}
  if( root==par ){*x=_x;*y=_y;return;}
  getCoords(dpy,par,x,y,w,h,lvl+1);
}

void getConfig(int coords[WINCOORDS])
{
  string key("MT");
  vector<string> lines,tokens;
  if( ! mgr.get("MT",lines) ) return;
  tokenize(lines[0],tokens);
  for(int i=WINX;i<WINCOORDS;++i)
    coords[i]=atoi(tokens[i].c_str());
}

void saveConfig(Display *dpy, Window win)
{
  string key("MT");
  int pos[WINCOORDS];
  getCoords(dpy,win,&pos[WINX],&pos[WINY],&pos[WINW],&pos[WINH]);
  ostringstream ostr;
  ostr<<pos[WINX]<<" "<<pos[WINY]<<" "<<pos[WINW]<<" "<<pos[WINH];
  string value = ostr.str();

  mgr.clear(key);
  mgr.set(key,value);

  key = string("WIN");
  mgr.clear(key);
  vector<VisThread*>::iterator i=threads.begin();
  for(int index=0;i!=threads.end();++i,++index){    
    glx* env = (*i)->env; 
    if(!env)continue;
    env->getCoords(&pos[WINX],&pos[WINY],&pos[WINW],&pos[WINH]);
    ostr.str("");
    ostr<<pos[WINX]<<" "<<pos[WINY]<<" "<<pos[WINW]<<" "<<pos[WINH];
    string value = ostr.str();
    mgr.set(key,value);
    
  }
}

void quitCB(Widget, XtPointer clientData, XtPointer)
{
  Widget parent = static_cast<Widget>(clientData);
  saveConfig(XtDisplay(parent),XtWindow(parent));
  setDone();
}

const float MAX=3.0;
const float MIN=-MAX;
const float INC=(MAX-MIN)/100.0f;
float f(float u, float v)
{
  //return exp( sin(u*v) - cos(3.0f*u + 2.0f*v)) / 10.0f;
  //return sin(v) * cos(u) - 1.0f/sin(v) + 1.0f/cos(u);
  return sin(u*v);
}

void cross(float v1[3], float v2[3], float res[3])
{
  res[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
  res[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
  res[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
}

void
sendColor(float val, float min, float max)
{
  const float LVL_0 = 0;
  const float LVL_1 = 0.556863;
  const float LVL_2 = 0.929412;
  const float LVL_3 = 0.992157;
  float percent = (val-min)/(max-min);
  unsigned char rgb[3];
  if( percent <= LVL_1 ){
    float t = (percent-LVL_0)/0.556863;
    if( t>1 )t=1;
    if( t<0 )t=0;
    rgb[0] = (unsigned char)(255.0*(0 + t*1));
    rgb[1] = (unsigned char)(255.0*(0 + t*0.0901961));
    rgb[2] = 0;
  } else if( percent <= LVL_2 ){
    float t = (percent-LVL_1)/0.372549;
    if( t>1 )t=1;
    if( t<0 )t=0;
    rgb[0] = 255;
    rgb[1] = (unsigned char)(255.0*(0.0901961 + t*0.831373));
    rgb[2] = 0;
  } else {
    float t = (percent-LVL_2)/0.0627451;
    if( t>1 )t=1;
    if( t<0 )t=0;
    rgb[0] = 255;
    rgb[1] = (unsigned char)(255.0*(0.921569 + t*0.0784314));
    rgb[2] = (unsigned char)(255.0*(0 + t * 1));
  }
  glColor3ubv(rgb);
}

void sendNormal(float u, float v)
{
  float delta = INC/20.0f;
  float o[3] = {u,v,f(u,v)};
  float l[3] = {u+delta,v,f(u+delta,v)};
  float r[3] = {u,v+delta,f(u,v+delta)};
  float left[3] = {l[0]-o[0],l[1]-o[1],l[2]-o[2]};
  float right[3] = {r[0]- o[0],r[1]- o[1],r[2]- o[2]};
  float forward[3];

  cross(right,left,forward);

  float lb[3] = {u-delta,v,f(u-delta,v)};
  float rb[3] = {u,v-delta,f(u,v-delta)};
  float leftb[3] = {lb[0] - o[0],lb[1] - o[1],lb[2] - o[2]};
  float rightb[3] = {rb[0] - o[0],lb[1] - o[1],lb[2] - o[2]};
  float back[3];

  cross(rightb,leftb,back);

  float ave[3] = {-1.0f * (forward[0] + back[0]),
                  -1.0f * (forward[1] + back[1]),
                  -1.0f * (forward[2] + back[2])
  };
  float mag = sqrt( ave[0]*ave[0] + ave[1]*ave[1] + ave[2]*ave[2] );
  ave[0] /= mag;
  ave[1] /= mag;
  ave[2] /= mag;
  glNormal3fv(ave);
}

void draw(glx* env, void* user)
{
  VisThread* vt = static_cast<VisThread*>(user);
  vt->redraws++;
  
  int N=100;
  float t = (vt->redraws % N) /(float)(N-1);
  for(float u=MIN;u<=MAX;u+=INC){
    glBegin(GL_TRIANGLE_STRIP);
    for(float v=MIN;v<=MAX;v+=INC){
      float z = t*f(u,v);
      sendColor(z,-1,1);
      sendNormal(u,v);
      glVertex3f(u,v,z);
      
      float zNext = t*f(u+INC,v);
      sendColor(zNext,-1,1);
      sendNormal(u+INC,v);
      glVertex3f(u+INC,v,zNext);
    }
    glEnd();
  }

  ostringstream ostr;
  ostr << "Redraws:"<<vt->redraws;
  env->showMessage(ostr.str());
}

void* visThread(void* user)
{
  _FANCYMESG("\tvisThread() [start]");

  double gmin[3]={MIN,MIN,f(MIN,MIN)};
  double gmax[3]={MAX,MAX,f(MAX,MAX)};

  VisThread* vt = static_cast<VisThread*>(user);
  vt->env = new glx();
  Glx::Trackball* tb = new Glx::Trackball(vt->env);
  tb->viewAll(gmin,gmax);
  vt->env->setPosition(vt->coords[WINX],vt->coords[WINY]);
  vt->env->setSize(vt->coords[WINW],vt->coords[WINH]);
  vt->env->addDrawFunc(draw,vt);
  vt->allocTimeout();
  vt->env->mainLoop();
  delete vt->env;
  vt->env=0;

  _FANCYMESG("\tvisThread() [ end ]");

  return (NULL);
}

void newCB(Widget, XtPointer user, XtPointer)
{
  Hold h(&workLock);
  ThreadData* threadData = static_cast<ThreadData*>(user);
  VisThread* vt = new VisThread(threadData);
  pthread_create(&vt->tid,0,visThread,vt);
  threads.push_back(vt);
}

void checkThreads(XtPointer user,XtIntervalId*)
{
  FANCYMESG("checkThreads [start]");
  Hold h(&workLock);
  XtAppContext app = static_cast<XtAppContext>(user);
  vector<VisThread*> alive=threads;
  threads.clear();
  
  vector<VisThread*>::iterator titer=alive.begin();
  for(int i=0 ; titer != alive.end() ; ++titer,++i ){
    VisThread* vt = *titer;    

    int res = pthread_kill(vt->tid,0);
    if( res == ESRCH ){
      FANCYMESG("thread ended");
      MESGVAR("checkThreads: joining",vt->tid);
      if( pthread_join(vt->tid,NULL) != 0 )
	perror("pthread_join");
    } else {
      threads.push_back(vt);      
    }
  }
  XtAppAddTimeOut(app,500,checkThreads,user);

  MESGVAR("checkThreads [ end ]",threads.size());
  FANCYMESG("checkThreads [ end ]");
}

void 
mainLoop(XtAppContext app, Display* dpy, Window win, int defcoords[WINCOORDS])
{
  vector<string> lines;
  int pos[WINCOORDS];
  XEvent xEvent;

  ThreadData* threadData=new ThreadData(defcoords);

  if( mgr.get("WIN",lines) ){
    for(int i=0;i<lines.size();++i){
      vector<string> tokens;
      tokenize(lines[i],tokens);
      for(int j=WINX;j<WINCOORDS;++j) 
	pos[j]=atoi(tokens[j].c_str()); 
      threadData->setCoords(pos);
      newCB(NULL,threadData,NULL);
      usleep(100000);
    }
  }
  XtAppAddTimeOut(app,500,checkThreads,app);
  
  while( ! allDone ){
    XtAppNextEvent(app, &xEvent); 
    XtDispatchEvent(&xEvent);
  }
  FANCYMESG("mainLoop [ end ]");
}

int
main(int argc, char** argv)
{
  int defcoords[WINCOORDS]={10,10,300,300};
  assert( XInitThreads() );
  XtToolkitInitialize();
  assert( XtToolkitThreadInitialize() );

  getConfig(defcoords);

  XtAppContext itsApp = XtCreateApplicationContext();  
  Display* itsDisplay = XtOpenDisplay(itsApp,(char*)NULL,
				      "MT", "MT", 
				      NULL,0,&argc, argv); 
  Widget itsWin = XtVaAppCreateShell("MT", "MT",
				     topLevelShellWidgetClass, itsDisplay, 
				     XmNx, defcoords[WINX], 
				     XmNy, defcoords[WINY],
				     XmNwidth, defcoords[WINW], 
				     XmNheight, defcoords[WINH],
				     NULL); 

  Widget itsBB = XtVaCreateManagedWidget("bb", xmBulletinBoardWidgetClass,
					 itsWin, XmNtraversalOn, True, NULL); 
 
  Widget quitButton = XtVaCreateManagedWidget("Quit",
					     xmPushButtonWidgetClass, 
					     itsBB, 
					     XmNx, 10, XmNy, 10,
					     XmNwidth, 60, XmNheight, 30,
					     NULL);
  XtAddCallback(quitButton, XmNactivateCallback, quitCB, itsWin);

  Widget newButton = XtVaCreateManagedWidget("New",
					     xmPushButtonWidgetClass, 
					     itsBB, 
					     XmNx, 10, XmNy, 60,
					     XmNwidth, 60, XmNheight, 30,
					     NULL);
  ThreadData* threadData=new ThreadData(defcoords);
  XtAddCallback(newButton, XmNactivateCallback, newCB, threadData);

  XtRealizeWidget(itsWin);
  XtMapWidget(itsWin);

  mainLoop(itsApp,itsDisplay,XtWindow(itsWin),defcoords);
  FANCYMESG("main() [ back from mainLoop ]");

  vector<VisThread*>::iterator titer=threads.begin();
  for( ; titer != threads.end() ; ++titer ){
    VisThread* vt = *titer;
    MESGVAR("checkThreads: joining",vt->tid);
    if( pthread_join(vt->tid,NULL) != 0 )
      perror("pthread_join");
  }

  FANCYMESG("main() [ end ]");
}
