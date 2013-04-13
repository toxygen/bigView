//////////////////////////////////////////////////////////////////////////
///////////////////////////// glxTrackball.C /////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <math.h>
#include <X11/keysym.h>
#include <assert.h>
#include "glxTrackball.h"

using namespace std;

//#define DEBUG 1
#include "debug.h"
const double MINSCALE=1e-12;
#define D2R(d) (float)(d* 0.01745329251994329576923690768)

static int viewID=0;
namespace Glx {
  Glx::Quat canonicalViews[] = {
    // QUAT:    Vx         Vy         Vz         Deg
    Glx::Quat(  0,         0,         0,         1),
    Glx::Quat(  1,         0,         0,         0),
    Glx::Quat(  0,         0,         1,         0),
    Glx::Quat(  0,        -1,         0,         0),
    Glx::Quat(  0.707107,  0,         0,        -0.707107),
    Glx::Quat( -0.707107,  0,         0,        -0.707107),
    Glx::Quat(  0,         0.707107,  0,        -0.707107),
    Glx::Quat(  0,        -0.707107,  0,        -0.707107),
    Glx::Quat(  0,         0,         0.707107, -0.707107),
    Glx::Quat(  0,         0,        -0.707107, -0.707107),
    Glx::Quat(  0.707107, -0.707107,  0,         0),
    Glx::Quat( -0.707107, -0.707107,  0,         0),
    Glx::Quat(  0.707107,  0,         0.707107,  0),
    Glx::Quat( -0.707107,  0,         0.707107,  0),
    Glx::Quat(  0,         0.707107,  0.707107,  0),
    Glx::Quat(  0,        -0.707107,  0.707107,  0),
    Glx::Quat(  0.5,       0.5,       0.5,       0.5),
    Glx::Quat( -0.5,       0.5,       0.5,      -0.5),
    Glx::Quat(  0.5,      -0.5,       0.5,       0.5),
    Glx::Quat( -0.5,      -0.5,       0.5,      -0.5),
    Glx::Quat(  0.5,      -0.5,       0.5,      -0.5),
    Glx::Quat( -0.5,      -0.5,       0.5,       0.5),
    Glx::Quat(  0.5,      -0.5,      -0.5,      -0.5),
    Glx::Quat( -0.5,      -0.5,      -0.5,       0.5),
  };
}

static int numViews=sizeof(Glx::canonicalViews)/sizeof(Glx::Quat);
static float axes[3][3]={{1,0,0},{0,1,0},{0,0,1}};
static int axis=2;
//static float radians = 0.01;
static float radians = 1./(16.*M_PI);

Glx::Trackball::Trackball(glx* env) :
  itsEnv(env),
  itsCol(0),itsRow(0),itsCols(1),itsRows(1),itsHgap(0),itsVgap(0),
  ortho(false),itsScale(1)
{
  reset();
  env->addProjFunc(Glx::Trackball::setProjection,this);
  env->addMouseDownFunc(startRotate,glx::LEFT,this);
  env->addMouseProcessFunc(processRotate,glx::LEFT,this);
  env->addMouseDownFunc(startScale,glx::MIDDLE,this);
  env->addMouseProcessFunc(processScale,glx::MIDDLE,this);
  env->addMouseDownFunc(startTranslate,glx::RIGHT,this);
  env->addMouseProcessFunc(processTranslate,glx::RIGHT,this);
  env->addEventFunc(processKey, this);

  env->projParams[glx::NEAR]  = glx::ParamPair(Glx::Trackball::getNear,this);
  env->projParams[glx::FAR]   = glx::ParamPair(Glx::Trackball::getFar,this);
  env->projParams[glx::FOV]   = glx::ParamPair(Glx::Trackball::getFOV,this);
  env->projParams[glx::PROJW] = glx::ParamPair(Glx::Trackball::getProjW,this);
  env->projParams[glx::PROJH] = glx::ParamPair(Glx::Trackball::getProjH,this);
}

