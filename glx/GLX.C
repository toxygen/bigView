//////////////////////////////////////////////////////////////////////////
///////////////////////////////// GLX.C //////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <sstream>
#include <fstream>
#include <X11/keysym.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/Core.h>
#include <X11/CoreP.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Xm/Protocols.h>
#include <GL/GLwDrawA.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <values.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <GLX.h>
#include <GL/glu.h>
#include <math.h>
#include <map>

#ifdef _XA_MWM_HINTS
#warning "_XA_MWM_HINTS is true!"
#else
#warning "_XA_MWM_HINTS is true!"
#endif


//#define DEBUG 1
#include "pdebug.h"

using namespace std;

////////////////////////////////////////////////////////////
#ifdef USEFREETYPE
////////////////////////////////////////////////////////////

# warning "Using Freetype [GLX.C]"
# include <X11/Xft/Xft.h> // for Xft: Freetype
# include <X11/extensions/Xrender.h> // for XGlyphInfo
# include <glxFFont.h>

namespace Priv {

  void setMessageFont(Display*,std::string, double);
  void setTitleFont(Display*,std::string, double);
  void setLabelFont(Display*,std::string, double);

  std::string genFontKey(std::string, double);
  XftFont *loadFont(Display*,std::string, double, std::string&);

  std::map<std::string,Glx::FontCache*> fontMap;
  std::map<std::string,XGlyphInfo*> extentsMap;
  std::deque<std::string> fontStack;
  std::string curFontKey;
  std::string mesgFontKey;
  std::string titleFontKey;
  std::string labelFontKey;
};

////////////////////////////////////////////////////////////
#else // NOT FREETYPE
////////////////////////////////////////////////////////////

# warning "NOT using Freetype [GLX.C]"

namespace Priv {
  void fontCreateBitmap(unsigned int fontBase);
  void fontDrawStr(unsigned int base, std::string str);
  unsigned int itsFontBase=0;
}

////////////////////////////////////////////////////////////
#endif // NOT FREETYPE
////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
///////////////////////////// Static Globals /////////////////////////////
//////////////////////////////////////////////////////////////////////////


namespace Priv {
  double modelMatrix[16];
  double projMatrix[16];
  int viewport[4];

  const float EPS = 1.0e-6;
  float origin[3] = {0.0, 0.0, 0.0};
  float x_axis[3] = {1.0, 0.0, 0.0};
  float y_axis[3] = {0.0, 1.0, 0.0};
  float z_axis[3] = {0.0, 0.0, 1.0};
};

GLXContext glx::itsGlobalGLXcontext=0;

unsigned char Glx::stip[Glx::STIPPLESIZE] = {
  0xAA,0xAA,0xAA,0xAA,0x55,0x55,0x55,0x55, 
  0xAA,0xAA,0xAA,0xAA,0x55,0x55,0x55,0x55, 
  0xAA,0xAA,0xAA,0xAA,0x55,0x55,0x55,0x55, 
  0xAA,0xAA,0xAA,0xAA,0x55,0x55,0x55,0x55, 
  0xAA,0xAA,0xAA,0xAA,0x55,0x55,0x55,0x55, 
  0xAA,0xAA,0xAA,0xAA,0x55,0x55,0x55,0x55, 
  0xAA,0xAA,0xAA,0xAA,0x55,0x55,0x55,0x55, 
  0xAA,0xAA,0xAA,0xAA,0x55,0x55,0x55,0x55, 
  0xAA,0xAA,0xAA,0xAA,0x55,0x55,0x55,0x55, 
  0xAA,0xAA,0xAA,0xAA,0x55,0x55,0x55,0x55, 
  0xAA,0xAA,0xAA,0xAA,0x55,0x55,0x55,0x55, 
  0xAA,0xAA,0xAA,0xAA,0x55,0x55,0x55,0x55, 
  0xAA,0xAA,0xAA,0xAA,0x55,0x55,0x55,0x55, 
  0xAA,0xAA,0xAA,0xAA,0x55,0x55,0x55,0x55, 
  0xAA,0xAA,0xAA,0xAA,0x55,0x55,0x55,0x55,
  0xAA,0xAA,0xAA,0xAA,0x55,0x55,0x55,0x55
};

