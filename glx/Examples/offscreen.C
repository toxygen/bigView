#include <iostream>
#include <string>
#include <assert.h>
#include <stdlib.h>
#include <X11/Intrinsic.h> // for XtToolkitInitialize
#include <Xm/BulletinB.h>
#include <GL/GLwDrawA.h>
#include <GL/gl.h>
#include <GL/glx.h>

using namespace std;

#define DEBUG 1
#ifdef DEBUG 
# define FANCYMESG(m) {std::cout << "===== " << m << " =====" << std::endl;}
# define MESG(m) {std::cout << m << std::endl;}
# define VAR(v)  {std::cout << #v << " = " << v << std::endl;}
# define VAR1(v)  {std::cout << v << std::endl;}
# define VAR2(v1,v2)  {std::cout << v1 << "," << v2 << std::endl;}
# define VAR3(v1,v2,v3) {std::cout << v1 <<","<< v2 <<","<< v3 << std::endl;}
# define MESGVAR(m,v) {std::cout  << m <<":"<< #v <<" = :"<< v << ":" << std::endl;}
#else
# define FANCYMESG(m)
# define MESG(m) 
# define VAR(v)  
# define VAR1(v1)
# define VAR2(v1,v2)
# define VAR3(v1,v2,v3)
# define MESGVAR(m,v) 
#endif

#define GL_ERROR(mesg) {						 \
  GLenum _error = glGetError();						 \
  switch( _error ){							 \
    case GL_NO_ERROR:							 \
      break;								 \
    case GL_INVALID_ENUM:						 \
      std::cerr << mesg << ": GL_INVALID_ENUM" << std::endl;		 \
      break;								 \
    case GL_INVALID_VALUE:						 \
      std::cerr << mesg << ": GL_INVALID_VALUE" << std::endl;		 \
      break;								 \
    case GL_INVALID_OPERATION:						 \
      std::cerr << mesg << ": GL_INVALID_OPERATION" << std::endl;	 \
      break;								 \
    case GL_STACK_OVERFLOW:						 \
      std::cerr << mesg << ": GL_STACK_OVERFLOW" << std::endl;		 \
      break;								 \
    case GL_STACK_UNDERFLOW:						 \
      std::cerr << mesg << ": GL_STACK_UNDERFLOW" << std::endl;		 \
      break;								 \
    case GL_OUT_OF_MEMORY:						 \
      std::cerr << mesg << ": GL_OUT_OF_MEMORY" << std::endl;		 \
      break;								 \
    default:								 \
      std::cerr << mesg << ": Unknown GLERROR: " << _error << std::endl; \
      break;								 \
  }									 \
}
class glx {
public:

  typedef void(*RenderFunc)(glx*);
  typedef void(*InitFunc)(glx*);
  typedef void(*EventFunc)(glx*,XEvent *);

  glx(RenderFunc f,InitFunc i,EventFunc e) :
    renderFunc(f),
    initFunc(i),
    eventFunc(e)
  {
    int argc=0;
    char** argv=0;
    int hRes=500,wRes=500;
    mode=0;

    XtToolkitInitialize();
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

    XtAddCallback(itsGLXarea, GLwNginitCallback, glx::init,this);
    XtAddCallback(itsGLXarea, XmNexposeCallback, glx::expose, this);
    itsGLXcontext = glXCreateContext(itsDisplay,itsVisual,None,GL_TRUE);
    XtAddEventHandler(itsGLXarea, KeyPressMask, False, glx::keyDown, this);
    XtRealizeWidget(itsWindow);
  }
  
