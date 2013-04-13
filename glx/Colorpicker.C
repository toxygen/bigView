#include <iostream>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/Scale.h>
#include <Xm/BulletinB.h>
#include <GL/GLwDrawA.h>
#include <GL/glx.h>
#include <math.h> // for floor
#include <assert.h>
#include "Colorpicker.h"

using namespace std;

#define DEBUG 1
#include "debug.h"

Colorpicker::Colorpicker(Display* d, Widget parent) : 
  itsDisplay(d),pctx(NULL),sctx(NULL),itsVisual(NULL),itsPickBuffer(NULL),
  itsParent(parent),itsFrame(NULL),itsPickWin(NULL),itsSampleWin(NULL),
  itsRScale(NULL),itsGScale(NULL),itsBScale(NULL),
  itsHScale(NULL),itsSScale(NULL),itsVScale(NULL), ownparent(false)

{
  itsRgb[0]=itsRgb[1]=itsRgb[2]=0.4;
  itsRgbChar[0]=itsRgbChar[1]=itsRgbChar[2]=0;
  itsHsv[0]=itsHsv[1]=0.;itsHsv[2]=0.4;

  bool needRealize=false;
  if( ! itsParent ){
    ownparent=true;
    needRealize=true;
    itsParent = 
      XtVaAppCreateShell("Colorpicker", "Colorpicker",
			 topLevelShellWidgetClass, itsDisplay, 
			 XmNx,     PARX, XmNy,       PARY,
			 XmNwidth, PARW, XmNheight, PARH,
			 XmNtitle, "Colorpicker",
			 XmNtraversalOn,True,
			 XmNmappedWhenManaged, TRUE,
			 NULL);
  } 
  itsFrame = 
    XtVaCreateManagedWidget("f",xmBulletinBoardWidgetClass,itsParent,
			    XmNx,     FX,   XmNy,      FY, 
			    XmNwidth, FW,   XmNheight, FH,
			    XmNshadowType,  XmSHADOW_ETCHED_IN,
			    XmNmarginWidth, 2, XmNmarginHeight, 2,
			    XmNtraversalOn, True,
			    XmNshadowThickness, 1,
			    NULL);

  addWidgets();
  if(needRealize)
    XtRealizeWidget(itsParent);
}

Colorpicker::~Colorpicker(void)
{
  delete [] itsPickBuffer;
  
  if( ownparent )
    XtDestroyWidget(itsParent);
  else
    XtDestroyWidget(itsFrame);

  glXDestroyContext(itsDisplay,pctx);
  glXDestroyContext(itsDisplay,sctx);
}

