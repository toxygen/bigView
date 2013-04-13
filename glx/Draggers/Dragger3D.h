// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Dragger3D.h - Virtual base class for all 3D draggers
///////////////////////////////////////////////////////////////////////////////

#ifndef GLX_DRAGGABLE_3D_H
#define GLX_DRAGGABLE_3D_H

#include <Draggers/Dragger.h>

namespace Glx {

  /**
   * class Draggable3D
   *
   * A base class for 3D draggers inside a glx window.
   */
  class Draggable3D : public Glx::Draggable {
  public:

    /**
     * CTOR for class Glx::Draggable3D.
     * @param env: glx class in which this will be used
     */
    Draggable3D(glx* env,bool registerDragger=true);

    // METHODS TO BE PROVIDED BY DERIVED CLASSES

    /**
     * update positional info for picking when views change
     */
    //virtual void viewHasChanged(glx* env) = 0;

    /**
     * update positional info for picking when views change
     */
    virtual void viewHasChanged(glx* env,
				const double viewAndProj[16], 
				const double inv[16]) = 0;

    /**
     * Derived classes are all 3D draggers
     */
    int is3D(void){return 1;}

  protected:
    //virtual void updateProj(glx*, const double viewAndProjMatrix[16])=0;
    void calcNormal(Glx::Vector& yLeft,Glx::Vector& yRight, 
		    double invMatrix[16],
		    Glx::Vector& eyePos, Glx::Vector& res);

    double itsCurInverse[16];

  };
} // namespace Glx

#endif
