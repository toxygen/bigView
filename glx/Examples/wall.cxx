#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <set>
#include <GLX.h>
#include <glxTrackpad.h>

#include "Draggers/WorldPalette.h"

#include "debug.h"

using namespace std;
int R=3,C=3;

struct Rect {
  Rect(void){
    lo[0]=hi[0]=lo[1]=hi[1]=0;
  }
  Rect(int l[2],int h[2]){
    lo[0]=l[0];
    hi[0]=h[0];
    lo[1]=l[1];
    hi[1]=h[1];
    reorder();
  }
  Rect(int x1,int y1,int x2,int y2)
  {
    lo[0]=x1;
    hi[0]=x2;
    lo[1]=y1;
    hi[1]=y2;
    reorder();
  }

  void swap(int& a, int& b){int save=a;a=b;b=save;}
  void reorder(void){
    if( lo[0]>hi[0] ) swap(lo[0],hi[0]);
    if( lo[1]>hi[1] ) swap(lo[1],hi[1]);
  }

  int lo[2],hi[2];

  friend std::ostream& operator<<(std::ostream& ostr, Rect& r){
    return ostr << "("<<r.lo[0]<<","<<r.lo[1]
		<<"),("<<r.hi[0]<<","<<r.hi[1]<<")";
  }

  bool operator==(const Rect& r1){
    if(lo[0]!=r1.lo[0])return false;
    if(hi[0]!=r1.hi[0])return false;
    if(lo[1]!=r1.lo[1])return false;
    if(hi[1]!=r1.hi[1])return false;
    return true;
  }
  
  friend bool intersect(const Rect& r1, const Rect& r2, Rect& res){
    res = Rect();
    for(int i=0;i<2;++i){
      if( r1.lo[i]>r2.hi[i] ||
	  r2.lo[i]>r1.hi[i] ||
	  r1.hi[i]<r2.lo[i] ||
	  r2.hi[i]<r1.lo[i] )
	return false;
    }
    for(int i=0;i<2;++i){
      res.lo[i]=r1.lo[i]>r2.lo[i] ? r1.lo[i] : r2.lo[i];
      res.hi[i]=r1.hi[i]<r2.hi[i] ? r1.hi[i] : r2.hi[i];
    }
    return true;
  }
};

class PixelRect : public Glx::WorldPalette {
public:
  enum {LO,HI};
  enum {X,Y};
  enum {MOVE=Glx::WorldPalette::MOVE,
	RESIZE=Glx::WorldPalette::RESIZE,
	CELL,RECT=1000};

  PixelRect(glx*,int,int);

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

  int selectedRect;

  void add(Rect& r);
  void del(Rect& r);

  void randColor(Glx::Vector&, int =5);

  std::vector<Rect> rects;
  std::vector<Glx::Vector> colors;
  std::set<Glx::Vector> colorset;
};

PixelRect::PixelRect(glx* env, int r, int c) :
  Glx::WorldPalette(env,0,0,1,1),
  R(r),C(c),
  iCell(UNSELECTED),jCell(UNSELECTED),selectedRect(UNSELECTED),needStart(true)
{
  if( r<c ) setSize( 1, (float)r/c );
  else setSize( (float)c/r,1 );
  keepaspect=true;
  unselectCells();
}

void PixelRect::add(Rect& r)
{ 
  Glx::Vector c;
  randColor(c);
  
  bool bad=false;
  vector<Rect>::iterator riter = rects.begin();
  for( ; riter != rects.end() && ! bad; ++ riter ){
    Rect i;
    bad=intersect(r,*riter,i);
  }
  if( ! bad ){
    rects.push_back(r); 
    colors.push_back(c);
    colorset.insert(c);
  }
}

void PixelRect::del(Rect& r)
{ 
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
  Rect r(i,j,i,j);

  bool found=false;
  iCell=jCell=selectedRect=UNSELECTED;
  int index=UNSELECTED;
  vector<Rect>::iterator riter = rects.begin();
  for(int i=0 ; riter != rects.end() && ! found ; ++i,++riter ){
    Rect junk;
    found=intersect(r,*riter,junk);
    if( found ) index=i;
  }
  itsSelectionDist=0;
  if( found ){
    selectedRect=PixelRect::RECT+index;
    return selectedRect;
  }
  iCell=i;
  jCell=j;
  return PixelRect::CELL + jCell*R + iCell;
}

