#include <GLX.h>
#include <Draggers/Colorbar.h>
#include <sstream>

Glx::Colorbar::Colorbar(glx* env, Colormap* cmap, int x, int y, int w, int h) :
  Palette(env,x,y,w,h),
  itsCmap(cmap),
  itsNumLabels(5),
  itsFormat("%6.3f"),
  itsScale(1.0),
  itsOffset(0.0),
  itsUseStippleFlag(true)
{
}

void 
Glx::Colorbar::draw(glx* env,void *user)
{
  if( ! itsVisibleFlag )
    return;

  int border = HANDLESIZE * 2;
  int envW = env->winWidth();
  int envH = env->winHeight();

  if( itsCurDragHandle != UNSELECTED )
    Glx::Palette::draw(env,user);

  glPushAttrib(GL_DEPTH_BUFFER_BIT|GL_LIGHTING_BIT|GL_POLYGON_STIPPLE_BIT);
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0,envW,0,envH,-1,1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  if( itsUseStippleFlag ){
    glColor3f(0,0,0);
    glEnable(GL_POLYGON_STIPPLE);
    glPolygonStipple(&Glx::stip[0]);
    glBegin(GL_QUADS);
    glVertex2f( itsX+itsW+5, itsY-5);
    glVertex2f( itsX-5,      itsY-5);
    glVertex2f( itsX-5,      itsY + itsH + 5);
    glVertex2f( itsX+itsW+5, itsY + itsH + 5);
    glEnd();
    glDisable(GL_POLYGON_STIPPLE);
  }

  itsCmap->preRender();
  
  double smin,smax;
  itsCmap->getMinmax(&smin,&smax);
  glColor3f(1,1,1);
  glBegin(GL_QUADS);
  glTexCoord1d(smin);glVertex2f(itsX+itsW, itsY);
  glTexCoord1d(smin);glVertex2f(itsX,      itsY);
  glTexCoord1d(smax);glVertex2f(itsX,      itsY+itsH);
  glTexCoord1d(smax);glVertex2f(itsX+itsW, itsY+itsH);
  glEnd();
  itsCmap->postRender();

  char buf[80];
  int charW=10,charH=12;
  float range = smax-smin;
  float scalarInc = range/(float)(itsNumLabels-1);
  float yInc = itsH/(float)(itsNumLabels-1);

  if( itsUseStippleFlag )
    env->setMessageColor(1,1,1);
  else
    env->setMessageColor(0,0,0);

  for(int i = 0 ; i < itsNumLabels ; i++){
    float t = (float)i/(float)(itsNumLabels-1);
    float curY = itsY + (float)i*yInc - charH/2;
    float curScalar = smin + t*range;
    float percent = (float)i/(float)(itsNumLabels-1);
    float label = itsOffset + itsScale * curScalar;
    sprintf(buf,itsFormat.c_str(),label);
    env->showMessage((int)(itsX+itsW+10),(int)curY,buf,itsUseStippleFlag);
  }
  if( itsTitle.length()>0)
    env->showMessage((int)(itsX),(int)(itsY - 2*charH), 
		     itsTitle,itsUseStippleFlag);

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();

}



