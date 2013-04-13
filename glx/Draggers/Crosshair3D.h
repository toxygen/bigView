// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Class Glx::Crosshair3D
///////////////////////////////////////////////////////////////////////////////

#ifndef GLX_Crosshair3D_h
#define GLX_Crosshair3D_h

#include <Draggers/Dragger3D.h>

namespace Glx {

  class Crosshair3D : public Glx::Draggable3D {
  public:
    enum{XPLANE,YPLANE,ZPLANE,NUM_HANDLES};

    Crosshair3D(glx*,bool registerDragger=true);
    ~Crosshair3D(void);
  
    // concrete classes MUST to fill methods from here...
    void draw(glx*,void *);
    void handleMouseUp(glx*,void*);
    void handleDrag(glx*,int,int,void*);
    int  idlePick(glx*,int,int,void*);
    // ... to here

    void viewHasChanged(glx* env);
    void viewHasChanged(glx* env,
			const double viewAndProj[16], 
			const double inv[16]);
  protected:
    int testPlane(glx*,int,Glx::Vector&,Glx::Vector&,double,double,double&);
    void updateProj(glx*, const double[16]);
    void getCorner(int dim, int index, Glx::Vector& res) const;
    int sign(double f);


    Glx::Vector cproj[3][4];
    Glx::Vector itsOffset;
    bool needsInit;
    bool itsNeedOffset;
    float lastY;
  };
}

#endif
