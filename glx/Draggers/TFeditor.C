// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Class Glx::TFeditor
///////////////////////////////////////////////////////////////////////////////

#include <Draggers/TFeditor.h>
#include <GLX.h>
#include <values.h> // for MAXFLOAT 
#include <assert.h>

#define DEBUG 1
#include "debug.h"

using namespace std;

Glx::TFeditor::TFeditor(glx* env, int x, int y, int w, int h) :
  Glx::Palette(env,x,y,w,h),
  N(DEFAULT_HANDLES),
  xArray(0),yArray(0),y2Array(0),
  itsSplineInterpolationFlag(1),
  itsKnotEditingMode(NORMAL_MODE),
  TOLERANCE(1.0e-6)
{
  reset();

  itsEnv->addMouseDownFunc(Glx::TFeditor::mouseDown,glx::LEFT,this);

  Display* xDisplay = XtDisplay(itsEnv->getGlx());
  itsDeleteCursor = XCreateFontCursor(xDisplay,XC_pirate);
  itsAddCursor = XCreateFontCursor(xDisplay,XC_dotbox);
  itsNormalCursor = XCreateFontCursor(xDisplay,XC_left_ptr); 
}

int 
Glx::TFeditor::idlePick(glx * env,int x, int y, void* user)
{
  int selectDist=10;
  itsSelectionDist = MAXFLOAT;

  for( int i=0; i<N ; i++){
    int index = FIRST_CROSSHAIR + i;
    double win[2]={xArray[i],yArray[i]};
    double pix[2];
    toPixels(win,pix);
    double dx = fabs(x-pix[0]);
    double dy = fabs((env->winHeight()-y)-pix[1]);
    if( dx<3 && dy<selectDist)
    {
      itsSelectionDist = dx<dy ? dx : dy;
      return index;
    } 
    else if( dy<3 && dx<selectDist)
    {
      itsSelectionDist = dx<dy ? dx : dy;
      return index;
    } 
  }

  // ok, no crosshairs selected, try corner tools

  // in the 'tray'?
  int lo[2]={itsX,itsY+itsH-BORDER};
  int hi[2]={itsX+BORDER,itsY+itsH};

  if( (env->winHeight()-y)>=lo[1] && (env->winHeight()-y)<=hi[1] ){

    if( x>=lo[0] && x<=hi[0] ){
      itsSelectionDist=0;
      return Glx::TFeditor::ADD;
    } 
    lo[0] += BORDER+2;
    hi[0] += BORDER+2;
    if( x>=lo[0] && x<=hi[0] ){
      itsSelectionDist=0;
      return Glx::TFeditor::DEL;
    } 
    lo[0] += BORDER+2;
    hi[0] += BORDER+2;
    if( x>=lo[0] && x<=hi[0] ){
      itsSelectionDist=0;
      return Glx::TFeditor::RESET;
    }
    lo[0] += BORDER+2;
    hi[0] += BORDER+2;
    if( x>=lo[0] && x<=hi[0] ){
      itsSelectionDist=0;
      return Glx::TFeditor::SPLINE;
    }
  }

  return Glx::Palette::idlePick(env,x,y,user);
}

void Glx::TFeditor::handleDrag(glx* env,int x,int y,void* user)
{
  if( itsCurDragHandle<FIRST_CROSSHAIR )
  {
    Glx::Palette::handleDrag(env,x,y,user);
  }
  else
  {
    int index = itsCurDragHandle-FIRST_CROSSHAIR;
    double pix[2]={x,env->winHeight()-y};
    double win[2];
    fromPixels(pix,win);
    forceBetween(win[0],0,1);
    forceBetween(win[1],0,1);

    if( index==0 )
    {
      // constrain the leftmost knot to x=0.0
      win[0]=0;
    }
    else if( index==N-1 )
    {
      // constrain the rightmost knot to x=1.0
      win[0]=1;
    }
    else
    {
      // keep knots in order x[0] < ... < x[i] < ... < x[n-1]
      double l=xArray[index-1];
      double r=xArray[index+1];
      forceBetween(win[0],l+TOLERANCE,r-TOLERANCE);
    }    
    setPos(index,win);
    recalcSpline();
  }
}

float Glx::TFeditor::sample(float xin)
{
  float res = splint(xArray, yArray, y2Array, N, xin);
  if( res <0.0 ) res = 0.0f;
  if( res > 1.0 ) res = 1.0f;
  return res;
}

