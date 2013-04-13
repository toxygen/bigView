//////////////////////////////////////////////////////////////////////////
///////////////////////////////// GLX.h //////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifndef _GLX_H_
#define _GLX_H_

#include <X11/Intrinsic.h> // for XtAppContext
#include <X11/keysym.h>
#include <GL/glx.h>
#include <math.h>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <pthread.h>

#include <Draggers/Dragger.h> // for draggers
#include <Draggers/Dragger3D.h> // for 3D draggers
#include <glxBool.h>

namespace Glx {
  enum {FONT_SIZE = 15};
  enum {END_OF_LIST=0};
  enum {FONTENTRIES=96,FONTENTRYSIZE=14};
  enum {STIPPLESIZE=128};
  extern unsigned char stip[Glx::STIPPLESIZE];
  extern unsigned char bitmapFont[Glx::FONTENTRIES][Glx::FONTENTRYSIZE];
}

/**
 * class glx
 *
 * A class to provide interactive OpenGL rendering under X windows.
 */

class glx {
public:

  enum { NoBorder, SlimBorder, FullBorder };

  // ARG. these are 1-based because X said so...
  enum BUTTON{NONE=0,LEFT=1,MIDDLE=2,RIGHT=3,
	      WHEEL_UP=4,WHEEL_DN=5,NUM_BUTTON=6};

  enum {REDRAW,QUIT}; // socket messages for threadsafe comm

  /**
   * UserFunc is a function prototype. User functions matching
   * this format can be registered to be called by glx 
   * in response to various events.
   */
  typedef void(*UserFunc)(glx*,void*);
  typedef std::pair<glx::UserFunc,void*> UserPair;

  /**
   * EventFunc is a function prototype. Key event callbacks
   * require this type of function signature.
   */
  typedef void(*EventFunc)(glx*,XEvent *,void*);
  typedef std::pair<glx::EventFunc,void*> EventPair;

  /**
   * MouseFunc is a function prototype. Raw mouse events
   * with pixel coords get called with this signature.
   */
  typedef void(*MouseFunc)(glx*,int,int,void*);
  typedef std::pair<glx::MouseFunc,void*> MousePair;

  typedef double (*ParamFunc)(void*);
  typedef std::pair<glx::ParamFunc,void*> ParamPair;
  enum ProjParams {NEAR,FAR,FOV,PROJW,PROJH,NPARAMS};

  /**
   * CTOR for class glx.
   * @param initFunc: gets called when the window is ready to draw
   * @param userData: data to be passed back to the user.
   */
  glx(glx::UserFunc initFunc=NULL,void* userData=NULL,
      int x=0,int y=0,int w=500,int h=500,
      int border=glx::FullBorder, bool fs=false);

  /**
   * CTOR for class glx. Use this sig when using multiple windows.
   * @param app: X app context
   * @param d: X display
   * @param initFunc: gets called when the window is ready to draw.
   * @param userData: data to be passed back to the user.
   */
  glx(XtAppContext app, Display* d, 
      glx::UserFunc initFunc=NULL,void* userData=NULL,
      int x=0,int y=0,int w=500,int h=500,
      int border=glx::FullBorder, bool fs=false);

  /**
   * CTOR for class glx. Use this to embed GLX in an X window
   * @param parent: Parent Widget, usually a Frame or Canvas widget
   * @param initFunc: gets called when the window is ready to draw.
   * @param userData: data to be passed back to the user.
   */

  glx(Widget parent, glx::UserFunc initFunc=NULL,void* userData=NULL,
      int x=0,int y=0,int w=500,int h=500);

  ~glx(void);

  ////////////////////
  // control stuff
  ////////////////////

  /**
   * Hand event loop over to X.
   */
  void  mainLoop(void);

  /**
   * Force a redraw.
   */  
  void  redraw(void){wakeup();}
  void  wakeup(int mesg=REDRAW);

  /**
   * Force app to quit
   */
  void  quit(void);

  ////////////////////
  // User callbacks
  ////////////////////

  /**
   * Register a function to be called when drawing needs to happen.
   */
  void  addPreDrawFunc(glx::UserFunc, void* user=0);
  void  addDrawFunc(glx::UserFunc, void* user=0);
  void  addPostDrawFunc(glx::UserFunc, void* user=0);
  void  addPostSwapFunc(glx::UserFunc, void* user=0);

  /**
   * Register a function to be called when the app is quitting.
   */
  void  addQuitFunc(glx::UserFunc, void* user=0);

  /**
   * Register a function to be called when the projection needs to be set.
   */
  void  addProjFunc(glx::UserFunc, void* user=0);

  /**
   * Register a function to be called when a keydown occurs.
   */
  void  addEventFunc(glx::EventFunc, void* user=0);

