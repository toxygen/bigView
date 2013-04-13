//////////////////////////////////////////////////////////////////////////
///////////////////////////// glxTrackpad.C //////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <math.h>
#include <X11/keysym.h>
#include "glxTrackpad.h"

using namespace std;

//#define DEBUG 1
#include "debug.h"

/**
 * Glx::Trackpad::Trackpad(glx*)
 *
 * CTOR for class Glx::Trackpad
 */

struct TBview {
  float v[3];
};

static vector<TBview> views;
static int viewID;

namespace {
  template<typename T>
  void clamp(T& _val, T _min, T _max){
    if(_val<_min)_val=_min; 
    if(_val>_max)_val=_max; 
  }
}

Glx::Trackpad::Trackpad(glx* env) :
  itsEnv(env),
  itsCol(0),itsRow(0),itsCols(1),itsRows(1)
{
  reset();
  env->addProjFunc(Glx::Trackpad::setProjection,this);
  env->addMouseDownFunc(Glx::Trackpad::startScale,glx::MIDDLE,this);
  env->addMouseProcessFunc(Glx::Trackpad::processScale,glx::MIDDLE,this);
  env->addMouseDownFunc(Glx::Trackpad::startTranslate,glx::RIGHT,this);
  env->addMouseProcessFunc(Glx::Trackpad::processTranslate,glx::RIGHT,this);

  env->addMouseDownFunc(Glx::Trackpad::wheelUp,glx::WHEEL_UP,this);
  env->addMouseDownFunc(Glx::Trackpad::wheelDn,glx::WHEEL_DN,this);

  env->addEventFunc(processKey, this);
}

void Glx::Trackpad::reset(void)
{
  itsEnabledFlag=true;
  savescale=itsScaleFactor=1;
  savex=itsXtrans=0;
  savey=itsYtrans=0;
  startScaleY=0;
  startTranslateX=0;
  startTranslateY=0;
  itsTranslationSensitivity=0.02;
  itsScaleSensitivity=2.0;
  MIN_SCALE=1.0e-6;
  MAX_SCALE=10000.0;
}

void Glx::Trackpad::setProjection(glx* env,void* user)
{
  Glx::Trackpad* _this = static_cast<Glx::Trackpad*>(user);
  _this->applyXforms();
}

void Glx::Trackpad::applyXforms(void)
{
  if( ! itsEnabledFlag ) return;
  double aspect = itsEnv->aspect();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-aspect,aspect,-1,1,-1,1);
  glGetDoublev(GL_PROJECTION_MATRIX,projMatrix);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glScalef(itsScaleFactor, itsScaleFactor, 1.0);
  double midcol = (double)(itsCols-1)/2.0;
  double midrow = (double)(itsRows-1)/2.0;
  double col = -midcol+itsCol;
  double row = -midrow+itsRow;
  double xOff = col*((2.0*aspect)/itsScaleFactor);
  double yOff = row*(2.0/itsScaleFactor);
  glTranslatef(-xOff+itsXtrans, -yOff+itsYtrans,0.0);
  glGetDoublev(GL_MODELVIEW_MATRIX,modelMatrix);
}

void Glx::Trackpad::startScale(glx*, int x, int y, void* user)
{
  double winx, winy;
  Glx::Trackpad* _this = static_cast<Glx::Trackpad*>(user);
  if( ! _this->itsEnabledFlag ) return;
  _this->itsEnv->pixelToWinCoords(x,y,&winx,&winy);
  _this->startScaleY = winy;
  _this->savescale = _this->itsScaleFactor;
}

void Glx::Trackpad::processScale(glx* env, int x, int y, void* user)
{
  double winx, winy;
  Glx::Trackpad* _this = static_cast<Glx::Trackpad*>(user);
  if( ! _this->itsEnabledFlag ) return;
  _this->itsEnv->pixelToWinCoords(x,y,&winx,&winy);
  double dy = winy - _this->startScaleY;

  _this->itsScaleFactor = _this->savescale *
    (float)(1.0-(float)dy/(float)_this->itsScaleSensitivity);
  clamp(_this->itsScaleFactor,_this->MIN_SCALE,_this->MAX_SCALE);
  _this->itsEnv->wakeup();
}

