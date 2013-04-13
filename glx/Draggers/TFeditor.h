// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Class Glx::TFeditor
///////////////////////////////////////////////////////////////////////////////

#ifndef GLX_TFEDITOR_H
#define GLX_TFEDITOR_H

#include <Draggers/Palette.h>
#include <Xm/Xm.h>
#include <X11/cursorfont.h>
#include <string>
#include <vector>

namespace Glx {

  class TFeditor : public Glx::Palette {
  public:
    enum {DEFAULT_HANDLES=4,MAX_HANDLES=16};
    enum {BORDER=12};
    enum {MOVE,RESIZE,ADD,DEL,RESET,SPLINE,FIRST_CROSSHAIR};
    enum EditMode {NORMAL_MODE=0,ADD_MODE=1,DEL_MODE=2};
    
    TFeditor(glx*,int x=60,int y=60,int w=200,int h=200);

    float  sample(float sampleX);
    void   reset(void);
    int    numHandles(){return N;}
    
    double getHandleX(int handleID);
    double getHandleY(int handleID);
    void   setHandleX(int handleID, double x);
    void   setHandleY(int handleID, double y);
    
    void   newHandles(std::vector< std::pair<double,double> >& handles);
    
    void   addHandle(double,double);
    void   deleteHandle(int handleID);
    
    void   setEditMode(TFeditor::EditMode mode);
    void   setSplineInterpolation(int enabled);
    
    glx*   getEnv(){return itsEnv;}
    
 protected:
    void   vert(double win[2]);
    void   vert(double xwin, double ywin);
    void   toPixels(double in[2], double out[2]);
    void   fromPixels(double in[2], double out[2]);

    void   getPos(int ID, double[2]); // normalized, [0..1]
    void   setPos(int ID, double[2]); // normalized, [0..1]
    void   setPos(int ID, double,double); // normalized, [0..1]

    void   deleteHandle(int,int);
    void   recalcSpline(); // shouldn't need to do this
    void   setCursor(TFeditor::EditMode mode);
    double splint(double* xa,double* ya,double* y2a,int n,double x);
    double lint(double x);
    void   spline(double* x, double* y, int n, 
		  double yp1, double ypn, double *y2);
    
    void   draw(glx*, void*);
    void   handleDrag(glx* env,int x,int y,void* user);
    int    idlePick(glx*,int,int,void*);

    static void mouseDown(glx*, int x, int y, void* user );
    //static void mouseDragged(Glx::Crosshair2D*,int,int,void*);

    void forceBetween(double& val, double lo, double hi);

    double* xArray;
    double* yArray;
    double* y2Array;
    double yp1;
    double ypn;
    int N;
    int itsSplineInterpolationFlag;
    int itsKnotEditingMode;    
    const double TOLERANCE;
    Cursor itsDeleteCursor;
    Cursor itsAddCursor;
    Cursor itsNormalCursor;
  };

} // namespace Glx

#endif
