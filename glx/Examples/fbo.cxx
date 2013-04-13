#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <values.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>
#include <assert.h>
#include "GLX.h"
#include <glxTrackball.h>
#include "debug.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

using namespace std;

/*
  - Framebuffer object: [FBO]
    - a set of logical buffers [color,depth,stencil,accum]
    - plus state: where rendering is directed... ->
    - a 'drawable'
    - attached images are src/dst for pixel ops
      - color and depth textures
      - color,depth.stencil renderbuffers
    
    - glGenFramebuffersEXT()

    - glBindFramebufferEXT(target,framebuffer)
      - EQ: make current
      - target=FRAMEBUFFER_EXT
      - framebuffer==0 => window system framebuffer

    - glCheckFramebufferStatusEXT()

    - glFramebufferTexture[123]DEXT()
      - attaches a texture object's 'image' to one of a framebuffer's
        logical buffers: COLOR/DEPTH/STENCIL/[ACCUM?]
      - texture must be: TEXTURE_2D|TEXTURE_RECTANGLE|TEXTURE_CUBE_...

    - glFramebufferRenderbufferEXT()
      - attaches a FB to one of an FBO's logical buffers [COLOR|DEPTH|STENCIL]

    - glFramebufferAttachmentParameterivEXT()
    - 

  - Renderbuffer: [RB]
    - contains 2D image
    - stores pixels resulting from rendering
    - NOT a texture

    - glGenRenderbuffersEXT()

    - glBindRenderbufferEXT()
      - make current

    - glRenderbufferStorageEXT()
      - specify dims/format of a RB [EQ ~ glTexImage2D w/o pixels]
      - can read/write using gl[Read|Draw]Pixels
      -

  - render to a non-window-owned framebuffer
    - offscreen-buffers [renderbuffers]
    - textures
  - renderbuffer image - pixels contained in a renderbuffer obj
  - framebuffer-attachable image - pixels that can be attached to a FBO
    - texture images
    - renderbuffer images
  - attachment point: where we attach: color/depth/stencil to the FBO

  ----------------------             +-----------------+
  |  FRAME BUFFER OBJ  |    +------->| TEXTURE OBJECT  |
  ----------------------    | +----->|                 |
  |                    |    | |      | [IMAGE] [IMAGE] | 
  | color attachment 0 -----+ | +--->| [IMAGE] [IMAGE] |
  | ...                |      | |    +-----------------+
  | color attachment N -------+ |    +------------------+
  | depth attachment -----------+--->| RENDERBUFFER OBJ |
  | stencil attachment ------------->|                  |
  | [other state]      |             | [IMAGE] [IMAGE]  | 
  ----------------------             | [IMAGE] [IMAGE]  |
                                     +------------------+
 */



GLuint fb=0;
GLuint depth_rb=0;
GLuint color_tex=0;


bool checkFrameBufferStatus(void)
{
  GLenum status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
  switch (status)
  {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
      return true;
      break;

    case GL_FRAMEBUFFER_BINDING_EXT: 
      cerr << "GL_FRAMEBUFFER_BINDING_EXT" << endl; 
      break;
    case GL_RENDERBUFFER_BINDING_EXT: 
      cerr << "GL_RENDERBUFFER_BINDING_EXT" << endl; 
      break;
    case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT: 
      cerr << "GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT" << endl; 
      break;
    case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT: 
      cerr << "GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT" << endl; 
      break;
    case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT: 
      cerr << "GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT" << endl; 
      break;
    case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT: 
      cerr << "GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT" << endl; 
      break;
    case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT: 
      cerr << "GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT" << endl; 
      break;

    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT: 
      cerr << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT" << endl; 
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT: 
      cerr << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT" << endl; 
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT: 
      cerr << "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT" << endl; 
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT: 
      cerr << "GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT" << endl; 
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT: 
      cerr << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT" << endl; 
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT: 
      cerr << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT" << endl; 
      break;
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT: 
      cerr << "GL_FRAMEBUFFER_UNSUPPORTED_EXT" << endl; 
      break;

    default:
      cerr << "FBO programmer error" << endl;
      cerr << status << endl;
      break;
  }
  return false;
}

#define printOpenGLError(mesg) printOglError(__FILE__, __LINE__,mesg)
int printOglError(char *file, int line,char* mesg)
{
  GLenum glErr;
  int retCode = 0;
  glErr = glGetError();
  while (glErr != GL_NO_ERROR) {
    printf("glError in file %s @ line %d: %s [%s]\n", 
	   file, line, gluErrorString(glErr),mesg);
    retCode = 1;
    glErr = glGetError();
  }
  return retCode;
}

