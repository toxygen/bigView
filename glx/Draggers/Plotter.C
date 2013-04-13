#include <GLX.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <Draggers/Palette.h>
#include "Plotter.h"

//#define DEBUG 1 
#include "debug.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////
///////////////////////// class Glx::LinePlotter /////////////////////////
//////////////////////////////////////////////////////////////////////////

Glx::LinePlotter::LinePlotter(glx* env, std::vector<float>& data,
			      int x, int y, int w, int h) :
  Glx::Palette(env,x,y,w,h),
  itsData(data)
{
  
}

int Glx::LinePlotter::idlePick(glx* env,int x,int y,void* user)
{
  itsLastXcoord=x;
  itsLastYcoord=y;
  int res = Glx::Palette::idlePick(env,x,y,user);
  itsEnv->wakeup();
  return res;
}

void 
Glx::LinePlotter::draw(glx* env, void *user)
{
  if( ! itsVisibleFlag )
    return;

  Glx::Palette::draw(env,user);

  if( itsData.size()==0 )
    return;

  int envW = env->winWidth();
  int envH = env->winHeight();

  glPushAttrib(GL_LIGHTING);
  glDisable(GL_LIGHTING);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0,envW,0,envH,-1,1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // draw faint outline of spline drawing area
  glLineWidth(1);
  glColor3f(0.0,0.1,0.0);
  glBegin(GL_LINE_LOOP);
  glVertex3d(itsX+BORDER,     itsY+BORDER,-0.1);
  glVertex3d(itsX+itsW-BORDER,itsY+BORDER,-0.1);
  glVertex3d(itsX+itsW-BORDER,itsY+itsH-BORDER,-0.1);
  glVertex3d(itsX+BORDER,     itsY+itsH-BORDER,-0.1);
  glEnd();

  glColor3f(0.0f,1.0f,0.0f);

  int N = itsData.size();
  int minX = itsX + Glx::LinePlotter::BORDER;
  int maxX = itsX + itsW - Glx::LinePlotter::BORDER;
  int plotW = maxX-minX;

  int minY = itsY + Glx::LinePlotter::BORDER;
  int maxY = itsY + itsH - Glx::LinePlotter::BORDER;
  int plotH = maxY-minY;

  float dx = 1.0/(float)itsData.size();
  float fx = 0.0f;
  
  vector<float>::iterator iter = itsData.begin();
  float min=MAXFLOAT;
  float max=-MAXFLOAT;
  for( int i=0 ; iter != itsData.end() ; iter++, i++ ){
    float fy = *iter;
    if( min > fy ) min = fy;
    if( max < fy ) max = fy;
  }
  float rangeY = max-min;
  glBegin(GL_POINTS);
  iter = itsData.begin();
  for( int i=0 ; iter != itsData.end() ; iter++, i++ ){
    float fx = i*dx;
    float fy = (*iter-min)/rangeY;
    int x = minX + (int)(fx * plotW);
    int y = minY + (int)(fy * plotH);
    glVertex2f(x,y);
  }
  glEnd();

  glLineWidth(1);
  glBegin(GL_LINE_STRIP);
  iter = itsData.begin();
  for( int i=0 ; iter != itsData.end() ; iter++, i++ ){
    float fx = i*dx;
    float fy = (*iter-min)/rangeY;
    int x = minX + (int)(fx * plotW);
    int y = minY + (int)(fy * plotH);
    glVertex2f(x,y);
  }
  glEnd();

  if( itsCurDragHandle == Glx::Palette::MOVE ){
    char buffer[256];
    int xPos=itsLastXcoord,yPos=itsLastYcoord;
    float xCoord = (float)(xPos-minX)/(float)plotW;
    int index = (int)(xCoord/dx);
    if( index>=0 && index<itsData.size() ){
      float xf = index*dx;
      float yf = (itsData[index]-min)/rangeY;
      int x = minX + (int)(xf * plotW);
      int y = minY + (int)(yf * plotH);
      glBegin(GL_LINES);
      glVertex2f(minX,y);
      glVertex2f(maxX,y);
      glVertex2f(x,minY);
      glVertex2f(x,maxY);
      glEnd();
    }
  }

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();

  if( itsCurDragHandle == Glx::Palette::MOVE ){
    char buffer[256];
    int xPos=itsLastXcoord,yPos=itsLastYcoord;
    //getPixelCoords(env,itsLastXcoord,itsLastYcoord,&xPos, &yPos);
    float xf = (float)(xPos-minX)/(float)plotW;
    int index = (int)(xf/dx);
    if( index>=0 && index<itsData.size() ){
      sprintf(buffer,"[%d]: %f",index,itsData[index]);
      env->showMessage(itsX,itsY,buffer);
    }
  }
}


