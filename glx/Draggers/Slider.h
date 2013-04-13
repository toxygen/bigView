// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Class Glx::Slider
///////////////////////////////////////////////////////////////////////////////

#ifndef GLX_SLIDER_H
#define GLX_SLIDER_H

#include <Draggers/Dragger2D.h>
#include <string>

namespace Glx {

  /**
   * class Glx::Slider
   *
   * A class implementing a slider in a glx window
   */
  class Slider : public Glx::Draggable2D {
  public:
 
    enum       {SLIDER_HANDLE=0,THUMB_HANDLE=1,SLIDER_HANDLES=2};
    enum       {THUMB_WIDTH=10,THICKNESS=20};
    enum Axis  {X,Y};
    enum State {NORMAL,MOVING,RESIZING};
    typedef void (*callback)(void*, float);

    /**
     * CTOR for class Glx::Slider.
     * @param env: glx class in which this will be used
     * @param axis: X [horizontal] or Y [vertical]
     * @param x: x location in pixels
     * @param y: y location in pixels
     * @param w: width in pixels
     * @param label: title of slider
     */
    Slider(glx* env, Glx::Slider::Axis axis=Glx::Slider::Y,
	   int x=60, int y=60, int w=200,
	   std::string label="");

    /**
     * DTOR for class Glx::Slider.
     */
    ~Slider(void){}

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
     * register a user method to be called while dragging
     */
    void setCallback(Glx::Slider::callback userFunc, void* userData=0){
      itsUserFunc=userFunc;
      itsUserData=userData;
    }

    void setMouseupCallback(Glx::Slider::callback userFunc, void* userData=0){
      itsMouseupUserFunc=userFunc;
      itsMouseupUserData=userData;
    }
    /**
     * set the acceptible ranges of values
     * @param minVal: lowest value
     * @param curVal: current value
     * @param maxVal: max value
     * @param callUserFlag: inform user of the range change?
     */
    void setRange(float minVal,float curVal,float maxVal,int callUserFlag=0){
      itsMinRange = minVal;
      itsCurValue = curVal;
      itsMaxRange = maxVal;
      updateFuncs();
      if( callUserFlag && itsUserFunc != NULL )
	itsUserFunc(itsUserData,itsCurValue);
      updateFuncs();
    }

    /**
     * set the lowest acceptible value
     * @param minValue: lowest value
     */
    void setMin(float minValue){
      itsMinRange=minValue;
      updateFuncs();
    }

    /**
     * set the current value
     * @param curValue: current value
     * @param callUserFlag: inform user of the range change?
     */
    void setCur(float curValue, int callUserFlag=0){
      itsCurValue = curValue;
      if( callUserFlag && itsUserFunc != NULL )
	itsUserFunc(itsUserData,itsCurValue);
      updateFuncs();
    }

    /**
     * set the highest acceptible value
     * @param maxValue: max value
     */
    void setMax(float maxValue){
      itsMaxRange=maxValue;
      updateFuncs();
    }

    void setLabel(std::string l){
      itsLabel=l;
    }

    /**
     * specify if values are rounded to nearest integer
     */
    void  setSnapToInt(bool enabled){itsSnapToIntFlag=enabled;}

    /**
     * return the lowest possible value
     */
    float getMin(void){return itsMinRange;}

    /**
     * return the current value
     */
    float getCur(void){return itsCurValue;}

    /**
     * return the highest possible value
     */
    float getMax(void){return itsMaxRange;}

    void keyEvent(int key, bool ctl, bool shift, bool alt);

  protected:

    struct func {
      float a,b,c,d;
      func() : a(0),b(0),c(0),d(0) {}
      float eval(float x, float y){
	return a*y - b*x - c + d;
      }
    };

    void setColor(int);
    void updateFuncs(void);
    bool pointInTri(float x, float y);

    int itsX,itsY,itsW; 
    Glx::Slider::func funcs[3];
    float itsMinRange;
    float itsCurValue;
    float itsMaxRange;
    int itsShowValueFlag;
    int itsLastX, itsLastY;
    int itsOffX, itsOffY;
    float itsLastCurValue;
    float itsCurScale;
    Glx::Slider::callback itsUserFunc;
    void* itsUserData;

    Glx::Slider::callback itsMouseupUserFunc;
    void* itsMouseupUserData;

    Glx::Slider::State itsState;
    Glx::Slider::Axis itsAxis;
    std::string itsLabel;
    bool itsSnapToIntFlag;
  };

}

#endif
