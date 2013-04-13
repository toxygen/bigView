#ifndef GLX_PALETTE_H
#define GLX_PALETTE_H

#include <Draggers/Dragger2D.h>

namespace Glx {

  /**
   * class Palette
   *
   * A base class for 2D draggers in a draggable rectangle inside a glx window.
   */
  class Palette : public Glx::Draggable2D {
    
  public:
    enum {MOVE,RESIZE,HANDLES};
    enum State {NORMAL,MOVING,RESIZING};
    enum {HANDLESIZE=10};
    
    /**
     * CTOR for class Glx::Palette.
     * @param env: glx class in which this will be used
     * @param x: x location in pixels
     * @param y: y location in pixels
     * @param w: width in pixels
     * @param h: height in pixels
     */
    Palette(glx* env, int x=60, int y=60, int w=200, int h=200);
    
    /**
     * DTOR for class Glx::Palette.
     */
    ~Palette(void);

    //////////////////////////////////////////////////////
    // concrete classes MUST to fill methods from here...
    //////////////////////////////////////////////////////

    /**
     * draw the dragger, called by glx
     */
    void draw(glx*,void *);

    /**
     * dragging has finished, called by glx
     */
    void handleMouseUp(glx*,void*);

    /**
     * dragging is occuring, called by glx
     */
    void handleDrag(glx*,int,int,void*);

    /**
     * compete for picking, called by glx
     */
    int  idlePick(glx*,int,int,void*);

    //////////////////////////////////////////////////////
    // ... to here
    //////////////////////////////////////////////////////

    //protected:
    int itsOffX, itsOffY;
    int itsX,itsY,itsW,itsH;
    Glx::Palette::State itsState;
  };
}

#endif