void Glx::TFeditor::setEditMode(Glx::TFeditor::EditMode mode)
{
  itsKnotEditingMode=mode;
  setCursor(mode);
}
void Glx::TFeditor::setSplineInterpolation(int enabled){
  itsSplineInterpolationFlag=enabled;
  callDrag();
}

void 
Glx::TFeditor::mouseDown(glx* env, int x, int y, void* user ) 
{
  Glx::TFeditor* _this = static_cast<Glx::TFeditor*>(user);
  switch( _this->itsKnotEditingMode ){
    case Glx::TFeditor::DEL_MODE:
      _this->deleteHandle(x,env->winHeight()-y);
      _this->setEditMode(Glx::TFeditor::NORMAL_MODE);
      break;
    case Glx::TFeditor::ADD_MODE:
      _this->addHandle(x,env->winHeight()-y);
      _this->setEditMode(Glx::TFeditor::NORMAL_MODE);
      break;
    case Glx::TFeditor::NORMAL_MODE:
      switch( _this->itsCurDragHandle ){
	case Glx::TFeditor::ADD:
	  _this->setEditMode(Glx::TFeditor::ADD_MODE);
	  break;
	case Glx::TFeditor::DEL:
	  _this->setEditMode(Glx::TFeditor::DEL_MODE);
	  break;
	case Glx::TFeditor::RESET:
	  _this->reset();
	  break;
	case Glx::TFeditor::SPLINE:
	  _this->itsSplineInterpolationFlag = ! 
	    _this->itsSplineInterpolationFlag;
	  break;
      } // switch itsCurDragHandle
      break;  
  } // switch itsKnotEditingMode
}

void
Glx::TFeditor::addHandle(double x, double y)
{
  double pix[2]={x,y},win[2];
  fromPixels(pix,win);

  for( int i=0; i<N-1 ; i++){
    double left[2]={xArray[i],yArray[i]};
    double right[2]={xArray[i+1],yArray[i+1]};

    if( win[0] > left[0] && win[0] < right[0] ){

      ++N;

      if( xArray != 0 ){
	double* tmpX = new double[N];
	int src=0,dst=0;
	for(int j=0; j<N ; j++ ){
	  if( j != i+1 )
	    tmpX[dst++]=xArray[src++];
	  else 
	    tmpX[dst++]=win[0];
	}
	delete [] xArray;
	xArray = tmpX;
      }
      if( yArray != 0 ){
	double* tmpY = new double[N];
	int src=0,dst=0;
	for(int j=0; j<N ; j++ ){
	  if( j != i+1 )
	    tmpY[dst++]=yArray[src++];
	  else 	    
	    tmpY[dst++]=win[1];
	}
	delete [] yArray;
	yArray = tmpY;
      }
      if( y2Array != 0 ){
	double* tmpY2 = new double[N];
	int src=0,dst=0;
	for(int j=0; j<N ; j++ ){
	  if( j != i+1 )
	    tmpY2[dst++]=y2Array[src++];
	  else 
	    tmpY2[dst++]=0;
	}
	delete [] y2Array;
	y2Array = tmpY2;
      }
      callDrag();
      return;
    }
  }
}

void Glx::TFeditor::deleteHandle(int handleID)
{
  if( handleID <= 0 || handleID >= N )
    return;
  
  if( xArray != 0 ){
    double* tmpX = new double[N-1];
    int src=0,dst=0;
    for(int i=0; i<N ; i++ ){
      if( i != handleID ){
	tmpX[dst++]=xArray[src++];
      } else 
	++src;
    }
    delete [] xArray;
    xArray = tmpX;
  }
  if( yArray != 0 ){
    double* tmpY = new double[N-1];
    int src=0,dst=0;
    for(int i=0; i<N ; i++ ){
      if( i != handleID ){
	tmpY[dst++]=yArray[src++];
      } else 
	++src;
    }
    delete [] yArray;
    yArray = tmpY;
  }
  if( y2Array != 0 ){
    double* tmpY2 = new double[N-1];
    int src=0,dst=0;
    for(int i=0; i<N ; i++ ){
      if( i != handleID ){
	tmpY2[dst++]=y2Array[src++];
      } else 
	++src;
    }
    delete [] y2Array;
    y2Array = tmpY2;
  }

  --N;

  yp1 = (yArray[1]-yArray[0])/(xArray[1]-xArray[0]);

  double endDY = yArray[N-1]-yArray[N-2];
  double endDX = xArray[N-1]-xArray[N-2];

  if( endDX == 0.0 )
    ypn = endDY/endDX;
    
  recalcSpline();
}