//////////////////////////////////////////////////////////////////////////
//////////////////////// class Glx::MultiPlotter /////////////////////////
//////////////////////////////////////////////////////////////////////////


Glx::MultiPlotter::MultiPlotter(glx* env, 
				std::vector<std::vector<float>*> &data,
				std::vector<std::string*> &names,
				int x, int y, int w, int h) : 
  itsDataSet(data), 
  itsNames(names), 
  itsZeroMin(false),
  Glx::Palette(env,x,y,w,h)
{
  
}

int Glx::MultiPlotter::idlePick(glx* env,int x,int y,void* user)
{
  itsLastXcoord=x;
  itsLastYcoord=y;
  int res = Glx::Palette::idlePick(env,x,y,user);
  itsEnv->wakeup();
  return res;
}


void 
Glx::MultiPlotter::draw(glx* env, void *user)
{
  if( ! itsVisibleFlag )
    return;
  Glx::Palette::draw(env,user);
  if( itsDataSet.size()==0 )
    return;

  int Dcnt = itsDataSet.size();
  int maxPoints = 0;
  for (int i=0; i<Dcnt; ++i) {
    int sz = itsDataSet[i]->size();
    if (sz > maxPoints)
      maxPoints = sz;
  }

  if (maxPoints == 0)
    return;

  int envW = env->winWidth();
  int envH = env->winHeight();

  glPushAttrib(GL_LIGHTING);
  glDisable(GL_LIGHTING);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0,envW,0,envH,-1,1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // draw faint outline of spline drawing area
  glLineWidth(1);
  glColor3f(0.0,0.1,0.0);
  glBegin(GL_LINE_LOOP);
  glVertex3d(itsX+BORDER,     itsY+BORDER,-0.1);
  glVertex3d(itsX+itsW-BORDER,itsY+BORDER,-0.1);
  glVertex3d(itsX+itsW-BORDER,itsY+itsH-BORDER,-0.1);
  glVertex3d(itsX+BORDER,     itsY+itsH-BORDER,-0.1);
  glEnd();

  glColor3f(0.0f,1.0f,0.0f);

  int minX = itsX + (int)(itsW*0.05);
  int maxX = itsX + itsW - (int)(itsW*0.05);
  int plotW = maxX-minX;

  int minY = itsY + (int)(itsH*0.05);
  int maxY = itsY + itsH - (int)(itsH*0.05);
  int plotH = maxY-minY;

  float dx = 1.0/(float)maxPoints;
  float fx = 0.0f;
  
  vector<vector<float>*>::iterator setiter;
  vector<float>::iterator iter;

  float min=MAXFLOAT;
  float max=-MAXFLOAT;
  for (setiter = itsDataSet.begin(); setiter != itsDataSet.end(); ++setiter)
  {
    for(iter = (*setiter)->begin(); iter != (*setiter)->end() ; iter++ ) {
      float fy = *iter;
      if( min > fy ) min = fy;
      if( max < fy ) max = fy;
    }
  }

  if (itsZeroMin)
    min = 0.0;

  float rangeY = max-min;

  glBegin(GL_POINTS);
  int di = 0;
  for (setiter=itsDataSet.begin(); setiter!=itsDataSet.end(); ++setiter, ++di)
  {
    int ioff = di%5;
    float green = (ioff <= 1 || ioff == 4) ? 1.0 : 0.0;
    float red = (ioff >= 1) ? 1.0 : 0.0;
    float blue = (ioff >= 3) ? 1.0 : 0.0;
    glColor3f(red,green,blue);
    iter = (*setiter)->begin();
    for( int j=0 ; iter != (*setiter)->end() ; iter++, j++ ){
      float fx = j*dx;
      float fy = (*iter-min)/rangeY;
      int x = minX + (int)(fx * plotW);
      int y = minY + (int)(fy * plotH);
      glVertex2f(x,y);
    }
  }
  glEnd();

  glLineWidth(1);

  di = 0;
  for (setiter=itsDataSet.begin(); setiter!=itsDataSet.end(); ++setiter, ++di)
  {
    int ioff = di%5;
    float green = (ioff <= 1 || ioff == 4) ? 1.0 : 0.0;
    float red = (ioff >= 1) ? 1.0 : 0.0;
    float blue = (ioff >= 3) ? 1.0 : 0.0;
    glColor3f(red,green,blue);
    glBegin(GL_LINE_STRIP);
    iter = (*setiter)->begin();
    for( int j=0 ; iter != (*setiter)->end() ; iter++, j++ ){
      float fx = j*dx;
      float fy = (*iter-min)/rangeY;
      int x = minX + (int)(fx * plotW);
      int y = minY + (int)(fy * plotH);
      glVertex2f(x,y);
    }
    glEnd();
  }

  float selected_fval;
  int selected_index = -1;
  const char *selected_name;

  if( itsCurDragHandle == Glx::Palette::MOVE ){

    char buffer[256];
    int xPos=(int)itsLastXcoord,yPos=(int)itsLastYcoord;
    float xCoord = (float)(xPos-minX)/(float)plotW;
    float yCoord = (float)(yPos-minY)/(float)plotH;
    int index = (int)(xCoord/dx);
    if( index>=0 && index<maxPoints ){
      selected_index = index;
      float xf = index*dx;
      float yf, closesty = plotH;
      for (setiter=itsDataSet.begin(); setiter!=itsDataSet.end(); ++setiter)
      {
        float fval = (*(*setiter))[index];
        float tyf = (fval-min)/rangeY;
        float tyfa = fabs(tyf-yCoord);
        if (tyfa < closesty) {
          closesty = tyfa, yf = tyf;
          selected_fval = fval;
          selected_name = itsNames[setiter-itsDataSet.begin()]->c_str();
        }
      }
      int x = minX + (int)(xf * plotW);
      int y = minY + (int)(yf * plotH);
      glBegin(GL_LINES);
      glVertex2f(minX,y);
      glVertex2f(maxX,y);
      glVertex2f(x,minY);
      glVertex2f(x,maxY);
      glEnd();
    }
  }

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();

  if (selected_index >= 0) {
    char buffer[256];
    sprintf(buffer,"%s[%d]: %f", selected_name, selected_index, selected_fval);
    env->showMessage(buffer);
  }

}

