#include <iostream>
#include <Xm/BulletinB.h>
#include <GL/GLwDrawA.h>
#include <GL/glu.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
using namespace std;

// g++ -c glxAtomic.C

// OK:
// g++ -o glxAtomic glxAtomic.o -lgen -lGL -lGLw -lX11 -lXt -lXm -lm

// BOMBS:
// g++ -o glxAtomic glxAtomic.o -lgen -lGL -lGLU -lGLw -lX11 -lXt -lXm -lm

//////////////////////////////////////////////////////////////////////////
///////////////////////////////// TYPES //////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct wakeupData {
  Display* display;
  Widget widget;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////// GLOBALS /////////////////////////////////
//////////////////////////////////////////////////////////////////////////

pthread_mutex_t printLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t workLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t needsWork = PTHREAD_COND_INITIALIZER;

pthread_mutex_t doneLock = PTHREAD_MUTEX_INITIALIZER;
int done=0;

pthread_mutex_t haltLock = PTHREAD_MUTEX_INITIALIZER;
int halted=0;

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

void xWakeup(wakeupData& d)
{
  XExposeEvent ev;
  ev.type = Expose; 
  ev.send_event = True;
  ev.display = d.display;
  ev.window = XtWindow(d.widget);
  ev.x = 0;
  ev.y = 0;
  ev.width = 1;
  ev.height = 2;
  ev.count = 0;
  Status status = 
    XSendEvent(d.display, XtWindow(d.widget),False,NoEventMask,(XEvent*)&ev);
  cout << "Status = " << status << endl;
  XFlush(d.display);
}

void *
work(void * arg)
{
  wakeupData* d = static_cast<wakeupData*>(arg);

  while( ! isDone() ){
    int res = pthread_cond_wait(&needsWork,&workLock);
    if( res != 0 ){
      cerr << "pthread_cond_wait() failed ["<<res<<"]!"<<endl;
      return 0;
    }
    for(int i=0 ; i<5 && ! isDone() && ! isHalted() ; i++ ){
      pthread_mutex_lock(&printLock);
      cout << "work["<<i<<"] sending X event!"<<endl;
      pthread_mutex_unlock(&printLock);
      xWakeup(*d);
      sleep(1);
    }
    if( isHalted() ){
      clearHalted();
    }
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////
///////////////////////////////// X CODE /////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class glx {
public:

  glx(void) {
    int argc=0;
    char** argv=0;
    int hRes=500,wRes=500;
    mode=0;

    XtToolkitInitialize();
    assert( XtToolkitThreadInitialize() );
    itsApp = XtCreateApplicationContext();
    itsDisplay = XtOpenDisplay(itsApp,0,"a","a", NULL,0, &argc, argv);
    
    Widget itsWindow = 
      XtVaAppCreateShell("top", "top", topLevelShellWidgetClass, itsDisplay,
			 XmNwidth, wRes, XmNheight, hRes, NULL);

    XVisualInfo *itsVisual = getVisual();
    
    itsGLXarea = 
      XtVaCreateManagedWidget("glx", glwDrawingAreaWidgetClass, 
			      itsWindow, XmNwidth, wRes, XmNheight, hRes,
			      GLwNvisualInfo, itsVisual, NULL); 

    XtAddCallback(itsGLXarea, XmNexposeCallback, glx::expose, this);    
    XtAddEventHandler(itsGLXarea, ButtonPressMask, False, glx::btnDown, NULL);

    itsGLXcontext = glXCreateContext(itsDisplay,itsVisual,None,GL_TRUE);
    XtRealizeWidget(itsWindow);
    XtMapWidget(itsWindow);
  }
  
  void mainLoop(void){
    XtAppMainLoop(itsApp);
  }

  static void 
  btnDown(Widget, XtPointer clientData, XEvent *, Boolean* _continue)
  {
    glx* _this = static_cast<glx*>(clientData);
    pthread_cond_broadcast(&needsWork);
    *_continue = TRUE;
  }

  XVisualInfo *getVisual(void) {
    XVisualInfo *vis;
    int list[32];
    int n = 0; 
    list[n++] = GLX_DOUBLEBUFFER;
    list[n++] = GLX_RGBA;
    list[n++] = (int) None;
    vis = glXChooseVisual(itsDisplay, DefaultScreen(itsDisplay), list);
    assert(vis);
    return( vis );
  }

  void draw_scene(void){
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glBegin(GL_TRIANGLES);
    switch( mode ){
      case 0:
	glColor3f(1.0,0.0,0.0); glVertex3f(1.0,0.0,0.0);
	glColor3f(0.0,1.0,0.0); glVertex3f(0.0,1.0,0.0);
	glColor3f(0.0,0.0,1.0); glVertex3f(0.0,0.0,1.0);
	mode=1;
	break;
      case 1:
	glColor3f(0.0,1.0,0.0); glVertex3f(1.0,0.0,0.0);
	glColor3f(0.0,0.0,1.0); glVertex3f(0.0,1.0,0.0);
	glColor3f(1.0,0.0,0.0); glVertex3f(0.0,0.0,1.0);
	mode=2;
	break;
      case 2:
	glColor3f(0.0,0.0,1.0); glVertex3f(1.0,0.0,0.0);
	glColor3f(1.0,0.0,0.0); glVertex3f(0.0,1.0,0.0);
	glColor3f(0.0,1.0,0.0); glVertex3f(0.0,0.0,1.0);
	mode=0;
	break;
    }
    glEnd();
  }

  static void expose(Widget w, XtPointer clientData, XtPointer callData){
    glx* thisGLX = (glx*)clientData;
    glXMakeCurrent(thisGLX->itsDisplay,XtWindow(w),thisGLX->itsGLXcontext);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    thisGLX->draw_scene();
    glFlush();
    glXSwapBuffers(thisGLX->itsDisplay, XtWindow(w));
  }

  XtAppContext itsApp;
  Display* itsDisplay;
  GLXContext itsGLXcontext;
  Widget itsGLXarea;
  int mode;
};

main()
{
  glx* g = new glx(); 

  pthread_t threadID;
  wakeupData* d = new wakeupData;
  d->display = g->itsDisplay;
  d->widget = g->itsGLXarea;
  int res = pthread_create(&threadID,0,work,d);
  if( res != 0 ){
    cerr << "pthread_create() failed ["<<res<<"]!" << endl;
    return 0;
  }

  g->mainLoop();
}