unsigned char 
Glx::bitmapFont[Glx::FONTENTRIES][Glx::FONTENTRYSIZE] = {
{32,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{33,0x00,0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x18,0x18,0x18,0x18,0x18},
{34,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x36,0x36,0x36,0x36},
{35,0x00,0x00,0x00,0x66,0x66,0xff,0x66,0x66,0xff,0x66,0x66,0x00,0x00},
{36,0x00,0x00,0x18,0x7e,0xff,0x1b,0x1f,0x7e,0xf8,0xd8,0xff,0x7e,0x18},
{37,0x00,0x00,0x0e,0x1b,0xdb,0x6e,0x30,0x18,0x0c,0x76,0xdb,0xd8,0x70},
{38,0x00,0x00,0x7f,0xc6,0xcf,0xd8,0x70,0x70,0xd8,0xcc,0xcc,0x6c,0x38},
{39,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x1c,0x0c,0x0e},
{40,0x00,0x00,0x0c,0x18,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x18,0x0c},
{41,0x00,0x00,0x30,0x18,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x18,0x30},
{42,0x00,0x00,0x00,0x00,0x99,0x5a,0x3c,0xff,0x3c,0x5a,0x99,0x00,0x00},
{43,0x00,0x00,0x00,0x18,0x18,0x18,0xff,0xff,0x18,0x18,0x18,0x00,0x00},
{44,0x00,0x00,0x30,0x18,0x1c,0x1c,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{45,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,0x00,0x00,0x00},
{46,0x00,0x00,0x00,0x38,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{47,0x00,0x60,0x60,0x30,0x30,0x18,0x18,0x0c,0x0c,0x06,0x06,0x03,0x03},
{48,0x00,0x00,0x3c,0x66,0xc3,0xe3,0xf3,0xdb,0xcf,0xc7,0xc3,0x66,0x3c},
{49,0x00,0x00,0x7e,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x78,0x38,0x18},
{50,0x00,0x00,0xff,0xc0,0xc0,0x60,0x30,0x18,0x0c,0x06,0x03,0xe7,0x7e},
{51,0x00,0x00,0x7e,0xe7,0x03,0x03,0x07,0x7e,0x07,0x03,0x03,0xe7,0x7e},
{52,0x00,0x00,0x0c,0x0c,0x0c,0x0c,0x0c,0xff,0xcc,0x6c,0x3c,0x1c,0x0c},
{53,0x00,0x00,0x7e,0xe7,0x03,0x03,0x07,0xfe,0xc0,0xc0,0xc0,0xc0,0xff},
{54,0x00,0x00,0x7e,0xe7,0xc3,0xc3,0xc7,0xfe,0xc0,0xc0,0xc0,0xe7,0x7e},
{55,0x00,0x00,0x30,0x30,0x30,0x30,0x18,0x0c,0x06,0x03,0x03,0x03,0xff},
{56,0x00,0x00,0x7e,0xe7,0xc3,0xc3,0xe7,0x7e,0xe7,0xc3,0xc3,0xe7,0x7e},
{57,0x00,0x00,0x7e,0xe7,0x03,0x03,0x03,0x7f,0xe7,0xc3,0xc3,0xe7,0x7e},
{58,0x00,0x00,0x00,0x38,0x38,0x00,0x00,0x38,0x38,0x00,0x00,0x00,0x00},
{59,0x00,0x00,0x30,0x18,0x1c,0x1c,0x00,0x00,0x1c,0x1c,0x00,0x00,0x00},
{60,0x00,0x00,0x06,0x0c,0x18,0x30,0x60,0xc0,0x60,0x30,0x18,0x0c,0x06},
{61,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0xff,0xff,0x00,0x00,0x00,0x00},
{62,0x00,0x00,0x60,0x30,0x18,0x0c,0x06,0x03,0x06,0x0c,0x18,0x30,0x60},
{63,0x00,0x00,0x18,0x00,0x00,0x18,0x18,0x0c,0x06,0x03,0xc3,0xc3,0x7e},
{64,0x00,0x00,0x3f,0x60,0xcf,0xdb,0xd3,0xdd,0xc3,0x7e,0x00,0x00,0x00},
{65,0x00,0x00,0xc3,0xc3,0xc3,0xc3,0xff,0xc3,0xc3,0xc3,0x66,0x3c,0x18},
{66,0x00,0x00,0xfe,0xc7,0xc3,0xc3,0xc7,0xfe,0xc7,0xc3,0xc3,0xc7,0xfe},
{67,0x00,0x00,0x7e,0xe7,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xe7,0x7e},
{68,0x00,0x00,0xfc,0xce,0xc7,0xc3,0xc3,0xc3,0xc3,0xc3,0xc7,0xce,0xfc},
{69,0x00,0x00,0xff,0xc0,0xc0,0xc0,0xc0,0xfc,0xc0,0xc0,0xc0,0xc0,0xff},
{70,0x00,0x00,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xfc,0xc0,0xc0,0xc0,0xff},
{71,0x00,0x00,0x7e,0xe7,0xc3,0xc3,0xcf,0xc0,0xc0,0xc0,0xc0,0xe7,0x7e},
{72,0x00,0x00,0xc3,0xc3,0xc3,0xc3,0xc3,0xff,0xc3,0xc3,0xc3,0xc3,0xc3},
{73,0x00,0x00,0x7e,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x7e},
{74,0x00,0x00,0x7c,0xee,0xc6,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06},
{75,0x00,0x00,0xc3,0xc6,0xcc,0xd8,0xf0,0xe0,0xf0,0xd8,0xcc,0xc6,0xc3},
{76,0x00,0x00,0xff,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0},
{77,0x00,0x00,0xc3,0xc3,0xc3,0xc3,0xc3,0xc3,0xdb,0xff,0xff,0xe7,0xc3},
{78,0x00,0x00,0xc7,0xc7,0xcf,0xcf,0xdf,0xdb,0xfb,0xf3,0xf3,0xe3,0xe3},
{79,0x00,0x00,0x7e,0xe7,0xc3,0xc3,0xc3,0xc3,0xc3,0xc3,0xc3,0xe7,0x7e},
{80,0x00,0x00,0xc0,0xc0,0xc0,0xc0,0xc0,0xfe,0xc7,0xc3,0xc3,0xc7,0xfe},
{81,0x00,0x00,0x3f,0x6e,0xdf,0xdb,0xc3,0xc3,0xc3,0xc3,0xc3,0x66,0x3c},
{82,0x00,0x00,0xc3,0xc6,0xcc,0xd8,0xf0,0xfe,0xc7,0xc3,0xc3,0xc7,0xfe},
{83,0x00,0x00,0x7e,0xe7,0x03,0x03,0x07,0x7e,0xe0,0xc0,0xc0,0xe7,0x7e},
{84,0x00,0x00,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0xff},
{85,0x00,0x00,0x7e,0xe7,0xc3,0xc3,0xc3,0xc3,0xc3,0xc3,0xc3,0xc3,0xc3},
{86,0x00,0x00,0x18,0x3c,0x3c,0x66,0x66,0xc3,0xc3,0xc3,0xc3,0xc3,0xc3},
{87,0x00,0x00,0xc3,0xe7,0xff,0xff,0xdb,0xdb,0xc3,0xc3,0xc3,0xc3,0xc3},
{88,0x00,0x00,0xc3,0x66,0x66,0x3c,0x3c,0x18,0x3c,0x3c,0x66,0x66,0xc3},
{89,0x00,0x00,0x18,0x18,0x18,0x18,0x18,0x18,0x3c,0x3c,0x66,0x66,0xc3},
{90,0x00,0x00,0xff,0xc0,0xc0,0x60,0x30,0x7e,0x0c,0x06,0x03,0x03,0xff},
{91,0x00,0x00,0x3c,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x3c},
{92,0x00,0x03,0x03,0x06,0x06,0x0c,0x0c,0x18,0x18,0x30,0x30,0x60,0x60},
{93,0x00,0x00,0x3c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x3c},
{94,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc3,0x66,0x3c,0x18},
{95,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{96,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x38,0x30,0x70},
{97,0x00,0x00,0x7f,0xc3,0xc3,0x7f,0x03,0xc3,0x7e,0x00,0x00,0x00,0x00},
{98,0x00,0x00,0xfe,0xc3,0xc3,0xc3,0xc3,0xfe,0xc0,0xc0,0xc0,0xc0,0xc0},
{99,0x00,0x00,0x7e,0xc3,0xc0,0xc0,0xc0,0xc3,0x7e,0x00,0x00,0x00,0x00},
{100,0x00,0x00,0x7f,0xc3,0xc3,0xc3,0xc3,0x7f,0x03,0x03,0x03,0x03,0x03},
{101,0x00,0x00,0x7f,0xc0,0xc0,0xfe,0xc3,0xc3,0x7e,0x00,0x00,0x00,0x00},
{102,0x00,0x00,0x30,0x30,0x30,0x30,0x30,0xfc,0x30,0x30,0x30,0x33,0x1e},
{103,0x7e,0xc3,0x03,0x03,0x7f,0xc3,0xc3,0xc3,0x7e,0x00,0x00,0x00,0x00},
{104,0x00,0x00,0xc3,0xc3,0xc3,0xc3,0xc3,0xc3,0xfe,0xc0,0xc0,0xc0,0xc0},
{105,0x00,0x00,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x00,0x18,0x00},
{106,0x38,0x6c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x00,0x00,0x0c,0x00},
{107,0x00,0x00,0xc6,0xcc,0xf8,0xf0,0xd8,0xcc,0xc6,0xc0,0xc0,0xc0,0xc0},
{108,0x00,0x00,0x7e,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x78},
{109,0x00,0x00,0xdb,0xdb,0xdb,0xdb,0xdb,0xdb,0xfe,0x00,0x00,0x00,0x00},
{110,0x00,0x00,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0xfc,0x00,0x00,0x00,0x00},
{111,0x00,0x00,0x7c,0xc6,0xc6,0xc6,0xc6,0xc6,0x7c,0x00,0x00,0x00,0x00},
{112,0xc0,0xc0,0xc0,0xfe,0xc3,0xc3,0xc3,0xc3,0xfe,0x00,0x00,0x00,0x00},
{113,0x03,0x03,0x03,0x7f,0xc3,0xc3,0xc3,0xc3,0x7f,0x00,0x00,0x00,0x00},
{114,0x00,0x00,0xc0,0xc0,0xc0,0xc0,0xc0,0xe0,0xfe,0x00,0x00,0x00,0x00},
{115,0x00,0x00,0xfe,0x03,0x03,0x7e,0xc0,0xc0,0x7f,0x00,0x00,0x00,0x00},
{116,0x00,0x00,0x1c,0x36,0x30,0x30,0x30,0x30,0xfc,0x30,0x30,0x30,0x00},
{117,0x00,0x00,0x7e,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0x00,0x00,0x00,0x00},
{118,0x00,0x00,0x18,0x3c,0x3c,0x66,0x66,0xc3,0xc3,0x00,0x00,0x00,0x00},
{119,0x00,0x00,0xc3,0xe7,0xff,0xdb,0xc3,0xc3,0xc3,0x00,0x00,0x00,0x00},
{120,0x00,0x00,0xc3,0x66,0x3c,0x18,0x3c,0x66,0xc3,0x00,0x00,0x00,0x00},
{121,0xc0,0x60,0x60,0x30,0x18,0x3c,0x66,0x66,0xc3,0x00,0x00,0x00,0x00},
{122,0x00,0x00,0xff,0x60,0x30,0x18,0x0c,0x06,0xff,0x00,0x00,0x00,0x00},
{123,0x00,0x00,0x0f,0x18,0x18,0x18,0x38,0xf0,0x38,0x18,0x18,0x18,0x0f},
{124,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18},
{125,0x00,0x00,0xf0,0x18,0x18,0x18,0x1c,0x0f,0x1c,0x18,0x18,0x18,0xf0},
{126,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x8f,0xf1,0x60,0x00,0x00,0x00},
{Glx::END_OF_LIST}
};

//////////////////////////////////////////////////////////////////////////
///////////////////////// Methods for class glx //////////////////////////
//////////////////////////////////////////////////////////////////////////

glx::glx(glx::UserFunc func, void* user,
	 int x,int y,int w, int h, int border, bool fs) :
  itsApp(0), 
  itsDisplay(0), 
  itsVisual(0), 
  itsGLXcontext(0), 
  itsWorkID(0), 
  itsGLXarea(0), 
  itsResizeWidget(0),
  done(false),
  itsAspectRatio(1.0f), 
  itsWinX(x),
  itsWinY(y),
  itsWinWidth(w),
  itsWinHeight(h),
  itsPrevWinOriginX(x),
  itsPrevWinOriginY(y),
  itsPrevWinWidth(w),
  itsPrevWinHeight(h),
  itsFullscreenFlag(fs),
  itsBorderFlag(border),
  itsLastBorderFlag(glx::FullBorder),
  waitingToDraw(false),
  itsSector(0),
  itsNumSectors(1),
  animating(false),
  step(0),steps(1),
  axisVisible(true),
  itsCurDragger(0),
  itsBG(0.0),
  highlighted(false),
  highlightcolor(1,1,0),
  dumping(false),
  event(0),
  buttonpressed(glx::NONE), 
  projParams(glx::NPARAMS)
{
  int argc=0;
  char** argv=0;
  itsMessageColor[0]=itsMessageColor[1]=itsMessageColor[2]=1;
  
  assert( XInitThreads() );
  XtToolkitInitialize();
  assert( XtToolkitThreadInitialize() );
  itsApp = XtCreateApplicationContext();
  itsDisplay = XtOpenDisplay(itsApp,0L,"a","a", NULL,0, &argc, argv);
  if( func !=NULL ) 
    itsInitFuncs.push_back( glx::UserPair(func,user) );
  for(int i=0;i<NPARAMS;++i) projParams[i] = glx::ParamPair(glx::dummy,this);
  build();
}

glx::glx(XtAppContext app, Display* disp, glx::UserFunc func, void* user,
	 int x,int y,int w, int h, int border, bool fs) :
  itsApp(app), itsDisplay(disp), 
  itsVisual(0), 
  itsGLXcontext(0), 
  itsWorkID(0), 
  itsGLXarea(0), 
  itsResizeWidget(0),
  done(false),
  itsAspectRatio(1.0f), 
  itsWinX(x),
  itsWinY(y),
  itsWinWidth(w),
  itsWinHeight(h),
  itsPrevWinOriginX(x),
  itsPrevWinOriginY(y),
  itsPrevWinWidth(w),
  itsPrevWinHeight(h),
  itsFullscreenFlag(fs),
  itsBorderFlag(border),
  itsLastBorderFlag(glx::FullBorder),
  waitingToDraw(false),
  itsSector(0),
  itsNumSectors(1),
  animating(false),
  step(0),steps(1),
  axisVisible(true),
  itsCurDragger(0),
  itsBG(0.0),
  highlighted(false),
  highlightcolor(1,1,0),
  dumping(false),
  event(0),
  buttonpressed(glx::NONE), 
  projParams(glx::NPARAMS)
{
  itsMessageColor[0]=itsMessageColor[1]=itsMessageColor[2]=1;
  if( func != NULL) 
    itsInitFuncs.push_back( glx::UserPair(func,user) );
  for(int i=0;i<NPARAMS;++i) projParams[i] = glx::ParamPair(glx::dummy,this);
  build();
}

glx::glx(Widget parent, glx::UserFunc func, void* user,
	 int x,int y,int w, int h) :
  itsApp(XtDisplayToApplicationContext(XtDisplay(parent))), 
  itsDisplay(XtDisplay(parent)), 

  itsVisual(0), 
  itsGLXcontext(0), 
  itsWorkID(0), 
  itsWindow(0), 
  itsGLXarea(0), 
  itsResizeWidget(0),
  done(false),
  itsAspectRatio(1.0f), 
  itsWinX(x),
  itsWinY(y),
  itsWinWidth(w),
  itsWinHeight(h),
  itsPrevWinOriginX(x),
  itsPrevWinOriginY(y),
  itsPrevWinWidth(w),
  itsPrevWinHeight(h),
  itsFullscreenFlag(false),
  itsBorderFlag(glx::NoBorder),
  itsLastBorderFlag(glx::NoBorder),
  waitingToDraw(false),
  itsSector(0),
  itsNumSectors(1),
  animating(false),
  step(0),steps(1),
  axisVisible(true),
  itsCurDragger(0),
  itsBG(0.0),
  event(0),
  buttonpressed(glx::NONE), 
  projParams(glx::NPARAMS)
{
  itsMessageColor[0]=itsMessageColor[1]=itsMessageColor[2]=1;
  if( func != NULL) 
    itsInitFuncs.push_back( glx::UserPair(func,user) );
  for(int i=0;i<NPARAMS;++i) projParams[i] = glx::ParamPair(glx::dummy,this);
  build(parent);
}

glx::~glx(void)
{
  FANCYMESG("glx::~glx");

  MESG("closing pipe");
  int res;
  res = close(p[0]);
  if(res != 0 ) _PERROR("close");

  res=close(p[1]);
  if(res != 0 ) _PERROR("close");
  MESG("closing pipe [done]");

  MESG("deleting draggers");
  VAR(itsDraggerList.size());

  vector<Glx::Draggable*> dvec=itsDraggerList; // work on a copy
  itsDraggerList.clear(); // clear before to avoid unregisterDragger problems
  vector<Glx::Draggable*>::iterator di=dvec.begin();
  for(;di!=dvec.end();++di) {
    Glx::Draggable* dragger = *di;
    MESGVARHEX("deleting dragger",dragger);
    VAR(typeid(*dragger).name());
    delete dragger;
  }
  MESG("deleting draggers [done]");
  
  MESG("cleaning up fonts");

#if 0

  // BLUTE: these are shared across GLXs it would be a BAD
  // idea to clean these up everytime a window is destroyed
  // take my word for it...

#ifdef USEFREETYPE
  map<string,Glx::FontCache*>::iterator fi=Priv::fontMap.begin();
  for(;fi!=Priv::fontMap.end();++fi) delete fi->second;
  map<string,XGlyphInfo*>::iterator xi=Priv::extentsMap.begin();
  for(;xi!=Priv::extentsMap.end();++xi) delete xi->second;  
#else
  glDeleteLists(Priv::itsFontBase,128);
#endif
  MESG("cleaning up fonts [done]");
#endif

  MESG("cleaning up window");
  VARHEX(itsWindow);

  if( itsWindow ) 
    XtDestroyWidget(itsWindow);
  if(itsGLXcontext)
    glXDestroyContext(itsDisplay,itsGLXcontext);

  MESG("cleaning up window [done]");


}

void all(Widget, XtPointer user, XEvent *event, Boolean* more)
{
  _FANCYMESG("////////////all////////////");
  *more=True;
}

void
glx::build(Widget parent)
{
  if( parent==NULL ){
    itsWindow = 
      XtVaAppCreateShell("glx", "glx", topLevelShellWidgetClass, itsDisplay,
			 XmNx, itsWinX, 
			 XmNy, itsWinY, 
			 XmNwidth, itsWinWidth, 
			 XmNheight, itsWinHeight, 
			 //XmNmwmDecorations, 1,
			 NULL);
    
    // trap the 'X' 'go-away' button
    XtVaSetValues(itsWindow, XmNdeleteResponse, XmDO_NOTHING, NULL);
    Atom WM_DELETE_WINDOW = XmInternAtom(itsDisplay,"WM_DELETE_WINDOW",False);
    XmRemoveWMProtocols(itsWindow, &WM_DELETE_WINDOW, 1 );
    XmAddWMProtocolCallback(itsWindow, WM_DELETE_WINDOW,glx::goaway,this);

  } else {
    // let parent size override our defaults
    if( itsWinX==0 && itsWinY==0 && itsWinWidth==500 && itsWinHeight==500 ){
      XtVaGetValues(parent, 
		    XmNwidth, &itsWinWidth,
		    XmNheight, &itsWinHeight,
		    NULL); 
    }
  }
  itsVisual = visual();
  
  itsGLXarea = 
    XtVaCreateManagedWidget("glx", glwDrawingAreaWidgetClass, 
			    parent ? parent : itsWindow, 
			    XmNx, itsWinX, 
			    XmNy, itsWinY, 
			    XmNwidth, itsWinWidth, 
			    XmNheight, itsWinHeight,
			    GLwNvisualInfo, itsVisual, NULL); 

  itsResizeWidget = itsWindow ? itsWindow : itsGLXarea;

  XtAddCallback(itsGLXarea, GLwNginitCallback, glx::xInit, this);
  XtAddCallback(itsGLXarea, XmNexposeCallback, glx::xExpose, this);
  XtAddCallback(itsGLXarea, XmNresizeCallback, glx::xResize, this);

  if( itsGlobalGLXcontext==0 ){
    itsGlobalGLXcontext = 
      glXCreateContext(itsDisplay,itsVisual,None,GL_TRUE);
    assert(itsGlobalGLXcontext);
  }
  itsGLXcontext = glXCreateContext(itsDisplay,itsVisual,
				   itsGlobalGLXcontext,GL_TRUE);
  assert(itsGLXcontext);

  XtAddEventHandler(itsGLXarea,KeyPressMask,     0,glx::xKeyDown, this);
  XtAddEventHandler(itsGLXarea,EnterWindowMask,  0,glx::xSetFocus,this);
  XtAddEventHandler(itsGLXarea,ButtonPressMask,  0,glx::xStart,   this);
  XtAddEventHandler(itsGLXarea,PointerMotionMask,0,glx::xProcess, this);
  XtAddEventHandler(itsGLXarea,ButtonReleaseMask,0,glx::xEnd,     this);

  // hack to see ALL events
  //unsigned long ALL=0x1FFFFFF; // 1<<[0..24] 2^25-1
  //XtAddRawEventHandler(itsGLXarea,ALL,0,all,this);

  pipe(p);
  XtAppAddInput(itsApp,p[0],(XtPointer)(XtInputReadMask),
		glx::xHandleSocketIO,this);

  XtRealizeWidget( itsWindow ? itsWindow : itsGLXarea);
  
  // hack to see ALL events
  //XSelectInput(itsDisplay, XtWindow(itsGLXarea), 0x1ffffff);

  setBorder(itsBorderFlag);
  fullscreen(itsFullscreenFlag);

  XtMapWidget(itsWindow ? itsWindow : itsGLXarea);
}

void
glx::xHandleSocketIO(XtPointer clientData, int*, XtInputId*)
{
  FANCYMESG("glx::xHandleSocketIO");
  char c;
  glx* _this = static_cast<glx*>(clientData);
  int res = ::read(_this->p[0],&c,sizeof(char));
  MESGVAR("glx::xHandleSocketIO(): reading from pipe[0]",res);
  if( res <=0 ){
    _PERROR("read");
  }
  switch( (int) c ){
    case glx::REDRAW:
      MESGVAR("xHandleSocketIO: message=REDRAW",(int)c);
      _this->draw();
      break;
    case glx::QUIT:
      MESGVAR("xHandleSocketIO: message=QUIT",(int)c);
      {
	_this->done=true;
	_this->waitingToDraw=false;
	
	XExposeEvent ev;
	ev.type = Expose;
	ev.send_event = True;
	ev.display = _this->itsDisplay;
	ev.window = XtWindow(_this->itsGLXarea);
	ev.count = 0;
	XSendEvent(_this->itsDisplay,XtWindow(_this->itsGLXarea),
		   False,NoEventMask,(XEvent*)&ev);
	XFlush(_this->itsDisplay);
	XSync(_this->itsDisplay,False);	
      }     
      break;
    default:
      _MESGVAR("xHandleSocketIO: message=UNKNOWN",(int)c);
      break;
  }

}

void  
glx::fullscreen(bool enabled)
{  
  XWindowChanges changes;
  Position xx,xy,xw,xh;
  Widget last=0,w = itsGLXarea;
  while( (w = XtParent(w)) ) last=w;
  Widget itsShell = last;
  Screen* screen = XtScreen(itsGLXarea);

  MESGVAR("glx::fullscreen",enabled);

  itsFullscreenFlag = enabled;
  VAR4(itsPrevWinOriginX,itsPrevWinOriginY,itsPrevWinWidth,itsPrevWinHeight);
  switch( itsFullscreenFlag ){
    case true:
      XtVaGetValues(itsShell,XmNwidth,&xw,
		    XmNheight,&xh,XmNx,&xx,XmNy,&xy,NULL);
      itsPrevWinWidth = xw;
      itsPrevWinHeight = xh;
      itsPrevWinOriginX = xx;
      itsPrevWinOriginY = xy;
      changes.x = 0;
      changes.y = 0;
      changes.width=WidthOfScreen(screen);
      changes.height=HeightOfScreen(screen);
      break;
    case false:
      changes.x = itsPrevWinOriginX;
      changes.y = itsPrevWinOriginY;
      changes.width=itsPrevWinWidth;
      changes.height=itsPrevWinHeight;
      break;
  }

  if (itsFullscreenFlag != (itsBorderFlag == NoBorder))
    borderless(itsFullscreenFlag);

  XConfigureWindow(itsDisplay,XtWindow(itsShell),
		   CWX|CWY|CWWidth|CWHeight,&changes);
}

////////////////////////////////////////////////////////////////////////

void glx::setBorder(int borderval)
{
  Widget last=0,w = itsGLXarea;
  while( (w = XtParent(w)) ) last=w;
  Widget itsShell = last;
  itsLastBorderFlag = itsBorderFlag;
  itsBorderFlag = borderval;

  XtUnmapWidget(itsShell);

  int decorations;
  switch (borderval) {
    case NoBorder: decorations = 0; break;
    case SlimBorder: decorations = MWM_DECOR_BORDER; break;
    case FullBorder: decorations = MWM_DECOR_ALL; break;
  }

  MotifWmHints hints;
  Atom atom = XInternAtom(itsDisplay,"_MOTIF_WM_HINTS",0);
  hints.flags = MWM_HINTS_DECORATIONS;
  hints.decorations = decorations;
  XChangeProperty(itsDisplay, XtWindow(itsShell), atom, atom, 32,
		  PropModeReplace, (unsigned char *) &hints, 4);
#ifdef _XA_MWM_HINTS
  PropMwmHints mwmhints;
  mwmhints.flags = MWM_HINTS_DECORATIONS;
  mwmhints.decorations = decorations;
  Atom mwmHintsAtom = XInternAtom(itsDisplay,_XA_MWM_HINTS,False);
  XChangeProperty(itsDisplay, XtWindow(itsShell),mwmHintsAtom,mwmHintsAtom,32,
                  PropModeReplace,(unsigned char *)&mwmhints,
                  PROP_MWM_HINTS_ELEMENTS);
#endif

  XtMapWidget(itsShell);
}

void glx::borderless(const int enabled)
{  
  if (enabled) setBorder((int)NoBorder);
  else setBorder(itsLastBorderFlag);
}

void glx::slimborder(const int enabled)
{  
  if (enabled) setBorder((int)SlimBorder);
  else setBorder(itsLastBorderFlag);
}

////////////////////////////////////////////////////////////////////////

void
glx::setPosition(int x, int y)
{
  int n = 0;
  Arg args[30];
  XtSetArg(args[n], XmNx, x); n++;
  XtSetArg(args[n], XmNy, y); n++;
  XtSetValues(XtParent(itsGLXarea),args,n);
}

void
glx::setSize(int w, int h)
{
  int n = 0;
  Arg args[30];
  XtSetArg(args[n], XmNwidth, w); n++;
  XtSetArg(args[n], XmNheight, h); n++;
  XtSetValues(XtParent(itsGLXarea),args,n);
  itsWinWidth = w;
  itsWinHeight = h;
  makeCurrent();
  glViewport(0, 0, itsWinWidth, itsWinHeight);
  GLenum err = glGetError();
  itsAspectRatio = (float)itsWinWidth/itsWinHeight;
  waitingToDraw=false;
  wakeup();
}

void 
glx::getCoords(Display *dpy, Window win,
	       int* x,int* y, int* w, int* h,int lvl)
{
  Window root,par,*children;
  int _x,_y;
  unsigned int nChildren,_w,_h,b,d;
  XQueryTree(dpy,win,&root,&par,&children,&nChildren);
  XGetGeometry(dpy,win,&root,&_x,&_y,&_w,&_h,&b,&d);
  if(lvl==0) {*w=_w;*h=_h;}
  if( root==par ){*x=_x;*y=_y;return;}
  getCoords(dpy,par,x,y,w,h,lvl+1);
}

void 
glx::getCoords(int* x,int* y, int* w, int* h)
{
  getCoords(itsDisplay,XtWindow(itsGLXarea),x,y,w,h);
}

void 
glx::mainLoop(void)
{
  FANCYMESG("glx::mainLoop");
  
  //cout << "glx::mainLoop() : starting...\n" << flush;
  XEvent xEvent;
  while( ! done ){
    XtAppNextEvent(itsApp, &xEvent); 
    XtDispatchEvent(&xEvent);
    //cout << "glx::mainLoop() : done = "<<done<<"\n";
  }
  //cout << "glx::mainLoop() : exiting...\n"; 
  callUserFuncs(itsQuitFuncs);   
}

void glx::setDone(void)
{
  FANCYMESG("glx::setDone");
  done=true;
  wakeup(glx::QUIT);
}

void
glx::updateViewport(void)
{
  XtVaGetValues(XtParent(itsGLXarea), 
		XmNwidth, &itsWinWidth,
		XmNheight, &itsWinHeight,
		NULL);
  makeCurrent();
  glViewport(0, 0, itsWinWidth, itsWinHeight);
  itsAspectRatio = (float)itsWinWidth/itsWinHeight;
  glGetIntegerv(GL_VIEWPORT, Priv::viewport);
  
  callUserFuncs(itsConfigureFuncs);
}

XVisualInfo *
glx::visual(void) 
{
  FANCYMESG("glx::visual");
  XVisualInfo *vis;
  int list[32];
  int n = 0; 
#if 0
  list[n++] = GLX_DOUBLEBUFFER;
  list[n++] = GLX_RGBA;
  list[n++] = GLX_DEPTH_SIZE;
  list[n++] = 1;
  list[n++] = (int) None;
#else
  list[n++] = GLX_DOUBLEBUFFER;
  list[n++] = GLX_RGBA;
  list[n++] = GLX_RED_SIZE;
  list[n++] = 1;
  list[n++] = GLX_GREEN_SIZE;
  list[n++] = 1;
  list[n++] = GLX_BLUE_SIZE;
  list[n++] = 1;
  list[n++] = GLX_ALPHA_SIZE;
  list[n++] = 1;
  list[n++] = GLX_DEPTH_SIZE;
  list[n++] = 1;
  list[n++] = GLX_ACCUM_RED_SIZE;
  list[n++] = 1;
  list[n++] = GLX_ACCUM_GREEN_SIZE;
  list[n++] = 1;
  list[n++] = GLX_ACCUM_BLUE_SIZE;
  list[n++] = 1;
  list[n++] = GLX_ACCUM_ALPHA_SIZE;
  list[n++] = 1;
  list[n++] = GLX_STENCIL_SIZE;
  list[n++] = 1;
  list[n++] = (int) None;
#endif
  vis = glXChooseVisual(itsDisplay, DefaultScreen(itsDisplay), list);
  assert(vis);
  return( vis );
}

void 
glx::background(float r, float g, float b)
{
  makeCurrent();  
  glClearColor(r,g,b,0);
}

void glx::setHighlightColor(float r, float g, float b)
{
  highlightcolor=Glx::Vector(r,g,b);
}

void glx::setHighlightColor(Glx::Vector color)
{
  highlightcolor=color;
}

void glx::setHighlighting(bool enabled)
{
  highlighted=enabled;
  wakeup();
}
bool glx::getHighlighting(void)
{
  return highlighted;
}

void glx::toggleDumping(void)
{
  dumping=!dumping;
  _VAR(dumping);
}

void 
glx::setWinTitle(std::string title)
{
  XtVaSetValues(XtParent(itsGLXarea),XmNtitle,title.c_str(),NULL);
}

int  
glx::makeCurrent(void)
{
  int res=0;
  FANCYMESG("glx::makeCurrent");

  if( ! XtIsRealized(itsGLXarea) )
    return res;
  res = glXMakeCurrent(itsDisplay,XtWindow(itsGLXarea),itsGLXcontext);
  //glXWaitX();
  if( ! res ){
    cerr << "glXMakeCurrent failed! (acquire)"<<endl;
  }
  return res;
}

void 
glx::wakeup(int mesg)
{
  FANCYMESG("glx::wakeup");
#if 0
  XExposeEvent ev;
  ev.type = Expose;
  ev.send_event = True;
  ev.display = itsDisplay;
  ev.window = XtWindow(itsGLXarea);
  ev.count = 0;
  ev.x=ev.y=ev.width=ev.height=5;
  MESG("glx::wakeup(): sending event");
  MESGVAR("glx::wakeup()",mesg);
  if( mesg==glx::QUIT ) done=true;
  Status res = XSendEvent(itsDisplay,
			  XtWindow(itsGLXarea),
			  False,
			  NoEventMask,
			  (XEvent*)&ev);
  XFlush(itsDisplay);
  XSync(itsDisplay,False);
#else
  //if( waitingToDraw ) return;
  char c=(char)mesg;
  int res = ::write(p[1],&c,sizeof(char));
  MESGVAR("glx::wakeup(): writing to pipe[1]",res);  
  if( res <=0 ) {
    _PERROR("write");
    _VAR(p[1]);
  }
  //res=fsync(p[1]); 
  //if( res <=0 ) {
  //  _PERROR("fsync");
  //  _VAR(res);
  //}
  waitingToDraw=true;
  XFlush(itsDisplay);
  XSync(itsDisplay,False);
  //glFlush();
  //glFinish();
#endif
}

void
glx::quit(void)
{
  setDone();
}

int 
glx::winWidth()
{
#if 0
  // itsWinWidth set in xResize
  Position xX,xY,xWidth,xHeight;
  XtVaGetValues(XtParent(itsGLXarea), 
		XmNx, &xX,
		XmNy, &xY,
		XmNwidth, &xWidth,
		XmNheight, &xHeight,
		NULL);
  itsWinWidth=xWidth;
  itsWinHeight=xHeight;
#endif
  return itsWinWidth;
}
int 
glx::winHeight()
{
  // itsWinHeight set in xResize
#if 0
  Position xX,xY,xWidth,xHeight;
  XtVaGetValues(XtParent(itsGLXarea), 
		XmNx, &xX,
		XmNy, &xY,
		XmNwidth, &xWidth,
		XmNheight, &xHeight,
		NULL);
  itsWinHeight=xHeight;
  itsWinWidth=xWidth;
#endif
  return itsWinHeight;
}

const int* glx::viewport(void){return Priv::viewport;}
const double* glx::modelMatrix(void){return Priv::modelMatrix;}
const double* glx::projMatrix(void){return Priv::projMatrix;}

float 
glx::aspect()
{
  Position xX,xY,xWidth,xHeight;
  XtVaGetValues(XtParent(itsGLXarea), 
		XmNx, &xX,
		XmNy, &xY,
		XmNwidth, &xWidth,
		XmNheight, &xHeight,
		NULL);
  itsWinWidth=xWidth;
  itsWinHeight=xHeight;
  itsAspectRatio = (float)itsWinWidth/itsWinHeight;
  return itsAspectRatio;
}

string 
glx::nextFilename(string pre,string post)
{
  struct stat statBuf;
  char buf[80];
  int res, found = 0;
  for(int i = 0; i < MAXINT && ! found ; i ++){
    //sprintf(buf,"%s.%04d.%s",pre.c_str(),i,post.c_str());
    sprintf(buf,"%s.%05d",pre.c_str(),i);
    res = stat(buf, &statBuf);
    if( res == -1 ){
      if( errno == ENOENT){
	found = 1;
	return string( buf );
      } else {
	perror("stat()");
	return string("");
      }
    } 
  }
  return string("");
}

void
glx::dumpImage(void)
{  
  dumpImage( nextFilename("glx","ppm") );
}

void
glx::dumpImage(string name)
{  
  makeCurrent();
  int width = winWidth();
  int height = winHeight();
  int depth = 3; // RGB
  int size = width * height * depth; // RGB
  unsigned char* pixels = new unsigned char[size];
  if( pixels == 0 ){
    cerr << "Error allocating memory" << endl;
    return;
  }
  cout << width << " x " << height << " x " << depth << " = " <<size<<endl;
  
  // ok, read the data
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_ROW_LENGTH,width); 
  glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,pixels);

  ofstream fout(name.c_str());
  fout << "P6 " << width << " " << height << " 255 "; 
  for(int row = (height-1) ; row >=0 ; row--){
    int offset = row * width * depth;
    fout.write((char*)&pixels[offset], width*depth*sizeof(unsigned char));
  }
  fout.close();

  delete [] pixels;
  
  cout << name << " written" << endl;
}

//////////////////////////////////////////////////////////////////////////
////////////////////////// USER CALLBACK STUFF ///////////////////////////
//////////////////////////////////////////////////////////////////////////

void 
glx::addQuitFunc(glx::UserFunc func, void* user)
{
  itsQuitFuncs.push_back( glx::UserPair(func,user) );
  //wakeup();
}

void 
glx::addPreDrawFunc(glx::UserFunc func, void* user)
{
  itsPreDrawFuncs.push_back( glx::UserPair(func,user) );
  //wakeup();
}

void 
glx::addDrawFunc(glx::UserFunc func, void* user)
{
  itsDrawFuncs.push_back( glx::UserPair(func,user) );
  //wakeup();
}
void 
glx::addPostDrawFunc(glx::UserFunc func, void* user)
{
  itsPostDrawFuncs.push_back( glx::UserPair(func,user) );
  //wakeup();
}
void 
glx::addPostSwapFunc(glx::UserFunc func, void* user)
{
  itsPostSwapFuncs.push_back( glx::UserPair(func,user) );
  //wakeup();
}

void 
glx::addEventFunc(glx::EventFunc func, void* user)
{
  itsEventFuncs.push_back( glx::EventPair(func,user) );
  //wakeup();
}

void 
glx::addMouseDownFunc(glx::MouseFunc func, glx::BUTTON btn, void* user) 
{
  itsMouseDownFuncs[btn].push_back( glx::MousePair(func,user) );
  //wakeup();
}

void 
glx::addMouseProcessFunc(glx::MouseFunc func, glx::BUTTON btn, void* user) 
{
  itsMouseProcFuncs[btn].push_back( glx::MousePair(func,user) );
  //wakeup();
}

void 
glx::addMouseUpFunc(glx::UserFunc func, glx::BUTTON btn,void* user) 
{
  if( btn==glx::NUM_BUTTON){
    for(int i=0;i<glx::NUM_BUTTON;++i)
      itsMouseUpFuncs[i].push_back( glx::UserPair(func,user) );      
  } else
    itsMouseUpFuncs[btn].push_back( glx::UserPair(func,user) );
  //wakeup();
}

void 
glx::addMouseIdleFunc(glx::MouseFunc func,void* user) 
{
  itsMouseIdleFuncs.push_back( glx::MousePair(func,user) );
  //wakeup();
}

void 
glx::addProjFunc(glx::UserFunc func, void* user)
{
  itsProjectionFuncs.push_back( glx::UserPair(func,user) );
  //wakeup();
}

void 
glx::addConfigureFunc(glx::UserFunc func,void* user) 
{
  itsConfigureFuncs.push_back( glx::UserPair(func,user) );
  //wakeup();
}

//////////////////////////////////////////////////////////////////////////
/////////////////////////// Matrix/View stuff ////////////////////////////
//////////////////////////////////////////////////////////////////////////

void glx::draw()
{
  xRender(this);
}

void glx::highlight(void)
{
  int w=winWidth(),h=winHeight();
  int b=20;

  GLint mode;
  glGetIntegerv(GL_MATRIX_MODE,&mode);

  glPushAttrib(GL_LIGHTING_BIT|GL_DEPTH_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0,w,0,h,-1,1);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glColor3fv(highlightcolor);
  glBegin(GL_QUADS);
  glVertex2f(0,0);
  glVertex2f(0,h);
  glVertex2f(b,h);
  glVertex2f(b,0);

  glVertex2f(0,h);
  glVertex2f(w,h);
  glVertex2f(w,h-b);
  glVertex2f(0,h-b);

  glVertex2f(w,h);
  glVertex2f(w,0);
  glVertex2f(w-b,0);
  glVertex2f(w-b,h);

  glVertex2f(w,0);
  glVertex2f(0,0);
  glVertex2f(0,b);
  glVertex2f(w,b);
  glEnd();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(mode);
  glPopAttrib();
}

void 
glx::perspective(float fov, float aspect, float near, float far)
{
  double fov2,t,b,r,l;

  FANCYMESG("glx::perspective");

  if( itsNumSectors==1 )
  {
    fov2 = ((fov*M_PI) / 180.0) / 2.0;
    t = near * tan(fov2);
    b = -t;
    r = t * aspect;
    l = -r;
  }
  else
  {
    fov2 = ((fov*M_PI) / 180.0) / 2.0;
    double gt = near * tan(fov2);
    double gb = -gt;
    double gr = gt * aspect;
    double gl = -gr;
    int col = itsSector % itsNumSectors;
    int row = (int)(itsSector / itsNumSectors);
    double delta = 1.0/(double)(itsNumSectors);
    l = gl + ((double)(col  ) * delta)*(gr-gl);
    r = gl + ((double)(col+1) * delta)*(gr-gl);
    t = gt + ((double)(row  ) * delta)*(gb-gt);
    b = gt + ((double)(row+1) * delta)*(gb-gt);
  }
  glMatrixMode(GL_PROJECTION);
  glFrustum(l,r,b,t,near,far);
}

void
glx::getEyePosition(Glx::Vector& pos)
{
#if 0
  double modelMatrix[16],viewMatrix[16],invMatrix[16];
  Glx::Vector eye;
  makeCurrent();
  glGetDoublev(GL_MODELVIEW_MATRIX,(double *)modelMatrix);
#endif
  double inv[16];
  Glx::Vector eye;
  Glx::inv4x4(Priv::modelMatrix, inv);
  Glx::xformVec(eye,inv,pos);
}

void
glx::getEyePosition(double invMatrix[16], Glx::Vector& pos)
{
  Glx::Vector eye;
  Glx::xformVec(eye,invMatrix,pos);
}

void 
glx::getProjectionVector(double x, double y, Glx::Vector& p)
{
  double modelMatrix[16],viewMatrix[16],invMatrix[16];
  Glx::Vector mousePos(x*proj(PROJW)/aspect(),y*proj(PROJH),-proj(NEAR));
  Glx::Vector mousePosXformed;
  Glx::Vector eye,eyepos;

  makeCurrent();
  glGetDoublev(GL_MODELVIEW_MATRIX,(double *)modelMatrix);
  Glx::inv4x4(modelMatrix, invMatrix);
  Glx::xformVec(eye,invMatrix,eyepos);
  Glx::xformVec(mousePos,invMatrix,mousePosXformed); 

  p[0] = mousePosXformed[0] - eyepos[0];
  p[1] = mousePosXformed[1] - eyepos[1];
  p[2] = mousePosXformed[2] - eyepos[2];   
}
void 
glx::getProjectionVector(double invMatrix[16], 
			 double x, double y, Glx::Vector& p)
{
  Glx::Vector mousePos(x*proj(PROJW)/aspect(),y*proj(PROJH),-proj(NEAR));
  Glx::Vector mousePosXformed;
  Glx::Vector eye,eyepos;

  Glx::xformVec(eye,invMatrix,eyepos);
  Glx::xformVec(mousePos,invMatrix,mousePosXformed); 

  p[0] = mousePosXformed[0] - eyepos[0];
  p[1] = mousePosXformed[1] - eyepos[1];
  p[2] = mousePosXformed[2] - eyepos[2];   
}

void 
glx::pixelToWinCoords(int x, int y, double* winx, double* winy)
{
  FANCYMESG("glx::pixelToWinCoords");
  VAR2(x,y);
  int viewport[4];
  makeCurrent();
  glGetIntegerv(GL_VIEWPORT, viewport);

  VAR4(viewport[0],viewport[1],viewport[2],viewport[3]);

  /* Map x and y from window coordinates [0..1,0..1] */
  *winx = (double)(x - viewport[0]) / (double)(viewport[2]);
  *winy = (double)(viewport[3] - y) / (double)(viewport[3]);

  /* Map to range -1 to 1 */
  *winx = *winx * 2.0 - 1.0;
  *winy = *winy * 2.0 - 1.0;

  VAR2(*winx,*winy);
}

void
glx::pixelToWorldCoords(int x, int y, double* xf, double* yf)
{  
  int winh = winHeight();
  double px,py,pz;
  unproject(x,y,0,
	    modelMatrix(),
	    projMatrix(),
	    viewport(),&px,&py,&pz);
  *xf = px;
  *yf = py;
}

void
glx::worldToPixelCoords(double x, double y, double z, int* xf, int* yf)
{  
  int winh = winHeight();
  double px,py,pz;
  project(x,y,z,
		  modelMatrix(),
		  projMatrix(),
		  viewport(),&px,&py,&pz);
  *xf = (int)round(px);
  *yf = (int)round(py);
}

static ostream& operator<<(ostream& ostr, const int ivec[4])
{
  return ostr << ivec[0] <<","
	      << ivec[1] <<","
	      << ivec[2] <<","
	      << ivec[3];
}

static ostream& operator<<(ostream& ostr, const double dvec[4])
{
  return ostr << dvec[0] <<","
	      << dvec[1] <<","
	      << dvec[2] <<","
	      << dvec[3];
}

int
glx::project(double objx, double objy, double objz, 
	     const double modelMatrix[16], 
	     const double projMatrix[16],
	     const int viewport[4],
	     double *winx, double *winy, double *winz)
{
  Glx::Vector in(objx,objy,objz),out;
  Glx::xformVec(in, modelMatrix, out);
  Glx::xformVec(out, projMatrix, in);

  //if( fabs(in[3])<EPS ) return GL_FALSE;

  /* Map x, y and z to range 0-1 */
  in[0] = in[0] * 0.5 + 0.5;
  in[1] = in[1] * 0.5 + 0.5;
  in[2] = in[2] * 0.5 + 0.5;
  
  /* Map x,y to viewport */
  in[0] = in[0] * viewport[2] + viewport[0];
  in[1] = in[1] * viewport[3] + viewport[1];
  
  *winx=in[0];
  *winy=in[1];
  *winz=in[2];
  return(GL_TRUE);
}

int
glx::unproject(double winx, double winy, double winz,
	       const double modelMatrix[16], 
	       const double projMatrix[16],
	       const int viewport[4],
	       double *objx, double *objy, double *objz)
{
  FANCYMESG("glx::unproject");
  VAR(winx);
  VAR(winy);
  VAR(winz);
  VAR(viewport);

  double finalMatrix[16];
  Glx::Vector in(winx,winy,winz),out;
  
  Glx::mult4x4(modelMatrix, projMatrix, finalMatrix);
  if (!Glx::inv4x4(finalMatrix, finalMatrix)) {
    MESG("inv4x4 failed!");
    return(GL_FALSE);
  }
  
  /* Map x and y from window coordinates */
  in[0] = (in[0] - viewport[0]) / viewport[2];
  in[1] = (in[1] - viewport[1]) / viewport[3];
  
  /* Map to range -1 to 1 */
  in[0] = in[0] * 2 - 1;
  in[1] = in[1] * 2 - 1;
  in[2] = in[2] * 2 - 1;
  
  Glx::xformVec(in, finalMatrix, out);

  *objx = out[0];
  *objy = out[1];
  *objz = out[2];
  return(GL_TRUE);
}

//////////////////////////////////////////////////////////////////////////
/////////////////////////// STATIC X CALLBACKS ///////////////////////////
//////////////////////////////////////////////////////////////////////////

void 
glx::goaway(Widget, XtPointer user, XtPointer)
{
  glx* _this = static_cast<glx*>(user);
  _this->callUserFuncs(_this->itsQuitFuncs);  
  _this->setDone();
}

Boolean 
glx::xRender(XtPointer user)
{
  FANCYMESG("glx::xRender");
  const int X=0,Y=1,Z=2;
  glx* _this = static_cast<glx*>(user);
  
  if(! _this->makeCurrent() ){
    cout << "MAKE CURRENT FAILED!" << endl;
    return FALSE;
  }

  if( _this->animating ){
    ++_this->step;
    if( (_this->step % _this->steps)==0 )
      _this->step=0;
  }

  _this->waitingToDraw=false;
  _this->resetFontPos();

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  if( _this->itsProjectionFuncs.size() )
  {
    _this->callUserFuncs(_this->itsProjectionFuncs);
  } 
  glGetDoublev(GL_PROJECTION_MATRIX,Priv::projMatrix);
  glGetDoublev(GL_MODELVIEW_MATRIX,Priv::modelMatrix);

  // see if we have any new draggers...
  while( _this->itsDraggerInitList.size() ){
    Glx::Draggable* dragger = _this->itsDraggerInitList.back(); 
    dragger->viewHasChanged(_this);
    _this->itsDraggerInitList.pop_back();
  }

  if( _this->itsBG < 0.5 ){
    glClearColor(0.0,0.0,(_this->itsBG*2.0)*0.8,0.0);
  } else {
    glClearColor((_this->itsBG-0.5)*2.0,(_this->itsBG-0.5)*2.0,1.0,0.0);
  }

  glClearDepth(1.0);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  /* show axis */
  if( _this->axisVisible ){
    int lw,lighting;
    glGetIntegerv(GL_LIGHTING,&lighting);
    glGetIntegerv(GL_LINE_WIDTH,&lw);
    glLineWidth(3);
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    glColor3fv(Priv::x_axis); 
    glVertex3fv(Priv::origin); 
    glVertex3fv(Priv::x_axis);
    glColor3fv(Priv::y_axis); 
    glVertex3fv(Priv::origin); 
    glVertex3fv(Priv::y_axis);
    glColor3fv(Priv::z_axis); 
    glVertex3fv(Priv::origin); 
    glVertex3fv(Priv::z_axis);
    glEnd();
    if( lighting != GL_FALSE) glEnable(GL_LIGHTING);
    glLineWidth(lw);
  }

  _this->callUserFuncs(_this->itsPreDrawFuncs);
  _this->callUserFuncs(_this->itsDrawFuncs);
  _this->callUserFuncs(_this->itsPostDrawFuncs);

  if( _this->itsDraggerList.size() > 0 ){    
    std::vector<Glx::Draggable*>::iterator iter=_this->itsDraggerList.begin();
    for( ; iter != _this->itsDraggerList.end() ; ++iter ){
      Glx::Draggable* dragger = *iter;
      dragger->draw(_this,NULL);
    }
  }

  if( _this->itsLabel.length() )
    _this->showLabel();

  if( _this->highlighted )
    _this->highlight();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glFinish();
  glXSwapBuffers(_this->itsDisplay,XtWindow(_this->itsGLXarea));
  glXWaitGL();
  XFlush(_this->itsDisplay);
  XSync(_this->itsDisplay,False);	

  if( _this->dumping )
    _this->dumpImage();

  _this->callUserFuncs(_this->itsPostSwapFuncs);

  if( _this->animating )
    _this->wakeup();

  FANCYMESG("glx::xRender [END]");
  return FALSE;
}

void 
glx::xSetFocus(Widget w, XtPointer user, XEvent *event, Boolean* more)
{
  FANCYMESG("glx::xSetFocus");
  *more=True;
  XmProcessTraversal(w,XmTRAVERSE_CURRENT);
}

void 
glx::xStart(Widget w, XtPointer user, XEvent *event, Boolean* more)
{
  FANCYMESG("glx::xStart");

  glx* _this = static_cast<glx*>(user);
  _this->event = event;


  XButtonEvent *buttonEvent = (XButtonEvent *)event;
  *more=True;
  glx::BUTTON btn = (glx::BUTTON)buttonEvent->button;
  VAR(buttonEvent->button);
  _this->buttonpressed = btn;

  if( _this->itsMouseDownFuncs[btn].size() )
  {
    _this->callMouseFuncs(_this->itsMouseDownFuncs[btn],
			  buttonEvent->x,buttonEvent->y);
  }
  if( _this->itsWorkID == 0 )
    _this->itsWorkID = XtAppAddWorkProc(_this->itsApp,glx::xRender,user);
}

void 
glx::xProcess(Widget w, XtPointer user, XEvent *event, Boolean* more)
{
  FANCYMESG("glx::xProcess");

  glx* _this = static_cast<glx*>(user);  
  _this->event = event;

  XButtonEvent *buttonEvent = (XButtonEvent *)event;
  glx::BUTTON btn = _this->buttonpressed;
  vector<Glx::Draggable*>::iterator iter;

  *more=True;
  VAR(_this->buttonpressed);
  switch(_this->buttonpressed){
    case glx::LEFT:
    case glx::MIDDLE:
    case glx::RIGHT:
      if( _this->itsCurDragger && 
	  _this->itsCurDragger->interested(_this->buttonpressed) )
      {
	_this->itsCurDragger->handleDrag(_this,
					 buttonEvent->x,buttonEvent->y,NULL);
      }
      else if( _this->itsMouseProcFuncs[btn].size() )
      {
	_this->callMouseFuncs(_this->itsMouseProcFuncs[btn],
			      buttonEvent->x,buttonEvent->y);
      }
      break;

      // This is the 'idle' state, look for draggers to select
    case glx::NONE:

      if(  _this->itsDraggerList.size() > 0 ){
	vector<Glx::Draggable*>::iterator selectedDragger = 
	  _this->itsDraggerList.end();
	int hndl,selectedHandle;
	float dist=MAXFLOAT,selectedDist=MAXFLOAT;

	iter =_this->itsDraggerList.begin();
	for( ; iter != _this->itsDraggerList.end() ; ++iter ){
	  Glx::Draggable* dragger = *iter;
	  if( ! dragger->getVisibility() )
	    continue;
	  hndl = dragger->idlePick(_this,buttonEvent->x,buttonEvent->y,NULL);

	  if( hndl != Glx::Draggable::UNSELECTED ){
	    dist = dragger->getSelectionDist();
	    if( dist < selectedDist ){
	      if( selectedDragger != _this->itsDraggerList.end() )
		(*selectedDragger)->unselect();
	      selectedDragger = iter;
	      selectedDist = dist;
	      selectedHandle = hndl;
	    } else {
	      dragger->unselect();
	    }
	  }
	} // for iter...

	if( selectedDragger != _this->itsDraggerList.end() ){
	  (*selectedDragger)->select(selectedHandle);
	   _this->itsCurDragger = *selectedDragger;
	} else
	   _this->itsCurDragger = NULL;
      }

      if( _this->itsMouseIdleFuncs.size() ){
	_this->callMouseFuncs(_this->itsMouseIdleFuncs,
			      buttonEvent->x,buttonEvent->y);
      }
      break;
  }
}

void 
glx::xEnd(Widget w, XtPointer user, XEvent *event, Boolean* more)
{
  FANCYMESG("glx::xEnd");

  glx* _this = static_cast<glx*>(user);
  _this->event = event;

  *more=True;
  glx::BUTTON btn = _this->buttonpressed;
  _this->buttonpressed = glx::NONE;
  if( _this->itsWorkID != 0 )
    XtRemoveWorkProc(_this->itsWorkID);
  _this->itsWorkID=0;

  if( _this->itsCurDragger && _this->itsCurDragger->interested(btn) )
  {
    _this->itsCurDragger->handleMouseUp(_this,NULL);
  } 
  else 
  {
    vector<Glx::Draggable*>::iterator iter =_this->itsDraggerList.begin();
    for( ; iter != _this->itsDraggerList.end() ; ++iter )
    {
      Glx::Draggable* dragger = *iter;
      dragger->viewHasChanged(_this);
      /*
      Glx::Draggable3D* d3d = dynamic_cast<Glx::Draggable3D*>(dragger);
      if( d3d )
	d3d->viewHasChanged(_this);
      */
    }
    
    if( _this->itsMouseUpFuncs[btn].size() )
    {
      _this->callUserFuncs(_this->itsMouseUpFuncs[btn]);
    }
  }
  
  _this->xRender(user);
}

void 
glx::xKeyDown(Widget, XtPointer user, XEvent *event, Boolean* more)
{
  FANCYMESG("glx::xKeyDown");
  glx* _this = static_cast<glx*>(user);
  _this->event = event;
  *more=True;

  KeySym ks;
  XComposeStatus status;
  char buffer[20];
  int buf_len = 20;
  XKeyEvent *kep = (XKeyEvent *)(event);
  XLookupString(kep, buffer, buf_len, &ks, &status);

  if( _this->itsCurDragger ){
    _this->itsCurDragger->keyEvent(ks,
				   kep->state & ControlMask,
				   kep->state & ShiftMask,
				   kep->state & Mod1Mask);
    return;
  }

  switch( ks ){
    case XK_x:
      _this->axisVisible = !_this->axisVisible;
      _this->wakeup();
      break;
    case XK_less:
      ++_this->steps;
      break;
    case XK_greater:
      --_this->steps;
      if( _this->steps<1 ) _this->steps=1;
      break;
	
    case XK_comma:
      _this->itsBG -= 0.1;
      if( _this->itsBG<0 )
	_this->itsBG = 0;
      _this->wakeup();
      break;
    case XK_period:
      _this->itsBG += 0.1;
      if( _this->itsBG>1 )
	_this->itsBG = 1;
      _this->wakeup();
      break;
    case XK_q:
      _this->callUserFuncs(_this->itsQuitFuncs);    
      _this->done=true;
      //exit(0);
      break;
    case XK_B:
      _this->toggleDumping();
      break;
    case XK_b:
      _this->dumpImage();
      break;
    case XK_a:
      _this->animating = ! _this->animating;
      _this->wakeup();
      break;
    case XK_Return:
      _this->fullscreen(! _this->itsFullscreenFlag);
      break;
    default:
      //if( _this->itsEventFuncs.size() )
      // _this->callEventFuncs(_this->itsEventFuncs,event);
      break;
  }
  if( _this->itsEventFuncs.size() )
    _this->callEventFuncs(_this->itsEventFuncs,event);

}

void 
glx::xInit(Widget w, XtPointer clientData, XtPointer callData)
{
  FANCYMESG("glx::xInit");
  glx* _this = static_cast<glx*>(clientData);

  glXMakeCurrent(_this->itsDisplay,XtWindow(w),_this->itsGLXcontext);
  glDepthFunc(GL_LEQUAL); 
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);  
  glMatrixMode(GL_MODELVIEW);
  glShadeModel (GL_SMOOTH);
  glClearAccum(0.0, 0.0, 0.0, 0.0);

#ifdef USEFREETYPE
  char* defMesgFace = {"Courier New"};
  char* defLabelFace = {"Times-Roman"};
  char* defTitleFace = {"Times New Roman"};

  char *envMesgFace = getenv("MESGFONT");
  char *envMesgSize = getenv("MESGSIZE");
  char *mesgFace = (envMesgFace==0) ? defMesgFace : envMesgFace;
  double mesgSize = (envMesgSize==0) ? 16. : strtod(envMesgSize,0);

  char *envLabelFace = getenv("LABELFONT");
  char *envLabelSize = getenv("LABELSIZE");
  char *labelFace = (envLabelFace==0) ? defLabelFace : envLabelFace;
  double labelSize = (envLabelSize==0) ? 64. : strtod(envLabelSize,0);

  char *envTitleFace = getenv("TITLEFONT");
  char *envTitleSize = getenv("TITLESIZE");
  char *titleFace = (envTitleFace==0) ? defTitleFace : envTitleFace;
  double titleSize = (envTitleSize==0) ? 128. : strtod(envTitleSize,0);

  string mesgFaceStr(mesgFace);
  string labelFaceStr(labelFace);
  string titleFaceStr(titleFace);

  Priv::setMessageFont(_this->itsDisplay,mesgFaceStr,mesgSize);
  Priv::setLabelFont(_this->itsDisplay,labelFaceStr,labelSize);
  Priv::setTitleFont(_this->itsDisplay,titleFaceStr,titleSize);
#else
  Priv::itsFontBase = glGenLists(128);
  Priv::fontCreateBitmap(Priv::itsFontBase);
#endif
  _this->resetFontPos();
  _this->callUserFuncs(_this->itsInitFuncs);
}

void 
glx::xExpose(Widget w, XtPointer clientData, XtPointer callData)
{
  FANCYMESG("glx::xExpose");
  glx* _this = static_cast<glx*>(clientData);
  _this->xRender(_this);
}

void 
glx::xResize(Widget w, XtPointer clientData, XtPointer callData)
{
  FANCYMESG("glx::xResize");
  glx* _this = static_cast<glx*>(clientData);
  _this->updateViewport();
}

void
glx::callUserFuncs(std::vector< glx::UserPair >& fVector)
{
  std::vector<glx::UserPair>::iterator iter = fVector.begin();
  for( ; iter != fVector.end() ; ++iter ){
    glx::UserPair p = *iter;
    p.first(this,p.second);
  }
}

void
glx::callEventFuncs(std::vector< glx::EventPair >& eVector, XEvent *e)
{
  std::vector<glx::EventPair>::iterator iter = eVector.begin();
  for( ; iter != eVector.end() ; ++iter ){
    glx::EventPair p = *iter;
    p.first(this,e,p.second);
  }
}

void
glx::callMouseFuncs(std::vector< glx::MousePair >& mVector, int x, int y)
{
  std::vector<glx::MousePair>::iterator iter = mVector.begin();
  for( ; iter != mVector.end() ; ++iter ){
    glx::MousePair p = *iter;
    p.first(this,x,y,p.second);
  }
}

void glx::registerDragger(Glx::Draggable* dragger)
{
  MESGVARHEX("adding dragger to itsDraggerList",dragger);
  VAR(typeid(*dragger).name());

  itsDraggerList.push_back(dragger);

  VAR(itsDraggerList.size());

  itsDraggerInitList.push_back(dragger);
  // BLUTE: experimental
  dragger->addUpdateFunc(glx::draggerChanged,this);
}

void glx::unregisterDragger(Glx::Draggable* dragger)
{
  MESGVARHEX("removing dragger from itsDraggerList",dragger);
  std::vector<Glx::Draggable*>::iterator iter = 
    std::find(itsDraggerList.begin(),
	      itsDraggerList.end(),dragger);
  if( iter != itsDraggerList.end() ){
    if( itsCurDragger == *iter )
      itsCurDragger=NULL;
    itsDraggerList.erase(iter);    
  }
}

void glx::draggerChanged(Glx::Draggable*,void* user)
{
  glx* _this = static_cast<glx*>(user);
  _this->wakeup();
}

void glx::viewHasChanged(void)
{
  std::vector<Glx::Draggable*>::iterator iter = itsDraggerList.begin();
  for( ; iter != itsDraggerList.end() ; ++iter ){
    Glx::Draggable *dragger = *iter;
    dragger->viewHasChanged(this);
    /*
    Glx::Draggable3D* d3d = dynamic_cast<Glx::Draggable3D*>(dragger);
    if( d3d )
      d3d->viewHasChanged(this);
    */
  }
  wakeup();
}

//////////////////////////////////////////////////////////////////////////
/////////////////////////////// FONT STUFF ///////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifdef USEFREETYPE 

std::string 
Priv::genFontKey(std::string face, double size)
{
  ostringstream ostr;
  ostr << face << "." << (int)size;
  return ostr.str();  
}

XftFont * 
Priv::loadFont(Display* xdpy, std::string face, double size, 
	       std::string& fontKey)
{
  fontKey = Priv::genFontKey(face,size);
  std::map<std::string,Glx::FontCache*>::iterator iter = 
    Priv::fontMap.find(fontKey);

  if( iter != fontMap.end() ){
    Glx::FontCache* fc = (*iter).second;
    return fc->font;
  }

  int xscn           = DefaultScreen(xdpy);
  fontMap[fontKey]   = new Glx::FontCache(xdpy,face,size);
  XftFont *font      = fontMap[fontKey]->font;
  XGlyphInfo* extent = new XGlyphInfo;

  memset(extent,0,sizeof(XGlyphInfo));
  char* test = {"Testing!"};
  XftTextExtents8(xdpy,font,(const FcChar8*)test,
		  strlen(test),extent);
  Priv::extentsMap[fontKey] = extent;
  return font;
}

void glx::resetFontPos(void)
{
  std::map<std::string,XGlyphInfo*>::iterator i = 
    Priv::extentsMap.find(Priv::curFontKey);
  assert(i!=Priv::extentsMap.end());
  XGlyphInfo* extents = (*i).second;
  itsCurFontLoc[0] = 30;
  itsCurFontLoc[1] = itsWinHeight - extents->height;
}

void glx::advanceFontPos(void)
{
  std::map<std::string,XGlyphInfo*>::iterator i = 
    Priv::extentsMap.find(Priv::curFontKey);
  assert(i!=Priv::extentsMap.end());
  XGlyphInfo* extents = (*i).second;
  itsCurFontLoc[1] -= extents->height;
}

void 
Priv::setMessageFont(Display* xdpy, std::string face, double size)
{
  string fontKey;
  XftFont *font = Priv::loadFont(xdpy,face,size,fontKey);
  if( font==0 ){
    cerr << "glx::setMessageFont: Error loading font:"<<face<<"."<<endl;
    cerr << "Message font not changed." << endl;
    return;
  }
  Priv::curFontKey=mesgFontKey=fontKey;
  //resetFontPos();
}

void 
Priv::setTitleFont(Display* xdpy, std::string face, double size)
{
  string fontKey;
  XftFont *font = Priv::loadFont(xdpy,face,size,fontKey);
  if( font==0 ){
    cerr << "glx::setTitleFont: Error loading font:"<<face<<"."<<endl;
    cerr << "Message font not changed." << endl;
    return;
  }
  titleFontKey=fontKey;
}

void 
Priv::setLabelFont(Display* xdpy, std::string face, double size)
{
  string fontKey;
  XftFont *font = Priv::loadFont(xdpy,face,size,fontKey);
  if( font==0 ){
    cerr << "glx::setLabelFont: Error loading font:"<<face<<"."<<endl;
    cerr << "Label font not changed." << endl;
    return;
  }
  Priv::labelFontKey=fontKey;
}

void 
glx::showTitle(std::vector<std::string>& messages)
{
  int w = winWidth();
  int h = winHeight();
  
  std::map<string,Glx::FontCache*>::iterator fi = 
    Priv::fontMap.find(Priv::titleFontKey);
  assert( fi != Priv::fontMap.end());  
  Glx::FontCache* fcache = (*fi).second;
  fcache->setColor(itsMessageColor);
  fcache->draw(w,h,messages);
}

void 
glx::showMessage(const int x, const int y, string mesg, bool stipple)
{
  FANCYMESG("glx::showMessage(int,int,string,bool) [XFT]");
  if( mesg.length() == 0) return;
  VAR4(x,y,mesg,stipple);

  Glx::FontCache* fcache=0;
  XGlyphInfo extents;
  int w = winWidth();
  int h = winHeight();

  VAR(Priv::curFontKey);

  std::map<string,Glx::FontCache*>::iterator fi = 
    Priv::fontMap.find(Priv::curFontKey);
  assert( fi != Priv::fontMap.end());
  fcache = (*fi).second;
  VARHEX(fcache);
  VARHEX(fcache->font);
  MESG("calling XftTextExtents8()");

  XftTextExtents8(getDisplay(),fcache->font,
		  (const FcChar8*)mesg.c_str(),mesg.length(),&extents);
  MESG("OK");

  if( stipple ){
    int pad=2;
    int yadj = extents.y-extents.height;
    int lox=x-pad, hix=x+extents.width+pad;
    int loy=y+yadj-pad, hiy=y+extents.height+yadj+pad;
    
    glPushAttrib(GL_DEPTH_BUFFER_BIT |GL_LIGHTING_BIT |GL_ENABLE_BIT);
    GLint mode;
    glGetIntegerv(GL_MATRIX_MODE,&mode);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0,w,0,h,-1.0,1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glColor3f(0,0,0);
    glEnable(GL_POLYGON_STIPPLE);
    glPolygonStipple(&Glx::stip[0]);
    glBegin(GL_QUADS);
    glVertex2f(lox,loy);
    glVertex2f(hix,loy);
    glVertex2f(hix,hiy);
    glVertex2f(lox,hiy);
    glEnd();
    glDisable(GL_POLYGON_STIPPLE);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(mode);
    glPopAttrib();
  }
  fcache->setColor(itsMessageColor);
  fcache->draw(x,y,mesg);
  FANCYMESG("glx::showMessage(int,int,string,bool) [XFT] [done]");
}

std::ostream& operator<<(std::ostream& ostr, XGlyphInfo& xgi)
{
  return 
    ostr<<"===XGlyphInfo===\n"
	<<"\twidth="<<xgi.width<<"\n"
	<<"\theight="<<xgi.height<<"\n"
	<<"\tx="<<xgi.x<<"\n"
	<<"\ty="<<xgi.y<<"\n"
	<<"\txOff="<<xgi.xOff<<"\n"
	<<"\tyOff="<<xgi.yOff<<"\n";
}

void glx::showMessage(string mesg,bool stipple)
{
  FANCYMESG("showMessage(string,bool)");
  VAR2(mesg,stipple);
  VAR(Priv::curFontKey);

  std::map<std::string,XGlyphInfo*>::iterator i = 
    Priv::extentsMap.find(Priv::curFontKey);

  assert(i!=Priv::extentsMap.end());

  XGlyphInfo* xgl = (*i).second;
  VAR(*xgl);

  XGlyphInfo* extents = (*i).second;
  int deltaY = extents->height;

  VAR(deltaY);
  VAR2V(itsCurFontLoc);

  showMessage(itsCurFontLoc[0],itsCurFontLoc[1],mesg,stipple);
  advanceFontPos();
}

void glx::showLabel(void)
{
  std::vector<string> labelVec;
  std::map<std::string,XGlyphInfo*>::iterator i=
    Priv::extentsMap.find(Priv::labelFontKey);
  assert(i!=Priv::extentsMap.end());
  XGlyphInfo* extents = (*i).second;

  int x=10;
  int deltaY = extents->height;

  breakIntoLines(itsLabel,labelVec);  

  string saveFont=Priv::curFontKey;
  Priv::curFontKey=Priv::labelFontKey;

  int y=10+labelVec.size() * extents->height;
  for(int line=0;line<labelVec.size();++line){
    showMessage(x, y, labelVec[line], true);
    y -= deltaY;
  }
  Priv::curFontKey=saveFont;  
}

void glx::setLabelFont(std::string face, double size)
{
  Priv::setLabelFont(itsDisplay,face,size);
}
void glx::setMessageFont(std::string face, double size)
{
  Priv::setMessageFont(itsDisplay,face,size);
}
void glx::setTitleFont(std::string face, double size)
{
  Priv::setTitleFont(itsDisplay,face,size);
}

//////////////////////////////////////////////////////////////////////////
#else // NOT FREETYPE
//////////////////////////////////////////////////////////////////////////

void Priv::fontCreateBitmap(unsigned int fontBase)
{
  int i;

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  for (i = 0;Glx::bitmapFont[i][0] != (unsigned char)Glx::END_OF_LIST; i++) {
    glNewList(fontBase+(unsigned int)Glx::bitmapFont[i][0], GL_COMPILE);
    glBitmap(8, 13, 0.0, 2.0, 10.0, 0.0, &Glx::bitmapFont[i][1]);
    glEndList();
  }
}

void Priv::fontDrawStr(unsigned int base, std::string str)
{
  glPushAttrib(GL_LIST_BIT);
  glListBase(base);
  glCallLists(str.length(),GL_UNSIGNED_BYTE,(unsigned char *)str.c_str());
  glPopAttrib();
}

void glx::resetFontPos(void)
{
  itsCurFontLoc[0] = 10;
  itsCurFontLoc[1] = winHeight() - 10 - Glx::FONT_SIZE;
}

void glx::advanceFontPos(void)
{
  itsCurFontLoc[1] -= Glx::FONT_SIZE;
}

void glx::showMessage(int x, int y, std::string mesg, bool stip)
{
  FANCYMESG("glx::showMessage(int,int,string,bool)");
  if( mesg.length() == 0) return;

  int len = mesg.length();
  int w = winWidth();
  int h = winHeight();

  glPushAttrib(GL_DEPTH_BUFFER_BIT|
	       GL_LIGHTING_BIT|
	       GL_POLYGON_STIPPLE_BIT|
	       GL_ENABLE_BIT);

  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_TEXTURE_1D);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_3D);

  GLfloat projectionMatrix[4][4];
  glGetFloatv(GL_PROJECTION_MATRIX,(GLfloat *)projectionMatrix);
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0,w,0,h,-1.0,1.0);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  if( stip ){
    int charW=10,charH=12;
    int slen = charW*len+2;
    glColor3f(0,0,0);
    glEnable(GL_POLYGON_STIPPLE);
    glPolygonStipple(&Glx::stip[0]);
    glBegin(GL_QUADS);
    glVertex2f(x,y-2);
    glVertex2f(x+slen,y-2);
    glVertex2f(x+slen,y+charH);
    glVertex2f(x,y+charH);
    glEnd();
    glDisable(GL_POLYGON_STIPPLE);
  }

  glColor3f(itsMessageColor[0],itsMessageColor[1],itsMessageColor[2]);
  glRasterPos2f(x, y);
  Priv::fontDrawStr(Priv::itsFontBase,mesg);

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf((GLfloat *)projectionMatrix);
  glMatrixMode(GL_MODELVIEW);

  glPopAttrib();
}