//////////////////////////////////////////////////////////////////////////
//////////////////////// class Glx::PointPlotter /////////////////////////
//////////////////////////////////////////////////////////////////////////

Glx::PointPlotter::PointPlotter(glx* env, 
				std::vector<double>* xdata,
				std::vector<double>* ydata,
				int x, int y, int w, int h) :
  Glx::Palette(env,x,y,w,h),
  itsXdata(xdata),itsYdata(ydata),needclear(true),N(0),
  selectionMode(BOXMODE),invert(false)
{
  updateRanges();
  updateWinvars();
}

void 
Glx::PointPlotter::setData(vector<double>* xdata, vector<double>* ydata)
{
  itsXdata=xdata;
  itsYdata=ydata;
  clear();
  updateRanges();
  updateWinvars();
}

void Glx::PointPlotter::viewHasChanged(glx*)
{
  updateWinvars();
}

void Glx::PointPlotter::updateRanges(void)
{
  if( itsXdata==0 || itsYdata==0 ) return;
  vector<double>& xdata = *itsXdata;
  vector<double>& ydata = *itsYdata;
  int nPoints = xdata.size();

  dmin[0]=dmin[1]=MAXFLOAT;
  dmax[0]=dmax[1]=-MAXFLOAT;
  for( int i=0 ; i<nPoints ; i++ ){
    if( dmin[0] > xdata[i] ) dmin[0] = xdata[i];
    if( dmax[0] < xdata[i] ) dmax[0] = xdata[i];
    if( dmin[1] > ydata[i] ) dmin[1] = ydata[i];
    if( dmax[1] < ydata[i] ) dmax[1] = ydata[i];
  }
  range[0] = dmax[0]-dmin[0];
  range[1] = dmax[1]-dmin[1];
}

