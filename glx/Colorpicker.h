#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <X11/Intrinsic.h>
#include <GL/glx.h>
#include <string>
#include <vector>

namespace Color {
  /**
   * returns max of three values
   */
  float max3(float a, float b, float c);

  /**
   * returns min of three values
   */
  float min3(float a, float b, float c);
  
  /**
   * converts colors: RGB to HSV
   */
  void rgb2hsv(float rgb[3], float hsv[3]);

  /**
   * converts colors: RGB to HSV
   */
  void rgb2hsv(float r, float g, float b, float& h, float& s, float& v);

  /**
   * converts colors: HSV to RGB
   */
  void hsv2rgb(float hsv[3], float rgb[3]);

  /**
   * converts colors: HSV to RGB
   */
  void hsv2rgb(float h, float s, float v, float& r, float& g, float& b);
}

/**
 * class Colorpicker
 *
 * A class to provide interactive color selection under X windows.
 */
class Colorpicker {
public:
  enum {PARX=10,PARY=10,PARW=260,PARH=400};
  enum {FX=10,FY=50,FW=200,FH=100};
  enum {PX=2,PY=2,PW=200,PH=100};
  enum {SX=215,SY=2,SW=35,SH=100};

  /**
   * UserFunc is a function prototype. User functions matching
   * this format can be registered to be called by Colorpicker 
   * in response to selecting a color.
   */
  typedef void(*UserFunc)(Colorpicker*,float[3],void*);
  typedef std::pair<Colorpicker::UserFunc,void*> UserPair;

  /**
   * CTOR for class Colorpicker.
   * @param display: Display where the program is being run
   * @param  parent: parent widget [XmFrame,etc] for embedding
   */
  Colorpicker(Display* display,Widget parent=NULL);
  ~Colorpicker(void);

  /**
   * Registers a function to be called when a color has been picked
   * @param func: method to call
   * @param user: data to pass back to the method
   */  
  void addListener(Colorpicker::UserFunc func, void* user);

  /**
   * Sets a default color to start with
   * @param rgb: color triplet
   */  
  void setSampleColorRGB(float rgb[3]){
    setSampleColorRGB(rgb[0],rgb[1],rgb[2]);
  }
  /**
   * Sets a default color to start with
   * @param r: red component
   * @param g: green component
   * @param b: blue component
   */  
  void setSampleColorRGB(float r, float g, float b){
    itsRgb[0]=r;
    itsRgb[1]=g;
    itsRgb[2]=b;
    itsRgbChar[0]=(unsigned char)(itsRgb[0]*255.0f);
    itsRgbChar[1]=(unsigned char)(itsRgb[1]*255.0f);
    itsRgbChar[2]=(unsigned char)(itsRgb[2]*255.0f);
    updateSliders(1);
  }
  
 protected:
  void setDefaults();
  void addWidgets();
  void updateSliders(int callUsers);
  void callListeners(void);

  static void xInitPick(Widget, XtPointer, XtPointer);
  static void xInitSample(Widget, XtPointer, XtPointer);
  static void xDrawPick(Widget, XtPointer, XtPointer);
  static void xDrawSample(Widget, XtPointer, XtPointer);
  static void xPickColor(Widget, XtPointer, XEvent *, Boolean*);
  static void xSlider(Widget w, XtPointer user, XtPointer call);

  std::vector<Colorpicker::UserPair> itsListeners;
  Display* itsDisplay;
  GLXContext pctx,sctx;
  XVisualInfo *itsVisual;
  float* itsPickBuffer;
  Widget itsParent;
  Widget itsFrame;
  Widget itsPickWin;
  Widget itsSampleWin;
  Widget itsRScale;
  Widget itsGScale;
  Widget itsBScale;
  Widget itsHScale;
  Widget itsSScale;
  Widget itsVScale;
  bool ownparent;

  unsigned char itsRgbChar[3];
  float itsRgb[3];
  float itsHsv[3];
};

#endif
