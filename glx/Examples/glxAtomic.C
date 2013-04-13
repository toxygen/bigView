#include <GLX.h>
#include <GL/glu.h>
#include <iostream>
using namespace std;

void
draw(glx*,void*)
{  
  unsigned char src[4]={127};
  unsigned char dst[9];

  glPixelStorei(GL_PACK_ALIGNMENT,1);
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
  gluScaleImage(GL_LUMINANCE,
		2, 2, GL_UNSIGNED_BYTE, src,
		3, 3, GL_UNSIGNED_BYTE, dst);
}

void initGL(glx* env,void* user)
{
  int w = env->winWidth();
  int h = env->winHeight();
  cout << w <<"x"<< h<<endl;
}

main()
{
  glx* g = new glx(initGL);  
  g->addDrawFunc(draw);
  g->mainLoop();
}