void
Glx::TFeditor::deleteHandle(int x, int y)
{
  double pix[2]={x,y},win[2];
  fromPixels(pix,win);

  float minDist=MAXFLOAT;
  int deletedIndex=-1;
  for(int i=0 ; i<N ; ++i ){
    double pos[2]={xArray[i],yArray[i]};
    double pix[2];
    toPixels(pos,pix);
    float dist = sqrt( (pos[0]-win[0])*(pos[0]-win[0]) + 
		       (pos[1]-win[1])*(pos[1]-win[1]) );
    if( dist < minDist ){
      minDist=dist;
      deletedIndex=i;
    }
  }
  if( deletedIndex != -1 )
    deleteHandle(deletedIndex);
}

void 
Glx::TFeditor::newHandles(vector< pair<double,double> >& handles)
{
  if( xArray != 0 ) delete [] xArray;
  if( yArray != 0 ) delete [] yArray;
  if( y2Array != 0 ) delete [] y2Array;

  N=handles.size();
  xArray = new double[N];
  yArray = new double[N];
  y2Array = new double[N];
    
  vector< pair<double,double> >::iterator iter = handles.begin();
  for( int i=0; iter != handles.end() ; iter++,i++ ){
    pair<double,double>& p = *iter;
    xArray[i] = p.first;
    yArray[i] = p.second; 
  }

  yp1 = (yArray[1]-yArray[0])/(xArray[1]-xArray[0]);
  ypn = (yArray[N-1]-yArray[N-2])/(xArray[N-1]-xArray[N-2]);
    
  recalcSpline();
}

void 
Glx::TFeditor::reset(void)
{
  if( xArray != 0 ) delete [] xArray;
  if( yArray != 0 ) delete [] yArray;
  if( y2Array != 0 ) delete [] y2Array;

  xArray = new double[N];
  yArray = new double[N];
  y2Array = new double[N];

  for(int i = 0 ; i < N ; i++ ){
    double percent = (double)i/(double)(N-1);
    xArray[i] = yArray[i] = percent;
  }
  recalcSpline();
}

double 
Glx::TFeditor::getHandleX(int handleID)
{
  if( handleID<0||handleID>=N)
    return 0.0f;
  else
    return xArray[handleID];
}

double 
Glx::TFeditor::getHandleY(int handleID)
{
  if( handleID<0||handleID>=N)
    return 0.0f;
  else
    return yArray[handleID];
}

void 
Glx::TFeditor::setHandleX(int handleID, double x)
{
  // BLUTE: maybe the user doesn't want this constraint?
  if( handleID>0 && handleID<N-1){
    xArray[handleID]=x;
    recalcSpline();
  }
}

void 
Glx::TFeditor::setHandleY(int handleID, double y)
{
  if( handleID>=0 && handleID<=N-1){
    yArray[handleID]=y;
    recalcSpline();
  }
}

void 
Glx::TFeditor::recalcSpline()
{
  yp1 = (yArray[1]-yArray[0])/(xArray[1]-xArray[0]);
  ypn = (yArray[N-1]-yArray[N-2])/(xArray[N-1]-xArray[N-2]);
  spline(xArray, yArray, N, yp1, ypn, y2Array);
  callDrag();
}

double 
Glx::TFeditor::lint(double x)
{
  for( int i=0; i<N-1 ; i++){
    if( x >= xArray[i] && x <= xArray[i+1] ){
      double delta=xArray[i+1]-xArray[i];
      if( delta==0.0 ) return yArray[i];
      double t = (x-xArray[i])/delta;
      return yArray[i] + t*(yArray[i+1]-yArray[i]);
    }
  }
  return 0.0;
}

