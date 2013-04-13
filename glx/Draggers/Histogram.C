#include <Draggers/Histogram.h>
#include <GLX.h>
#include <sstream>
#include <assert.h>

//#define DEBUG 1
#include "debug.h"

using namespace std;

const float MINSCALE = 1.0e-6;
const float MAXSCALE = 1e6;

const float XLO  = -0.5;
const float XHI  =  0.5;
const float YLO  =  0.0;
const float YHI  =  1.0;
const float YADJ = 0.05;

Glx::Histogram::Histogram(glx* env, std::vector<u_int64_t>& data,
			  int x, int y, int w, int h) :  
  Glx::Palette(env,x,y,w,h),
  itsData(data),
  selectionStart(Glx::Draggable::UNSELECTED),
  lastSelectionLo(Glx::Draggable::UNSELECTED),
  lastSelectionHi(Glx::Draggable::UNSELECTED),
  numBins( data.size() ),
  dragging(false),
  startOffsetX(0),startScaleY(0),
  saveOffset(0),saveScale(0),itsOffset(0)
{
  resetXforms();
  init(selectedBins,numBins);
  itsEnv->addMouseDownFunc(Glx::Histogram::mouseDown,glx::LEFT,this);
}

void Glx::Histogram::draw(glx* env,void *user)
{
  if( ! itsVisibleFlag )
    return;

  int lw;
  int vp[4];
  float curDepth = -0.2;
  glGetIntegerv(GL_LINE_WIDTH,&lw);
  glGetIntegerv(GL_VIEWPORT,&vp[0]);

  //glPushAttrib(GL_DEPTH_BUFFER_BIT|GL_LIGHTING_BIT);
  glPushAttrib(GL_DEPTH_BUFFER_BIT|GL_LIGHTING_BIT|GL_POLYGON_STIPPLE_BIT);
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glLineWidth(1);

  int border = HANDLESIZE;
  int envW   = env->winWidth();
  int envH   = env->winHeight();

  Glx::Palette::draw(env,user);

  vector<u_int64_t>::iterator iter = itsData.begin();
  u_int64_t maxCount=0,selectedCount=0;
  for( ; iter != itsData.end() ; ++iter ){
    if( *iter > maxCount )
      maxCount=*iter;
  }

  bool uselog=true;
  double normval = uselog ? log((double)(maxCount-1)) : (maxCount-1);

  /////////////////////////////
  // draw window-level stuff //
  /////////////////////////////

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0,envW,0,envH,-1,1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glColor3f(0,0,0);
  glEnable(GL_POLYGON_STIPPLE);
  glPolygonStipple(&Glx::stip[0]);
  glBegin(GL_QUADS);
  glVertex3f( itsX+itsW+5, itsY-5,         curDepth);
  glVertex3f( itsX-5,      itsY-5,         curDepth);
  glVertex3f( itsX-5,      itsY + itsH + 5,curDepth);
  glVertex3f( itsX+itsW+5, itsY + itsH + 5,curDepth);
  glEnd();
  glDisable(GL_POLYGON_STIPPLE);

  curDepth += 0.1;

  // = 'RESET' button 
  int lo[2]={itsX,itsY+itsH-HANDLESIZE};
  int hi[2]={itsX+HANDLESIZE,itsY+itsH};
  if( itsCurDragHandle==Glx::Histogram::RESET )
    glColor3f(0.5f,0.0f,0.0f);
  else
    glColor3f(0.0f,0.5f,0.0f);
  glBegin(GL_LINE_LOOP);
  glVertex3d(lo[0],lo[1],curDepth);
  glVertex3d(hi[0],lo[1],curDepth);
  glVertex3d(hi[0],hi[1],curDepth);
  glVertex3d(lo[0],hi[1],curDepth);
  glEnd();
  glBegin(GL_LINES);
  glVertex3d(lo[0]+2, lo[1]+HANDLESIZE/3,   curDepth);
  glVertex3d(hi[0]-1, lo[1]+HANDLESIZE/3,   curDepth);
  glVertex3d(lo[0]+2, lo[1]+2*HANDLESIZE/3, curDepth);
  glVertex3d(hi[0]-1, lo[1]+2*HANDLESIZE/3, curDepth);
  glEnd();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  curDepth += 0.1;

  ///////////////////////////////
  // draw viewport level stuff //
  ///////////////////////////////

  glViewport(itsX+border,itsY,projW(),projH());
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(XLO,XHI,YLO,YHI,-1,1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  float numBoxes = itsData.size();
  float binWidth = (float)1.0/numBoxes;

  // show histogram boxes and last selection

  glScalef(itsScale,1,0);     
  glTranslatef(itsOffset,0,0);

  iter = itsData.begin();
  for( int i=0 ; iter != itsData.end() ; ++iter,++i ){
    if( selectedBins[i] || *iter == 0)
      continue;
    glColor3f(0,1,0);

    float thisval = uselog ? log((double)*iter) : (double)*iter;
    float yLo = YLO+YADJ;
    float yHi = YLO+YADJ + (1-2*YADJ) * thisval/normval;
    float xLo = XLO + i * binWidth;
    float xHi = XLO + (i+1) * binWidth;
    glBegin(GL_LINE_LOOP);
    glVertex3f(xLo,yLo,curDepth);
    glVertex3f(xHi,yLo,curDepth);
    glVertex3f(xHi,yHi,curDepth);
    glVertex3f(xLo,yHi,curDepth);
    glEnd();
    if( i>=lastSelectionLo && i<=lastSelectionHi ){
      glBegin(GL_LINES);
      glVertex3f(xLo,YLO+YADJ/2,curDepth);
      glVertex3f(xHi,YLO+YADJ/2,curDepth);
      glEnd();
    }
  }

  curDepth += 0.1;

  iter = itsData.begin();
  for( int i=0 ; iter != itsData.end() ; ++iter,++i ){
    if( ! selectedBins[i] || *iter == 0 )
      continue;
    selectedCount += *iter;
    glColor3f(1,0,0);
    float thisval = uselog ? log((double)*iter) : (double)*iter;
    float yLo = YLO+YADJ;
    float yHi = YLO+YADJ + (1-2*YADJ) * thisval/normval;
    float xLo = XLO + i * binWidth;
    float xHi = XLO + (i+1) * binWidth;
    glBegin(GL_LINE_LOOP);
    glVertex3f(xLo,yLo,curDepth);
    glVertex3f(xHi,yLo,curDepth);
    glVertex3f(xHi,yHi,curDepth);
    glVertex3f(xLo,yHi,curDepth);
    glEnd();
    glBegin(GL_LINES);
    glVertex3f(xLo,YLO+YADJ/2,curDepth);
    glVertex3f(xHi,YLO+YADJ/2,curDepth);
    glEnd();
  }

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glLineWidth(lw);
  glViewport(vp[0],vp[1],vp[2],vp[3]);

  // Show Usage messages

  glColor3f(1,1,1);
  switch( itsCurDragHandle ){
    case Glx::Histogram::RESET:
      env->showMessage("Reset transforms");
      break;
    default:      
      break;
  }

  glPopAttrib();
}

void Glx::Histogram::handleMouseUp(glx* env,void* user)
{
  bool resetSelection=true;
  if( itsCurDragHandle==TRACKPAD )
    resetSelection=false;
  Glx::Palette::handleMouseUp(env,user);
  dragging=false;
  if( ! resetSelection )
    return;
  selectionStart=Glx::Draggable::UNSELECTED;
  if( ! Glx::any(selectedBins) )
    return;
  Glx::getSelection(selectedBins, &lastSelectionLo, &lastSelectionHi);
  callSelectionFuncs(lastSelectionLo,lastSelectionHi);
}

void Glx::Histogram::handleDrag(glx* env,int x,int y,void* user)
{
  int localY=env->winHeight()-y;

  FANCYMESG("Glx::Histogram::handleDrag");
  VAR(dragging);

  if( itsCurDragHandle == Glx::Palette::MOVE ||
      itsCurDragHandle == Glx::Palette::RESIZE )
  {
    Glx::Palette::handleDrag(env,x,y,user);
    return;
  }
      
  int selected = getSelected(x,localY);

  // see if we're scaling
  switch( env->buttonpressed ){
    case glx::LEFT:
      if( ! dragging ) // mouse down
      {
	reset(selectedBins);
	if( selected != Glx::Draggable::UNSELECTED ){
	  selectionStart = selected;
	  selectedBins[selectionStart-FIRSTBIN]=true;
	}
      }
      else // mouse process    
      {
	if( selected != Glx::Draggable::UNSELECTED ){
	  if( selectionStart == Glx::Draggable::UNSELECTED )
	    selectionStart=selected;      
	  int lo = (selected<=selectionStart) ? selected : selectionStart;
	  int hi = (lo==selectionStart) ? selected : selectionStart;
	  Glx::select(selectedBins, lo-FIRSTBIN, hi-FIRSTBIN);
	}
      }
      break;

    case glx::MIDDLE:
      if( ! dragging ) // mouse down
      {
	startScaleY = localY;
	saveScale = itsScale;
      }
      else // mouse process    
      {
	float deltay = localY - startScaleY;
	float dy = deltay / itsH;
	itsScale = saveScale * (float)(1.0-(float)dy);
	if(itsScale<MINSCALE)itsScale=MINSCALE;
	if(itsScale>MAXSCALE)itsScale=MAXSCALE;
      }
      // force
      itsCurDragHandle = Glx::Histogram::TRACKPAD; 
      break;

    case glx::RIGHT:
      if( ! dragging ) // mouse down
      {
	startOffsetX = x;
	saveOffset = itsOffset;
	MESGVAR2("mouse down",startOffsetX,saveOffset);
      }
      else // mouse process    
      {
	int border = HANDLESIZE * 2;
	int envW = env->winWidth();
	float dx = x - startOffsetX;
	MESGVAR2("before",itsOffset,dx);
	itsOffset = saveOffset + (float)dx/(projW()*itsScale);
	MESGVAR("after",itsOffset);
      }
      // force
      itsCurDragHandle = Glx::Histogram::TRACKPAD;
      break;
  } // switch
  dragging=true;
}

// Y is reversed from X here, 
// no need to invert
int Glx::Histogram::getSelected(int x, int y)
{
  double realX;
  pixelToWin(x,realX); // maps pixels to [-0.5 .. 0.5]

  if( x < itsX+HANDLESIZE || x > itsX + itsW - HANDLESIZE )
    return Glx::Draggable::UNSELECTED;    
  if( y < itsY || y > itsY + itsH )
    return Glx::Draggable::UNSELECTED;    

  if( realX < XLO || realX > XHI )
    return Glx::Draggable::UNSELECTED;
  
  // binf is the normalized index into the histogram
  float binf = realX - XLO; // 0..1
  float numBins = itsData.size();
  float binWidth = (float)1.0/numBins;
  assert(binf>=0 && binf<=1);
  
  // 0..itsData.size()-1
  float binNumber = binf/binWidth;

  // 0..itsData.size()-1
  int selected = (int)(binNumber);

  if( selected>=0 && selected<itsData.size() )
    return selected + FIRSTBIN;
  return Glx::Draggable::UNSELECTED;
}

int Glx::Histogram::idlePick(glx* env,int x,int y,void* user)
{
  int localY=env->winHeight()-y;
  itsSelectionDist = MAXFLOAT;
  reset(selectedBins);  
  int selected = getSelected(x,localY);
  if( selected != Glx::Draggable::UNSELECTED ){
    reset(selectedBins);
    selectedBins[selected-FIRSTBIN]=true;
    itsSelectionDist=0;
    callCandidateFuncs(selected-FIRSTBIN);
    return selected+FIRSTBIN;
  }

  int lo[2]={itsX+HANDLESIZE,itsY+HANDLESIZE};
  int hi[2]={itsX+itsW-HANDLESIZE,itsY+itsH-HANDLESIZE};

  // in the viewport?
  if( x>=lo[0] && x<=hi[0] && localY>=lo[1] && localY<=hi[1] )
  {
    itsSelectionDist=0;
    return Glx::Histogram::TRACKPAD;
  }

  // in the 'tray'?
  lo[0]=itsX;
  lo[1]=itsY+itsH-HANDLESIZE;
  hi[0]=itsX+HANDLESIZE;
  hi[1]=itsY+itsH;
  if(localY>=lo[1] && localY<=hi[1] ){
    if( x>=lo[0] && x<=hi[0] ){
      itsSelectionDist=0;
      return Glx::Histogram::RESET;
    } 
  }
  callCandidateFuncs(Glx::Draggable::UNSELECTED);
  return Glx::Palette::idlePick(env,x,y,user);
}

void
Glx::Histogram::callCandidateFuncs(int candidate)
{

  // call candidate funcs
  std::vector<Glx::Histogram::CandidatePair>::iterator iter = 
    itsCandidateFuncs.begin();
  for( ; iter != itsCandidateFuncs.end() ; ++iter ){
    Glx::Histogram::CandidatePair p = *iter;
    p.first(itsEnv,this,candidate,p.second);
  }
}

void 
Glx::Histogram::addCandidateFunc(Glx::Histogram::CandidateFunc func,void* user)
{
  itsCandidateFuncs.push_back( Glx::Histogram::CandidatePair(func,user) );
}

void 
Glx::Histogram::callSelectionFuncs(int lo, int hi)
{
  // call selection funcs
  std::vector<Glx::Histogram::SelectionPair>::iterator iter = 
    itsSelectionFuncs.begin();
  for( ; iter != itsSelectionFuncs.end() ; ++iter ){
    Glx::Histogram::SelectionPair p = *iter;
    p.first(itsEnv,this,lo,hi,p.second);
  }
}

void 
Glx::Histogram::addSelectionFunc(Glx::Histogram::SelectionFunc func,void* user)
{
  itsSelectionFuncs.push_back( Glx::Histogram::SelectionPair(func,user) );
}

void 
Glx::Histogram::mouseDown(glx* env, int x, int y, void* user ) 
{
  Glx::Histogram* _this = static_cast<Glx::Histogram*>(user);
  switch( _this->itsCurDragHandle  ){
    case Glx::Histogram::RESET:
      _this->resetXforms();
      break;
  }
}

void Glx::Histogram::resetXforms(void)
{
  itsOffset = 0.0;
  itsScale  = 1.0;
}

// maps real coords [-0.5 .. 0.5] into window coord [0..env->winWidth()]
void Glx::Histogram::winToPixel(double xf, int& x)
{
  double xMin = itsX + HANDLESIZE;
  double w    = itsW - 2*HANDLESIZE;
  double L    = XLO / itsScale - itsOffset;
  double R    = XHI / itsScale - itsOffset;
  x           = (int)(xMin + w * (xf - L)/(R-L));
}

// maps window coord [0..env->winWidth()] into real coords [-0.5 .. 0.5]
void Glx::Histogram::pixelToWin(int x, double& xf)
{
  double xMin = itsX + HANDLESIZE;
  double xMax = itsX+itsW - HANDLESIZE;
  double tx   = (x - xMin)/(xMax-xMin);
  double L    = XLO / itsScale - itsOffset;
  double R    = XHI / itsScale - itsOffset;
  xf          = L + tx * (R-L);
}

//////////////////////////////////////////////////////////////////////////
///////////////////////// Class ScalarHistogram //////////////////////////
//////////////////////////////////////////////////////////////////////////

Glx::ScalarHistogram::ScalarHistogram(glx* env, 
				      std::vector<u_int64_t>& bins,
				      double smin, double smax,
				      int x, int y, int w, int h) :
  Glx::Histogram(env,bins,x,y,w,h),
  scalarMin(smin),scalarMax(smax),lastX(0)
{
  lo=scalarMin;
  hi=scalarMax;
  thumbs[0].itsParent=this;
  thumbs[0].itsValue = &lo;
  thumbs[1].itsParent=this;
  thumbs[1].itsValue = &hi;
}

int Glx::ScalarHistogram::idlePick(glx* env,int x,int y,void* user)
{
  int localY=env->winHeight()-y; // ARG!!!
  pixelToWin(x,lastX);

  if( x >= itsX && x <= (itsX+itsW) &&
      localY >= itsY && localY <= (itsY+itsH) )
  {   
    if( thumbs[0].idlePick(env,x,localY,user) != Glx::Draggable::UNSELECTED )
      return Glx::ScalarHistogram::LO;
    if( thumbs[1].idlePick(env,x,localY,user) != Glx::Draggable::UNSELECTED )
      return Glx::ScalarHistogram::HI;
  }
  return Glx::Histogram::idlePick(env,x,y,user);
}

void 
Glx::ScalarHistogram::update(double smin, double smax)
{
  lo=scalarMin=smin;
  hi=scalarMax=smax;
}

void    
Glx::ScalarHistogram::draw(glx* env, void* user)
{
  int lw,vp[4];
  ostringstream ostr;
  Glx::Histogram::draw(env,user);
  
  glGetIntegerv(GL_LINE_WIDTH,&lw);
  glGetIntegerv(GL_VIEWPORT,&vp[0]);

  glPushAttrib(GL_DEPTH_BUFFER_BIT|GL_LIGHTING_BIT);
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glLineWidth(1);

  glViewport(itsX+Glx::Palette::HANDLESIZE,itsY,projW(),projH());
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(XLO,XHI,YLO,YHI,-1,1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glScalef(itsScale,1,0);     
  glTranslatef(itsOffset,0,0);

  glColor3f(1,1,1);
  thumbs[0].draw(env,user);  
  thumbs[1].draw(env,user);

  if( itsCurDragHandle==Glx::ScalarHistogram::LO )
  {
    glColor3f(1,0,0);
    thumbs[0].draw(env,user);    
  } 
  else if( itsCurDragHandle==Glx::ScalarHistogram::HI )
  {
    glColor3f(1,0,0);
    thumbs[1].draw(env,user);
  }

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glViewport(vp[0],vp[1],vp[2],vp[3]);
  glLineWidth(lw);

  if( itsCurDragHandle != Glx::Draggable::UNSELECTED ){
    float range=scalarMax-scalarMin;
    float t = range*lastX + scalarMin + range/2;
    ostr << "scalar = " << t;
    string str = ostr.str();
    env->showMessage(itsX,itsY+itsH,str);
  }
  // realX is the real X coord of the mouse

  glPopAttrib();
}

double proj(double x)
{
}

void 
Glx::ScalarHistogram::handleDrag(glx* env,int x,int y,void* user)
{
  int ilo,ihi;
  double projX,dx;
  double flo,fhi;
  double t,range=scalarMax-scalarMin;
  
  switch( itsCurDragHandle ){
    case Glx::ScalarHistogram::LO:   
      thumbs[1].calcXDX(projX,dx);
      winToPixel(projX,ihi); 
      if( x < ihi - Glx::ScalarHistogram::TOL ){
	pixelToWin(x,flo);
	lastX = flo;
	float range=scalarMax-scalarMin;
	float t = range*flo + scalarMin + range/2;	
	lo = t;
      }
      break;
    case Glx::ScalarHistogram::HI:
      thumbs[0].calcXDX(projX,dx);
      winToPixel(projX,ilo); 
      if( x > ilo + Glx::ScalarHistogram::TOL ){
	pixelToWin(x,fhi);
	lastX = fhi;
	float t = range*fhi + scalarMin + range/2;
	hi = t;	
      }
      break;
    default:
      Glx::Histogram::handleDrag(env,x,y,user);
      break;
  }
}

//////////////////////////////////////////////////////////////////////////
/////////////////// class Glx::ScalarHistogram::Thumb ////////////////////
//////////////////////////////////////////////////////////////////////////

int Glx::ScalarHistogram::Thumb::idlePick(glx *env, int x, int y, void*)
{
  int thumbX;
  double projX,dx;
  calcXDX(projX,dx);
  
  itsParent->winToPixel(projX,thumbX);
  bool in = true;
  if( fabs((float)thumbX - x) > Glx::ScalarHistogram::TOL )
    in = false;
  return in ? itsID : UNSELECTED;
}

void Glx::ScalarHistogram::Thumb::draw(glx* env,void *)
{
  double projX,dx;
  calcXDX(projX,dx);

  glLineWidth(2);
  glBegin(GL_LINES);
  glVertex2d(projX,0);
  glVertex2d(projX,1);
  glEnd();
  glBegin(GL_TRIANGLES);
  glVertex2d(projX-dx,0);
  glVertex2d(projX,   YADJ);
  glVertex2d(projX+dx,0);
  glEnd();
}

void Glx::ScalarHistogram::Thumb::calcXDX(double& x,double& dx)
{
  double smin=itsParent->scalarMin,smax=itsParent->scalarMax;
  double range=smax-smin;
  x = (*itsValue-smin)/range - 0.5;
  dx = ((YADJ/2)/itsParent->aspect())/itsParent->itsScale;
}

void Glx::ScalarHistogram::handleMouseUp(glx* env,void* user)
{
  if( itsCurDragHandle==Glx::ScalarHistogram::LO || 
      itsCurDragHandle==Glx::ScalarHistogram::HI ){

    callFuncs(itsUpFuncs);
  }
  Glx::Histogram::handleMouseUp(env,user);
}

//////////////////////////////////////////////////////////////////////////
////////////////////// vector<bool> helper methods ///////////////////////
//////////////////////////////////////////////////////////////////////////

bool Glx::any(vector<bool>& bits)
{
  bool foundAny=false;
  vector<bool>::iterator binIter=bits.begin();
  for( ; binIter != bits.end() && ! foundAny ; ++binIter )
    foundAny = *binIter;
  return foundAny;
}

void Glx::init(vector<bool>& bits, int size)
{
  bits.clear();
  bits.reserve(size);
  for(int i=0 ; i<size ; ++i )
    bits.push_back(false);
}

void Glx::reset(vector<bool>& bits)
{
  vector<bool>::iterator iter = bits.begin();
  for( ; iter != bits.end() ; ++iter )
    *iter = false;
}

void Glx::select(vector<bool>& bits, int index)
{
  reset(bits);
  bits[index]=true;
}

void Glx::select(vector<bool>& bits, int lo, int hi)
{
  reset(bits);
  for(int i=lo ; i <= hi ; ++i )
    bits[i]=true;
}

void Glx::getSelection(vector<bool>& bits, int* lo, int* hi)
{
  *lo=bits.size()+1;
  *hi=-1;
  for(int i=0; i<bits.size() ; ++i){
    if( bits[i] ){
      if(i<*lo) *lo = i;
      if(i>*hi) *hi = i;
    }
  }
}
