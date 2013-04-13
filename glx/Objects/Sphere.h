// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Glx::Sphere.h : simple sphere class
// for information contact Tim Sandstrom (sandstro@nas.nasa.gov)
///////////////////////////////////////////////////////////////////////////////

#ifndef GLX_SPHERE_H
#define GLX_SPHERE_H

#include <Objects/Orientable.h>
#include <glxVector.h>

namespace Glx {

  /**
   * class Glx::Sphere
   *
   * A orientable sphere, works for draggers, 3D points, etc
   * Stored in a display list, this draws quickly.
   * Should probably use a gluSphere instead.
   */

  class Sphere : public Glx::Orientable {
  public:
    Sphere(void);
    Sphere(float xyz[3], float r);
    Sphere(float x, float y, float z, float r);

    void setCenter(float x, float y, float z);
    void setCenter(float xyz[3]);
    void setCenter(Glx::Vector& v);
    void setRadius(float r);
    void setCenterCrosshairs(int v);
    void set(float xyz[3], float r);

    /**
     * specify how 'smooth' the sphere is, basically this is the number
     * of times an icosehedron is subdivided [default=3]
     * @param lvl: 0=>icosehedron
     * 
     */ 
    void setComplexity(int lvl);
    void setWireframe(bool enabled){itsWireframeFlag=enabled;}
    void draw(glx*,void*);

    float getRadius(void){ return itsRadius; }
    int   getComplexity(void){ return itsComplexity; }
    void  getCenter(float xyz[3]);
    void  getCenter(Glx::Vector& v);

  protected:
    void  checkNeedObj(void);
    void  genVertList(void);
    void  rotate_vec2d(float [2], float);
    void  buildMat(double [16],Glx::Vector&,Glx::Vector&,Glx::Vector&);
    float projection(Glx::Vector&, Glx::Vector&);
    void  sph_interp(Glx::Vector&, Glx::Vector&, float, Glx::Vector&);
    void  sendTri(Glx::Vector&, Glx::Vector&, Glx::Vector&, int);
    void  rot_z(Glx::Vector&, float, Glx::Vector&);
    void  icos(int);
    void  wireSphere(int,int);

    enum{NUM_VERTS=12};
  
    Glx::Vector vertex_list[NUM_VERTS];
    int needsVertexList;
    int itsComplexity;
    int wireFrameMode;
    Glx::Vector itsCenter;
    float itsRadius;
    int itsGlyphID;
    int itsWireGlyphID;
    bool itsWireframeFlag;
  };
} // namespace Glx
#endif

