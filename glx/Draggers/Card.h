// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Class Glx::CardDragger;
///////////////////////////////////////////////////////////////////////////////

#ifndef GLX_CardDragger_H
#define GLX_CardDragger_H

#include <GLX.h>
#include <Draggers/SV.h>

namespace Glx {

  class CardDragger : public Glx::Draggable3D {
  public:
    enum {SPHERE,PLANE,NORMAL,CORNER,NUM_HANDLES};
    
    CardDragger(glx* env);
    ~CardDragger(void);

    // concrete classes MUST to fill methods from here...
    void draw(glx*, void*);
    void handleMouseUp(glx*,void*);
    void handleDrag(glx*,int,int,void*);
    int  idlePick(glx*,int,int,void*);
    // ... to here

    // From Draggable3D
    void viewHasChanged(glx*);
    void viewHasChanged(glx*, const double[16], const double[16]);
    // ... to here

    void  setScale(float scale);

    void  setPosition(Glx::Vector&, bool call=false);
    void  setPosition(float [3], bool call=false);
    void  setPosition(float,float,float, bool call=false);

    void  getVector(Glx::Vector& v);
    void  setVector(Glx::Vector& v, bool call=false);
    void  setVector(float v[3], bool call=false);
    void  setVector(float, float, float, bool call=false);

    void  getOrientation(Glx::Quat& orientationQuat) const;
    void  setOrientation(const Glx::Quat& orientationQuat, bool call=false);

  protected:
    class Plane {
    public:
      Plane(void):a(0),b(0),c(0),x(0),y(0),z(0){}
      float a,b,c; // normal
      float x,y,z; // pt
    };

    double calcDist(int i, double x, double y, double* t);
    void   getCorner(int index, Glx::Vector& res) const;
    void   updateProj(glx* env, const double vp[16]);
    int    sign(double f);
    static void drag(Draggable*,void* user);
    static void done(Draggable*,void* user);

    Glx::SVDragger* itsSVDragger;

    Plane snapPlane; // potential canonical plane to 'snap' to
    bool needsInit,needsCoord;
    double itsWidth,itsHeight;
    double lastX,lastY;
    Glx::Vector iHat,jHat; 
    double itsHandleScale;

    Glx::Vector v1[2];     // 2D projections of the corner handle
    Glx::Vector v2[2];
    Glx::Vector n[2];
    Glx::Vector Q[2];
    double c[2];
    Glx::Vector cProj[4];
  };
} // namespace glx

#endif