void Glx::Trackball::reset(void)
{
  worldQuat.reset();
  itsEnabledFlag=true;
  itsEnabledTransFlag=true;
  itsXtrans=0;
  itsYtrans=0;
  itsZtrans= -5;
  startTranslateX=0;
  startTranslateY=0;
  startRotateX=0;
  startRotateY=0;
  itsSensitivity=0.2;
  itsNear = 0.1;
  itsFar  = 100;
  itsFOV  = (float)M_PI/(float)6.0;
  spinning=false;
  spinQuat = Glx::Quat(radians,axes[axis]);
  itsCenter.reset();
}

void Glx::Trackball::viewAll(const double* gmin,const double* gmax)
{
  double distToProj = itsEnv->proj(glx::NEAR);
  double fov        = itsEnv->proj(glx::FOV);
  double aspect     = itsEnv->aspect();

  // half of the width, height of the projection plane
  double projHeightHalved = distToProj * tan(fov/2.0);
  double projWidthHalved = projHeightHalved * aspect;

  // half of the bounding box dimensions
  double bbDepthHalved  = (gmax[2]-gmin[2])/2.0;
  double bbHeightHalved = (gmax[1]-gmin[1])/2.0;
  double bbWidthHalved  = (gmax[0]-gmin[0])/2.0;

  // center rotations about the center of the bounding box
  itsCenter.set(0.5 * (gmax[0] + gmin[0]),
		0.5 * (gmax[1] + gmin[1]),
		0.5 * (gmax[2] + gmin[2]));


  // determine the distance to the eye
  double wideDist=(bbWidthHalved*distToProj)/projWidthHalved + bbDepthHalved;
  double highDist=(bbHeightHalved*distToProj)/projHeightHalved + bbDepthHalved;
  double eyeZ    =(wideDist>highDist) ? wideDist : highDist;

  // move back from the object
  itsXtrans = 0.0;
  itsYtrans = 0.0;
  itsZtrans = -eyeZ;

  double depth[3] = {gmax[0]-gmin[0],gmax[1]-gmin[1],gmax[2]-gmin[2]};
  double mag     = sqrt(depth[0]*depth[0]+depth[1]*depth[1]+depth[2]*depth[2]);
  itsFar         = 5 * mag;
  itsNear        = itsFar/1000.;
  itsSensitivity = 1./itsFar;
  VAR3(itsNear,itsFar,itsSensitivity);
}

void Glx::Trackball::applyXforms(void)
{
  if( ! itsEnabledFlag ) return;
  double m[16];
  double near = itsNear;
  double far  = itsFar;
  double fov = itsFOV;
  double half_fov = fov/2.0;
  double aspect = itsEnv->aspect();
  double hgap = 1. - (double)(itsHgap/2.)/itsEnv->winWidth();
  double vgap = 1. - (double)(itsVgap/2.)/itsEnv->winHeight();
  double t = near * tan(half_fov);
  double b = -t;
  double r = t * aspect;
  double l = -r;

  if( itsRows>1 || itsCols>1 ){
    _VAR4(itsHgap,itsVgap,hgap,vgap);

    double gt =  t;// * vgap;
    double gb = -t;
    double gr =  r;// * hgap;
    double gl = -r;
  
    double dx = 1.0/(double)itsCols;
    double dy = 1.0/(double)itsRows;
    /*
    l = gl + ((double)(itsCol  ) * dx)*(gr-gl);
    r = gl + ((double)(itsCol+1) * dx)*(gr-gl);
    b = gb + ((double)(itsRow  ) * dy)*(gt-gb);
    t = gb + ((double)(itsRow+1) * dy)*(gt-gb);
    */
    l = gl + ((double)(itsCol  ) * dx)*(gr-gl);
    r = gl + ((double)(itsCol+1) * dx)*(gr-gl);
    b = gb + ((double)(itsRow  ) * dx)*(gt-gb);
    t = gb + ((double)(itsRow+1) * dx)*(gt-gb);
    assert(gt>gb);
    assert(t>b);
  }

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if( ortho )
    glOrtho(l,r,b,t,near,far);
  else
  {
    glFrustum(l,r,b,t,near,far);
    VAR4(l,r,b,t);
    VAR2(near,far);
  }
  glGetDoublev(GL_PROJECTION_MATRIX,(double *)proj);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(itsXtrans,itsYtrans,itsZtrans);
  glScalef(itsScale,itsScale,itsScale);
  if( spinning )
    worldQuat.worldSpaceRotate(spinQuat);
  worldQuat.buildMatrix(m);
  glMultMatrixd(m);
  glTranslatef(-itsCenter[0],-itsCenter[1],-itsCenter[2]);
  glGetDoublev(GL_MODELVIEW_MATRIX,(double *)view);
}

