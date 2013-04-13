#include <iostream>
#include <sstream>

#include "NodeBlock.h"

#include "debug.h"

using namespace std;

Glx::NodeBlock::NodeBlock(glx* env, 
			  iTuple& initcoord, 
			  int p1, int p2,  
			  float x, float y, float w, float h) :
  Glx::WorldPalette(env,x,y,w,h),
  dims(iTuple::idims),coord(initcoord),P1(p1),P2(p2),
  colorfunc(Glx::NodeBlock::defColor),colordata(0)
{
  keepaspect=true;
}
void Glx::NodeBlock::defColor(void*,iTuple&,float c[3])
{
  c[0]=c[1]=0.;
  c[2]=0.5;
}
/*
Glx::NodeBlock::NodeBlock(glx* env, 
			  vector<int>& initdims,
			  vector<int>& initcoord, 
			  int p1, int p2,  
			  float x, float y, float w, float h) :
  Glx::WorldPalette(env,x,y,w,h),
  dims(initdims),coord(initcoord),P1(p1),P2(p2),
  colorfunc(Glx::NodeBlock::defColor),colordata(0)
{
  keepaspect=true;
}

void Glx::NodeBlock::defColor(void*,vector<int>&,float c[3])
{
  c[0]=c[1]=0.;
  c[2]=0.5;
}
*/

void 
Glx::NodeBlock::draw(glx* env, void* user)
{  
  int R=dims[P1],C=dims[P2];
  MESGVAR2("draw",P1,P2);
  MESGVAR2("draw",R,C);

  double pad = 0.05 * itsW;
  double w  = itsW - 2*pad;
  double h  = itsH - 2*pad;
  double dx = w/(double)(C);
  double dy = h/(double)(R);
  double xlo=itsX+pad,xhi = itsX+itsW-pad;
  double ylo=itsY+pad,yhi = itsY+itsH-pad;
  int lw;

  glGetIntegerv(GL_LINE_WIDTH,&lw);

  glPushAttrib(GL_DEPTH_BUFFER_BIT|GL_LIGHTING_BIT);
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_COLOR_MATERIAL);

  glColor3f(0,0,0);
  glBegin(GL_QUADS);
  for(int r=0;r<R;++r){
    double rf = ylo + r * dy;
    coord[P1]=r;
    for(int c=0;c<C;++c){
      double cf = xlo + c * dx;
      coord[P2]=c;
      float color[3];
      colorfunc(colordata,coord,color);
      glColor3fv(color);

      glVertex3f(cf,   rf,-0.1);
      glVertex3f(cf,   rf+dy,-0.1);
      glVertex3f(cf+dx,rf+dy,-0.1);
      glVertex3f(cf+dx,rf,-0.1);
    }
  }
  glEnd();

  glColor3f(0,1,1);
  glBegin(GL_LINES);
  for(int r=0;r<=R;++r){
    double rt = (double)r/(R);
    double rf = ylo + rt * (yhi-ylo);
    glVertex3f(xlo,rf,0.0);
    glVertex3f(xhi,rf,0.0);
  }
  for(int c=0;c<=C;++c){
    double ct = (double)c/(C);
    double cf = xlo + ct * (xhi-xlo);
    glVertex3f(cf,ylo,0.0);
    glVertex3f(cf,yhi,0.0);
  }
  glEnd();  
  
  glLineWidth(lw);

  Glx::WorldPalette::draw(env,user);

  /*
  ostringstream ostr;
  ostr << "[";
  for(int i=0;i<coord.size();++i){
    ostr << coord[i];
    if( i<coord.size()-1)
      ostr<<",";
  }
  ostr << "]";  
  string scoord = coord.str();
  env->showMessage(itsX,itsY,scoord);
  */
  
}

