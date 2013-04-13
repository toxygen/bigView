//////////////////////////////////////////////////////////////////////////
///////////////////////////// glxTrackball.h /////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifndef GLX_TRACKBALL_H
#define GLX_TRACKBALL_H

#include "GLX.h"
#include "glxVector.h"
#include "glxQuat.h"

namespace Glx {
  extern void trackball(Glx::Quat& q,float p1x,float p1y,float p2x,float p2y);

  /**
   * class Trackball
   *
   * A class to provide 3D view adjustments within a glx window.
   * Left mouse controls rotation.
   * Middle mouse controls scale [zooming].
   * Right mouse controls screen space translation [panning].
   * Left/right bracket keys '[' and ']' dec/inc translation sensitivity.
   * Hitting 'r' resets the view.
   */

  class Trackball {
  public:

    /** 
     * CTOR for class Glx::Trackpad
     * Register mouse/key event callbacks with env.
     * @param env parent class
     */
    Trackball(glx* env);

    /**
     * manually build projection and modelview matrixes
     */
    void applyXforms(void);

    /**
     * enable/disable all responses to mouse motions
     */
    void enable(bool enabled){itsEnabledFlag=enabled;}

    /**
     * enable/disable translations
     */
    void enableTranslation(bool enabled){itsEnabledTransFlag=enabled;}

    /**
     * reset the view
     */
    void reset(void);

    /**
     * specify this position in a matrix of screens
     */
    void setSector(int col, int row, int cols, int rows,
		   int hgap=0, int vgap=0)
      {
	itsCol=col;
	itsRow=row;
	itsCols=cols;
	itsRows=rows;
	itsHgap=hgap;
	itsVgap=vgap;
      }

    /**
     * enable/disable spinning
     */
    void enableSpin(bool enabled){spinning=enabled;}

    /**
     * sets spin amount
     */
    void setSpin(float radians, float axis[3]){
      spinQuat.set(radians,axis);
      spinQuat.normalize();
    }

    //protected:

    void viewAll(const double*, const double*);

    static void setProjection(glx*, void*);
    static void startRotate(glx*, int, int, void*);
    static void processRotate(glx*, int, int, void*);
    static void startScale(glx*, int, int, void*);
    static void processScale(glx*, int, int, void*);
    static void startTranslate(glx*, int, int, void*);
    static void processTranslate(glx*, int, int, void*);
    static void processKey(glx*,XEvent *,void*);

    static double getNear(void* usr){
      Glx::Trackball* _this = static_cast<Glx::Trackball*>(usr);
      return _this->itsNear;
    }
    static double getFar(void* usr){
      Glx::Trackball* _this = static_cast<Glx::Trackball*>(usr);
      return _this->itsFar;
    }
    static double getFOV(void* usr){
      Glx::Trackball* _this = static_cast<Glx::Trackball*>(usr);
      return _this->itsFOV;
    }
    static double getProjH(void* usr){
      Glx::Trackball* _this = static_cast<Glx::Trackball*>(usr);
      return _this->itsNear * tan( _this->itsFOV/2.0 );
    }
    static double getProjW(void* usr){
      Glx::Trackball* _this = static_cast<Glx::Trackball*>(usr);
      return _this->getProjH(usr) * _this->itsEnv->aspect();
    }

    void saveView(std::string);
    void loadView(std::string);

    void setOrtho(bool enabled){
      ortho=enabled;
      if(ortho)
	itsScale=savescale;
      else 
	itsScale=1.;
    }
    bool getOrtho(void){return ortho;}

  public:
    glx* itsEnv;
    double itsXtrans;
    double itsYtrans;
    double itsZtrans;
    Glx::Vector itsCenter;
    double savex,savey,savez,savescale;
    double startTranslateX,startTranslateY,startTranslateZ;
    double startRotateX,startRotateY;
    double itsSensitivity;
    double MIN_SCALE;
    double MAX_SCALE;
    Glx::Quat worldQuat;
    Glx::Quat saveQuat;
    bool itsEnabledFlag;
    bool itsEnabledTransFlag;
    int itsRow;
    int itsCol;
    int itsRows;
    int itsCols;
    int itsHgap;
    int itsVgap;
    double itsNear;
    double itsFar;
    double itsScale;
    double itsFOV;
    bool spinning;
    Glx::Quat spinQuat;
    double proj[16];
    double view[16];
    double frustum[6]; // l,r,b,t,near,far
    bool ortho;
  };
} // namespace Glx
#endif