#if 0
static double EPS = 1.0e-12;
static void 
xform(const double v[4], const double m[16], double res[4])
{
  double tmp[4];
  tmp[0] = v[0]*m[ 0]+v[1]*m[ 4]+v[2]*m[ 8]+m[12];
  tmp[1] = v[0]*m[ 1]+v[1]*m[ 5]+v[2]*m[ 9]+m[13];
  tmp[2] = v[0]*m[ 2]+v[1]*m[ 6]+v[2]*m[10]+m[14];
  tmp[3] = v[0]*m[ 3]+v[1]*m[ 7]+v[2]*m[11]+m[15];
  if( fabs(tmp[3]) > EPS ){
    tmp[0] /= tmp[3];
    tmp[1] /= tmp[3];
    tmp[2] /= tmp[3];
  }
  res[0]=tmp[0];
  res[1]=tmp[1];
  res[2]=tmp[2];
  res[3]=tmp[3];
}
#endif

void Glx::Trackball::setProjection(glx* env,void* user)
{
  Glx::Trackball* _this = static_cast<Glx::Trackball*>(user);
  if( ! _this->itsEnabledFlag ) return;
  double m[16];
  double near = _this->itsNear;
  double far  = _this->itsFar;
  double fov = _this->itsFOV;
  double half_fov = fov/2.0;
  double aspect = _this->itsEnv->aspect();
  double hgap = (double)(_this->itsHgap/2.)/_this->itsEnv->winWidth();
  double vgap = (double)(_this->itsVgap/2.)/_this->itsEnv->winHeight();
  double t = near * tan(half_fov);
  double b = -t;
  double r = t * aspect;
  double l = -r;

  // ok, we assume a square output
  // sectors are numbered as follows
  // 
  //      0 |  1  | ... |  N-1 
  //      N | N+1 | N+2 | 2N-1
  //     2N | ... | ... | ...
  //    ... | ... | ... | ...
  // (N-1)N | ... | ... | N*N-1

  if( _this->itsRows>1 || _this->itsCols>1 ){

    double gt =  t;
    double gb = -t;
    double gr =  r;
    double gl = -r;


    //double gt = t;
    //double gb = b;
    //double gr = r;
    //double gl = l;
  
    double dx = 1.0/(double)_this->itsCols;
    double dy = 1.0/(double)_this->itsRows;

    l = gl + ((double)(_this->itsCol  ) * dx)*(gr-gl);
    r = gl + ((double)(_this->itsCol+1) * dx)*(gr-gl);
    b = gb + ((double)(_this->itsRow  ) * dx)*(gt-gb);
    t = gb + ((double)(_this->itsRow+1) * dx)*(gt-gb);

    VAR2(dx,dy);
    VAR2(hgap,vgap);
    VAR4(gt,gb,gl,gr);
    double vpx = r-l;
    double vpy = t-b;
    hgap *= vpx;
    vgap *= vpy;

    MESG("---[before]---");    
    VAR4(t,b,l,r);

    l+=hgap/2.;
    r-=hgap/2.;
    b+=vgap/2.;
    t-=vgap/2.;
    MESG("---[after]---");  
    VAR4(t,b,l,r);

    assert(gt>gb);
    assert(t>b);
    /*
    cout << "(col,row)="<<_this->itsCol<<","<<_this->itsRow<<"\t"
	 << "(gb,gt)="<<gb<<","<<gt<<"\t"
	 << "(b,t)="<<b<<","<<t<<endl;
    */
  }

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  /*  
  cout << "glFrustum("
       << l<<","<<r<<","<<b<<","<<t<<","<<near<<","<<far<<")"<<endl;
  */
  if( _this->ortho )
    glOrtho(l,r,b,t,near,far);
  else
  {
    glFrustum(l,r,b,t,near,far);
    VAR4(l,r,b,t);
    VAR2(near,far);
  }
  _this->frustum[0]=l;
  _this->frustum[1]=r;
  _this->frustum[2]=b;
  _this->frustum[3]=t;
  _this->frustum[4]=near;
  _this->frustum[5]=far;

  glGetDoublev(GL_PROJECTION_MATRIX,(double *)_this->proj);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  VAR4(_this->itsXtrans,_this->itsYtrans,_this->itsZtrans,_this->itsScale);
  glTranslatef(_this->itsXtrans,_this->itsYtrans,_this->itsZtrans);
  glScalef(_this->itsScale,_this->itsScale,_this->itsScale);
  if( _this->spinning )
    _this->worldQuat.worldSpaceRotate(_this->spinQuat);
  VAR(_this->worldQuat);
  _this->worldQuat.buildMatrix(m);
  glMultMatrixd(m);
  glTranslatef(-_this->itsCenter[0],-_this->itsCenter[1],-_this->itsCenter[2]);

  glGetDoublev(GL_MODELVIEW_MATRIX,(double *)_this->view);

#if 0
  VAR4(l,t,r,b);
  VAR3(_this->itsXtrans,_this->itsYtrans,_this->itsZtrans);
  VAR3V(_this->itsCenter);
  VAR( _this->worldQuat );

  double preeye[4] = {0,0,0,1};
  double eye[4];
  double inv[16];

  inv4x4(_this->view,inv);
  xform(preeye,inv,eye);

  printf("=== VIEW ===\n");
  for(int i=0; i<4; i++){
    for(int j=0; j< 4; j++){
      printf("  % 8.5f",_this->view[i*4+j]);
    }
    printf("\n");
  }
  printf("\n");

  printf("=== INV ===\n");
  for(int i=0; i<4; i++){
    for(int j=0; j< 4; j++){
      printf("  % 8.5f",inv[i*4+j]);
    }
    printf("\n");
  }
  printf("\n");

  printf("[eye] %f,%f,%f\n",eye[0],eye[1],eye[2]);
#endif

}

