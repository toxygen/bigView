#ifndef GLX_FFONT_H
#define GLX_FFONT_H

#include <string>
#include <vector>
#include <map>
#include <X11/Xft/Xft.h>

namespace Glx {

  struct Glyph {
    char ch;
    int index;     // FT's index for this char
    int x,y;       // local offset for this glyph
    int w,h;       // size of the glyph square
    float slo,shi,tlo,thi; // tex coords
    int xlo,ylo;   // offset into texture, in pixels
    float dx,dy;   // dist to move after drawing, not including kerning
    int texID;     // texture this glyph lives in
    int texW,texH; // width of the texture this glyph lives in
  };

  // Cache the font glyphs [images of chars] into a texture
  // for faster rendering, uses Xft/Freetype2 lib

  class FontCache {
  public:
    FontCache(Display*, std::string face, double size);  
    FontCache(Display*, XftFont*);  
    ~FontCache(void);  

    void draw(int winw, int winh, std::vector<std::string>&);
    void draw(int x, int y, std::string);
    void setColor(float r, float g, float b);
    void setColor(float rgb[3]);

    void allocTextures(FT_Face,int,int,float,float,int,
		       std::vector<Glx::Glyph>&);    
    void genGlyphs(XftFont*, std::vector<Glx::Glyph>&);
    int bigEnough(int); 

    Display* xdpy;
    XftFont* font;

    std::vector<Glx::Glyph> glyphs;
    const float fscale;
    float rgb[3];
  };

};

#endif
