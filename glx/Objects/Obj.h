// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Obj.h : base class for Glx objects
// for information contact Tim Sandstrom (sandstro@nas.nasa.gov)
///////////////////////////////////////////////////////////////////////////////

#ifndef GLX_OBJ
#define GLX_OBJ

class glx;

namespace Glx {

  /**
   * class Glx::Obj, abstract base class for all Glx graphical objects
   */

  class Obj {
  public:
    
    /**
     * draw the object
     */
    virtual void draw(glx*,void*) = 0;
  };

} // namespace Glx

#endif

