#include <iostream>
#include <fstream>
#include <string>
#include <assert.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include <GLX.h>
#include <glxTrackball.h>
#include <Objects/Sphere.h>

#define DEBUG 1
#include "debug.h"
using namespace std;

const char* vsrc = {
  "varying vec3 vNormal;"
  "varying vec3 vVertex;"
  "void main(void)"
  "{"
  "vVertex = gl_Vertex.xyz;"
  "vNormal = gl_Normal;"
  "gl_Position = ftransform();"
  "}\0"
};

const char* fsrc = {
  "varying vec3 vNormal;"
  "varying vec3 vVertex;"
  "void main(void)"
  "{"
  "if(vVertex.z>0. && vVertex.z<0.1) discard;"
  "gl_FragColor = vec4(vVertex.x,vVertex.y,vVertex.z,1.);"
  "}\0"
};

#define printOpenGLError() printOglError(__FILE__, __LINE__)
int printOglError(char *file, int line)
{
  GLenum glErr;
  int retCode = 0;
  
  glErr = glGetError();
  while (glErr != GL_NO_ERROR) {
    printf("glError in file %s @ line %d: %s\n", 
		   file, line, gluErrorString(glErr));
    retCode = 1;
    glErr = glGetError();
  }
  return retCode;
}

void printInfoLog(GLhandleARB obj)
{
  int infologLength=0,charsWritten=0;
  char *infoLog;
  
  glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB,
							&infologLength);

  if (infologLength > 0) {
    infoLog = (char *)malloc(infologLength+1);
    glGetInfoLogARB(obj, infologLength, &charsWritten, infoLog);
    printf("ERROR: [%s][%d]\n",infoLog,charsWritten);
    free(infoLog);
  }
}

glx* env=0;
GLhandleARB pobj,vprog,fprog;
Glx::Sphere* sphere=0;

void draw(glx* env, void*)
{
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
  sphere->draw(env,NULL);
}

void 
quit(glx *env, void*)
{
}

void initGL(glx* env, void* user)
{
  sphere = new Glx::Sphere();
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

#if 1
  vprog = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
  glShaderSourceARB(vprog, 1, &vsrc,NULL);
  glCompileShaderARB(vprog);
  printInfoLog(vprog);

  fprog = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
  glShaderSourceARB(fprog, 1, &fsrc,NULL);
  glCompileShaderARB(fprog);
  printInfoLog(fprog);

  pobj = glCreateProgramObjectARB();
  glAttachObjectARB(pobj,vprog);
  glAttachObjectARB(pobj,fprog);

  glLinkProgramARB(pobj);
  printInfoLog(pobj);

  glUseProgramObjectARB(pobj);
#endif

  new Glx::Trackball(env);
  env->addDrawFunc(draw);
  env->addQuitFunc(quit);  
}

int
main(int argc, char** argv)
{
  env = new glx(initGL);
  env->mainLoop();

  return 0;
}
