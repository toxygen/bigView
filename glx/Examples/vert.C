#include <GLX.h>
#include <GL/glu.h>
#include <math.h>
#include <unistd.h>
#include <glxTrackball.h>
#include <unistd.h>
using namespace std;

//#define DEBUG 1
#include "debug.h"
#include "geom.h"

void
draw(glx* env,void*)
{
  glColor3f(0,0,1);
  glEnableClientState (GL_VERTEX_ARRAY);
  glVertexPointer (3, GL_FLOAT, 0, vertices);
  glDrawElements (GL_TRIANGLES, 162*3, GL_UNSIGNED_BYTE, indices);
  glDisableClientState (GL_VERTEX_ARRAY);
}

void initGL(glx* env,void* user)
{
  Glx::Trackball* trackball = new Glx::Trackball(env);
  env->addDrawFunc(draw);
}

void keydown(glx*,XEvent *event,void*)
{
  KeySym ks;
  XComposeStatus status;
  char buffer[20];
  int buf_len = 20;
  XKeyEvent *kep = (XKeyEvent *)(event);
  XLookupString(kep, buffer, buf_len, &ks, &status);
  switch( ks ){
    case 'p':
      cout << "float vertices[100][3] = {\n";
      for(int i=0 ; i< 100 ; ++i ){
	cout << "{" 
	     << vertices[i][0] <<", "
	     << vertices[i][1] <<", "
	     << vertices[i][2] <<"}, // " << i << endl;
      }
      cout << "}; // verts=100" << endl;
      cout << "GLubyte indices[162][3] = {\n";
      for(int i=0; i<162 ; ++i ){
	cout << "{"
	     << (int)indices[i][0] <<", "
	     << (int)indices[i][1] <<", "
	     << (int)indices[i][2] <<"}, ";
	if( i % 2 )
	  cout << endl;
      }
      cout << "};// indices=81" << endl;
      break;
  }
}

main(int argc, char** argv)
{
  glx* env = new glx(initGL);
  env->addEventFunc(keydown);
  env->mainLoop();
}