void Glx::Trackpad::startTranslate(glx*, int x, int y, void* user)
{
  double winx, winy;
  Glx::Trackpad* _this = static_cast<Glx::Trackpad*>(user);
  if( ! _this->itsEnabledFlag ) return;
  _this->itsEnv->pixelToWinCoords(x,y,&winx,&winy);
  _this->startTranslateX = winx;
  _this->startTranslateY = winy;
  _this->savex=_this->itsXtrans;
  _this->savey=_this->itsYtrans;
}

void Glx::Trackpad::processTranslate(glx* env, int x, int y, void* user)
{
  double winx, winy;
  Glx::Trackpad* _this = static_cast<Glx::Trackpad*>(user);
  if( ! _this->itsEnabledFlag ) return;
  _this->itsEnv->pixelToWinCoords(x,y,&winx,&winy);

  double dx = winx - _this->startTranslateX;
  double dy = winy - _this->startTranslateY;
  _this->itsXtrans = _this->savex + dx/_this->itsTranslationSensitivity;
  _this->itsYtrans = _this->savey + dy/_this->itsTranslationSensitivity;
  VAR2(dx,dy);
  VAR2(_this->itsXtrans,_this->itsYtrans);
  _this->itsEnv->wakeup();
}

void Glx::Trackpad::wheelUp(glx*, int x, int y, void* user)
{
  Glx::Trackpad* _this = static_cast<Glx::Trackpad*>(user);
  if( ! _this->itsEnabledFlag ) return;
  double dy = 1.0;

  _this->itsScaleFactor = _this->itsScaleFactor *
    (float)(1.0-(float)dy/(float)_this->itsScaleSensitivity);
  clamp(_this->itsScaleFactor,_this->MIN_SCALE,_this->MAX_SCALE);
  _this->itsEnv->wakeup();
}

void Glx::Trackpad::wheelDn(glx*, int x, int y, void* user)
{
  Glx::Trackpad* _this = static_cast<Glx::Trackpad*>(user);
  if( ! _this->itsEnabledFlag ) return;
  double dy = -1.0;

  _this->itsScaleFactor = _this->itsScaleFactor *
    (float)(1.0-(float)dy/(float)_this->itsScaleSensitivity);
  clamp(_this->itsScaleFactor,_this->MIN_SCALE,_this->MAX_SCALE);
  _this->itsEnv->wakeup();
}

