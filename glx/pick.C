
#include <Xm/Xm.h>
#include <math.h>
#include <assert.h>
#include "Colorpicker.h"
#include "GLX.h"
#include "glxTrackball.h"
#include <Draggers/Slider.h>

glx* env=0;

const float MAX=3.0;
const float MIN=-MAX;
const float INC=(MAX-MIN)/100.0f;
float alpha = -0.5;
float beta=M_PI;

float f(float u, float v)
{
  u = sin(beta*u);
  v = cos(beta*v);
  float t = sqrt(u*u + v*v);
  return exp( alpha * t*t);
  //return 0.2 * sin(2*u + v - cos(u) + sin(v*3));
  // return exp( sin(u*v) - cos(3.0f*u + 2.0f*v)) / 10.0f;
  //return sin(v) * cos(u) - 1.0f/sin(v) + 1.0f/cos(u);
}

float delta = INC/20.0f;
float d2 = INC/10.;
float d2sq = (INC*INC)/100.;

void sendNormal(float u, float v)
{
  Glx::Vector n(d2 * (f(u-delta,v)-f(u+delta,v)),
		d2 * (f(u,v-delta)-f(u,v+delta)),
		d2sq);
  glNormal3fv(n);
}

int count=0;


void
draw(glx* env,void* user)
{
  //std::cout << "///////////////////////////////////////////////"<<std::endl;
  //std::cout << "Draw("<<++count<<")"<<std::endl;

  float* rgb = static_cast<float*>(user);
  float u,v;
  glColor3f(0,1,0);
  GLenum type = GL_TRIANGLE_STRIP;
  //  if( flags[SPARSE] )
  //    type = GL_POINTS;
  //  else if( flags[WIREFRAME] )
  //    type = GL_LINE_STRIP;
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_NORMALIZE);

  glColor3fv(rgb);
  for(u=MIN;u<=MAX;u+=INC){
    glBegin(type);
    for(v=MIN;v<=MAX;v+=INC){

      sendNormal(u,v);
      glVertex3f(u,v,f(u,v));

      sendNormal(u+INC,v);
      glVertex3f(u+INC,v,f(u+INC,v));
    }
    glEnd();
  }
  env->setMessageColor(1,1,1);
  env->showMessage("Adjust Color");
}

void setColor(Colorpicker*,float rgb[3],void* user)
{
  float* color = static_cast<float*>(user);
  color[0]=rgb[0];
  color[1]=rgb[1];
  color[2]=rgb[2];
  if(env) env->wakeup();
}

void
alphaListener(void* user, float t)
{
  glx* env=static_cast<glx*>(user);
  alpha=t;
  env->wakeup();
}
void
betaListener(void* user, float t)
{
  glx* env=static_cast<glx*>(user);
  beta=t;
  env->wakeup();
}

void initGL(glx* env,void* user)
{
  static float AMBIENT_DEF = (float)0.0;
  static float DIFFUSE_DEF = (float)1.0;
  static float SPECULAR_DEF = (float)1.0;
  static float GAMBIENT_DEF = (float)0.2;
  static float itsLightPos[4] = {1.0,1.0,1.0,1.0};
  GLfloat itsAmbient[4]={AMBIENT_DEF,AMBIENT_DEF,AMBIENT_DEF,1.0};
  GLfloat itsDiffuse[4]={DIFFUSE_DEF,DIFFUSE_DEF,DIFFUSE_DEF,1.0};
  GLfloat itsSpecular[4]={SPECULAR_DEF,SPECULAR_DEF,SPECULAR_DEF,1.0};
  GLfloat itsGlobalAmbient[4]={GAMBIENT_DEF,GAMBIENT_DEF,GAMBIENT_DEF,1.0};
  GLfloat itsMaterialShininess[1]={128.0};
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, itsSpecular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, itsMaterialShininess);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, itsDiffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, itsAmbient);
  glLightfv(GL_LIGHT0, GL_AMBIENT, itsAmbient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, itsDiffuse);
  glLightfv(GL_LIGHT0, GL_POSITION, itsLightPos);
  glLightfv(GL_LIGHT0, GL_SPECULAR, itsSpecular);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, itsGlobalAmbient);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  Glx::Slider* alphaSlider = new Glx::Slider(env,Glx::Slider::Y,30,60);
  alphaSlider->setRange(-5,-0.5,5);
  alphaSlider->setCallback(alphaListener,env);

  Glx::Slider* betaSlider = new Glx::Slider(env,Glx::Slider::Y,170,60);
  betaSlider->setRange(-5,M_PI,5);
  betaSlider->setCallback(betaListener,env);

  env->addDrawFunc(draw,user);
  env->setWinTitle("pick");
}

int
main(int argc, char** argv)
{
  float rgb[3]={0.,1.,0.};
  static char defaultGeomString[] = {"200x150+10+10"};

  XtToolkitInitialize();
  assert( XtToolkitThreadInitialize() );

  XtAppContext itsApp = XtCreateApplicationContext();
  assert(itsApp);
  Display* itsDisplay = XtOpenDisplay(itsApp,(char*)NULL,
                                      "XtHello", "XtHello",
                                      NULL,0,&argc, argv);
  assert(itsDisplay);
  
  Colorpicker* cp = new Colorpicker(itsDisplay);
  cp->setSampleColorRGB(rgb);
  cp->addListener(setColor, rgb);
  
  env = new glx(itsApp,itsDisplay,initGL,rgb);
  env->setPosition(280,10);
  Glx::Trackball* trackball = new Glx::Trackball(env);

  env->setLabel("Pick");

  env->mainLoop();
  
  delete env;
  delete cp;
  delete trackball;
}
