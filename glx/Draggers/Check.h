// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Class Glx::Check
///////////////////////////////////////////////////////////////////////////////

#ifndef GLX_CHECK_H
#define GLX_CHECK_H

#include <Draggers/Dragger2D.h>
#include <string>

namespace Glx {

  /**
   * class Glx::Check
   *
   * A class implementing a checkbox in a glx window
   */
  class Check : public Glx::Draggable2D {
  public:
 
    enum       {CHECK_HANDLE=0,CHECK_HANDLES=1};
    enum State {NORMAL,MOVING};
    typedef void (*callback)(void*, bool);

    /**
     * CTOR for class Glx::Check.
     * @param env: glx class in which this will be used
     * @param x: x location in pixels
     * @param y: y location in pixels
     * @param w: width in pixels
     * @param h: width in pixels
     * @param label: title of checkbox
     */
    Check(glx* env, 
	  int x=60, int y=60, int w=60, int h=60,
	  std::string label="");

    /**
     * DTOR for class Glx::Check.
     */
    ~Check(void){}

    // concrete classes need to fill these methods in

    /**
     * draw the dragger, called by glx
     */
    void draw(glx*, void*);

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

    /**
     * register a user method to be called when state changes
     */
    void setCallback(Glx::Check::callback userFunc, void* userData){
      itsUserFunc=userFunc;
      itsUserData=userData;
    }

    void setChecked(bool enabled){
      itsCurValue=enabled;
    }

    bool getChecked(void){return itsCurValue;}
    std::string getLabel(void){return itsLabel;}

  protected:
    static void mouseDown(glx*,int,int,void*);

    void setColor(int);

    int itsX,itsY,itsW,itsH;
    bool itsCurValue;
    int itsLastX, itsLastY;
    int itsOffX, itsOffY;
    Glx::Check::callback itsUserFunc;
    void* itsUserData;
    Glx::Check::State itsState;
    std::string itsLabel;
    bool readyForChange;
  };

}

#endif