  void mainLoop(void){
    XtAppMainLoop(itsApp);
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

  void wakeup()
  {
    XExposeEvent ev;
    ev.type = Expose;
    ev.send_event = True;
    ev.display = itsDisplay;
    ev.window = XtWindow(itsGLXarea);
    ev.x = 0;
    ev.y = 0;
    ev.width = 1;
    ev.height = 2;
    ev.count = 0;
    XSendEvent(itsDisplay, XtWindow(itsGLXarea),False,
	       NoEventMask,(XEvent*)&ev);
    XFlush(itsDisplay);
  }

  static void keyDown(Widget, XtPointer user, XEvent *event, Boolean* more)
  {
    glx* _this = static_cast<glx*>(user);
    *more=True;
    if( _this->eventFunc )
      _this->eventFunc(_this,event);
    else
      exit(0);
  }

  static void init(Widget w, XtPointer clientData, XtPointer callData){
    glx* _this = static_cast<glx*>(clientData);
    glXMakeCurrent(_this->itsDisplay,XtWindow(w),_this->itsGLXcontext);
    if( _this->initFunc )
      _this->initFunc(_this);
  }
  static void expose(Widget w, XtPointer clientData, XtPointer callData){
    glx* _this = static_cast<glx*>(clientData);
    glXMakeCurrent(_this->itsDisplay,XtWindow(w),_this->itsGLXcontext);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    _this->renderFunc(_this);
    glFlush();
    glXSwapBuffers(_this->itsDisplay, XtWindow(w));
  }

  glx::RenderFunc renderFunc;
  glx::InitFunc initFunc;
  glx::EventFunc eventFunc;
  XtAppContext itsApp;
  Display* itsDisplay;
  GLXContext itsGLXcontext;
  Widget itsGLXarea;
  int mode;
};

XVisualInfo *getVisual(Display* dpy) 
{
  XVisualInfo *vis;
  int list[32];
  int n = 0; 
  list[n++] = GLX_RGBA;
  list[n++] = GLX_RED_SIZE;
  list[n++] = 1;
  list[n++] = GLX_GREEN_SIZE;
  list[n++] = 1;
  list[n++] = GLX_BLUE_SIZE;
  list[n++] = 1;
  list[n++] = GLX_DEPTH_SIZE;
  list[n++] = 1;
  list[n++] = GLX_ALPHA_SIZE;
  list[n++] = 1;
  list[n++] = (int) None;
  vis = glXChooseVisual(dpy, DefaultScreen(dpy), list);  
  assert(vis);
  cout << "VisualID: " << vis->visualid << endl;
  return( vis );
}

static void 
swapWords(unsigned* buf, int numWords)
{
  register int i;
  register unsigned* src = buf;

  for(i = 0 ; i < numWords ; i++, src++ ){

    //assert(*src==0xffff0000);
      //cout << hex << *src << endl;

    *src = ((*src & 0x000000ff) << 24) |
      ((*src & 0x0000ff00) << 8) |
      ((*src & 0x00ff0000) >> 8) |
      ((*src & 0xff000000) >> 24);
  }
}

void
show(Display* dpy, GLXFBConfig config, string name, int att)
{
  int value;
  glXGetFBConfigAttrib(dpy,config,att,&value);
  switch( att ){

    case GLX_DRAWABLE_TYPE:
      switch( value ){
	case GLX_WINDOW_BIT:
	  cout << name << ": GLX_WINDOW_BIT" << endl;
	  break;
	case GLX_PIXMAP_BIT:
	  cout << name << ": GLX_PIXMAP_BIT" << endl;
	  break;
	case GLX_PBUFFER_BIT:
	  cout << name << ": GLX_PBUFFER_BIT." << endl;
	  break;
      }
      break;
    case GLX_X_VISUAL_TYPE:
      switch( value ){
	case GLX_TRUE_COLOR:
	  cout << name << ": GLX_TRUE_COLOR" << endl;
	  break;
	case GLX_DIRECT_COLOR:
	  cout << name << ": GLX_DIRECT_COLOR" << endl;
	  break;
	case GLX_PSEUDO_COLOR:
	  cout << name << ": GLX_PSEUDO_COLOR" << endl;
	  break;
	case GLX_STATIC_COLOR:
	  cout << name << ": GLX_STATIC_COLOR" << endl;
	  break;
	case GLX_GRAY_SCALE:
	  cout << name << ": GLX_GRAY_SCALE" << endl;
	  break;
	case GLX_STATIC_GRAY:
	  cout << name << ": GLX_STATIC_GRAY" << endl;
	  break;
	case GLX_NONE:
	  cout << name << ": GLX_NONE" << endl;
	  break;
      }
      break;
      
    case GLX_CONFIG_CAVEAT:
      switch( value ){
	case GLX_NONE:
	  cout << name << ": GLX_NONE" << endl;
	  break;
	case GLX_SLOW_CONFIG:
	  cout << name << ": GLX_SLOW_CONFIG" << endl;
	  break;
	case GLX_NON_CONFORMANT_CONFIG:
	  cout << name << ": GLX_NON_CONFORMANT_CONFIG" << endl;
	  break;
      }
      break;
    default:
      cout << name << ":" << value << endl;
      break;
  }
}

void
showConfig(Display* dpy, XVisualInfo* vi )
{
  int numConfigs=0;
  GLXFBConfig * configs = 
    glXGetFBConfigs( dpy, 0, &numConfigs );
  for(int i=0 ; i< numConfigs ; i++ ){
    int vid;
    glXGetFBConfigAttrib(dpy,configs[i],GLX_VISUAL_ID,&vid);
    if( vid != vi->visualid )
      continue;
    cout << "--------------------"<<i<<"--------------------" << endl;
    show(dpy,configs[i],"GLX_FBCONFIG_ID",GLX_FBCONFIG_ID);
    show(dpy,configs[i],"GLX_BUFFER_SIZE",GLX_BUFFER_SIZE);
    show(dpy,configs[i],"GLX_LEVEL",GLX_LEVEL);
    show(dpy,configs[i],"GLX_DOUBLEBUFFER",GLX_DOUBLEBUFFER);
    show(dpy,configs[i],"GLX_STEREO",GLX_STEREO);
    show(dpy,configs[i],"GLX_AUX_BUFFERS",GLX_AUX_BUFFERS);
    show(dpy,configs[i],"GLX_RED_SIZE",GLX_RED_SIZE);
    show(dpy,configs[i],"GLX_GREEN_SIZE",GLX_GREEN_SIZE);
    show(dpy,configs[i],"GLX_BLUE_SIZE",GLX_BLUE_SIZE);
    show(dpy,configs[i],"GLX_ALPHA_SIZE",GLX_ALPHA_SIZE);
    show(dpy,configs[i],"GLX_DEPTH_SIZE",GLX_DEPTH_SIZE);
    show(dpy,configs[i],"GLX_STENCIL_SIZE",GLX_STENCIL_SIZE);
    show(dpy,configs[i],"GLX_ACCUM_RED_SIZE",GLX_ACCUM_RED_SIZE);
    show(dpy,configs[i],"GLX_ACCUM_GREEN_SIZE",GLX_ACCUM_GREEN_SIZE);
    show(dpy,configs[i],"GLX_ACCUM_BLUE_SIZE",GLX_ACCUM_BLUE_SIZE);
    show(dpy,configs[i],"GLX_ACCUM_ALPHA_SIZE",GLX_ACCUM_ALPHA_SIZE);
    show(dpy,configs[i],"GLX_RENDER_TYPE",GLX_RENDER_TYPE);
    show(dpy,configs[i],"GLX_DRAWABLE_TYPE",GLX_DRAWABLE_TYPE);
    show(dpy,configs[i],"GLX_X_RENDERABLE",GLX_X_RENDERABLE);
    show(dpy,configs[i],"GLX_VISUAL_ID",GLX_VISUAL_ID);
    show(dpy,configs[i],"GLX_X_VISUAL_TYPE",GLX_X_VISUAL_TYPE);
    show(dpy,configs[i],"GLX_CONFIG_CAVEAT",GLX_CONFIG_CAVEAT);
    show(dpy,configs[i],"GLX_TRANSPARENT_TYPE",GLX_TRANSPARENT_TYPE);
    show(dpy,configs[i],"GLX_TRANSPARENT_INDEX_VALUE",GLX_TRANSPARENT_INDEX_VALUE);
    show(dpy,configs[i],"GLX_TRANSPARENT_RED_VALUE",GLX_TRANSPARENT_RED_VALUE);
    show(dpy,configs[i],"GLX_TRANSPARENT_GREEN_VALUE",GLX_TRANSPARENT_GREEN_VALUE);
    show(dpy,configs[i],"GLX_TRANSPARENT_BLUE_VALUE",GLX_TRANSPARENT_BLUE_VALUE);
    show(dpy,configs[i],"GLX_TRANSPARENT_ALPHA_VALUE",GLX_TRANSPARENT_ALPHA_VALUE);
    show(dpy,configs[i],"GLX_MAX_PBUFFER_WIDTH",GLX_MAX_PBUFFER_WIDTH);
    show(dpy,configs[i],"GLX_MAX_PBUFFER_HEIGHT",GLX_MAX_PBUFFER_HEIGHT);
    show(dpy,configs[i],"GLX_MAX_PBUFFER_PIXELS",GLX_MAX_PBUFFER_PIXELS);
  }
}

unsigned int itsID=0;

void initGL(glx*)
{
  int textureSize = 256;
  int elementSize = 4;
  unsigned char* tex = new unsigned char[textureSize*elementSize];
  for(int i = 0 ; i<textureSize ; i++){
    if( i % 10 == 0 ){
      tex[i*elementSize+0]=0;
      tex[i*elementSize+1]=0;
      tex[i*elementSize+2]=0;
      tex[i*elementSize+3]=255;
    } else {
      float percent = (float)i/(float)(textureSize-1);
#if 0
      tex[i*elementSize+0]=(unsigned char)(60.0*percent);
      tex[i*elementSize+1]=(unsigned char)(120.0*percent);
      tex[i*elementSize+2]=(unsigned char)(255.0*percent);
      tex[i*elementSize+3]=255;
#endif
      const float LVL_0 = 0;
      const float LVL_1 = 0.556863;
      const float LVL_2 = 0.929412;
      const float LVL_3 = 0.992157;
      if( percent <= LVL_1 ){
	float t = (percent-LVL_0)/0.556863;
	tex[i*elementSize+0] = (unsigned char)(255.0*(0 + t*1));
	tex[i*elementSize+1] = (unsigned char)(255.0*(0 + t*0.0901961));
	tex[i*elementSize+2] = 0;
      } else if( percent <= LVL_2 ){
	float t = (percent-LVL_1)/0.372549;
	tex[i*elementSize+0] = 255;
	tex[i*elementSize+1] = (unsigned char)(255.0*(0.0901961 + t*0.831373));
	tex[i*elementSize+2] = 0;
      } else {
	float t = (percent-LVL_2)/0.0627451;
	tex[i*elementSize+0] = 255;
	tex[i*elementSize+1] = (unsigned char)(255.0*(0.921569 + t*0.0784314));
	tex[i*elementSize+2] = (unsigned char)(255.0*(0 + t * 1));
      }
      tex[i*elementSize+3] = 255;
    }
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT,1);

  glGenTextures(1,&itsID);
  assert( glGetError()==GL_NO_ERROR );
  
  glBindTexture(GL_TEXTURE_1D,itsID);
  assert( glGetError()==GL_NO_ERROR );

  glTexParameterf(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_1D,GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);  
  glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_REPLACE);   
  //glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_DECAL);  
  //glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_MODULATE);
  assert( glGetError()==GL_NO_ERROR );

  glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,textureSize,
	       0,GL_RGBA,GL_UNSIGNED_BYTE,tex);
  assert( glGetError()==GL_NO_ERROR );
}

unsigned char* pixels=0;
void 
draw(glx* GLX)
{
  if( pixels==0 )
    return;
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ROW_LENGTH,500); 
  glDrawPixels( 500, 500, GL_RGBA, GL_UNSIGNED_BYTE, pixels); 
}

int
main(int argc, char** argv)
{
  int w = 500;
  int h = 500;
  VAR(w);
  VAR(h);

  XtToolkitInitialize();
  XtAppContext app = XtCreateApplicationContext();
  Display* dpy = XtOpenDisplay(app,0,"a","a", NULL,0, &argc, argv);
  assert(dpy);

  XVisualInfo *vi = getVisual(dpy);
  assert(vi);
  VAR(vi->depth);

  showConfig(dpy,vi);

  MESG("RootWindow");
  Window drawable = RootWindow(dpy, DefaultScreen(dpy));
  assert(drawable);

  MESG("XCreatePixmap");
  Pixmap xPixmap = XCreatePixmap(dpy, drawable, w, h, vi->depth);
  assert(xPixmap);

  MESG("glXCreateGLXPixmap");
  GLXPixmap glxPixmap = glXCreateGLXPixmap(dpy,vi,xPixmap);
  assert(glxPixmap);

  MESG("glXCreateContext");
  GLXContext glxContext = glXCreateContext(dpy,vi,NULL,False);//not direct
  assert(glxContext);
  assert( glXIsDirect(dpy,glxContext) == 0 );

  MESG("glXMakeCurrent()");
  assert( glXMakeCurrent( dpy, glxPixmap, glxContext ) );

  initGL(0);

  glDrawBuffer(GL_FRONT_LEFT);
  GL_ERROR("glDrawBuffer(GL_FRONT_LEFT)");
  glClearColor(0,0,1,0);
  //glClearColor(0.5,0.5,0.5,0.5);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  GL_ERROR("glClear()");

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_1D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0,0,0,0);
  glBindTexture(GL_TEXTURE_1D,itsID);

  glBegin(GL_TRIANGLES);
  glTexCoord1f(0.0); glVertex3f(1.0,0.0,0.0);
  glTexCoord1f(0.5); glVertex3f(0.0,1.0,0.0);
  glTexCoord1f(1.0); glVertex3f(0.0,0.0,1.0);
  glEnd();

