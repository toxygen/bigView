#ifndef _WorldPalette_h_
#define _WorldPalette_h_

#include <GLX.h>
#include <Draggers/Palette.h>

namespace Glx {

  class WorldPalette : public Glx::Draggable2D {
  
  public:
    enum {MOVE,RESIZE,HANDLES};
    enum State {NORMAL,MOVING,RESIZING};
    enum {HANDLESIZE=10};

    typedef void (*callback)(WorldPalette*, 
			     double, double,
			     double, double, void*);

    WorldPalette(glx* env, float x=0, float y=0., float w=1., float h=1.);
    ~WorldPalette(void);
    void draw(glx*,void *);
    void handleMouseUp(glx*,void*);
    void handleDrag(glx*,int,int,void*);
    int  idlePick(glx*,int,int,void*);

    void updateProjection(void);

    // called on mouse up
    void viewHasChanged(glx*);

    // called when window resizes
    static void configureChanged(glx*,void*);

    void toPixels(glx*,double,double,double*,double*);
    void toWorld(glx*,double,double,double*,double*);

    void keepAspect(float a){keepaspect=true;aspect=a;}
    void setSnap(double s){snapX=snapY=s;}
    void setSnap(double x, double y){snapX=x;snapY=y;}

    void setDragCallback(Glx::WorldPalette::callback func, void* data=0){
      itsDragFunc=func;
      itsDragData=data;
    }
    void setDoneCallback(Glx::WorldPalette::callback func, void* data=0){
      itsDoneFunc=func;
      itsDoneData=data;
    }
    void setBold(bool enabled){bold=enabled;}
    void moveleft(bool calluser=false);
    void moveright(bool calluser=false);
    void moveup(bool calluser=false);
    void movedown(bool calluser=false);
    void pageleft(bool calluser=false);
    void pageright(bool calluser=false);
    void pageup(bool calluser=false);
    void pagedown(bool calluser=false);
    void setPos(float,float,bool calluser=false);
    void setSize(float,float,bool calluser=false);

  protected:
    double snapX,snapY;
    double itsPixelX,itsPixelY,itsPixelW,itsPixelH;
    double itsX,itsY,itsW,itsH;
    double itsOffX,itsOffY;
    int winw,winh;
    WorldPalette::State itsState;
    bool keepaspect;
    bool bold;
    double aspect;
    Glx::WorldPalette::callback itsDragFunc;
    void* itsDragData;
    Glx::WorldPalette::callback itsDoneFunc;
    void* itsDoneData;

  };
}; // namespace Glx
#endif