void
PixelRect::unselectCells(void)
{
  cells[LO][X]=cells[LO][Y]=cells[HI][X]=cells[HI][Y]=UNSELECTED;
  selectedRect=UNSELECTED;
}

void 
PixelRect::handleDrag(glx* env,int x,int y,void* user)
{
  if( itsCurDragHandle >= PixelRect::CELL &&
      itsCurDragHandle <  PixelRect::RECT )
  {
    toCell(x,y,&iCell,&jCell);

    if( needStart ){
      cells[LO][X]=cells[HI][X]=iCell;
      cells[LO][Y]=cells[HI][Y]=jCell;
      needStart=false;
    } else {
      cells[HI][X]=iCell;
      cells[HI][Y]=jCell;
    }
  } else if(itsCurDragHandle >=PixelRect::RECT ) {
    
  } else
    Glx::WorldPalette::handleDrag(env,x,y,user);
}

void 
PixelRect::handleMouseUp(glx* env,void* user)
{
  WorldPalette::handleMouseUp(env,user);
  needStart=true;
}

void PixelRect::randColor(Glx::Vector& c, int K)
{
  double W=1./(K-1);

  bool found=true;
  while( found ){
    c[0] = W * (int)( drand48() * (float)K );
    c[1] = W * (int)( drand48() * (float)K );
    c[2] = W * (int)( drand48() * (float)K );
    if( c==Glx::Vector(0,0,0) ) continue;
    if( c==Glx::Vector(1,1,1) ) continue;
    set<Glx::Vector>::iterator iter = colorset.find(c);
    found = ( iter != colorset.end() );
  }
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
  double Z=-0.1,DZ=0.025;
  int lw;

  glGetIntegerv(GL_LINE_WIDTH,&lw);

  glPushAttrib(GL_DEPTH_BUFFER_BIT|GL_LIGHTING_BIT);
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);

  glColor3f(0.1,0.1,0.2);
  glBegin(GL_QUADS);
  for(int r=0;r<R;++r){
    double rf = ylo + r * dy;
    for(int c=0;c<C;++c){
      double cf = xlo + c * dx;
      VAR2(r,c);
      glVertex3f(cf,   rf,Z);
      glVertex3f(cf,   rf+dy,Z);
      glVertex3f(cf+dx,rf+dy,Z);
      glVertex3f(cf+dx,rf,Z);
    }
  }
  glEnd();
  Z += DZ;

  glColor3f(0,0,0);
  glLineWidth(2);
  glBegin(GL_LINES);
  for(int r=0;r<=R;++r){
    double rt = (double)r/(R);
    double rf = ylo + rt * (yhi-ylo);
    glVertex3f(xlo,rf,Z);
    glVertex3f(xhi,rf,Z);
  }
  for(int c=0;c<=C;++c){
    double ct = (double)c/(C);
    double cf = xlo + ct * (xhi-xlo);
    glVertex3f(cf,ylo,Z);
    glVertex3f(cf,yhi,Z);
  }
  glEnd();  
  Z += DZ;
  glLineWidth(1);

  if( cells[0][X] != UNSELECTED && cells[0][Y] != UNSELECTED &&
      cells[1][X] != UNSELECTED && cells[1][Y] != UNSELECTED )
  {
    double clo[2],chi[2];

    int lx=0,hx=1,ly=0,hy=1;
    if( cells[0][X]>cells[1][X] ) {lx=1;hx=0;}
    if( cells[0][Y]>cells[1][Y] ) {ly=1;hy=0;}
    clo[X] = xlo + cells[lx][X] * dx;
    clo[Y] = ylo + cells[ly][Y] * dy;
    chi[X] = xlo + (cells[hx][X]+1) * dx;
    chi[Y] = ylo + (cells[hy][Y]+1) * dy;

    glColor3f(1,1,0);
    glLineWidth(3);
    glBegin(GL_LINE_LOOP);
    glVertex3f(clo[0],clo[1],Z);
    glVertex3f(clo[0],chi[1],Z);
    glVertex3f(chi[0],chi[1],Z);
    glVertex3f(chi[0],clo[1],Z);
    glEnd();
    glLineWidth(1);
  }
  Z += DZ;
  
  if( iCell != UNSELECTED && jCell != UNSELECTED ){
    float f=0.01;
    glColor3f(1,0,0);
    glBegin(GL_LINE_LOOP);
    double rf = ylo + jCell * dy;
    double cf = xlo + iCell * dx;
    glVertex3f(cf+f,   rf+f,Z);
    glVertex3f(cf+f,   rf+dy-f,Z);
    glVertex3f(cf+dx-f,rf+dy-f,Z);
    glVertex3f(cf+dx-f,rf+f,Z);
    glEnd();    
  }
  Z += DZ;

  vector<Rect>::iterator riter = rects.begin();
  vector<Glx::Vector>::iterator citer = colors.begin();
  for(int i=0 ; riter != rects.end();++i,++riter,++citer ){
    Rect& r = *riter;
    Glx::Vector& c = *citer;
    float f=0.0;
    double clo[2],chi[2];
    clo[X] = xlo + r.lo[X] * dx;
    clo[Y] = ylo + r.lo[Y] * dy;
    chi[X] = xlo + (r.hi[X]+1) * dx;
    chi[Y] = ylo + (r.hi[Y]+1) * dy;

    if( i==selectedRect-PixelRect::RECT )
      f += 0.005;
    glBegin(GL_QUADS);
    glColor3fv(c);
    glVertex3f(clo[0]+f,clo[1]+f,Z);
    glVertex3f(clo[0]+f,chi[1]-f,Z);
    glVertex3f(chi[0]-f,chi[1]-f,Z);
    glVertex3f(chi[0]-f,clo[1]+f,Z);
    glEnd();
    glLineWidth(1);
    glColor3f(0,0,0);
    glBegin(GL_LINE_LOOP);
    glVertex3f(clo[0]+f,clo[1]+f,Z);
    glVertex3f(clo[0]+f,chi[1]-f,Z);
    glVertex3f(chi[0]-f,chi[1]-f,Z);
    glVertex3f(chi[0]-f,clo[1]+f,Z);
    glEnd();
    glLineWidth(2);
  }
  Z += DZ;

  glLineWidth(lw);
  Glx::WorldPalette::draw(env,user);
}