void Glx::Trackball::startRotate(glx*, int x, int y, void* user)
{ 
  Glx::Trackball* _this = static_cast<Glx::Trackball*>(user);
  if( ! _this->itsEnabledFlag ) return;
  _this->itsEnv->pixelToWinCoords(x,y,
				  &_this->startRotateX,
				  &_this->startRotateY);
  _this->saveQuat = _this->worldQuat;
}

void Glx::Trackball::processRotate(glx* env, int x, int y, void* user)
{
  Glx::Trackball* _this = static_cast<Glx::Trackball*>(user);
  Glx::Quat dq;
  double winx, winy;
  if( ! _this->itsEnabledFlag ) return;
  _this->itsEnv->pixelToWinCoords(x,y,&winx,&winy);
  Glx::trackball(dq,_this->startRotateX,_this->startRotateY,winx,winy);
  _this->worldQuat = _this->saveQuat * dq; // world space rotate
  _this->itsEnv->wakeup();
}

void Glx::Trackball::startScale(glx*, int x, int y, void* user)
{
  double winx, winy;
  Glx::Trackball* _this = static_cast<Glx::Trackball*>(user);
  if( ! _this->itsEnabledFlag ) return;  
  _this->itsEnv->pixelToWinCoords(x,y,&winx,&winy);
  _this->startTranslateZ = winy;
  if( _this->ortho ){
    _this->savescale = _this->itsScale;
  } else {
    _this->savez = _this->itsZtrans;
  }
}
  