double 
Glx::TFeditor::splint(double* xa,double* ya,double* y2a,
		      int n,double x)
{
  int klo,khi,k;
  double h,b,a;
  if( ! itsSplineInterpolationFlag ) return lint(x);
  klo = 0;
  khi = n-1;
  while (khi-klo > 1) {
    k = (khi+klo) >> 1;
    if (xa[k] > x) khi = k;
    else klo = k;
  }
  h = xa[khi] - xa[klo];
  if (h==0.0) 
    return 0.0;
  a = (xa[khi]-x)/h;
  b = (x-xa[klo])/h;
  return (a*ya[klo] + b*ya[khi] + ((a*a*a-a)*y2a[klo] +(b*b*b-b)*y2a[khi]) 
	  * (h*h) / 6.0);
}

void 
Glx::TFeditor::spline(double* x, double* y, 
		      int n, double yp1, double ypn, double *y2)
{
  /* given arrays of data points x[0..n-1] and y[0..n-1], computes the
     values of the second derivative at each of the data points
     y2[0..n-1] for use in the splint function */
  
  int i,k;
  double p,qn,sig,un,u[MAX_HANDLES];
  
  y2[0] = u[0] = 0.0;
  
  for (i=1; i<n-1; i++) {
    sig = ((double) x[i]-x[i-1]) / ((double) x[i+1] - x[i-1]);
    p = sig * y2[i-1] + 2.0;
    y2[i] = (sig-1.0) / p;
    u[i] = (((double) y[i+1]-y[i]) / (x[i+1]-x[i])) - 
      (((double) y[i]-y[i-1]) / (x[i]-x[i-1]));
    u[i] = (6.0 * u[i]/(x[i+1]-x[i-1]) - sig*u[i-1]) / p;
  }
  qn = un = 0.0;
  
  y2[n-1] = (un-qn*u[n-2]) / (qn*y2[n-2]+1.0);
  for (k=n-2; k>=0; k--)
    y2[k] = y2[k]*y2[k+1]+u[k];
}

void Glx::TFeditor::vert(double win[2])
{
  double pix[2];
  forceBetween(win[0],0,1);
  forceBetween(win[1],0,1);
  toPixels(win,pix);
  glVertex2d(pix[0],pix[1]);
}
void Glx::TFeditor::vert(double xwin, double ywin)
{
  forceBetween(xwin,0,1);
  forceBetween(ywin,0,1);
  double win[2]={xwin,ywin};
  double pix[2];
  toPixels(win,pix);
  glVertex2d(pix[0],pix[1]);
}

