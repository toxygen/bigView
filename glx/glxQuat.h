//////////////////////////////////////////////////////////////////////////
/////////////////////////////// glxQuat.h ////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifndef GLX_QUAT
#define GLX_QUAT

#include <iostream>
#include <math.h>

namespace Glx {
  class Quat;

  /**
   *
   */
  void toR4(const double s3[4], double t, double r4[4]);
  void fromR4(const double r4[4], double s3[4]);

  /**
   * linear interpolate between to Quats
   */
  void slerp(const Glx::Quat&, const Glx::Quat&, float t, Glx::Quat& res);

  /**
   * quaternion dot product
   */  
  const float dot(const Glx::Quat& v1, const Glx::Quat& v2);

  /**
   * quaternion multiplication
   */  
  const Glx::Quat operator*(const Glx::Quat&, const Glx::Quat&);

  /**
   * quaternion multiplication
   */  
  const Glx::Quat operator*(const Glx::Quat&, const float&);

  /**
   * quaternion sum
   */  
  const Glx::Quat operator+(const Glx::Quat&, const Glx::Quat&);

  /**
   * write a quaternion to an ostream
   */
  std::ostream& operator<<(std::ostream&,const Glx::Quat&);

  /**
   * read a quaternion from an istream
   */
  std::istream& operator>>(std::istream&,Glx::Quat&);

  /**
   * compare two quats
   */
  bool operator==(const Glx::Quat&, const Glx::Quat&);

  /**
   * order two quats
   */
  bool operator<(const Glx::Quat&, const Glx::Quat&);

  /**
   * class Glx::Quat
   *
   * A class to handle rotational math. Rotations are represented
   * by an axis of rotation, v[], and the degree of rotation 
   * about that axis, radians. Internally this is laid out like this:
   *
   *  v[0] = axis[0] * sin(radians/2);
   *  v[1] = axis[1] * sin(radians/2);
   *  v[2] = axis[2] * sin(radians/2);
   *  v[3] = cos(radians/2);
   */

  class Quat {
  public:
    /**
     * CTOR for class Glx::Quat
     */
    Quat(void);

    /**
     * CTOR for class Glx::Quat
     * @param radians: amount of rotation about the axis
     * @param axis: axis of rotation
     */
    Quat(const float radians, const float axis[3]);

    /**
     * CTOR for class Glx::Quat
     */
    Quat(const Glx::Quat& q);

    /**
     * CTOR for class Glx::Quat
     * @param v0: x component of axis of rotation
     * @param v1: y component of axis of rotation
     * @param v2: z component of axis of rotation
     * @param c: rotation [in radians] about the axis
     */
    Quat(float v0, float v1, float v2, float c);

    /**
     * CTOR for class Glx::Quat, determine the quaternion
     * required to rotate v1 into alignment with v2
     * @param v1: source vector
     * @param v2: result vector
     */
    Quat(const float v1[3], const float v2[3]);

    /**
     * make a 4x4 rotation matrix
     */
    void  buildMatrix(double m[16]) const;
    
    /**
     * replace components
     */
    void  set(const float radians, const float axis[3] );
    void  get(float& radians, float axis[3] );

    /**
     * quaternion length
     */
    const float magnitude(void) const;
    
    /**
     * force unit length
     */
    void  normalize(void);

    /**
     * zero out components
     */  
    void  reset(void);

    /**
     * extract a component
     */
    float& operator[](const int index);

    /**
     * assignment operator
     */
    Glx::Quat& operator=(const Glx::Quat&);

    /**
     * add another quaternion
     */
    Glx::Quat& operator+=(const Glx::Quat&);
    
    /**
     * scale this quaternion
     */
    Glx::Quat& operator*=(const float);
    
    /**
     * post-rotation: this = this * that
     */
    void objectSpaceRotate(const Glx::Quat&);

    /**
     * pre-rotation: this = that * this
     */
    void worldSpaceRotate(const Glx::Quat&);

    void toS3(double [4]);

    friend void slerp(const Glx::Quat&, const Glx::Quat&,float, Glx::Quat&);
    friend const float dot(const Glx::Quat&, const Glx::Quat&);
    friend const Glx::Quat operator*(const Glx::Quat&, const Glx::Quat&);
    friend const Glx::Quat operator*(const Glx::Quat&, const float&);
    friend const Glx::Quat operator+(const Glx::Quat&, const Glx::Quat&);
    friend std::ostream& operator<<(std::ostream&,const Glx::Quat&);
    friend std::istream& operator>>(std::istream&,Glx::Quat&);

    float v[4];
  };
} // namespace Glx

#endif