void Glx::Trackball::processScale(glx* env, int x, int y, void* user)
{
  double winx, winy;
  Glx::Trackball* _this = static_cast<Glx::Trackball*>(user);  
  if( ! _this->itsEnabledFlag ) return; 
  _this->itsEnv->pixelToWinCoords(x,y,&winx,&winy);
  double dz = _this->startTranslateZ - winy;
  if( _this->ortho ){
    _this->itsScale = _this->savescale + dz/(100.*_this->itsSensitivity);
    if( _this->itsScale<MINSCALE )
      _this->itsScale=MINSCALE;
    _MESGVAR("ortho",_this->itsScale);
  } else {
    _this->itsZtrans = _this->savez + dz/_this->itsSensitivity;
  }
  _this->itsEnv->wakeup();
}

void Glx::Trackball::startTranslate(glx*, int x, int y, void* user)
{ 
  double winx, winy;
  Glx::Trackball* _this = static_cast<Glx::Trackball*>(user);  
  if( ! _this->itsEnabledFlag ) return; 
  if( ! _this->itsEnabledTransFlag ) return;
  _this->itsEnv->pixelToWinCoords(x,y,&winx,&winy);
  _this->startTranslateX = winx;
  _this->startTranslateY = winy;
  _this->savex=_this->itsXtrans;
  _this->savey=_this->itsYtrans;
}
  
void Glx::Trackball::processTranslate(glx* env, int x, int y, void* user)
{
  double winx, winy;
  Glx::Trackball* _this = static_cast<Glx::Trackball*>(user);
  if( ! _this->itsEnabledFlag ) return;
  if( ! _this->itsEnabledTransFlag ) return;
  _this->itsEnv->pixelToWinCoords(x,y,&winx,&winy);
  double dy = winy - _this->startTranslateY;
  double dx = winx - _this->startTranslateX;
  _this->itsXtrans = _this->savex + dx/_this->itsSensitivity;
  _this->itsYtrans = _this->savey + dy/_this->itsSensitivity;
  _this->itsEnv->wakeup();
}

void Glx::Trackball::processKey(glx*,XEvent *event,void* user)
{
  float axes[3][3]={{1,0,0},{0,1,0},{0,0,1}};
  Glx::Trackball* _this = static_cast<Glx::Trackball*>(user);
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
    case XK_x:
      MESG("XK_x");
      VAR(shiftDown);
      VAR(ctlDown);

      if( altDown ){
	Glx::Quat q(D2R(5.),axes[0]);
	_this->worldQuat.worldSpaceRotate(q);
      } else if( ctlDown ){
	Glx::Quat q(-D2R(5.),axes[0]);
	_this->worldQuat.worldSpaceRotate(q);
      } else {
	axis=0;
	_this->spinQuat.set(radians,axes[axis]);
      }
      _this->itsEnv->wakeup();
      break;
    case XK_y:
      if( altDown ){
	Glx::Quat q(D2R(5.),axes[1]);
	_this->worldQuat.worldSpaceRotate(q);
      } else if( ctlDown ){
	Glx::Quat q(-D2R(5.),axes[1]);
	_this->worldQuat.worldSpaceRotate(q);
      } else {
	axis=1;
	_this->spinQuat.set(radians,axes[axis]);
      }
      _this->itsEnv->wakeup();
      break;
    case XK_z:
      if( altDown ){
	Glx::Quat q(D2R(5.),axes[2]);
	_this->worldQuat.worldSpaceRotate(q);
      } else if( ctlDown ){
	Glx::Quat q(-D2R(5.),axes[2]);
	_this->worldQuat.worldSpaceRotate(q);
      } else {
	axis=2;
	_this->spinQuat.set(radians,axes[axis]);
      }
      _this->itsEnv->wakeup();
      break;
    case XK_Up:
      radians*=1.1;
      _this->spinQuat.set(radians,axes[axis]);
      break;
    case XK_Down:
      radians/=1.1;
      _this->spinQuat.set(radians,axes[axis]);
      break;

    case XK_Right:
      ++viewID;
      if( viewID>= numViews) viewID=0;
      _this->worldQuat=Glx::canonicalViews[viewID];
      VAR2(viewID,_this->worldQuat);
      _this->itsEnv->wakeup();
      break;

    case XK_Left:
      --viewID;
      if( viewID<0 ) viewID=numViews-1;
      _this->worldQuat=Glx::canonicalViews[viewID];
      VAR2(viewID,_this->worldQuat);
      _this->itsEnv->wakeup();
      break;

    case XK_bracketleft:
      _this->itsSensitivity /= 0.8;
      cout << "Sensitivity = "<<_this->itsSensitivity << endl;
      break;
    case XK_bracketright:
      _this->itsSensitivity *= 0.8;
      if(_this->itsSensitivity < MINSCALE )
	_this->itsSensitivity = MINSCALE;
      cout << "Sensitivity = "<<_this->itsSensitivity << endl;
      break;
    case XK_r:
      _this->reset();
      _this->itsEnv->wakeup();
      break;
    case XK_s:
      _this->spinning = ! _this->spinning;
      _this->itsEnv->setAnimation(_this->spinning);
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
    case XK_o:
      _this->ortho = !_this->ortho;
      if(_this->ortho)
	_this->itsScale=_this->savescale;
      else 
	_this->itsScale=1.;
      _this->itsEnv->wakeup();
      break;
  }
  MESGVAR("Trackball::processKey [done]",ks);
}

