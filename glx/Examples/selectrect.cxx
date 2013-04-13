#include <iostream>
#include <GLX.h>
#include <glxTrackpad.h>

#include "Draggers/WorldPalette.h"

#include "debug.h"

using namespace std;

class PixelRect : public Glx::WorldPalette {
public:
  enum {LO,HI};
  enum {X,Y};
  enum {MOVE=Glx::WorldPalette::MOVE,
	RESIZE=Glx::WorldPalette::RESIZE,
	CELL};


  PixelRect(glx*,int,int,float,float,float,float);

  int  idlePick(glx*,int,int,void*);
  void handleDrag(glx*,int,int,void*);
  void handleMouseUp(glx*,void*);

  void draw(glx*, void*);
  void toCell(int,int,int*,int*);
  void unselectCells(void);


  int R,C;
  int iCell,jCell;
  int cells[2][2];
  bool needStart;
};

PixelRect::PixelRect(glx* env,int r, int c,
		     float x, float y, float w, float h) :
  Glx::WorldPalette(env,x,y,w,h),
  R(r),C(c),
  iCell(UNSELECTED),jCell(UNSELECTED),needStart(true)
{
  keepaspect=true;
  unselectCells();
}

void PixelRect::toCell(int x, int y, int* i, int *j)
{
  int pad = (int)(0.05 * itsPixelW);
  int xlo = (int)(itsPixelX+pad);
  int xhi = (int)(itsPixelX+itsPixelW-pad);
  int ylo = (int)(itsPixelY+pad);
  int yhi = (int)(itsPixelY+itsPixelH-pad);
  int wx = xhi-xlo,wy=yhi-ylo;

  y = winh-y;
  double xt = (double)(x-xlo)/wx;
  double yt = (double)(y-ylo)/wy;
  *i = (int)( xt * C);
  *j = (int)( yt * R);
  if( xt<0 || xt>1 ) *i = UNSELECTED;
  if( yt<0 || yt>1 ) *i = UNSELECTED;
}

int PixelRect::idlePick(glx* env,int x,int y,void* user)
{
  int i,j;

  itsSelectionDist = MAXFLOAT;
  toCell(x,y,&i,&j);
  if( i<0 || i>=C || j<0 || j>=R )
    return Glx::WorldPalette::idlePick(env,x,y,user);
  itsSelectionDist=0;
  iCell=i;
  jCell=j;
  return PixelRect::CELL + jCell*R + iCell;
}

void
PixelRect::unselectCells(void)
{
  cells[LO][X]=cells[LO][Y]=cells[HI][X]=cells[HI][Y]=UNSELECTED;
}

void 
PixelRect::handleDrag(glx* env,int x,int y,void* user)
{
  if( itsCurDragHandle >= PixelRect::CELL ){
    toCell(x,y,&iCell,&jCell);

    if( needStart ){
      cells[LO][X]=cells[HI][X]=iCell;
      cells[LO][Y]=cells[HI][Y]=jCell;
      needStart=false;
    } else {
      cells[HI][X]=iCell;
      cells[HI][Y]=jCell;
    }
  } else
    Glx::WorldPalette::handleDrag(env,x,y,user);
}

void 
PixelRect::handleMouseUp(glx* env,void* user)
{
  WorldPalette::handleMouseUp(env,user);
  needStart=true;
}

void 
PixelRect::draw(glx* env, void* user)
{  
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

  glBegin(GL_QUADS);
  for(int r=0;r<R;++r){
    double rf = ylo + r * dy;
    for(int c=0;c<C;++c){
      double cf = xlo + c * dx;
      VAR2(r,c);
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

  if( cells[0][X] != UNSELECTED && cells[0][Y] != UNSELECTED &&
      cells[1][X] != UNSELECTED && cells[1][Y] != UNSELECTED )
  {
    double ct,rt,clo[2],chi[2];

    int lx=0,hx=1,ly=0,hy=1;
    if( cells[0][X]>cells[1][X] ) {lx=1;hx=0;}
    if( cells[0][Y]>cells[1][Y] ) {ly=1;hy=0;}
    
    ct = (double)(cells[lx][X])/(C);
    rt = (double)(cells[ly][Y])/(R);
    clo[X] = xlo + ct * w;
    clo[Y] = ylo + rt * w;
    
    ct = (double)(cells[hx][X]+1)/(C);
    rt = (double)(cells[hy][Y]+1)/(R);
    chi[X] = xlo + ct * w;
    chi[Y] = ylo + rt * w;

    glColor3f(1,1,0);
    glLineWidth(3);
    glBegin(GL_LINE_LOOP);
    glVertex2d(clo[0],clo[1]);
    glVertex2d(clo[0],chi[1]);
    glVertex2d(chi[0],chi[1]);
    glVertex2d(chi[0],clo[1]);
    glEnd();
    glLineWidth(1);
  }
  
  if( iCell != UNSELECTED && jCell != UNSELECTED ){
    glColor3f(1,0,0);
    glBegin(GL_LINE_LOOP);
    double rf = ylo + jCell * dx;
    double cf = xlo + iCell * dx;
    glVertex3f(cf,   rf,0.01);
    glVertex3f(cf,   rf+dx,0.01);
    glVertex3f(cf+dx,rf+dx,0.01);
    glVertex3f(cf+dx,rf,0.01);
    glEnd();    
  }
  
  glLineWidth(lw);

  Glx::WorldPalette::draw(env,user);
}

void draw(glx* env, void*)
{
}

void initGL(glx* env, void* user)
{
  env->addDrawFunc(draw);
  Glx::Trackpad* tb = new Glx::Trackpad(env);
  PixelRect* pr = new PixelRect(env,5,5,0,0,1,1);
}

int main(int argc, char** argv)
{
  glx* env = new glx(initGL);
  env->mainLoop();
  return 1;
}
