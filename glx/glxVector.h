// Emacs mode -*-c++-*- //
//////////////////////////////////////////////////////////////////////////////
// glxVector.h: Vector classes 
//////////////////////////////////////////////////////////////////////////////

#ifndef GLX_VECTOR
#define GLX_VECTOR

#include <iostream>

namespace Glx {
  class Vector;

  /**
   * dot product of two vectors
   */
  const float dot(const Glx::Vector& v1, const  Glx::Vector& v2);

  /**
   * cross product of two vectors
   */
  const Glx::Vector cross(const Glx::Vector& v1, const Glx::Vector& v2);
  
  /**
   * scale a vector
   */
  const Glx::Vector operator*(const Glx::Vector& v1, const float scalar);

  /**
   * scale a vector
   */
  const Glx::Vector operator*(const float scalar, const Glx::Vector& v1);

  /**
   * sum of two vectors
   */
  const Glx::Vector operator+(const Glx::Vector& v1, const Glx::Vector& v2);

  /**
   * difference of two vectors
   */
  const Glx::Vector operator-(const Glx::Vector& v1, const Glx::Vector& v2);
  
  /**
   * compare two vectors
   */
  bool operator==(const Glx::Vector&, const Glx::Vector&);

  /**
   * order two vectors
   */
  bool operator<(const Glx::Vector&, const Glx::Vector&);

  /**
   * multiply a vector by a 4x4 matrix
   */
  void xformVec(const Glx::Vector&,const double [16], Glx::Vector&); 

  /**
   * multiply a vector by a 4x4 matrix
   */
  void xformVec(const double m[4], const double n[16], double res[4]);

  /**
   * multiply two 4x4 matrices
   */
  void mult4x4(const double m[16], const double n[16], double res[16]);

  /**
   * try to invert a 4x4 matrix
   */
  int  inv4x4(const double m[16], double res[16]);
 
  std::ostream& operator<<(std::ostream&,const double[16]);

  /**
   * write a vector to an ostream
   */
  std::ostream& operator<<(std::ostream&,const Glx::Vector&);

  /**
   * read a vector from an istream
   */
  std::istream& operator>>(std::istream&,Glx::Vector&);

  /**
   * class Glx::Vector
   *
   * A class to provide basic linear algebra math operations
   */
  
  class Vector {
    
  public:
    /**
     * CTOR for class Glx::Vector
     */
    Vector(void);
    /**
     * CTOR for class Glx::Vector 
     */
    Vector(const float[3]);
    /**
     * CTOR for class Glx::Vector 
     */
    Vector(const Glx::Vector&);
    /**
     * CTOR for class Glx::Vector 
     */
    Vector(float, float, float);
  
    /**
     * replace with contents of array
     */
    void  copy(float v[3]) const;

    /**
     * return the length
     */
    float magnitude(void) const;

    /**
     * return the length
     */
    float mag(void) const;

    /**
     * force unit length
     */    
    void  normalize(void);
    
    /**
     * zero out components
     */  
    void  reset(void);

    /**
     * replace components
     */
    void  set(const float v0,const float v1, const float v2);

    /**
     * replace components
     */
    void  set(const float xyz[3]);
  
    /**
     * add another vector to this one
     */
    Glx::Vector& operator+=(const Glx::Vector&);

    /**
     * subtract another vector from this one
     */
    Glx::Vector& operator-=(const Glx::Vector&);

    /**
     * scale this vector
     */
    Glx::Vector& operator*=(const float);

    /**
     * assignment operator
     */
    Glx::Vector& operator=(const Glx::Vector&);
    
    /**
     * extract a component
     */
    float& operator[](const int index);

    /**
     * extract a const component
     */
    const float& operator[](const int i)const;

    /**
     * allow float* conversion
     */
    operator const float*(void) const;
    operator float*(void);

    friend const float Glx::dot(const Glx::Vector&, const Glx::Vector&);
    friend const Glx::Vector Glx::cross(const Glx::Vector&,const Glx::Vector&);
    friend const Glx::Vector Glx::operator*(const Glx::Vector&,float);
    friend const Glx::Vector Glx::operator*(const float, const Glx::Vector&);
    friend const Glx::Vector Glx::operator+(const Glx::Vector&, 
					    const Glx::Vector&);
    friend const Glx::Vector Glx::operator-(const Glx::Vector&, 
					    const Glx::Vector&);
    friend bool operator==(const Glx::Vector&, const Glx::Vector&);
    friend std::ostream& Glx::operator<<(std::ostream&,const Glx::Vector&);
    friend std::istream& Glx::operator>>(std::istream&,Glx::Vector&);
    friend void Glx::xformVec(const Glx::Vector&,const double[16],Glx::Vector&); 
  protected:
    float v[3];
  };
} // namespace Glx

#endif
