// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Class Draggable
///////////////////////////////////////////////////////////////////////////////

#ifndef _DRAGGABLE_H
#define _DRAGGABLE_H

#include <algorithm>
#include <vector>
#include <list>
#include <map>
#include <stack>
#include <values.h> // for MAXFLOAT
#include <glxVector.h>

class glx;

namespace Glx {
  class Draggable;
  
  /**
   * Glx::DraggerCallback is a function prototype. User functions 
   * matching this format can be registered to be called by 
   * draggers in response to dragging.
   */
  typedef void (*DraggerCallback)(Draggable*,void*);
  typedef std::pair<DraggerCallback,void*> DragPair;
  
  /**
   * class Glx::Draggable
   *
   * An abstract base class to allow direct user manipulation
   * in screen and world spaces inside a glx window.
   */

  class Draggable {
  public:
    enum {UNSELECTED=-1};

    /**
     * CTOR for class Glx::Draggable.
     * @param env: glx class in which this will be used
     */
   Draggable(glx* env, bool registerDragger=true);

    /**
     * DTOR for class Glx::Draggable.
     */
    virtual ~Draggable(void);

    //////////////////////////////////////////////////////
    // main user interface calls 
    //////////////////////////////////////////////////////

    /**
     * register a user method to be called when dragging
     */
    void  addDragFunc(Glx::DraggerCallback func, void* user);

    /**
     * register a user method to be called when dragging is finished
     */
    void  addUpFunc(Glx::DraggerCallback func, void* user);

    /**
     * register a user method to be called when
     * the scale or selected handle has changed
     */
    void  addUpdateFunc(Glx::DraggerCallback func, void* user);

    /**
     * return the selected handle, UNSELECTED otherwise
     */
    int   getCurDragHandle(void){return itsCurDragHandle;}

    /**
     * get the current scale 
     */
    virtual float getScale(void){return itsScale;}

    /**
     * get the dragger's visibility
     */
    bool  getVisibility(void){return itsVisibleFlag;}
    
    /**
     * get the dragger's center
     */
    virtual void getPosition(Glx::Vector& p){p=itsPos;}  

    /**
     * get the dragger's center
     */  
    virtual void getPosition(float p[3]){itsPos.copy(p);}

    /**
     * get the 'distance' to the mouse, 
     */
    float getSelectionDist(void){return itsSelectionDist;}

    //////////////////////////////////////////////////////
    // concrete classes MUST fill all methods from here...
    //////////////////////////////////////////////////////

    /**
     * draw the dragger, called by glx
     */
    virtual void draw(glx*, void*)=0;

    /**
     * dragging has finished, called by glx
     */
    virtual void handleMouseUp(glx*,void*)=0;

    /**
     * dragging is occuring, called by glx
     */
    virtual void handleDrag(glx*,int,int,void*)=0;

    /**
     * compete for picking, called by glx
     */
    virtual int  idlePick(glx*,int,int,void*)=0;

    //////////////////////////////////////////////////////
    // ... to here
    //////////////////////////////////////////////////////

    // concrete classes may wish to override these
    
    /**
     * select a specific handle
     */
    virtual void  setCurDragHandle(int handle);
    
    /**
     * set the scale
     */
    virtual void  setScale(float s);
    
    /**
     * set the visibility
     */
    virtual void  setVisibility(bool enabled);

    /**
     * set the center
     */
    virtual void  setPosition(Glx::Vector& p, bool call=false);
    virtual void  setPosition(float p[3], bool call=false);
    virtual void  setPosition(float, float, float, bool call=false);
    
    /**
     * select a specific handle
     */ 
    virtual void  select (int dragHandle);

    /**
     * unselect all handles
     */
    virtual void  unselect(void);
    
    /** 
     * returns true is this dragger is 3D
     */
    virtual int   is3D(void);

    /** 
     * mouse has potentially changed the dragger's position
     */
    virtual void viewHasChanged(glx*);

    /**
     * which button does this dragger listen to
     */
    virtual bool interested(int btn);

    /**
     * draggers can consume key events
     */
    virtual void keyEvent(int key, bool ctl, bool shift, bool alt){}

  protected:
    // concrete classes may wish to override these
    virtual void doneWithDrag(void);

    void callFuncs(std::vector< Glx::DragPair >& funcs);
    void callDrag(void);
    void setDefaults(void);
    
    glx*  itsEnv;
    int   itsCurDragHandle;
    float itsScale;
    float itsSelectionDist;
    bool  itsVisibleFlag;

    std::vector< Glx::DragPair > itsDragFuncs;
    std::vector< Glx::DragPair > itsUpFuncs;
    std::vector< Glx::DragPair > itsUpdateFuncs;
    Glx::Vector itsPos;
    std::vector<int> itsMouseFlags;
    bool itsNeedsInitFlag;
  };
}

#endif