#if 0
  glBegin(GL_TRIANGLES);
  glColor3f(1.0,0.0,0.0); glVertex3f(1.0,0.0,0.0);
  glColor3f(0.0,1.0,0.0); glVertex3f(0.0,1.0,0.0);
  glColor3f(0.0,0.0,1.0); glVertex3f(0.0,0.0,1.0);
  glEnd();
#endif

  int size = w*h*sizeof(unsigned char)*4; // RGBA
  int numWords = w*h;
  pixels = new unsigned char[size];
  assert(pixels);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  GL_ERROR("glPixelStorei()");

  glPixelStorei(GL_UNPACK_ROW_LENGTH,w); 
  GL_ERROR("glPixelStorei()");
  MESG("glReadPixels()");

  glReadBuffer(GL_FRONT_LEFT); 
  GL_ERROR("glReadBuffer()");

  glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
  GL_ERROR("glReadPixels()");

  swapWords( (unsigned int*)pixels,numWords);

  // delete while the old context is still current

  // cleanup
  MESG("glXDestroyContext()");
  glXDestroyContext(dpy,glxContext);
  MESG("glXDestroyGLXPixmap()");
  glXDestroyGLXPixmap(dpy,glxPixmap);
  MESG("XFreePixmap()");
  XFreePixmap(dpy,xPixmap);
  //delete [] pixels;

  swapWords( (unsigned int*)pixels,numWords);
  glx* g = new glx(&draw,&initGL,0);  
  g->mainLoop();
}
