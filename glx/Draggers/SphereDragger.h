// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Class Glx::SphereDragger
///////////////////////////////////////////////////////////////////////////////

#ifndef GLX_SPHEREDRAGGER_H
#define GLX_SPHEREDRAGGER_H

#include <Draggers/Dragger3D.h>
#include <Objects/Sphere.h>

namespace Glx {

  class SphereDragger : public Glx::Draggable3D {
  public:
    enum{SPHERE,NUM_HANDLES};

    SphereDragger(glx*,bool registerDragger=true);
    ~SphereDragger(void);
  
    // concrete classes MUST to fill methods from here...
    void draw(glx*,void *);
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

    void  getOrientation(Glx::Quat& orientationQuat);
    void  setOrientation(const Glx::Quat& orientationQuat, bool call=false);
  
    void  setWireMode(bool enabled){itsWireMode = enabled;}
    void  enableRotations(bool enabled){itsEnableRotationFlag = enabled;}

    void  getEye(Glx::Vector& eye){eye = itsEye;}
    void  getInv(double*& inv){inv = itsCurInv;}
    void  getVP(double*& inv){inv = itsVP;}
    double* getVP(void){return itsVP;}
    double* getInv(void){return itsCurInv;}

  protected:
    void updateProjection(glx*env);

    Glx::Sphere* itsSphere;
    Glx::Vector  itsViewVector;
    Glx::Vector  itsEye;
    Glx::Vector  itsSphereProjection;
    Glx::Quat    itsSaveQuat;
    double       itsVP[16];
    double       itsCurInv[16];
    double       itsSphereProjectionRadius;
    double       itsSphereEyeDistance;
    bool         needsInit;
    bool         needsCoord;
    bool         itsWireMode;
    bool         itsEnableRotationFlag;  
    double       startX,startY,lastY;
  };
}

#endif
