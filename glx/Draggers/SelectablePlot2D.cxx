#include <iostream>
#include <vector>
#include <Draggers/SelectablePlot2D.h>
#include "pdebug.h"
using namespace std;

using namespace std;

static const float EPS = 1.0e-6;

Glx::SelectablePlot2D::SelectablePlot2D(glx* env, 
					vector<double>* x,
					vector<double>* y) : itsEnv(env)
{
  plotter=new Glx::PointPlotter(env,x,y);
  plotter->addUpFunc(SelectablePlot2D::mouseUp,this);
  plotter->setSelected(&this->selected);
  update();
}

Glx::SelectablePlot2D::~SelectablePlot2D(void)
{
  delete plotter;
}

void
Glx::SelectablePlot2D::draw(glx* env, void *user)
{
  plotter->draw(env,user);
}

void 
Glx::SelectablePlot2D::setData(vector<double>* x,vector<double>* y)
{
  int npoints=x->size();
  selected.reserve(npoints);
  selected.assign(npoints,plotter->invert);

  plotter->setData(x,y);
  update();      
}

void 
Glx::SelectablePlot2D::addUpFunc(Glx::SelectablePlot2D::Callback func, 
				 void* user)
{
  itsDragFuncs.push_back( make_pair(func,user) );
}

void
Glx::SelectablePlot2D::callFuncs(vector<Glx::SelectablePlot2D::DragPair>& f)
{
  FANCYMESG("Glx::SelectablePlot2D::callFuncs");
  vector<Glx::SelectablePlot2D::DragPair>::iterator iter = f.begin();
  for( ; iter != f.end() ; ++iter ){
    Glx::SelectablePlot2D::DragPair p = *iter;
    p.first(this,p.second);
  }
}

// static 
void 
Glx::SelectablePlot2D::mouseUp(Glx::Draggable* dragger,void* user)
{
  FANCYMESG("Glx::SelectablePlot2D::mouseUp");
  SelectablePlot2D* _this = static_cast<SelectablePlot2D*>(user);      
  glx* env = _this->itsEnv;
  double* dmin = _this->dmin;
  double* dmax = _this->dmax;
  vector<bool>& selected = _this->selected;
  Glx::PointPlotter* plotter = _this->plotter;
  vector<float>& pntx = plotter->pntx;
  vector<float>& pnty = plotter->pnty;
  int envH = env->winHeight();
  float lo[2],hi[2];

  if(plotter->itsXdata==0 || plotter->itsYdata==0 ){
    _this->callFuncs( _this->itsDragFuncs );
    return;
  }

  vector<double>& xdata = * plotter->itsXdata;
  vector<double>& ydata = * plotter->itsYdata;

  int npoints=xdata.size();
  if( npoints==0 ){
    _this->callFuncs( _this->itsDragFuncs );    
    return;
  }

  selected.reserve(npoints);
  selected.assign(npoints,plotter->invert);

  switch( plotter->selectionMode ){

    case Glx::PointPlotter::BOXMODE:
      for(int pnt=0;pnt<npoints;++pnt) {
	if( xdata[pnt]<plotter->lobox[0] ||
	    xdata[pnt]>plotter->hibox[0] ||
	    ydata[pnt]<plotter->lobox[1] ||
	    ydata[pnt]>plotter->hibox[1] )
	  continue;
	selected[pnt]= !plotter->invert;
      }
      
      break;

    case Glx::PointPlotter::LASSOMODE:
      if( pntx.size()==0 ) return;
      for(int pnt=0;pnt<npoints;++pnt) {

	int hits=0;

	// extend a +X ray from the given pntx[pnt],pnty[pnt]
	// and count intersections with the line segments
	// of the polygon
	
	for(int i=0;i<pntx.size()-1;++i){

	  // this segment goes from: 
	  // <pntx[i],pnty[i]> to <pntx[i+1],pnty[i+1]>

	  // both Y's above the test point, no intersection
	  if( pnty[i]>ydata[pnt] && pnty[i+1]>ydata[pnt] ) continue;

	  // both Y's below the test point, no intersection
	  if( pnty[i]<=ydata[pnt] && pnty[i+1]<=ydata[pnt] ) continue;

	  // one above, one below from here...

	  // both X's are left of the test point => no intersection
	  if( pntx[i]<xdata[pnt] && pntx[i+1]<xdata[pnt] ) continue;

	  // both X's are right of the test point => intersect
	  if( pntx[i]>=xdata[pnt] && pntx[i+1]>=xdata[pnt] ) {
	    ++hits;
	    continue;
	  }
	  
	  // ok: one above, one below, one left, one right
	  // find intersection:

	  // find y-crossing of [+X] ray starting at xdata[pnt],ydata[pnt]
	  // with line segment: <pntx[i],pnty[i]> => <pntx[i+1],pnty[i+1]>
	  // 
	  //
	  // ydata[pnt] = pnty[i] + T * (pnty[i+1]-pnty[i])
	  //
	  //      ydata[pnt]-pnty[i]
	  // T = -------------------
	  //      pnty[i+1]-pnty[i]
	  //
	  // 
	  // X = pntx[i] + T * (pntx[i+1]-pntx[i])
	  //
	  // intersection is true if X >= test point's X
	  // 
	  
	  // parallel to within precision, assume 'above', ignore
	  if( fabs(pnty[i+1]-pnty[i])<EPS ){
	    continue;
	  }

	  double t = (ydata[pnt]-pnty[i])/(pnty[i+1]-pnty[i]);
	  double x = pntx[i] + t * (pntx[i+1]-pntx[i]);
	  if( x>=xdata[pnt] ){
	    ++hits;
	    continue;
	  }

	} // for i...
	bool select=( hits % 2==1 ) ? true : false;

	if( select )
	  selected[pnt]=!plotter->invert;
      }
      break;

  } // switch

  _this->callFuncs( _this->itsDragFuncs );

} // mouseUp()

void Glx::SelectablePlot2D::update(void)
{
  if(plotter->itsXdata==0 || plotter->itsYdata==0 ) return;
  vector<double>& xdata = * plotter->itsXdata;
  vector<double>& ydata = * plotter->itsYdata;
  int npoints=xdata.size();
  selected.reserve(npoints);
  selected.assign(npoints,plotter->invert);

  dmin[0]=dmin[1]=MAXFLOAT;
  dmax[0]=dmax[1]=-MAXFLOAT;
  for( int i=0 ; i<npoints ; i++ ){
    if( dmin[0] > xdata[i] ) dmin[0] = xdata[i];
    if( dmax[0] < xdata[i] ) dmax[0] = xdata[i];
    if( dmin[1] > ydata[i] ) dmin[1] = ydata[i];
    if( dmax[1] < ydata[i] ) dmax[1] = ydata[i];
  }      
}
