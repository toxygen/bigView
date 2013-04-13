// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Class Glx::MultiSlider
///////////////////////////////////////////////////////////////////////////////
#ifndef GLX_MULTISLIDER_H
#define GLX_MULTISLIDER_H

#include <Draggers/Dragger2D.h>
#include <string>

namespace Glx {

  /**
   * class Glx::Multislider
   *
   * A class implementing a slider with multiple handles in a glx window
   */

  class Multislider : public Glx::Draggable2D {
  public:
  
    enum       {THUMB_WIDTH=10,THICKNESS=20};
    enum Axis  {X,Y};
    enum State {NORMAL,MOVING,RESIZING};
    typedef void (*callback)(void*, std::vector<float>&);
    typedef float (*Filter)(void*,float);

    static void defaultUserFunc(void*, std::vector<float>&);

    Multislider(glx*, int handles,
		Glx::Multislider::Axis =Glx::Multislider::Y,
		int x=60, int y=60, int w=200,
		std::string label="");
    ~Multislider(void);

    // concrete classes need to fill these methods in
    void draw(glx*, void*);
    void handleMouseUp(glx*,void*);
    void handleDrag(glx*,int,int,void*);
    int  idlePick(glx*,int,int,void*);

    void setCallback(Glx::Multislider::callback userFunc, void* userData){
      itsUserFunc=userFunc;
      itsUserData=userData;
    }
    void setLabel(std::string l){
      itsLabel=l;
    }

    void  setRange(float lo, float hi,bool callUser=false);  
    void  getRange(float& lo, float& hi){
      lo=itsMinRange;
      hi=itsMaxRange;
    }
    void  set(std::vector<float>&, bool callUser=false);  
    void  get(std::vector<float>&);

    void  set(int, float, bool callUser=false);
    float get(int);

    void  setSnapToInt(bool enabled){itsSnapToIntFlag=enabled;}
    void  setUserFilter( Filter, void* );

  protected:

    void viewHasChanged(glx*);
    void updateFuncs(void);
    bool pointInTri(float x, float y);
    void setColor(int);

    Glx::Multislider::Axis itsAxis;
    int itsNumHandles;
    int itsX,itsY,itsW; 

    float itsMinRange;
    std::vector<float> itsValues;
    float itsMaxRange;
    Glx::Multislider::callback itsUserFunc;
    void* itsUserData;
    Glx::Multislider::State itsState;
    int itsShowValueFlag;
    bool itsSnapToIntFlag;
    std::string itsLabel;

    int itsLastX, itsLastY;
    int itsOffX, itsOffY;

    Filter userFilter;
    void* userFilterData;

    struct func {
      float a,b,c,d;
      func() : a(0),b(0),c(0),d(0) {}
      float eval(float x, float y){
	return a*y - b*x - c + d;
      }
    };

    struct Thumb {
      Glx::Multislider* itsParent;
      int itsID;
      float* itsValue;
      Glx::Multislider::func funcs[3]; // three halfplanes

      int idlePick(glx*,int,int,void*);
      void draw(glx*,void *);
      void updateFuncs(void);      
    };
    std::vector<Glx::Multislider::Thumb*> thumbs;
  };
} // namespace Glx
#endif
