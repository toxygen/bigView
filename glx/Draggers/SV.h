// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Class Glx::SVDragger
///////////////////////////////////////////////////////////////////////////////

#ifndef GLX_SVDragger_H
#define GLX_SVDragger_H

#include <GLX.h>
#include <Draggers/SphereDragger.h>
#include <Draggers/Vector.h>

namespace Glx {

  class SVDragger : public Glx::Draggable3D {

  public:
    enum {SPHERE,NORMAL,NUM_HANDLES};
    
    SVDragger(glx* env,bool registerDragger=true);
    ~SVDragger(void);

    // concrete classes MUST to fill methods from here...
    void draw(glx*, void*){}
    void handleMouseUp(glx*,void*);
    void handleDrag(glx*,int,int,void*){}
    int  idlePick(glx*,int,int,void*);
    // ... to here

    // From Draggable3D
    void viewHasChanged(glx*);
    void viewHasChanged(glx*, const double[16], const double[16]){}
    // ... to here

    void  setScale(float scale);

    void  setPosition(Glx::Vector&, bool call=false);
    void  setPosition(float [3], bool call=false);
    void  setPosition(float,float,float, bool call=false);

    void  getVector(Glx::Vector& v);
    void  setVector(Glx::Vector& v, bool call=false);
    void  setVector(float v[3], bool call=false);
    void  setVector(float, float, float, bool call=false);

    void  getOrientation(Glx::Quat& orientationQuat);
    void  setOrientation(const Glx::Quat& orientationQuat, bool call=false);

    void  select(int dragHandle){
      switch( dragHandle ){
	case Glx::SVDragger::SPHERE: 
	  itsSphere->select(Glx::SphereDragger::SPHERE);
	  itsVector->unselect();
	  break;
	case Glx::SVDragger::NORMAL: 
	  itsVector->select(Glx::VectorDragger::VECTOR);
	  itsSphere->unselect();
	  break;
      }
    }
    void  unselect(void){
      itsSphere->unselect();
      itsVector->unselect();
    }

    void enableRotations(bool enabled){
      itsSphere->enableRotations(enabled);
    }
    Glx::SphereDragger* itsSphere;
    Glx::VectorDragger* itsVector;
  protected:
    static void dragSphere(Draggable*,void*);
    static void dragVector(Draggable*,void*);
  };
} // namespace glx

#endif
