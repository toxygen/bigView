#include <GLX.h>
#include <GL/glu.h>
#include <iostream>
#include <glxColormap.h>
#include <Draggers/Colorbar.h>

using namespace std;

double scalarMin = -1.0f,scalarMax=1.0f;

void
draw(glx*,void*)
{  
}

void initGL(glx* env,void* user)
{
  int w = env->winWidth();
  int h = env->winHeight();
  cout << w <<"x"<< h<<endl;
  Glx::Colormap* cmap = new Glx::Colormap;
  cmap->setMinmax(&scalarMin,&scalarMax);  
  Glx::Colorbar* colorbar = new Glx::Colorbar(env,cmap,20,265,20,200);
}

main()
{
  glx* g = new glx(initGL);  
  g->addDrawFunc(draw);
  g->mainLoop();
}
