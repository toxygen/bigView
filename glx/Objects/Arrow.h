// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Arrow.h : simple arrow class
// for information contact Tim Sandstrom (sandstro@nas.nasa.gov)
///////////////////////////////////////////////////////////////////////////////

#ifndef GLX_ARROW_H
#define GLX_ARROW_H

#include <Objects/Orientable.h>
#include <glxVector.h>

namespace Glx {

  /**
   * class Glx::Arrow
   *
   * A orientable arrow, works for 3D vectors, pointers, draggers, etc
   * Tail sits at a position, points along a direction.
   */

  class Arrow : public Glx::Orientable {
  public:
    enum {WIREARROW,TUBEARROW};

    Arrow(void);
    Arrow(float xyz[3]);
    Arrow(float x, float y, float z);
    Arrow(Glx::Vector& v);

    void  draw(glx*, void*);
    void  setScale(const float s);
    void  setPos(const float x, const float y, const float z);
    void  setPos(const float xyz[3]);
    void  setPos(const Glx::Vector& v);
    void  setVec(const float x, const float y, const float z);
    void  setVec(const float xyz[3]);   
    void  setVec(const Glx::Vector& v);

    float getScale(void);
    void  getPos(float vec[3]);
    void  getPos(Glx::Vector& v);
    void  getVec(float vec[3]);
    void  getVec(Glx::Vector& v);

    void setWireframe(bool enabled){itsWireframeFlag=enabled;}

  protected:
    void setDefaults(void);
    void checkNeedObj(void);
    void buildTubeArrow(void);
    void buildWireArrow(void);

    Glx::Vector itsPosition;
    float itsScale;
    bool itsWireframeFlag;
    static int itsGlyphID[2];
  };
} // namespace Glx

#endif