  /**
   * Register a function to be called when a mousedown occurs.
   */
  void  addMouseDownFunc(glx::MouseFunc,
			 glx::BUTTON btn=glx::LEFT,
			 void* user=0);

  /**
   * Register a function to be called when the mouse is dragged.
   */
  void  addMouseProcessFunc(glx::MouseFunc, 
			    glx::BUTTON btn=glx::LEFT,
			    void* user=0);

  /**
   * Register a function to be called when the mouse is released.
   */
  void  addMouseUpFunc(glx::UserFunc,glx::BUTTON btn=glx::LEFT,void* user=0);

  /**
   * Register a function to be called when the mouse is moving but not pressed.
   */
  void  addMouseIdleFunc(glx::MouseFunc,void* user=0);

  /**
   * Register a function to be called when the window is resized
   */
  void  addConfigureFunc(glx::UserFunc,void* user=0);

  /**
   * Display a text message in the window.
   */
  void  showMessage(int x, int y, std::string mesg, bool stip=true);
  void  showMessage(std::string mesg, bool stip=true);
  void  setMessageColor(float r,float g, float b){
    itsMessageColor[0]=r;
    itsMessageColor[1]=g;
    itsMessageColor[2]=b;
  }
  void setMessageColor(float c[3]){
    setMessageColor(c[0],c[1],c[2]);
  }
  /**
   * Display a text message in the window using 3D coords.
   */
  void showMessage(std::string mesg,float x,float y,float z, bool stip=true);

  /**
   * Display a set of lines, centered
   */
  void showTitle(std::vector<std::string>&);

  /**
   * Display a label in lower left corner
   */
  void setLabel(std::string label);

#ifdef USEFREETYPE
  void setTitleFont(std::string font, double size);
  void setMessageFont(std::string font, double size);
  void setLabelFont(std::string font, double size);
  void pushFont(std::string, double size);
  void popFont(void);
#endif

  ////////////////////
  // environment settings
  ////////////////////

  /**
   * Show/Hide the axis.
   */
  void  showAxis(bool enabled){axisVisible=enabled;wakeup();}

  /**
   * Set the background color.
   */
  void  background(float r, float g, float b);
  void  background(float b){itsBG=b;}

  /**
   * set highlighting
   */
  void setHighlightColor(float r, float g, float b);
  void setHighlightColor(Glx::Vector);
  void setHighlighting(bool);
  bool getHighlighting(void);
  void toggleDumping(void);
  void setDumping(bool enabled){dumping=enabled;}
  bool getDumping(void){return dumping;}

  void setWinTitle(std::string);

  ////////////////////
  // stuff for manual drawing
  ////////////////////

  /**
   * make this context the current one, required when using
   * multiple contexts [i.e. glx objects]
   */
  int   makeCurrent(void);

  /**
   * Force a redraw.
   */
  void  draw();

  /**
   * Specify this position in a matrix of screens.
   */
  void  setSector(int s, int n){itsSector=s;itsNumSectors=n;}

  /**
   * Save out a PPM image.
   */
  
  void  dumpImage(void);
  void  dumpImage(std::string name);

  // window stuff
  void  fullscreen(bool enabled);
  void  setBorder(int borderval);
  void  borderless(const int t_or_f);
  void  slimborder(const int t_or_f);
  void  setPosition(int x, int y);
  void  setSize(int w, int h);
  void  getCoords(int* x,int* y, int* w, int* h);
  int   winWidth();
  int   winHeight();

  const int* viewport(void);
  const double* modelMatrix(void);
  const double* projMatrix(void);

  float aspect();
  void highlight();
  
  // X stuff
  XtAppContext getApp(){return itsApp;}
  Display*     getDisplay(){return itsDisplay;}
  XVisualInfo* getVisual(){return itsVisual;}
  Widget       getGlx(){return itsGLXarea;}
  Window       getWindow(){return XtWindow(itsWindow);}
  GLXContext   getContext(){return itsGLXcontext;}
  GLXContext   getGlobalContext(){return itsGlobalGLXcontext;}

  // view/matrix stuff  
  void pixelToWinCoords(int x, int y, double* winx, double* winy);
  void pixelToWorldCoords(int x, int y, double* winx, double* winy);
  void worldToPixelCoords(double, double, double, int* winx, int* winy);

  int project(double objx, double objy, double objz, 
	      const double modelMatrix[16], 
	      const double projMatrix[16],
	      const int viewport[4],
	      double *winx, double *winy, double *winz);

  int unproject(double winx, double winy, double winz,
		const double modelMatrix[16], 
		const double projMatrix[16],
		const int viewport[4],
		double *objx, double *objy, double *objz);

  void registerDragger(Glx::Draggable* dragger);
  void unregisterDragger(Glx::Draggable* theDragger);
  static void draggerChanged(Glx::Draggable*,void*);

