#include <GLX.h>
#include <GL/glu.h>
#include <Xm/BulletinB.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/TextF.h>
#include <Xm/RowColumn.h> // XmCreateOptionMenu,XmCreatePulldownMenu
#include <iostream>
#include <glxTrackball.h>

using namespace std;

bool rotatingCamera=false;
int frame=0;
int delay=30;
int stepsTilFrame=delay;
float radius = 2.5;

const float MAX=3.0;
const float MIN=-MAX;
const float INC=(MAX-MIN)/50.0f;

class Evaluator {
public:
  enum Type {Constant, A_T, A_COS_B_T, A_SIN_B_T, NUM_EVALUATOR_TYPES};
  
  Evaluator(Type _type, float _p1, float _p2=0.0){
    type = _type;
    switch( type ){
      case Constant:  c = _p1; break;
      case A_T:       a = _p1; break;
      case A_COS_B_T: a = _p1; b = _p2; break;
      case A_SIN_B_T: a = _p1; b = _p2; break;
    }
  }
  
  float evaluate(float t);

  Evaluator::Type type;
  static std::string TypeLabels[Evaluator::NUM_EVALUATOR_TYPES];
  float a;
  float b;
  float c; // constant
};

Evaluator pxEval(Evaluator::A_COS_B_T,10,M_PI/180.0f);
Evaluator pyEval(Evaluator::Constant,3);
Evaluator pzEval(Evaluator::A_SIN_B_T,10,M_PI/180.0f);

Evaluator cxEval(Evaluator::Constant,0.0f);
Evaluator cyEval(Evaluator::Constant,0.0f);
Evaluator czEval(Evaluator::Constant,0.0f);

Evaluator uxEval(Evaluator::Constant,0.0f);
Evaluator uyEval(Evaluator::Constant,0.0f);
Evaluator uzEval(Evaluator::Constant,1.0f);

string Evaluator::TypeLabels[Evaluator::NUM_EVALUATOR_TYPES] = {
  "Constant", "a*t", "a*cos(b*t)", "a*sin(b*t)"
};

float Evaluator::evaluate(float t){
  switch( type ){

    case Evaluator::Constant: 
      return c;

    case Evaluator::A_T: 
      return a * t;

    case Evaluator::A_COS_B_T: 
      return a * cos( b * t );

    case Evaluator::A_SIN_B_T: 
      return a * sin( b * t );

    default: break;      
  }
}

