#include <GLX.h>
#include <values.h>
#include <assert.h>
#include <glxTrackball.h>
using namespace std;

glx* env=0;
const float MIN=-M_PI,MAX=M_PI,INC=0.1;
float sMin=-1,sMax=1;
GLuint texID=0;

float f(float u, float v)
{
  return sin(u*v);
}

void draw(glx*, void*)
{
  float x,y,z;
  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_1D);
  glBindTexture(GL_TEXTURE_1D,texID);
  for(x = MIN; x <= MAX ; x += INC ){
    glBegin(GL_QUAD_STRIP);
    for(y = MIN; y <= MAX ; y += INC ){
      z = fabs(sin(x*y));
      glTexCoord1f(z);
      glVertex3f(x,y,z);
      z = fabs(sin((x+INC)*y));
      glTexCoord1f(z);
      glVertex3f(x+INC,y,z);
    }    
    glEnd();
  }
  glDisable(GL_TEXTURE_1D);
  assert( glGetError()==GL_NO_ERROR );
}

void init(glx* env,void*)
{
  env->makeCurrent();

  float cmap[2][3]={{0,0,0},{1,1,1}};
  int lod=0;
  glGenTextures(1,&texID);
  glBindTexture(GL_TEXTURE_1D,texID);
  glTexParameterf(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_1D,GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
  glTexImage1D(GL_TEXTURE_1D,0,GL_RGB,2,0,GL_RGB,GL_FLOAT,cmap);
  assert( glGetError()==GL_NO_ERROR );

  Glx::Trackball* trackball = new Glx::Trackball(env);
  env->addDrawFunc(draw);
}

main()
{
  env = new glx(init);
  env->mainLoop();
}