void 
Glx::TFeditor::draw(glx* env, void* user)
{
  Glx::Palette::draw(env,user);
  int lw,ps,handle;
  int envW = env->winWidth();
  int envH = env->winHeight();
  double win[2],pix[2];

  glPushAttrib(GL_DEPTH_BUFFER_BIT|GL_LIGHTING_BIT);
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glGetIntegerv(GL_LINE_WIDTH,&lw);
  glGetIntegerv(GL_POINT_SIZE,&ps);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0,envW,0,envH,-1,1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // draw faint outline of spline drawing area
  glLineWidth(1);
  glColor3f(0.0,0.1,0.0);
  glBegin(GL_LINE_LOOP);
  glVertex3d(itsX+BORDER,     itsY+BORDER,-0.1);
  glVertex3d(itsX+itsW-BORDER,itsY+BORDER,-0.1);
  glVertex3d(itsX+itsW-BORDER,itsY+itsH-BORDER,-0.1);
  glVertex3d(itsX+BORDER,     itsY+itsH-BORDER,-0.1);
  glEnd();

  // + 'ADD' button 

  if( itsCurDragHandle==Glx::TFeditor::ADD )
    glColor3f(0.5f,0.0f,0.0f);
  else
    glColor3f(0.0f,0.5f,0.0f);
  int lo[2]={itsX,itsY+itsH-BORDER};
  int hi[2]={itsX+BORDER,itsY+itsH};
  glBegin(GL_LINE_LOOP);
  glVertex3d(lo[0],lo[1],-0.1);
  glVertex3d(hi[0],lo[1],-0.1);
  glVertex3d(hi[0],hi[1],-0.1);
  glVertex3d(lo[0],hi[1],-0.1);
  glEnd();
  glBegin(GL_LINES);
  glVertex3d(lo[0]+BORDER/2, lo[1]+2,-0.1);
  glVertex3d(lo[0]+BORDER/2, hi[1]-1,-0.1);
  glVertex3d(lo[0]+2,        lo[1]+BORDER/2,-0.1);
  glVertex3d(hi[0]-1,        lo[1]+BORDER/2,-0.1);
  glEnd();

  // - 'DEL' button 
  if( itsCurDragHandle==Glx::TFeditor::DEL )
    glColor3f(0.5f,0.0f,0.0f);
  else
    glColor3f(0.0f,0.5f,0.0f);
  lo[0] += BORDER+2;
  hi[0] += BORDER+2;
  glBegin(GL_LINE_LOOP);
  glVertex3d(lo[0],lo[1],-0.1);
  glVertex3d(hi[0],lo[1],-0.1);
  glVertex3d(hi[0],hi[1],-0.1);
  glVertex3d(lo[0],hi[1],-0.1);
  glEnd();
  glBegin(GL_LINES);
  glVertex3d(lo[0]+2,        lo[1]+BORDER/2,-0.1);
  glVertex3d(hi[0]-1,        lo[1]+BORDER/2,-0.1);
  glEnd();

  // = 'RESET' button 
  if( itsCurDragHandle==Glx::TFeditor::RESET )
    glColor3f(0.5f,0.0f,0.0f);
  else
    glColor3f(0.0f,0.5f,0.0f);
  lo[0] += BORDER+2;
  hi[0] += BORDER+2;
  glBegin(GL_LINE_LOOP);
  glVertex3d(lo[0],lo[1],-0.1);
  glVertex3d(hi[0],lo[1],-0.1);
  glVertex3d(hi[0],hi[1],-0.1);
  glVertex3d(lo[0],hi[1],-0.1);
  glEnd();
  glBegin(GL_LINES);
  glVertex3d(lo[0]+2,        lo[1]+BORDER/3,-0.1);
  glVertex3d(hi[0]-1,        lo[1]+BORDER/3,-0.1);
  glVertex3d(lo[0]+2,        lo[1]+2*BORDER/3,-0.1);
  glVertex3d(hi[0]-1,        lo[1]+2*BORDER/3,-0.1);
  glEnd();

  // = 'SPLINE' button 
  if( itsCurDragHandle==Glx::TFeditor::SPLINE )
    glColor3f(0.5f,0.0f,0.0f);
  else
    glColor3f(0.0f,0.5f,0.0f);
  lo[0] += BORDER+2;
  hi[0] += BORDER+2;
  glBegin(GL_LINE_LOOP);
  glVertex3d(lo[0],lo[1],-0.1);
  glVertex3d(hi[0],lo[1],-0.1);
  glVertex3d(hi[0],hi[1],-0.1);
  glVertex3d(lo[0],hi[1],-0.1);
  glEnd();
  glBegin(GL_LINES);
  glVertex3d(lo[0]+2, lo[1]+0.2*BORDER,-0.1);
  glVertex3d(hi[0]-1, lo[1]+0.4*BORDER,-0.1);
  glVertex3d(hi[0]-1, lo[1]+0.4*BORDER,-0.1);
  glVertex3d(lo[0]+2, lo[1]+0.6*BORDER,-0.1);
  glVertex3d(lo[0]+2, lo[1]+0.6*BORDER,-0.1);
  glVertex3d(hi[0]-1, lo[1]+0.8*BORDER,-0.1);
  glEnd();

  // draw the interpolating spline
  glLineWidth(1);
  glColor3f(1,1,1);
  glBegin(GL_LINE_STRIP);
  double xMin=xArray[0];
  double xMax=xArray[N-1];
  double xCur=xMin;
  double dx = (xMax-xMin)/255.0f; 
  for(int i=0; i<256 ; i++){
    xCur = i*dx;
    double yCur = splint(xArray,yArray,y2Array,N,xCur);
    forceBetween(xCur,0,1);
    forceBetween(yCur,0,1);
    vert(xCur,yCur);
  }
  glEnd(); 

  // draw the crosshairs
  glLineWidth(1);
  glBegin(GL_LINES);
  handle=itsCurDragHandle-FIRST_CROSSHAIR;
  for( int i=0; i<N ; i++){
    double win[2]={xArray[i],yArray[i]};
    double pix[2]; 
    toPixels(win,pix);
    if( i==handle )
      glColor3f(0.5f,0.0f,0.0f);
    else
      glColor3f(0.0f,0.5f,0.0f);
    glVertex3d(pix[0]-10,pix[1],0);
    glVertex3d(pix[0]+10,pix[1],0);
    glVertex3d(pix[0],pix[1]-10,0);
    glVertex3d(pix[0],pix[1]+10,0);
  }
  glEnd();

  glColor3f(1,1,1);
  glPointSize(5);
  glBegin(GL_POINTS);
  for( int i=0; i<N ; i++){
    double win[2]={xArray[i],yArray[i]};
    double pix[2]; 
    toPixels(win,pix);
    glVertex3d(pix[0],pix[1],0.01);
  }
  glEnd();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glColor3f(1,1,1);
  switch( itsCurDragHandle ){
    case Glx::Draggable::UNSELECTED:
    case Glx::TFeditor::MOVE:
    case Glx::TFeditor::RESIZE:
      break;
    case Glx::TFeditor::ADD:
      env->showMessage("Add a knot");
      break;
    case Glx::TFeditor::DEL:
      env->showMessage("Delete a knot");
      break;
    case Glx::TFeditor::RESET:
      env->showMessage("Reset knots");
      break;
    case Glx::TFeditor::SPLINE:
      env->showMessage("Toggle spline/linear interpolation");
      break;
      
    default:      
      handle=itsCurDragHandle-FIRST_CROSSHAIR;
      if( handle>=0 && handle<N )
	env->showMessage("Drag knot");	
      break;
  }
  glLineWidth(lw);
  glPointSize(ps);
  glPopAttrib();
}