void Glx::Trackball::saveView(std::string filename)
{
  ofstream fout(filename.c_str());
  if( ! fout ){
    cout << "ERROR: can't open '" << filename << "' for write." << endl;
    cout << "view not saved." << endl;
    return;
  }

  fout << worldQuat << endl;
  fout << itsXtrans << " " << itsYtrans << " " << itsZtrans << endl;
  fout << itsCenter[0] << " " << itsCenter[1] << " " << itsCenter[2] << endl;
  fout << itsNear << " " << itsFar << endl;
  fout.close();
  cout << "viewing parameters saved in '" << filename << "'" << endl;
}

void Glx::Trackball::loadView(std::string filename)
{
  ifstream fin(filename.c_str());
  if( ! fin ){
    cout << "ERROR: can't open '" << filename << "' for read." << endl;
    cout << "view not retrieved." << endl;
    return;
  }

  fin >> worldQuat;
  fin >> itsXtrans >> itsYtrans >> itsZtrans;
  fin >> itsCenter[0] >> itsCenter[1] >> itsCenter[2];
  fin >> itsNear >> itsFar;
  fin.close();
  cout << "viewing parameters retrieved from '" << filename << "'" << endl;

  cout << worldQuat << endl;
  cout << itsXtrans << " " << itsYtrans << " " << itsZtrans << endl;
  cout << itsCenter[0] << " " << itsCenter[1] << " " << itsCenter[2] << endl;
  cout << itsNear << " " << itsFar << endl;

  itsEnv->viewHasChanged();
}


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// ENUMS and CONSTANTS
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

static float TRACKBALLSIZE = (float)0.8;
static float getRotationSensitivity(void){return 500.0f;}

static void  vzero(float *v);
static void  vset(float *v, float x, float y, float z);
static void  vsub(const float *src1, const float *src2, float *dst);
static void  vcopy(const float *v1, float *v2);
static void  vcross(const float *v1, const float *v2, float *cross);
static float vlength(const float *v);
static void  vscale(float *v, float div);
static void  vnormal(float *v);
static float vdot(const float *v1, const float *v2);
static void  vadd(const float *src1, const float *src2, float *dst);
static void  axis_to_quat(float a[3], float phi, float q[4]);
static float tb_project_to_sphere(float r, float x, float y);

////////////////////////////////////////////////////////////////////////
// Trackball
////////////////////////////////////////////////////////////////////////