static void normalize(float v[3])
{
  float r;
  
  r = sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
  if (r == 0.0) return;
  
  v[0] /= r;
  v[1] /= r;
  v[2] /= r;
}
static void cross(float v1[3], float v2[3], float result[3])
{
  result[0] = v1[1]*v2[2] - v1[2]*v2[1];
  result[1] = v1[2]*v2[0] - v1[0]*v2[2];
  result[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

static void makeIdentityf(GLfloat m[16])
{
    m[0+4*0] = 1; m[0+4*1] = 0; m[0+4*2] = 0; m[0+4*3] = 0;
    m[1+4*0] = 0; m[1+4*1] = 1; m[1+4*2] = 0; m[1+4*3] = 0;
    m[2+4*0] = 0; m[2+4*1] = 0; m[2+4*2] = 1; m[2+4*3] = 0;
    m[3+4*0] = 0; m[3+4*1] = 0; m[3+4*2] = 0; m[3+4*3] = 1;
}

void 
_gluLookAt(GLdouble eyex, GLdouble eyey, GLdouble eyez, 
	   GLdouble centerx, GLdouble centery, GLdouble centerz, 
	   GLdouble upx, GLdouble upy, GLdouble upz)
{
  int i;
  float forward[3], side[3], up[3];
  GLfloat m[4][4];
  
  forward[0] = centerx - eyex;
  forward[1] = centery - eyey;
  forward[2] = centerz - eyez;
  
  up[0] = upx;
  up[1] = upy;
  up[2] = upz;
  
  normalize(forward);
  
  /* Side = forward x up */
  cross(forward, up, side);
  normalize(side);
  
  /* Recompute up as: up = side x forward */
  cross(side, forward, up);
  
  makeIdentityf(&m[0][0]);
  m[0][0] = side[0];
  m[1][0] = side[1];
  m[2][0] = side[2];
  
  m[0][1] = up[0];
  m[1][1] = up[1];
  m[2][1] = up[2];
  
  m[0][2] = -forward[0];
  m[1][2] = -forward[1];
  m[2][2] = -forward[2];
  
  glMultMatrixf(&m[0][0]);
  glTranslated(-eyex, -eyey, -eyez);
}

float f(float u, float v)
{
  //return exp( sin(u*v) - cos(3.0f*u + 2.0f*v)) / 10.0f;
  //return sin(v) * cos(u) - 1.0f/sin(v) + 1.0f/cos(u);
  return sin(u*v);
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

void showObj(void)
{
  float u,v;
  glColor3f(0,1,0);
  GLenum type = GL_TRIANGLE_STRIP;
  for(u=MIN;u<=MAX;u+=INC){
    glBegin(type);
    for(v=MIN;v<=MAX;v+=INC){
      float z = f(u,v);
      sendColor(z,-1,1);
      sendNormal(u,v);
      glVertex3f(u,v,z);
      
      float zNext = f(u+INC,v);
      sendColor(zNext,-1,1);
      sendNormal(u+INC,v);
      glVertex3f(u+INC,v,zNext);
    }
    glEnd();
  }
}

static float d2r(double d){
  return d*(M_PI/180.0f);
}

void
draw(glx* env,void*)
{
  if( rotatingCamera ){

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float pos[3],cntr[3],up[3];

    pos[0]=pxEval.evaluate(frame);
    pos[1]=pyEval.evaluate(frame);
    pos[2]=pzEval.evaluate(frame);

    cntr[0]=cxEval.evaluate(frame);
    cntr[1]=cyEval.evaluate(frame);
    cntr[2]=czEval.evaluate(frame);

    up[0]=uxEval.evaluate(frame);
    up[1]=uyEval.evaluate(frame);
    up[2]=uzEval.evaluate(frame);

    _gluLookAt(pos[0],pos[1],pos[2],
	       cntr[0],cntr[1],cntr[2],
	       up[0],up[1],up[2]);
  }
  
  showObj();

  if( rotatingCamera ){
    --stepsTilFrame;
    if( stepsTilFrame==0 ){
      ++frame;
      stepsTilFrame=delay;
    }
    env->wakeup();
  }
}

void initGL(glx* env,void* user)
{
  env->addDrawFunc(draw);
  Glx::Trackball* trackball = new Glx::Trackball(env);
}

void 
toggleRotation(Widget w, XtPointer user, XtPointer call)
{
  glx* env = static_cast<glx*>(user);
  rotatingCamera = ! rotatingCamera;
  env->wakeup();
}

main()
{
  glx* g = new glx(initGL);
  int WX=510,WY=0,WW = 400, WH=500;
  int FX=10, FY=50;
  int FW = WW-(FX+FX);
  int FH = WH-(FY+FX);
  Widget cwin = 
    XtVaAppCreateShell("RotateControl", "RotateControl",
		       topLevelShellWidgetClass, g->getDisplay(), 
		       XmNx,     WX, XmNy,      WY, 
		       XmNwidth, WW, XmNheight, WH,
		       XmNtraversalOn,True,
		       XmNmappedWhenManaged, TRUE,
		       NULL);
  Widget frame = 
    XtVaCreateManagedWidget("f",xmBulletinBoardWidgetClass,cwin,
			    XmNx,     FX,   XmNy,      FY, 
			    XmNwidth, FW,   XmNheight, FH,
			    XmNshadowType,  XmSHADOW_ETCHED_IN,
			    XmNmarginWidth, 2, XmNmarginHeight, 2,
			    XmNtraversalOn, True,
			    XmNshadowThickness, 1,
			    NULL);

  Widget rotate = XtVaCreateManagedWidget("Rotate",
					  xmToggleButtonWidgetClass, frame,
					  XmNx,     30,   XmNy,      30, 
					  XmNwidth, 120,  XmNheight, 30, NULL);
  
  XtAddCallback(rotate,XmNvalueChangedCallback,toggleRotation,g);
  
  XtRealizeWidget(cwin);			
 
  g->mainLoop();
}