void
Glx::PointPlotter::updateWinvars(void)
{
  envW = itsEnv->winWidth();
  envH = itsEnv->winHeight();
  minX = itsX + Glx::PointPlotter::BORDER;
  maxX = itsX + itsW - Glx::PointPlotter::BORDER;
  plotW = maxX-minX;
  minY = itsY + Glx::PointPlotter::BORDER;
  maxY = itsY + itsH - Glx::PointPlotter::BORDER;
  plotH = maxY-minY;
}

void 
Glx::PointPlotter::setSelected(vector<bool>* boolvec)
{
  selected=boolvec;
}

void Glx::PointPlotter::pix2real(float ix, float iy, float* fx, float* fy)
{
  float tx = (ix-minX)/plotW;
  float ty = (envH-iy-minY)/plotH;
  * fx = dmin[0] + tx * range[0];
  * fy = dmin[1] + ty * range[1];  
}

void Glx::PointPlotter::real2pix(float fx, float fy, float* ix, float* iy)
{
  float tx = (fx-dmin[0])/range[0];
  float ty = (fy-dmin[1])/range[1];
  * ix = minX + tx * plotW;
  * iy = minY + ty * plotH;  
}

int Glx::PointPlotter::idlePick(glx* env,int x,int y,void* user)
{
  int candidate=Glx::Draggable::UNSELECTED;
  int lo[2]={itsX+BORDER,itsY+BORDER};
  int hi[2]={itsX+itsW-BORDER,itsY+itsH-BORDER};

  itsLastXcoord=x;
  itsLastYcoord=y;

  // outside plot area?
  if( x<lo[0] || x>hi[0] || y>envH-lo[1] || y<envH-hi[1] )
  {
    candidate = Glx::Palette::idlePick(env,x,y,user);
  }
  else // in the viewport
  {
    itsSelectionDist=0;
    candidate=Glx::PointPlotter::DRAW;
  }

  itsEnv->wakeup();
  return candidate;
}

void Glx::PointPlotter::handleMouseUp(glx* env,void* user)
{
  FANCYMESG("Glx::PointPlotter::handleMouseUp");
  Glx::Palette::handleMouseUp(env,user);
  updateWinvars();

  switch( selectionMode ){
    case LASSOMODE:
      if( N>0 ){
	pntx.push_back(pntx[0]);
	pnty.push_back(pnty[0]);  
      }    
    case BOXMODE:
      break;
  }
  needclear=true;

  doneWithDrag();
}