void setcube(float v[8][3],float gmin[3], float gmax[3])
{
  /* Setup cube vertex data. */
  v[0][0] = v[1][0] = v[2][0] = v[3][0] = gmin[0];
  v[4][0] = v[5][0] = v[6][0] = v[7][0] = gmax[0];
  v[0][1] = v[1][1] = v[4][1] = v[5][1] = gmin[1];
  v[2][1] = v[3][1] = v[6][1] = v[7][1] = gmax[1];
  v[0][2] = v[3][2] = v[4][2] = v[7][2] = gmin[2];
  v[1][2] = v[2][2] = v[5][2] = v[6][2] = gmax[2];
}

void drawCube(float gmin[3], float gmax[3])
{
  float n[6][3]={{-1,0,0},{0,1,0},{1,0,0},{0,-1,0},{0,0,1},{0,0,-1}}; 
  int f[6][4]={{0,1,2,3},{3,2,6,7},{7,6,5,4},{4,5,1,0},{5,6,2,1},{7,4,0,3}};  
  float v[8][3];

  setcube(v,gmin,gmax);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glBegin(GL_QUADS);
  for(int i = 0; i < 6; i++) {
    glNormal3fv(&n[i][0]);
    glVertex3fv(&v[f[i][0]][0]);
    glVertex3fv(&v[f[i][1]][0]);
    glVertex3fv(&v[f[i][2]][0]);
    glVertex3fv(&v[f[i][3]][0]);
  }
  glEnd();
}

void draw(glx* env, void*)
{
  float gmin[3]={-0.5,-0.5,-0.5};
  float gmax[3]={0.5,0.5,0.5};

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

  glPushAttrib(GL_LIGHTING_BIT|GL_TEXTURE_BIT);
  glClearDepth(1.0);
  glClearColor(0,0,0,0);
  glColor3f(1,1,1);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  drawCube(gmin,gmax);

  glPopAttrib();

  // <draw to the window, reading from the depth_tex>

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glBindTexture(GL_TEXTURE_2D, color_tex);

  glPushAttrib(GL_LIGHTING_BIT|GL_TEXTURE_BIT);
  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
  
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0,1,0,1,-1,1);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
    
  glBegin(GL_QUADS);
  glTexCoord2f(0,0);glVertex2f(0,0);
  glTexCoord2f(1,0);glVertex2f(1,0);
  glTexCoord2f(1,1);glVertex2f(1,1);
  glTexCoord2f(0,1);glVertex2f(0,1);
  glEnd();
  
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glPopAttrib();
}

//#define TEX_TARGET GL_TEXTURE_RECTANGLE_ARB
#define TEX_TARGET GL_TEXTURE_2D

void genFrameBuffer(int vp[2])
{
  if( fb==0 )        glGenFramebuffersEXT(1, &fb);
  if( depth_rb==0 )  glGenRenderbuffersEXT(1, &depth_rb);
  if( color_tex==0 ) glGenTextures(1, &color_tex);

  // bind to the FBO
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

  // create texture object and attach to FBO
  glBindTexture(TEX_TARGET,color_tex);
  glTexImage2D(TEX_TARGET,0,GL_RGBA16F_ARB,vp[0],vp[1],0,
	       GL_RGBA,GL_FLOAT,0);
  glTexParameteri(TEX_TARGET,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(TEX_TARGET,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameterf(TEX_TARGET,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
  glTexParameterf(TEX_TARGET,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
  glBindTexture(TEX_TARGET, 0);

  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
			    GL_COLOR_ATTACHMENT0_EXT,
			    TEX_TARGET,color_tex, 0);

  
  // init our depth buffer
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depth_rb);
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, 
			   GL_DEPTH_COMPONENT24, vp[0], vp[1]);

  // attach it to the FBO
  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
			       GL_DEPTH_ATTACHMENT_EXT,
			       GL_RENDERBUFFER_EXT, depth_rb);

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

  checkFrameBufferStatus();
}


void resize(glx* env, void*)
{
  env->makeCurrent();
  int vp[2]={env->winWidth(),env->winHeight()};
  genFrameBuffer(vp);
}

void initGL(glx* env, void*)
{
  env->addDrawFunc(draw);
  Glx::Trackball* tb = new Glx::Trackball(env);
  env->addConfigureFunc(resize);
  resize(env,NULL);
}

int main(int argc, char** argv)
{
  glx* env = new glx(initGL);
  env->mainLoop();
  return 1;
}