  void getEyePosition(Glx::Vector& pos);
  void getEyePosition(double[16], Glx::Vector&);
  void getProjectionVector(double, double, Glx::Vector&);
  void getProjectionVector(double[16], double, double, Glx::Vector&);

  void applyProjections(void){
    makeCurrent();
    callUserFuncs(itsProjectionFuncs);
  }
  void setAnimation(bool enabled){animating=enabled;if(animating)wakeup();}
  bool getAnimation(void){return animating;}
  int  getStep(void){return step;}

  void viewHasChanged(void);
  bool drawPending(void){return waitingToDraw;}
  std::string nextFilename(std::string pre,std::string post);
  void setDone(void);
  bool getDone(void){return done;}

  void setCurDragger(Glx::Draggable* d){itsCurDragger=d;}

protected:

  void build(Widget parent=(Widget)NULL);
  XVisualInfo *visual(void);
  void perspective(float fov, float aspect, float near, float far);
  void updateViewport(void);
  void getCoords(Display*,Window,int*,int*,int*,int*,int=0);

  void resetFontPos(void);
  void advanceFontPos(void);
  void breakIntoLines(std::string, std::vector<std::string>&);
  void showLabel(void);

  // static X callbacks
  static void goaway(Widget, XtPointer, XtPointer);
  static Boolean xRender(XtPointer);
  static void xKeyDown(Widget, XtPointer, XEvent *, Boolean *);
  static void xStart(Widget, XtPointer, XEvent *, Boolean *);
  static void xProcess(Widget, XtPointer, XEvent *, Boolean *);
  static void xEnd(Widget, XtPointer, XEvent *, Boolean *);
  static void xSetFocus(Widget, XtPointer, XEvent *, Boolean *);
  static void xInit(Widget, XtPointer, XtPointer);
  static void xExpose(Widget, XtPointer, XtPointer);
  static void xResize(Widget, XtPointer, XtPointer);
  static void xHandleSocketIO(XtPointer, int*, XtInputId*);

  // user callbacks
  void callUserFuncs(std::vector<glx::UserPair>&);
  void callEventFuncs(std::vector<glx::EventPair>&, XEvent *);
  void callMouseFuncs(std::vector<glx::MousePair>&, int, int);

  std::vector< glx::UserPair > itsQuitFuncs;
  std::vector< glx::UserPair > itsPreDrawFuncs;
  std::vector< glx::UserPair > itsDrawFuncs;
  std::vector< glx::UserPair > itsPostDrawFuncs;
  std::vector< glx::UserPair > itsPostSwapFuncs;
  std::vector< glx::UserPair > itsInitFuncs;
  std::vector< glx::UserPair > itsProjectionFuncs;
  std::vector< glx::EventPair > itsEventFuncs;
  std::vector< glx::UserPair > itsConfigureFuncs;

  std::vector< glx::MousePair > itsMouseIdleFuncs;
  std::vector< glx::MousePair > itsMouseDownFuncs[glx::NUM_BUTTON];
  std::vector< glx::MousePair > itsMouseProcFuncs[glx::NUM_BUTTON];
  std::vector< glx::UserPair >  itsMouseUpFuncs[glx::NUM_BUTTON];

  // X variables
  XtAppContext itsApp;
  Display* itsDisplay;
  XVisualInfo *itsVisual;
  GLXContext itsGLXcontext;
  XtWorkProcId itsWorkID;
  static GLXContext itsGlobalGLXcontext;  
  Widget itsWindow; // may be null if embedded
  Widget itsGLXarea;
  Widget itsResizeWidget;
  Glx::sBool done;

  // user transforms
  float itsAspectRatio;
  int itsWinX;
  int itsWinY;
  int itsWinWidth;
  int itsWinHeight;
  int itsPrevWinOriginX;
  int itsPrevWinOriginY;
  int itsPrevWinWidth;
  int itsPrevWinHeight;
  Glx::sBool itsFullscreenFlag;
  int itsBorderFlag,itsLastBorderFlag;

  Glx::sBool waitingToDraw;
  int itsSector;
  int itsNumSectors;
  int p[2]; // pipe for cross-thread messages
  Glx::sBool animating;
  int step,steps;
  Glx::sBool axisVisible;
  int itsCurFontLoc[2];
  std::vector<Glx::Draggable*> itsDraggerList;
  std::vector<Glx::Draggable*> itsDraggerInitList;
  Glx::Draggable* itsCurDragger;
  float itsMessageColor[3];
  float itsBG;
  Glx::sBool highlighted;
  Glx::Vector highlightcolor;
  std::string itsLabel;
  bool dumping;

 public:
  XEvent *event;
  glx::BUTTON buttonpressed;

  std::vector<glx::ParamPair> projParams;
  static double dummy(void*){return 0;}
  double proj(ProjParams param){
    ParamPair p = projParams[param];
    ParamFunc f = p.first;
    void* user  = p.second;
    return (f)(user);
  }

};

#endif