void Glx::Trackpad::processKey(glx*,XEvent *event,void* user)
{
  Glx::Trackpad* _this = static_cast<Glx::Trackpad*>(user);
  if( ! _this->itsEnabledFlag ) return;
  KeySym ks;
  XComposeStatus status;
  char buffer[20];
  int buf_len = 20;
  XKeyEvent *kep = (XKeyEvent *)(event);
  XLookupString(kep, buffer, buf_len, &ks, &status);
  int ctlDown    = kep->state & ControlMask;
  int shiftDown  = kep->state & ShiftMask;
  int altDown    = kep->state & Mod1Mask;
  switch( ks ){

    case XK_s: { // Save a view...
	TBview tbview;
	tbview.v[0]=_this->itsScaleFactor;
	tbview.v[1]=_this->itsXtrans;
	tbview.v[2]=_this->itsYtrans;
	views.push_back(tbview);
	VAR(views.size());
      }
      break;

    case XK_Right: // Move 'up' through saved views
      if( ctlDown ){
	if( views.size()==0 ) break;
	++viewID;
	if( viewID>= views.size()) viewID=0;
	VAR(viewID);
	_this->itsScaleFactor=views[viewID].v[0];
	_this->itsXtrans=views[viewID].v[1];
	_this->itsYtrans=views[viewID].v[2];
	_this->itsEnv->wakeup();
      }
      break;

    case XK_Left: // Move 'down'
      if( ctlDown ){
	if( views.size()==0 ) break;
	--viewID;
	if( viewID<0 ) viewID=views.size()-1;
	VAR(viewID);
	_this->itsScaleFactor=views[viewID].v[0];
	_this->itsXtrans=views[viewID].v[1];
	_this->itsYtrans=views[viewID].v[2];
	_this->itsEnv->wakeup();
      } 
      break;

    case XK_Up:
      break;
    case XK_Down:
      break;

    case XK_braceleft:
      _this->itsScaleSensitivity /= 0.8;
      cout << "Scale Sensitivity = "<<_this->itsScaleSensitivity << endl;
      break;
    case XK_braceright:
      _this->itsScaleSensitivity *= 0.8;
      if(_this->itsScaleSensitivity <= 1.0e-6 )
	_this->itsScaleSensitivity = 1.0e-6;
      cout << "Scale Sensitivity = "<<_this->itsScaleSensitivity << endl;
      break;
    case XK_bracketleft:
      _this->itsTranslationSensitivity /= 0.8;
      cout << "Translate Sensitivity = "
	   <<_this->itsTranslationSensitivity << endl;
      break;
    case XK_bracketright:
      _this->itsTranslationSensitivity *= 0.8;
      if(_this->itsTranslationSensitivity <= 1.0e-6 )
	_this->itsTranslationSensitivity = 1.0e-6;
      cout << "Translate Sensitivity = "
	   <<_this->itsTranslationSensitivity << endl;
      break;
    case XK_r:
      _this->reset();
      _this->itsEnv->wakeup();
      break;
    case XK_v:{
      string filename;
      cout << "===== Saving View =====" << endl;
      cout << "Filename>";
      cin>>filename;
      _this->saveView(filename);
    }
      break;
    case XK_V:{
      string filename;
      cout << "===== Load View =====" << endl;
      cout << "Filename>";
      cin>>filename;
      _this->loadView(filename);
    }
      break;
  }
}

void Glx::Trackpad::saveView(std::string filename)
{
  ofstream fout(filename.c_str());
  if( ! fout ){
    cout << "ERROR: can't open '" << filename << "' for write." << endl;
    cout << "view not saved." << endl;
    return;
  }

  fout << itsScaleFactor << endl;
  fout << itsXtrans << endl;
  fout << itsYtrans << endl;
  fout.close();
  cout << "viewing parameters saved in '" << filename << "'" << endl;
}

void Glx::Trackpad::loadView(std::string filename)
{
  ifstream fin(filename.c_str());
  if( ! fin ){
    cout << "ERROR: can't open '" << filename << "' for read." << endl;
    cout << "view not retrieved." << endl;
    return;
  }

  fin >> itsScaleFactor;
  fin >> itsXtrans;
  fin >> itsYtrans;

  fin.close();
  cout << "viewing parameters retrieved from '" << filename << "'" << endl;
  //itsEnv->viewHasChanged();
  itsEnv->wakeup();
}

static double fit(double val, double lo, double hi)
{
  return (val<lo)? lo : ((val>hi) ? hi : val);
}

static double min(double a, double b)
{
  return a<b ? a : b;
}

void Glx::Trackpad::viewAll(double lo[2], double hi[2], double pad)
{
  double dx = (2.*pad)+hi[0]-lo[0],dy = (2*pad)+hi[1]-lo[1];
  double tx = -0.5 * (lo[0]+hi[0]),ty = -0.5 * (lo[1]+hi[1]);
  double sx=1.,sy=1.;

  // add a little border by underestimating the scale
  if( fabs(dx)>MIN_SCALE )
    sx = 1.9/dx;
  if( fabs(dy)>MIN_SCALE )
    sy = 1.9/dy;

  sx = fit(sx,MIN_SCALE,MAX_SCALE);
  sy = fit(sy,MIN_SCALE,MAX_SCALE);
  itsScaleFactor = min(sx,sy);
  itsXtrans = tx;
  itsYtrans = ty;

}