void draw(glx* env, void*)
{
}

void done(Glx::WorldPalette* wp, 
	  double, double,
	  double, double, void*)
{
  PixelRect* pr = dynamic_cast<PixelRect*>(wp);
  int lo[2]={pr->cells[PixelRect::LO][PixelRect::X],
	     pr->cells[PixelRect::LO][PixelRect::Y]};
  int hi[2]={pr->cells[PixelRect::HI][PixelRect::X],
	     pr->cells[PixelRect::HI][PixelRect::Y]};

  Rect r(lo,hi) ;
  pr->add( r );
}

void initGL(glx* env, void* user)
{
  int ww,wh;
  double vw,vh;

  if( R<C ){
    ww=500;wh=((float)R/C)*500.;
    vw=1.;vh=((float)R/C);
  } else {
    wh=500;ww=((float)C/R)* 500.;
    vh=1.;vw=(float)(C/R);
  }
  _VAR2(ww,wh);
  _VAR2(vw,vh);

  env->showAxis(false);
  env->setSize(ww,wh);
  env->addDrawFunc(draw);
  Glx::Trackpad* tb = new Glx::Trackpad(env); 

  double vmin[2]={0,0};
  double vmax[2]={vw,vh};
  
  tb->viewAll(vmin,vmax,0.);
  tb->itsTranslationSensitivity=1.;
  PixelRect* pr = new PixelRect(env,R,C);
  pr->setDoneCallback(done);
}

int main(int argc, char** argv)
{
  int c;

  srand48( (unsigned) time(NULL) );

  if( argc==1 ){
    cerr<<"usage:"<<argv[0]<<" MxN"<<endl;
    return 0;
  }
  sscanf(argv[optind],"%dx%d",&R,&C);
  VAR2(R,C);
  if( R<=0 || C<=0 ){
    cerr << "Error parsing wall size. Example wall size: 8x16"<<endl;
    return 0;
  }
  Rect ar(0,4,10,6);
  Rect br(4,0,6,10);
  Rect cr(4,0,6,10);
  Rect ir;

  _VAR(ar);
  _VAR(br);
  _VAR(cr);

  _VAR( (ar==br) );
  _VAR( (ar==cr) );
  _VAR( (cr==br) );
  _VAR( (br==cr) );
  _VAR( intersect(ar,br,ir) );
  _VAR(ir);

  glx* env = new glx(initGL);
  env->mainLoop();
  return 1;
}
