// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Class Glx::Crosshair2D
///////////////////////////////////////////////////////////////////////////////

#ifndef GLX_CROSSHAIR2D_H
#define GLX_CROSSHAIR2D_H

#include <Draggers/Dragger2D.h>

namespace Glx {
  
  /**
   * class Glx::Crosshair2D
   *
   * A class implementing a Crosshair in a glx window
   */

  class Crosshair2D : public Glx::Draggable2D {
public:
 
    enum {CROSSHAIR2D_HANDLE=0,CROSSHAIR2D_HANDLES=1};  
    typedef void (*callback)(Crosshair2D*, int, int, void*);

    /**
     * CTOR for class Glx::Crosshair2D.
     * @param env: glx class in which this will be used
     * @param x: x location in pixels
     * @param y: y location in pixels
     */
    Crosshair2D(glx* env, int x=0, int y=0);

    /**
     * DTOR for class Glx::Crosshair2D.
     */
    ~Crosshair2D(void);

    void draw(glx*, void*);
    void handleMouseUp(glx*,void*);
    void handleDrag(glx*,int,int,void*);
    int  idlePick(glx*,int,int,void*);
    void setCallback(Glx::Crosshair2D::callback userFunc, void* userData){
      itsUserFunc=userFunc;
      itsUserData=userData;
    }
    Glx::Crosshair2D::callback itsUserFunc;
    void* itsUserData;

public:
    static float itsScale;
  };

  /**
   * class Glx::Crosshair2Df
   *
   * A class implementing a Crosshair in a glx window
   * using REAL coords , dammit
   */

  class Crosshair2Df : public Glx::Draggable2D {
public:
 
    enum {CROSSHAIR2D_HANDLE=0,CROSSHAIR2D_HANDLES=1};  
    enum Axis {X,Y};
    enum {LO,HI};

    typedef void (*callback)(Crosshair2Df*, float, float, void*);

    /**
     * CTOR for class Glx::Crosshair2Df.
     * @param env: glx class in which this will be used
     * @param x: x location in pixels
     * @param y: y location in pixels
     */
    Crosshair2Df(glx* env, float x=0., float y=0.);

    /**
     * DTOR for class Glx::Crosshair2Df.
     */
    ~Crosshair2Df(void);

    void draw(glx*, void*);
    void handleMouseUp(glx*,void*);
    void handleDrag(glx*,int,int,void*);
    int  idlePick(glx*,int,int,void*);
    void setCallback(Glx::Crosshair2Df::callback userFunc, void* userData){
      itsUserFunc=userFunc;
      itsUserData=userData;
    }
    void constrain(Crosshair2Df::Axis, bool);
    void limit(Crosshair2Df::Axis, double lo, double hi);
    Glx::Crosshair2Df::callback itsUserFunc;
    void* itsUserData;
    
    bool constrained[2];
    bool limited[2];
    float itsPosf[2];
    double limits[2][2];
    static float itsScale;
  };

}

#endif