void 
Glx::TFeditor::setCursor(Glx::TFeditor::EditMode mode)
{
  Widget w = itsEnv->getGlx();  
  switch( mode ){
    case Glx::TFeditor::NORMAL_MODE:
      XDefineCursor(XtDisplay(w),XtWindow(w),itsNormalCursor);
      break;
    case Glx::TFeditor::ADD_MODE:
      XDefineCursor(XtDisplay(w),XtWindow(w),itsAddCursor);
      break;
    case Glx::TFeditor::DEL_MODE:
      XDefineCursor(XtDisplay(w),XtWindow(w),itsDeleteCursor);
      break;
    default:break;
  }
  XFlush( XtDisplay(w) );
}

void Glx::TFeditor::toPixels(double in[2], double out[2])
{
  // must be normalized
  assert(0<=in[0] && in[0]<=1);
  assert(0<=in[1] && in[1]<=1);

  out[0]=out[1]=0; // default

  // size of palette, in pixels, considering borders
  double width[2] = {itsW-2*BORDER,itsH-2*BORDER}; 

  // offset into window, in pixels
  out[0] = itsX+BORDER + in[0]*width[0];
  out[1] = itsY+BORDER + in[1]*width[1];
}

void Glx::TFeditor::forceBetween(double& val, double lo, double hi)
{
  if( val<lo )
  {
    val=lo;
  } 
  else if(val>hi )
  {
    val=hi;
  }
}

void Glx::TFeditor::fromPixels(double in[2], double out[2])
{
  const double EPS = 1.0e-6;
  double width[2] = {itsW-2*BORDER,itsH-2*BORDER};
  double lo[2] = {itsX+BORDER,itsY+BORDER}; 
  double hi[2] = {itsX+itsW-BORDER,itsY+itsH-BORDER}; 

  // watch for very small palettes
  if( fabs(width[0])<EPS || fabs(width[1])<EPS )
    return;

  forceBetween(in[0],lo[0],hi[0]);
  forceBetween(in[1],lo[1],hi[1]);

  double dx[2] = {in[0] - lo[0], in[1] - lo[1]};
  //assert(lo[0]<=in[0] && in[0]<=hi[0]);
  //assert(lo[1]<=in[1] && in[1]<=hi[1]);

  out[0]=out[1]=0; // default

  // percentage into palette, considering borders  
  out[0] = dx[0]/width[0];
  out[1] = dx[1]/width[1];
}

void Glx::TFeditor::getPos(int index, double xy[2])
{
  xy[0]=xArray[index];
  xy[1]=yArray[index];
}

void Glx::TFeditor::setPos(int index, double x, double y)
{
  xArray[index]=x;
  yArray[index]=y;
}

void Glx::TFeditor::setPos(int index, double xy[2])
{
  xArray[index]=xy[0];
  yArray[index]=xy[1];
}