void 
Colorpicker::addWidgets()
{
  int attr[] = {GLX_RGBA,None};
  itsVisual = glXChooseVisual(itsDisplay, DefaultScreen(itsDisplay), attr);
  assert(itsVisual);
  pctx = glXCreateContext(itsDisplay,itsVisual,0,GL_TRUE);
  sctx = glXCreateContext(itsDisplay,itsVisual,0,GL_TRUE);
  itsPickWin = 
    XtVaCreateManagedWidget("Pick", glwDrawingAreaWidgetClass, itsFrame,
			    XmNx, PX, XmNy, PY, XmNwidth, PW, XmNheight, PH,
			    GLwNvisualInfo, itsVisual, NULL); 
  XtAddCallback(itsPickWin, GLwNginitCallback, &Colorpicker::xInitPick, this);
  XtAddCallback(itsPickWin, XmNexposeCallback, &Colorpicker::xDrawPick, this);
  XtAddEventHandler(itsPickWin, ButtonPressMask, False,
		    Colorpicker::xPickColor, this);
  XtAddEventHandler(itsPickWin,PointerMotionMask,0,
		    Colorpicker::xPickColor, this);
  itsSampleWin = 
    XtVaCreateManagedWidget("Sample", glwDrawingAreaWidgetClass, itsFrame,
			    XmNx, SX, XmNy, SY, XmNwidth, SW, XmNheight, SH,
			    GLwNvisualInfo, itsVisual, NULL);

  XtAddCallback(itsSampleWin,GLwNginitCallback,&Colorpicker::xInitSample,this);
  XtAddCallback(itsSampleWin,XmNexposeCallback,&Colorpicker::xDrawSample,this);

  XtVaCreateManagedWidget("Red",xmLabelWidgetClass,
			  itsFrame, XmNx, 205,XmNy, 135, NULL);
  XtVaCreateManagedWidget("Green",xmLabelWidgetClass,
			  itsFrame, XmNx, 200,XmNy, 170, NULL);
  XtVaCreateManagedWidget("Blue",xmLabelWidgetClass,
			  itsFrame, XmNx, 205,XmNy, 205, NULL);
  itsRScale = XtVaCreateManagedWidget("Red", xmScaleWidgetClass, itsFrame,
				      XmNx, 30, XmNy, 110, XmNwidth, 140, 
				      XmNorientation, XmHORIZONTAL,
				      XmNwidth, 140, XmNshowValue, 1, 
				      XmNminimum,0,XmNmaximum,255,NULL);
  XtAddCallback(itsRScale, XmNvalueChangedCallback,Colorpicker::xSlider,this);
  XtAddCallback(itsRScale, XmNdragCallback, Colorpicker::xSlider, this);
  itsGScale = XtVaCreateManagedWidget("Green", xmScaleWidgetClass, itsFrame,
				      XmNx, 30, XmNy, 150, XmNwidth, 140, 
				      XmNorientation, XmHORIZONTAL,
				      XmNwidth, 140, XmNshowValue, 1, 
				      XmNminimum,0,XmNmaximum,255,NULL);
  XtAddCallback(itsGScale, XmNvalueChangedCallback,Colorpicker::xSlider,this);
  XtAddCallback(itsGScale, XmNdragCallback, Colorpicker::xSlider, this);
  itsBScale = XtVaCreateManagedWidget("Green", xmScaleWidgetClass, itsFrame,
				      XmNx, 30, XmNy, 190, XmNwidth, 140, 
				      XmNorientation, XmHORIZONTAL,
				      XmNwidth, 140, XmNshowValue, 1, 
				      XmNminimum,0,XmNmaximum,255,NULL);
  XtAddCallback(itsBScale, XmNvalueChangedCallback,Colorpicker::xSlider,this);
  XtAddCallback(itsBScale, XmNdragCallback, Colorpicker::xSlider, this);

  XtVaCreateManagedWidget("Hue",xmLabelWidgetClass,itsFrame,
			  XmNx,205,XmNy,270,NULL);
  XtVaCreateManagedWidget("Sat",xmLabelWidgetClass,itsFrame,
			  XmNx,205,XmNy,310,NULL);
  XtVaCreateManagedWidget("Val",xmLabelWidgetClass,itsFrame,
			  XmNx,205,XmNy,350,NULL);
  itsHScale = XtVaCreateManagedWidget("Red", xmScaleWidgetClass, itsFrame,
				      XmNx, 30, XmNy, 250, XmNwidth, 140, 
				      XmNorientation, XmHORIZONTAL,
				      XmNwidth, 140, XmNshowValue, 1, 
				      XmNminimum,0,XmNmaximum,360,NULL);
  XtAddCallback(itsHScale, XmNvalueChangedCallback,Colorpicker::xSlider,this);
  XtAddCallback(itsHScale, XmNdragCallback, Colorpicker::xSlider, this);
  itsSScale = XtVaCreateManagedWidget("Green", xmScaleWidgetClass, itsFrame,
				      XmNx, 30, XmNy, 290, XmNwidth, 140, 
				      XmNorientation, XmHORIZONTAL,
				      XmNwidth, 140, XmNshowValue, 1, 
				      XmNminimum,0,XmNmaximum,100,NULL);
  XtAddCallback(itsSScale, XmNvalueChangedCallback,Colorpicker::xSlider,this);
  XtAddCallback(itsSScale, XmNdragCallback, Colorpicker::xSlider, this);
  itsVScale = XtVaCreateManagedWidget("Green", xmScaleWidgetClass, itsFrame,
				      XmNx, 30, XmNy, 330, XmNwidth, 140, 
				      XmNorientation, XmHORIZONTAL,
				      XmNwidth, 140, XmNshowValue, 1, 
				      XmNminimum,0,XmNmaximum,100,NULL);
  XtAddCallback(itsVScale, XmNvalueChangedCallback,Colorpicker::xSlider,this);
  XtAddCallback(itsVScale, XmNdragCallback, Colorpicker::xSlider, this);
}

void 
Colorpicker::xPickColor(Widget w, XtPointer user, 
			XEvent *e, Boolean* _continue)
{
  int vp[4];
  XButtonEvent *event = (XButtonEvent*)e;
  long mask=(Button1MotionMask|Button2MotionMask|Button3MotionMask);
  bool btn = event->state & mask;
  if( ! btn ) return;
  Colorpicker* _this = static_cast<Colorpicker*>(user);
  assert(glXMakeCurrent(_this->itsDisplay,XtWindow(w),_this->pctx));
  glGetIntegerv(GL_VIEWPORT, vp);
  int row = vp[3]-event->y,col = event->x;
  glReadPixels(col,row,1,1,GL_RGB,GL_UNSIGNED_BYTE,&_this->itsRgbChar[0]);   
  _this->itsRgb[0] = (float)_this->itsRgbChar[0]/(float)255;
  _this->itsRgb[1] = (float)_this->itsRgbChar[1]/(float)255;
  _this->itsRgb[2] = (float)_this->itsRgbChar[2]/(float)255;
  Color::rgb2hsv(_this->itsRgb,_this->itsHsv);
  _this->updateSliders(1);
  *_continue = TRUE;
  //assert(glXMakeCurrent(_this->itsDisplay,None,NULL));
  Colorpicker::xDrawSample(_this->itsSampleWin, _this, NULL);
}