void Glx::PointPlotter::handleDrag(glx* env,int x,int y,void* user)
{
  FANCYMESG("Glx::PointPlotter::handleDrag");
  if( itsCurDragHandle == Glx::Palette::MOVE ||
      itsCurDragHandle == Glx::Palette::RESIZE )
  {
    Glx::Palette::handleDrag(env,x,y,user);
    updateWinvars();
    return;
  }
  float fx,fy;
  pix2real(x,y,&fx,&fy);

  switch( selectionMode ){
    case LASSOMODE:
      if(needclear)
	clear();
      if(N>0){
	if( x==itsLastXcoord && y==itsLastYcoord )
	  return;
	static const float EPS = 1.0e-6;
	float pdx=(float)pntx[N]-pntx[N-1];
	float pdy=(float)pnty[N]-pnty[N-1];
	float cdx=(float)fx-pntx[N];
	float cdy=(float)fy-pnty[N];
	if( fabs(pdx-cdx)<EPS && fabs(pdy-cdy)<EPS ){
	  pntx[N]=fx;
	  pnty[N]=fy;
	  return;
	}
      }
      
      pntx.push_back(fx);
      pnty.push_back(fy);
      ++N;
      break;
    case BOXMODE:
      MESG("BOXMODE");
      if( needclear ){
	needclear=false;
	startbox[0]=fx;
	startbox[1]=fy;
      }

      if( fx<startbox[0] ){
	lobox[0]=fx;
	hibox[0]=startbox[0];
      } else {
	lobox[0]=startbox[0];
	hibox[0]=fx;
      }

      if( fy<startbox[1] ){
	lobox[1]=fy;
	hibox[1]=startbox[1];
      } else {
	lobox[1]=startbox[1];
	hibox[1]=fy;
      }

      break;
  }
  itsLastXcoord=x;
  itsLastYcoord=y;
}

void 
Glx::PointPlotter::keyEvent(int key, bool ctl, bool shift, bool alt)
{
  switch( key ){
    case XK_i:
    case XK_I:
      invert = ! invert;
      doneWithDrag();
      break;

    case XK_m:
    case XK_M:
      switch(selectionMode) {
	case BOXMODE:
	  selectionMode=LASSOMODE;
	  break;
	case LASSOMODE:
	  selectionMode=BOXMODE;
	  break;
      }
      doneWithDrag();
      break;
  }
  itsEnv->wakeup();
}