void glx::showMessage(std::string mesg, bool stip)
{
  showMessage(itsCurFontLoc[0], itsCurFontLoc[1], mesg, stip);
  itsCurFontLoc[1] -= Glx::FONT_SIZE;
}

void glx::showLabel(void)
{
  int x=10;
  int deltaY = Glx::FONT_SIZE;
  std::vector<string> labelVec;

  breakIntoLines(itsLabel,labelVec);  

  int y = 10 + labelVec.size() * Glx::FONT_SIZE;
  for(int line=0;line<labelVec.size();++line){
    showMessage(x, y, labelVec[line], true);
    y -= deltaY;
  }

}

//////////////////////////////////////////////////////////////////////////
#endif  // NOT FREETYPE
//////////////////////////////////////////////////////////////////////////

void glx::showMessage(std::string mesg, float x, float y, float z, bool stip)
{
  double modelMatrix[16];
  double projMatrix[16];
  int viewport[4];
  double xproj,yproj,zproj;
  
  makeCurrent();
  glGetDoublev(GL_MODELVIEW_MATRIX,(double *)modelMatrix);
  glGetDoublev(GL_PROJECTION_MATRIX,(double *)projMatrix);
  glGetIntegerv(GL_VIEWPORT, viewport);

  project( x, y, z, modelMatrix, projMatrix, viewport,
	   &xproj,&yproj,&zproj);
  
  showMessage((int)xproj,(int)yproj,mesg,stip);
}

void 
glx::breakIntoLines(string input, vector<string>& res)
{
  int i=0,start=0,end=0;
  while( i<input.length() ){
    if( input[i]=='\\' && input[i+1]=='n' ){
      end=i;
      res.push_back( string( input, start, end-start) );
      i += 2;
      start=i;
    } else 
      ++i;
  }
  if( start!=end )
    res.push_back( string( input, start, end-start) );
  else if( start==0 && end==0 )
    res.push_back( input );    
}

void 
glx::setLabel(std::string label)
{
  itsLabel=label;
}