/*
 * Ok, simulate a track-ball.  Project the points onto the virtual
 * trackball, then figure out the axis of rotation, which is the cross
 * product of P1 P2 and O P1 (O is the center of the ball, 0,0,0)
 * Note:  This is a deformed trackball-- is a trackball in the center,
 * but is deformed into a hyperbolic sheet of rotation away from the
 * center.  This particular function was chosen after trying out
 * several variations.
 *
 * It is assumed that the arguments to this routine are in the range
 * (-1.0 ... 1.0)
 */

void
Glx::trackball(Glx::Quat& q,float p1x,float p1y,float p2x,float p2y)
{
  float a[3]; /* Axis of rotation */
  float phi;  /* how much to rotate about axis */
  float p1[3], p2[3], d[3];
  float t;
  
  if (p1x == p2x && p1y == p2y) {
    /* Zero rotation */
    vzero(q.v);
    q[3] = 1.0;
    return;
  }
  
  /*
   * First, figure out z-coordinates for projection of P1 and P2 to
   * deformed sphere
   */
  vset(p1,p1x,p1y,tb_project_to_sphere(TRACKBALLSIZE,p1x,p1y));
  vset(p2,p2x,p2y,tb_project_to_sphere(TRACKBALLSIZE,p2x,p2y));
  
  /*
   *  Now, we want the cross product of P1 and P2
   */
  vcross(p2,p1,a);
  vnormal(a);
  /*
   *  Figure out how much to rotate around that axis.
   */
  vsub(p1,p2,d);
  t = vlength(d) / (2.0*TRACKBALLSIZE);
  
  /*
   * Avoid problems with out-of-control values...
   */
  if (t > 1.0) t = 1.0;
  if (t < -1.0) t = -1.0;
  phi = 2.0 * asin(t);

  axis_to_quat(a,phi,q.v);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// TRACKBALL STUFF
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

static void
vzero(float *v)
{
    v[0] = 0.0;
    v[1] = 0.0;
    v[2] = 0.0;
}

static void
vset(float *v, float x, float y, float z)
{
    v[0] = x;
    v[1] = y;
    v[2] = z;
}

static void
vsub(const float *src1, const float *src2, float *dst)
{
    dst[0] = src1[0] - src2[0];
    dst[1] = src1[1] - src2[1];
    dst[2] = src1[2] - src2[2];
}

static void
vcopy(const float *v1, float *v2)
{
    register int i;
    for (i = 0 ; i < 3 ; i++)
        v2[i] = v1[i];
}

static void
vcross(const float *v1, const float *v2, float *cross)
{
    float temp[3];

    temp[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
    temp[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
    temp[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
    vcopy(temp, cross);
}

float
vlength(const float *v)
{
    return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

static void
vscale(float *v, float div)
{
    v[0] *= div;
    v[1] *= div;
    v[2] *= div;
}

static void
vnormal(float *v)
{
    vscale(v,1.0/vlength(v));
}

float
vdot(const float *v1, const float *v2)
{
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

static void
vadd(const float *src1, const float *src2, float *dst)
{
    dst[0] = src1[0] + src2[0];
    dst[1] = src1[1] + src2[1];
    dst[2] = src1[2] + src2[2];
}

/*
 *  Given an axis and angle, compute quaternion.
 */
static void
axis_to_quat(float a[3], float phi, float q[4])
{
  vnormal(a);
  vcopy(a,q);
  vscale(q,sin(phi/2.0));
  q[3] = cos(phi/2.0);
}

/*
 * Project an x,y pair onto a sphere of radius r OR a hyperbolic sheet
 * if we are away from the center of the sphere.
 */
static float
tb_project_to_sphere(float r, float x, float y)
{
  float d, t, z;
  
  d = sqrt(x*x + y*y);
  if (d < r * 0.70710678118654752440) {    /* Inside sphere */
    z = sqrt(r*r - d*d);
  } else {           /* On hyperbola */
    t = r / 1.41421356237309504880;
    z = t*t / d;
  }
  return z;
}