void 
Glx::PointPlotter::draw(glx* env, void *user)
{
  FANCYMESG("draw");
  if( ! itsVisibleFlag )
    return;

  Glx::Palette::draw(env,user);

  if( itsXdata==0 || itsYdata==0 )
    return;

  if( itsXdata->size()==0 || itsYdata->size()==0 )
    return;

  envW = itsEnv->winWidth();
  envH = itsEnv->winHeight();

  int border = Glx::PointPlotter::BORDER;
  vector<double>& xdata = *itsXdata;
  vector<double>& ydata = *itsYdata;

  int vp[4],lw,ps;
  glGetIntegerv(GL_VIEWPORT,vp);
  glGetIntegerv(GL_LINE_WIDTH,&lw);
  glGetIntegerv(GL_POINT_SIZE,&ps);

  // draw faint outline of drawing area

  // glViewport(itsX,itsY,itsW,itsH);

  glPushAttrib(GL_LIGHTING);
  glDisable(GL_LIGHTING);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  //glOrtho(0,itsW,0,itsH,-1,1);
  glOrtho(0,envW,0,envH,-1,1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glLineWidth(1);
  glColor3f(0,.1,0);

  glBegin(GL_LINE_LOOP);
  glVertex3f(itsX+border,     itsY+border,-0.1);
  glVertex3f(itsX+itsW-border,itsY+border,-0.1);
  glVertex3f(itsX+itsW-border,itsY+itsH-border,-0.1);
  glVertex3f(itsX+border,     itsY+itsH-border,-0.1);
  glEnd();

  // draw mouse coords
  if( itsCurDragHandle == Glx::PointPlotter::DRAW ){
    int ix=itsLastXcoord;
    int iy=envH-itsLastYcoord;
    
    float d=border/2;
    glColor3f(0,1,0);
    glBegin(GL_LINES);
    glVertex3f(ix,itsY-d,-0.1);
    glVertex3f(ix,itsY+d,-0.1);
    glVertex3f(ix,itsY+itsH-d,-0.1);
    glVertex3f(ix,itsY+itsH+d,-0.1);
    glVertex3f(itsX-d,iy,-0.1);
    glVertex3f(itsX+d,iy,-0.1);
    glVertex3f(itsX+itsW-d,iy,-0.1);
    glVertex3f(itsX+itsW+d,iy,-0.1);
    glEnd();
  }

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  // plot points

  glColor3f(0,1,0);

  int nPoints = itsXdata->size();
  float dx = 1.0/(float)N;
  float fx = 0.0f;

  // glViewport(minX,minY,plotW,plotH);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  //glOrtho(0,1,0,1,-1,1);
  glOrtho(0,envW,0,envH,-1,1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  if( selectionMode==LASSOMODE )
  {    
    glBegin(GL_LINE_STRIP);
    for(int i=0;i<pntx.size();++i){
      float x,y;
      real2pix(pntx[i],pnty[i],&x,&y);
      glVertex3f(x,y,0.15);
    }
    glEnd();
  } 
  else if(selectionMode==BOXMODE )
  {
    float lo[2],hi[2];
    real2pix(lobox[0],lobox[1],&lo[0],&lo[1]);
    real2pix(hibox[0],hibox[1],&hi[0],&hi[1]);
    glBegin(GL_LINE_LOOP);
    glVertex3f(lo[0],lo[1],0.15);
    glVertex3f(lo[0],hi[1],0.15);
    glVertex3f(hi[0],hi[1],0.15);
    glVertex3f(hi[0],lo[1],0.15);
    glEnd();
  }

  glPushAttrib(GL_DEPTH_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);

  // draw non selected points
  glPointSize(1);
  glColor3f(0,1,0);
  glBegin(GL_POINTS);  
  for( int i=0 ; i<nPoints ; i++ ){
    float x,y;
    real2pix(xdata[i],ydata[i],&x,&y);
    if( selected==NULL || ! selected->operator[](i) )
      glVertex3f(x,y,0.1);
  }
  glEnd();

  // draw selected points
  glPointSize(3);
  glColor3f(1,0,0);
  glBegin(GL_POINTS);   
  for( int i=0 ; i<nPoints ; i++ ){
    float x,y;
    real2pix(xdata[i],ydata[i],&x,&y);
    if( selected && selected->operator[](i) ) 
      glVertex3f(x,y,0.1);
  }
  glEnd();
  glPopAttrib(); // depth

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glPopAttrib(); // lighting
  glViewport(vp[0],vp[1],vp[2],vp[3]);
  glLineWidth(lw);
  glPointSize(ps);

  if( itsCurDragHandle == Glx::PointPlotter::DRAW ){
    ostringstream ostr;
    float lo[2],hi[2];
    switch( selectionMode ){
      case BOXMODE:
	ostr << "Selection mode: BOX [hit 'm' to change]";
	env->showMessage(ostr.str());
	ostr.str("");
	pix2real(lobox[0],lobox[1],&lo[0],&lo[1]);
	pix2real(hibox[0],hibox[1],&hi[0],&hi[1]);
	ostr << "X: ["<<lo[0]<<","<<hi[0]<<"]";
	env->showMessage(ostr.str());
	ostr.str("");
	ostr << "Y: ["<<lo[1]<<","<<hi[1]<<"]";
	env->showMessage(ostr.str());
	break;
      case LASSOMODE: 
	ostr << "Selection mode: LASSO [hit 'm' to change]";
	env->showMessage(ostr.str());
	break;
    }
  }
  
  if( itsCurDragHandle == Glx::PointPlotter::DRAW ){
    ostringstream ostr;
    float xval,yval;
    pix2real(itsLastXcoord,itsLastYcoord,&xval,&yval);
    int d=border/2;

    ostr << xval;
    env->showMessage(itsLastXcoord+d,minY-3*d,ostr.str());
    ostr.str("");

    //hack to keep text from obscuring the points
    int ycoord = (itsLastXcoord<(itsX+120)) ? 
      envH-itsLastYcoord+20 : envH-itsLastYcoord+d;

    ostr << yval;
    env->showMessage(minX+d,ycoord,ostr.str());

    ostr.str("");
  }

}


void Glx::PointPlotter::clear(void)
{
  needclear=false;
  switch( selectionMode ){
    case LASSOMODE:
      N=0;
      pntx.clear();
      pnty.clear();
    case BOXMODE:
      break;
  }
}
