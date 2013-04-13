#ifndef _SELECTABLEPLOT2D_H_
#define _SELECTABLEPLOT2D_H_

#include <GLX.h>
#include <Draggers/Plotter.h>
#include <vector>

namespace Glx {

  class SelectablePlot2D {
  public:
    typedef void (*Callback)(Glx::SelectablePlot2D*,void*);
    typedef std::pair<Glx::SelectablePlot2D::Callback,void*> DragPair;

    SelectablePlot2D(glx*,std::vector<double>*,std::vector<double>*);
    ~SelectablePlot2D(void);

    void setData(std::vector<double>*,std::vector<double>*);
    void setVisibility(bool enabled){plotter->setVisibility(enabled);}
    void addUpFunc(Glx::SelectablePlot2D::Callback func, void* user=0);
    void draw(glx*, void *);
    std::vector<bool> selected;

    void getMouseCoord(float* x, float* y){
      float xval,yval;
      plotter->pix2real(plotter->itsLastXcoord,
			plotter->itsLastYcoord,&xval,&yval);
      *x=xval;
      *y=yval;
    }

  protected:
    static void mouseUp(Glx::Draggable*,void*);
    void update(void);
    void callFuncs(std::vector<Glx::SelectablePlot2D::DragPair>&);

    glx* itsEnv;
    Glx::PointPlotter* plotter;
    std::vector<float> xvec,yvec;
    double dmin[2],dmax[2];
    std::vector< Glx::SelectablePlot2D::DragPair > itsDragFuncs;

  }; // class Glx::SelectablePlot2D

}; // namespace Glx

#endif
