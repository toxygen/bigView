//////////////////////////////////////////////////////////////////////////
///////////////////////////// glxTrackpad.h //////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifndef GLX_TRACKPAD_H
#define GLX_TRACKPAD_H

#include "GLX.h"

namespace Glx {
  
  /**
   * class Trackpad
   *
   * A class to provide 2D view adjustments within a glx window.
   * Middle mouse controls scale [zooming].
   * Right mouse controls screen space translation [panning].
   * Left/right brace keys '{' and '}' dec/inc scale sensitivity.
   * Left/right bracket keys '[' and ']' dec/inc translation sensitivity.
   */

  class Trackpad {
  public:

    /** 
     * CTOR for class Glx::Trackpad
     * Register mouse/key event callbacks with env.
     * @param env parent class
     */
    Trackpad(glx* env);

    /**
     * manually build projection and modelview matrixes
     */
    void applyXforms(void);

    /**
     * enable/disable response to mouse motions
     */
    void enable(bool enabled){itsEnabledFlag=enabled;}

    /**
     * reset the view
     */
    void reset(void);

    /**
     * specify this position in a matrix of screens
     */
    void setSector(int col, int row, int cols, int rows)
      {
	itsCol=col;
	itsRow=row;
	itsCols=cols;
	itsRows=rows;
      }

    static void setProjection(glx*, void*);
    static void startScale(glx*, int, int, void*);
    static void processScale(glx*, int, int, void*);
    static void startTranslate(glx*, int, int, void*);
    static void processTranslate(glx*, int, int, void*);
    static void processKey(glx*,XEvent *,void*);

    static void wheelUp(glx*, int, int, void*);
    static void wheelDn(glx*, int, int, void*);

    void saveView(std::string);
    void loadView(std::string);
    void viewAll(double lo[2], double hi[2],double pad=0.);

    double itsTranslationSensitivity;
    double itsScaleSensitivity;
    double itsScaleFactor;
    double itsXtrans;
    double itsYtrans;
    double modelMatrix[16],projMatrix[16];

  private:
    glx* itsEnv;
    double savex,savey,savescale;
    double startTranslateX,startTranslateY,startScaleY;
    double MIN_SCALE;
    double MAX_SCALE;
    bool itsEnabledFlag;
    int itsRow;
    int itsCol;
    int itsRows;
    int itsCols;
  };
}
#endif
