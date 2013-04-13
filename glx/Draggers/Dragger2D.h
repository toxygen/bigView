// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Dragger2D.h - Virtual base class for all 2D draggers
///////////////////////////////////////////////////////////////////////////////

#ifndef GLX_DRAGGABLE_2D_H
#define GLX_DRAGGABLE_2D_H

#include <Draggers/Dragger.h>

namespace Glx {

  /**
   * class Draggable2D
   *
   * A base class for 2D draggers inside a glx window.
   */
  
  class Draggable2D : public Glx::Draggable {
  public:

    /**
     * CTOR for class Glx::Draggable2D.
     * @param env: glx class in which this will be used
     */
    Draggable2D(glx* env) : Glx::Draggable(env){}

    /**
     * DTOR for class Glx::Draggable2D.
     */
    virtual ~Draggable2D(void){}
  
    // METHODS TO BE PROVIDED BY DERIVED CLASSES

    /**
     * set position using 2D coords
     */
    virtual void setPosition(float x, float y, int callUser=1);

    /**
     * set position using 2D coords
     */
    virtual void setPosition(float xy[2], int callUser=1);

    /**
     * set position using 2D coords
     */
    virtual void getPosition(float xy[2]);

    /**
     * Not a 3D dragger
     */
    int is3D(void){return 0;}
  };
}

#endif
