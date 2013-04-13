// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Glx::Orientable.h : basis class for objects with a quaternion
// for information contact Tim Sandstrom (sandstro@nas.nasa.gov)
///////////////////////////////////////////////////////////////////////////////

#ifndef GLX_ORIENTABLE_OBJ
#define GLX_ORIENTABLE_OBJ

#include <Objects/Obj.h>
#include <glxQuat.h>

namespace Glx {

  /**
   * class Glx::Orientable
   *
   * An base class for all Glx graphical objects
   * that can be rotated
   */

  class Orientable : public Glx::Obj {
  public:

    Orientable(void);

    /**
     * CTOR for class Glx::Orientable.
     * @param quat: initial rotation
     */
    Orientable(const Glx::Quat& quat);

    /**
     * Set the rotation
     * @param quat: new rotation
     */
    void  setOrientation(const Glx::Quat& quat);

    /**
     * Get the rotation
     * @param quat: location for result
     */
    void  getOrientation(Glx::Quat& quat) const;

    /**
     * Set the rotation: [pre-rotate] cur = quat * cur
     * @param quat: rotation to apply
     */
    void  worldSpaceRotate(Glx::Quat& quat);

  protected:
    void setDefaults(void);

    Glx::Quat itsQuat;
    double itsCurMat[16];
  };
} // namespace Glx
#endif