void 
Colorpicker::xDrawPick(Widget w, XtPointer user, XtPointer)
{
  int vp[4];
  Colorpicker* _this = static_cast<Colorpicker*>(user);
  assert(glXMakeCurrent(_this->itsDisplay,XtWindow(w),_this->pctx));
  glGetIntegerv(GL_VIEWPORT, vp);
  int width=vp[2]-vp[0],height=vp[3]-vp[1];
  if(_this->itsPickBuffer==0){
    _this->itsPickBuffer = new float[width*height*3];
    float dx = 1.0f/(float)vp[2];
    for(int row=0; row<height ;row++ ){
      for(int col=0 ; col<width ; col++ ){
	float h = (float)col*dx,v = 1.0f,s = 1.0f,color[3];
	Color::hsv2rgb(h*360.f,s,v,color[0],color[1],color[2]);
	_this->itsPickBuffer[(row*width+col)*3 + 0] = color[0];
	_this->itsPickBuffer[(row*width+col)*3 + 1] = color[1];
	_this->itsPickBuffer[(row*width+col)*3 + 2] = color[2];
      }
    } 
  }
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDrawPixels(width,height,GL_RGB,GL_FLOAT,_this->itsPickBuffer);
  glFlush();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  //assert(glXMakeCurrent(_this->itsDisplay,None,NULL));// Release the context
}

void 
Colorpicker::xInitPick(Widget w, XtPointer clientData, XtPointer callData)
{
  Colorpicker* _this = static_cast<Colorpicker*>(clientData);
  assert(glXMakeCurrent(_this->itsDisplay,XtWindow(w),_this->pctx));
  glViewport(0, 0, PW, PH);
  glClearColor(0,0,0,0);
  //assert(glXMakeCurrent(_this->itsDisplay,None,NULL));
}
void 
Colorpicker::xInitSample(Widget w, XtPointer clientData, XtPointer callData)
{
  Colorpicker* _this = static_cast<Colorpicker*>(clientData);
  assert(glXMakeCurrent(_this->itsDisplay,XtWindow(w),_this->sctx));
  glViewport(0, 0, SW, SH);
  glClearColor(0,0,0,0);
  //assert(glXMakeCurrent(_this->itsDisplay,None,NULL));
}

extern void dump_opengl_state( FILE* OUT );
void 
Colorpicker::xDrawSample(Widget w, XtPointer user, XtPointer)
{
  Colorpicker* _this = static_cast<Colorpicker*>(user);
  assert(glXMakeCurrent(_this->itsDisplay,XtWindow(w),_this->sctx));
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0,1,0,1,-1,1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBegin(GL_QUADS);
  glColor3f(_this->itsRgb[0],_this->itsRgb[1],_this->itsRgb[2]);
  glVertex2f(0,0);glVertex2f(0,1);glVertex2f(1,1);glVertex2f(1,0);
  glEnd();
  glFlush();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  //assert(glXMakeCurrent(_this->itsDisplay,None,NULL));
}

void 
Colorpicker::xSlider(Widget w, XtPointer user, XtPointer call)
{
  Colorpicker* _this = static_cast<Colorpicker*>(user);
  XmScaleCallbackStruct* cb = (XmScaleCallbackStruct*)call;
  int index;
  if( w==_this->itsRScale ) index=0;
  else if( w==_this->itsGScale ) index=1;
  else if( w==_this->itsBScale ) index=2;
  else if( w==_this->itsHScale ) index=3;
  else if( w==_this->itsSScale ) index=4;
  else if( w==_this->itsVScale ) index=5;

  switch(index){
    case 0:
    case 1:
    case 2:
      _this->itsRgbChar[index]=cb->value;
      _this->itsRgb[index] = (float)_this->itsRgbChar[index]/(float)255;
      Color::rgb2hsv(_this->itsRgb,_this->itsHsv);
      break;
    case 3:
      _this->itsHsv[0]=(float)cb->value;
      Color::hsv2rgb(_this->itsHsv,_this->itsRgb);
      _this->itsRgbChar[0]=(unsigned char)(_this->itsRgb[0]*255.0f);
      _this->itsRgbChar[1]=(unsigned char)(_this->itsRgb[1]*255.0f);
      _this->itsRgbChar[2]=(unsigned char)(_this->itsRgb[2]*255.0f);
      break;
    case 4:
      _this->itsHsv[1]=(float)cb->value/100.f;
      Color::hsv2rgb(_this->itsHsv,_this->itsRgb);
      _this->itsRgbChar[0]=(unsigned char)(_this->itsRgb[0]*255.0f);
      _this->itsRgbChar[1]=(unsigned char)(_this->itsRgb[1]*255.0f);
      _this->itsRgbChar[2]=(unsigned char)(_this->itsRgb[2]*255.0f);
      break;
    case 5:
      _this->itsHsv[2]=(float)cb->value/100.f;
      Color::hsv2rgb(_this->itsHsv,_this->itsRgb);
      _this->itsRgbChar[0]=(unsigned char)(_this->itsRgb[0]*255.0f);
      _this->itsRgbChar[1]=(unsigned char)(_this->itsRgb[1]*255.0f);
      _this->itsRgbChar[2]=(unsigned char)(_this->itsRgb[2]*255.0f);
      break;
  }
  _this->updateSliders(1);
  Colorpicker::xDrawSample(_this->itsSampleWin, _this, NULL);
}

