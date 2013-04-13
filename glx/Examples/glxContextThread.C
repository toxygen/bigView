#include <iostream>
#include <assert.h>
#include <stdlib.h>
#include <values.h>
#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <math.h>
#include <X11/keysym.h>
#include <values.h>
#include <errno.h>
#include <GLX.h>
#include "pdebug.h"
using namespace std;

// g++ -o glxContext -I/usr/X11R6/include glxContext.C -L/usr/X11R6/lib 
//    -lGL -lGLw -lX11 -lXt -lXm -lm

//////////////////////////////////////////////////////////////////////////
//////////////////////////////// GLOBALS /////////////////////////////////
//////////////////////////////////////////////////////////////////////////

GLuint itsObjID=MAXINT;

// Condition variable by which the main thread starts the build
pthread_mutex_t workLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  needsWork = PTHREAD_COND_INITIALIZER;

// Condition var by which the build thread informs the
// main thread that has a valid context to create display
// list objects. Main thread cannot begin until this is true
pthread_mutex_t contextLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  threadHasContext = PTHREAD_COND_INITIALIZER;

// overall completion flag, set when main thread exits
pthread_mutex_t doneLock = PTHREAD_MUTEX_INITIALIZER;
int done=0;

// flag used when build thread has completed something
// to be consumed by the main thread. Build thread will
// not continue until this flag has been cleared
pthread_mutex_t readyLock = PTHREAD_MUTEX_INITIALIZER;
int isReady=0;

// flag used to signal the build thread to stop the build
// partial results of the build are thrown away
pthread_mutex_t haltLock = PTHREAD_MUTEX_INITIALIZER;
int halted=0;

int dumpingFrames=0;

//////////////////////////////////////////////////////////////////////////
////////////////////////////// THREAD CODE ///////////////////////////////
//////////////////////////////////////////////////////////////////////////

int isDone()
{
  int res=0;
  pthread_mutex_lock(&doneLock);
  res = done;
  pthread_mutex_unlock(&doneLock);
  return res;
}

void setDone()
{
  pthread_mutex_lock(&doneLock);
  done = 1;
  pthread_mutex_unlock(&doneLock);
}

int isHalted()
{
  int res=0;
  pthread_mutex_lock(&haltLock);
  res = halted;
  pthread_mutex_unlock(&haltLock);
  return res;
}

void clearHalted()
{
  pthread_mutex_lock(&haltLock);
  halted = 0;
  pthread_mutex_unlock(&haltLock);
}

void halt()
{
  pthread_mutex_lock(&haltLock);
  halted = 1;
  pthread_mutex_unlock(&haltLock);
}

void
mesg(string message)
{
  _MESG(message);
}

const float MAX=3.0;
const float MIN=-MAX;
const float INC=(MAX-MIN)/400.0f;
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

void *
produce(void * arg)
{
  glx* g = static_cast<glx*>(arg);
  int mode=0;

  cout << "===== work =====" << endl;

  glXMakeCurrent(g->getDisplay(),
		 XtWindow(g->getGlx()),
		 g->getGlobalContext());

  pthread_cond_broadcast(&threadHasContext);

  while( ! isDone() ){

    pthread_mutex_lock(&workLock);
    int res = pthread_cond_wait(&needsWork,&workLock);
    if( res != 0 ){
      cerr << "pthread_cond_wait() failed ["<<res<<"]!"<<endl;
      return 0;
    }
    
    cout << "starting work..." << endl;
    int N=10;
    int i=0;
    while( i<10 && ! isDone() && ! isHalted() ) {

      pthread_mutex_lock(&readyLock);
      if( isReady==0 )
      {
	float t=i/(float)(N-1);

	mesg("\t\t===producing ===");
	GLenum err;
	if( itsObjID==MAXINT) {
	  itsObjID = glGenLists(1);
	  cout << "itsObjID = " << itsObjID << endl;
	  err = glGetError();
	  assert(err==GL_NO_ERROR);
	}
	glNewList(itsObjID, GL_COMPILE);
	err = glGetError();
	assert(err==GL_NO_ERROR);

	GLenum type = GL_TRIANGLE_STRIP;
	for(float u=MIN;u<=MAX;u+=INC){
	  glBegin(type);
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
	glEndList();
	g->wakeup();
	++i;
	isReady=1;
      }
      pthread_mutex_unlock(&readyLock);
    } // for i=... 
    dumpingFrames=0;
    pthread_mutex_unlock(&workLock);
    if( isHalted() ){
      clearHalted();
    }
  }
  return 0;
}

void doWork(glx*,XEvent *event,void*)
{
  KeySym ks;
  XComposeStatus status;
  char buffer[20];
  int buf_len = 20;
  XKeyEvent *kep = (XKeyEvent *)(event);
  XLookupString(kep, buffer, buf_len, &ks, &status);
  dumpingFrames=0;
  if( ks==XK_q )
    exit(0);
  else if( ks==XK_h )
    halt();
  else if( ks==XK_d ){
    dumpingFrames=1;
    pthread_cond_signal(&needsWork);
  } else
    pthread_cond_signal(&needsWork);
}

void draw(glx* g,void*)
{
  if( itsObjID==MAXINT )
    return;
  glDisable(GL_LIGHTING);
  glCallList(itsObjID);
  int res = pthread_mutex_trylock(&readyLock);
  if( res != EBUSY ){
    if( isReady == 1 ){
      mesg("\t\t=== consuming ===");
      isReady=0;
      if( dumpingFrames )
	g->dumpImage();
    }
    pthread_mutex_unlock(&readyLock);
  } else {
    g->wakeup();
  }
}

int
main(int argc, char** argv)
{
  XtToolkitInitialize();
  assert( XtToolkitThreadInitialize() );

  XtAppContext app = XtCreateApplicationContext();
  Display* display = XtOpenDisplay(app,0,"a","a", NULL,0, &argc, argv);

  glx* g = new glx(app,display);
  g->setPosition(10,10);
  g->addEventFunc(doWork);
  g->addDrawFunc(draw);

  pthread_t producerID;
  int res = pthread_create(&producerID,0,produce,g);
  if( res != 0 ){
    cerr << "pthread_create() failed ["<<res<<"]!" << endl;
    return 0;
  }

  pthread_cond_wait(&threadHasContext,&contextLock);
  g->mainLoop();
}
