// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Class Glx::VectorDragger
///////////////////////////////////////////////////////////////////////////////

#ifndef GLX_VECTOR_H
#define GLX_VECTOR_H

#include <Draggers/Dragger3D.h>
#include <Objects/Arrow.h>

namespace Glx {

  class VectorDragger : public Draggable3D {
  public:
    enum {VECTOR,NUM_HANDLES};
    
    VectorDragger(glx*,bool registerDragger=true);
    ~VectorDragger(void);
  
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

    float getScale(void);
    void  setScale(float scale);

    void  setPosition(Glx::Vector& p, bool call=false);
    void  setPosition(float p[3], bool call=false);
    void  setPosition(float, float, float, bool call=false);

    void  setVector(Glx::Vector& v, bool call=false);
    void  setVector(float v[3], bool call=false);
    void  setVector(float, float, float, bool call=false);
    void  getVector(Glx::Vector& v);

    void  getOrientation(Glx::Quat& orientationQuat);
    void  setOrientation(const Glx::Quat& orientationQuat, bool call=false);

  protected: 
    void   updateProj(glx*, const double[16]);
    double calcDistFromVector(double,double,double*);

    Glx::Arrow* itsArrow;
    Glx::Vector itsVector;

    // passive dragging support
    bool needsInit;

    Glx::Vector point1;
    Glx::Vector point2;
    Glx::Vector v1;
    Glx::Vector v2;
    Glx::Vector n;
    Glx::Vector Q;
    float lastY;
    float c;
    float itsTubeVecProjectionRadius;
    float itsTubeVecRadius;
    Glx::Vector itsCurEyePos;

    bool itsNeedOffset;
    Glx::Vector itsOffset;

  }; // class Glx::VectorDragger
} // namespace Glx

#endif
