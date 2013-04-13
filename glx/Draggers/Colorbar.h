// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Class Glx::Colorbar
///////////////////////////////////////////////////////////////////////////////

#ifndef GLX_COLORBAR_H
#define GLX_COLORBAR_H

#include <string>
#include <vector>
#include <GLX.h>
#include <glxColormap.h>
#include <Draggers/Palette.h>

namespace Glx {
  class Colorbar : public Glx::Palette {
  public:
    Colorbar(glx* env,Colormap* cmap,int x=60,int y=60,int w=30,int h=200);
    void draw(glx*,void *);
    void setFormat(std::string str){itsFormat=str;itsEnv->wakeup();}
    void setTitle(std::string str){itsTitle=str;itsEnv->wakeup();}
    void setNumLabels(int num){itsNumLabels=num;itsEnv->wakeup();}   
    void setColormap(Colormap* cmap){itsCmap=cmap;itsEnv->wakeup();} 
    void setScale(float scale){itsScale=scale;itsEnv->wakeup();}
    void setOffset(float offset){itsOffset=offset;itsEnv->wakeup();}
    void setStipple(bool enabled){itsUseStippleFlag=enabled;itsEnv->wakeup();}

  protected:
    int itsNumLabels;
    std::string itsFormat;
    std::string itsTitle;
    Glx::Colormap* itsCmap;
    double itsScale;
    double itsOffset;
    bool itsUseStippleFlag;
  }; // class Glx::Colorbar
};

#endif