void 
Colorpicker::updateSliders(int callUsers)
{ 
  Arg args[3];
  int n = 0;
  XtSetArg(args[n], XmNvalue, itsRgbChar[0]); n++;
  XtSetValues(itsRScale,args,n);
  n = 0;
  XtSetArg(args[n], XmNvalue, itsRgbChar[1]); n++;
  XtSetValues(itsGScale,args,n);
  n = 0;
  XtSetArg(args[n], XmNvalue, itsRgbChar[2]); n++;
  XtSetValues(itsBScale,args,n);
  n = 0;
  XtSetArg(args[n], XmNvalue, (int)(itsHsv[0])); n++;
  XtSetValues(itsHScale,args,n);
  n = 0;
  XtSetArg(args[n], XmNvalue, (int)(itsHsv[1]*100.f)); n++;
  XtSetValues(itsSScale,args,n);
  n = 0;
  XtSetArg(args[n], XmNvalue, (int)(itsHsv[2]*100.f)); n++;
  XtSetValues(itsVScale,args,n);
  if( callUsers )
    callListeners();
}

void 
Colorpicker::addListener(Colorpicker::UserFunc func, void* user)
{
  itsListeners.push_back( Colorpicker::UserPair(func,user) );
}

void Colorpicker::callListeners(void)
{
  vector<Colorpicker::UserPair>::iterator iter = itsListeners.begin();
  for( ; iter != itsListeners.end() ; ++iter ){
    Colorpicker::UserPair p = *iter;
    p.first(this,itsRgb,p.second);
  }
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////// namespace Color /////////////////////////////
//////////////////////////////////////////////////////////////////////////

float 
Color::max3(float a, float b, float c)
{
  if( a>b )
    return a>c ? a : c;
  else if ( b>c )
    return b;
  else
    return c;
}
  
float 
Color::min3(float a, float b, float c)
{
  if( a<b )
    return a<c ? a : c;
  else if ( b<c )
    return b;
  else
    return c;
}

void 
Color::rgb2hsv(float rgb[3], float hsv[3])
{
  rgb2hsv(rgb[0],rgb[1],rgb[2],hsv[0],hsv[1],hsv[2]);
}
  
void 
Color::rgb2hsv(float r, float g, float b, float& h, float& s, float& v)
{
  float min = min3(r,g,b);
  float max = max3(r,g,b);
    
  v = max; // value
  if( max > 0 )
    s = (max-min)/max; // saturation
  else
    s = 0.0f;
    
  if( s == 0.0f )
    h=0.0; // UNDEFINED in Foley, et al
  else
  {
    float delta=max-min;
    if( r==max )
      h = (g-b)/delta;
    else if( g==max )
      h = 2.0f+(b-r)/delta;
    else if( b==max )
      h = 4.0f + (r-g)/delta;
    h *= 60.0f;
    if(h<0)
      h+= 360.0f;
  }
}
  
void 
Color::hsv2rgb(float hsv[3], float rgb[3])
{
  hsv2rgb(hsv[0],hsv[1],hsv[2],rgb[0],rgb[1],rgb[2]);
}

void Color::hsv2rgb(float h, float s, float v, float& r, float& g, float& b)
{
  if( s == 0.0 ){
    r=g=b=v;
  } else {
    if(h==360.0f) h=0.0;
    h /= 60.0f; // hTmp [0..6)
    int i = (int)floor(h);
    float f = h-i; // fractional part
    float p = v*(1.0f-s);
    float q = v*(1.0f-(s*f));
    float t = v*(1.0f-(s*(1.0f-f)));
    switch( i ){
      case 0: r=v;g=t;b=p; break;
      case 1: r=q;g=v;b=p; break;
      case 2: r=p;g=v;b=t; break;
      case 3: r=p;g=q;b=v; break;
      case 4: r=t;g=p;b=v; break;
      case 5: r=v;g=p;b=q; break;
    }
  }
}
