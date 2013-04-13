#ifdef USEFREETYPE
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/extensions/shape.h>
#include <GL/glx.h>
#include <assert.h>
#include <X11/Xft/Xft.h>
#include FT_OUTLINE_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <glxFFont.h>
using namespace std;

#warning "compiling glxFFont"

//#define DEBUG 1
#include "debug.h"

//////////////////////////////////////////////////////////////////////////
////////////////////////// class Glx::FontCache //////////////////////////
//////////////////////////////////////////////////////////////////////////

Glx::FontCache::FontCache(Display* dpy, string face, double size) : 
  xdpy(dpy), font(0), fscale(1./60.)
{
  FANCYMESG("Glx::FontCache::FontCache()");
  VAR2(face,size);

  int scrn = DefaultScreen(xdpy);
  font=XftFontOpen(xdpy,scrn,
		   XFT_FAMILY,XftTypeString,face.c_str(),
		   XFT_SIZE,XftTypeDouble, size,NULL);  
  if( font==0 ){
    cerr << "Glx::FontCache: unable to open font: " << face <<"."<<endl;
    cerr << "              : Trying Times..."<<flush;
    font=XftFontOpen(xdpy,scrn,
		     XFT_FAMILY,XftTypeString,"Times-Roman",
		     XFT_SIZE,XftTypeDouble, size,NULL);  
    if( font==0 ){
      cerr << "failed. " << endl;
      cerr << "Glx::FontCache: unable to open Times-Roman." << endl;
      cerr << "              : Punting." << endl;
    } else {
      cerr << "OK!" <<endl;
    }
  }
  genGlyphs(font,glyphs);
}

Glx::FontCache::FontCache(Display* dpy, XftFont * f) :
  xdpy(dpy), font(f), fscale(1./60.)
{
  genGlyphs(font,glyphs);
}

Glx::FontCache::~FontCache(void)
{
  XftFontClose(xdpy,font);
  std::vector<Glx::Glyph>::iterator iter = glyphs.begin();
  for( ; iter != glyphs.end() ; ++iter ){
    Glx::Glyph& glyph = *iter;
    glDeleteTextures(1,(const GLuint*)&glyph.texID);
  }
}

void
Glx::FontCache::draw(int x, int y, string str)
{ 
  FANCYMESG("Glx::FontCache::draw");

  GLuint texID=0xffffffff;
  float xpos=0;
  FT_Face face = XftLockFace(font);
  FT_Error ferr;

  int vp[4],saveMatMode,saveTextureID,saveBlendSrc,saveBlendDst;
  GLboolean saveBlendEnabled;
  GLboolean saveTex2Enabled;
  glGetIntegerv(GL_VIEWPORT, vp);
  glGetIntegerv(GL_MATRIX_MODE,&saveMatMode);
  glGetBooleanv(GL_TEXTURE_2D,&saveTex2Enabled);
  glGetIntegerv(GL_TEXTURE_BINDING_2D,&saveTextureID);
  glGetBooleanv(GL_BLEND,&saveBlendEnabled);
  glGetIntegerv(GL_BLEND_SRC,&saveBlendSrc);
  glGetIntegerv(GL_BLEND_DST,&saveBlendDst);

  int Width=vp[2]-vp[0],Height=vp[3]-vp[1];

  glPushAttrib(GL_DEPTH_BUFFER_BIT | 
	       GL_LIGHTING_BIT |     
	       GL_COLOR_BUFFER_BIT | // GL_BLEND
	       GL_CURRENT_BIT);      // color

  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
  glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0,Width,0,Height,-1,1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glEnable(GL_TEXTURE_2D);
  glColor3fv(rgb);

  glTranslatef(x,y,0);
  xpos=0;

  VAR(str.length());
  VAR(glyphs.size());

  for( int i=0 ; i<str.length() ; ++i){
    char ch = str[i];
    Glx::Glyph& glyph = glyphs[ch];
    if( glyph.ch!=ch )continue;

    if( texID != glyph.texID ){
      glBindTexture( GL_TEXTURE_2D, glyph.texID);
      texID = glyph.texID;
    }

    glBegin(GL_QUADS);
    glTexCoord2f( glyph.slo, glyph.thi );
    glVertex2f( xpos + glyph.x,           glyph.y );
    
    glTexCoord2f( glyph.slo, glyph.tlo );
    glVertex2f( xpos + glyph.x,           glyph.y - glyph.h );
    
    glTexCoord2f( glyph.shi, glyph.tlo );
    glVertex2f( xpos + glyph.x + glyph.w, glyph.y - glyph.h );
    
    glTexCoord2f( glyph.shi, glyph.thi );
    glVertex2f( xpos + glyph.x + glyph.w, glyph.y);
    glEnd();
    if( i != str.length()-1 ){
      char cnext = str[i+1];
      Glx::Glyph gnext = glyphs[cnext];
      FT_Vector kernAdvance;
      ferr = FT_Get_Kerning(face, glyph.index, gnext.index, 
			    FT_KERNING_UNSCALED, &kernAdvance);
      if( ! ferr )
	xpos += (kernAdvance.x + glyph.dx)/64.; // note, diff than fscale
    }
  }
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib(); 

  glBlendFunc(saveBlendSrc,saveBlendDst);  
  glMatrixMode(saveMatMode);

  if( saveBlendEnabled )
    glEnable(GL_BLEND);
  else
    glDisable(GL_BLEND);

  if( saveTex2Enabled )
    glEnable(GL_TEXTURE_2D);
  else
    glDisable(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D,saveTextureID);
  FANCYMESG("Glx::FontCache::draw[done]");
}

void
Glx::FontCache::draw(int winw, int winh, std::vector<std::string>& lines)
{ 
  if( lines.empty() ) return;

  // calc the extents
  XGlyphInfo* extents = new XGlyphInfo[lines.size()];  
  memset(extents,0,lines.size()*sizeof(XGlyphInfo));  
  int textw=0,texth=0;
  vector<string>::iterator iter = lines.begin();
  for(int i=0 ; iter != lines.end() ; ++iter,++i ){
    const FcChar8* str = (const FcChar8*) (*iter).c_str();
    XftTextExtents8(xdpy,font,str,(*iter).length(),&extents[i]);
    texth += extents[i].height;
    if( extents[i].width > textw ) 
      textw=extents[i].width;    
  }

  int linepad = (unsigned int)((float)extents[0].height/1.);
  int hpad = (winh - texth)/2;

  // render the text, centered

  for(int i=0 ; i<lines.size() ; ++i ){   
    int xpos = (winw - extents[i].width)/2;
    int ypos = (int)(winh + extents[0].height/2. - 
		     (hpad + i * (extents[0].height + linepad)));
    draw(xpos, ypos, lines[i]);
  }

  delete [] extents;  
}

void 
Glx::FontCache::setColor(float r, float g, float b)
{
  rgb[0]=r;
  rgb[1]=g;
  rgb[2]=b;
}

void 
Glx::FontCache::setColor(float color[3])
{
  rgb[0]=color[0];
  rgb[1]=color[1];
  rgb[2]=color[2];
}

int 
Glx::FontCache::bigEnough(int val)
{
  // power of 2 big enough for a given size
  int res=1;
  while( res<val )
    res<<=1;
  return res;
}

void 
Glx::FontCache::allocTextures(FT_Face face,int loGlyph, int hiGlyph,
			      float glyphWidth, float glyphHeight, int pad,
			      vector<Glx::Glyph>& glyphs)
{
  int glyphsLeft = hiGlyph-loGlyph+1;
  int glyphBase  = loGlyph;
  unsigned char* tex;
  GLuint texID;
  int texW,texH;
  int glyphsPerRow,rowsNeeded,rowsInThisTexture,glyphsInThisTexture;
  bool done =false;
  GLint maxSize;
  float gw = glyphWidth+pad;
  float gh = glyphHeight+pad;
  FT_Error ferr;

  int saveTextureID;
  glGetIntegerv(GL_TEXTURE_BINDING_2D,&saveTextureID);
  glGetIntegerv( GL_MAX_TEXTURE_SIZE, (GLint*)&maxSize);

  while( glyphsLeft>0 ){

    texW = bigEnough((int)( (float)glyphsLeft * gw ));
    texW = texW>maxSize ? maxSize : texW;      
    glyphsPerRow = (int) ( (float)texW / gw );   
    rowsNeeded = (int)glyphsLeft/glyphsPerRow + 1;
    texH = bigEnough( (int)( (float)rowsNeeded * gh ));
    texH = texH>maxSize ? maxSize : texH;
    rowsInThisTexture = (int)( (float)texH / gh );

    glyphsInThisTexture = rowsInThisTexture * glyphsPerRow;
    glyphsInThisTexture = glyphsInThisTexture>glyphsLeft ? glyphsLeft :
      glyphsInThisTexture;

    tex = new unsigned char[texW*texH];
    assert(tex);
    memset(tex,0,texW*texH);

    glGenTextures( 1, (GLuint*)&texID);

    glBindTexture( GL_TEXTURE_2D, texID);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, texW, texH, 0, 
		  GL_ALPHA, GL_UNSIGNED_BYTE, tex);
    delete [] tex;

    for(int i=0 ; i<glyphsInThisTexture; ++i){
      FT_BBox bbox;
      char ch = (char)(loGlyph + i);
      Glx::Glyph& glyph = glyphs[ch];
      glyph.ch = ch;
      glyph.index = FT_Get_Char_Index (face, ch);
      ferr = FT_Load_Glyph(face, glyph.index, FT_LOAD_DEFAULT);
      FT_Outline_Get_CBox( &(face->glyph->outline), &bbox);      
      float dx = (fscale * (float)(bbox.xMax-bbox.xMin));
      float dy = (fscale * (float)(bbox.yMax-bbox.yMin));

      int row = i / glyphsPerRow;
      int col = i % glyphsPerRow;
      float xlo = pad + (float)col * gw;
      float ylo = pad + (float)row * gh;

      // offset, in pixels, into the texture image
      glyphs[glyphBase + i].xlo = (int)(xlo+0.5);
      glyphs[glyphBase + i].ylo = (int)(ylo+0.5);
      
      // texture coords for this glyph and texture
      // NOTE: Y coords are reversed
      glyphs[glyphBase + i].slo = (float)xlo/texW;
      glyphs[glyphBase + i].shi = (float)(xlo+dx)/texW;
      glyphs[glyphBase + i].tlo = (float)(ylo+dy)/texH; 
      glyphs[glyphBase + i].thi = (float)ylo/texH;
      glyphs[glyphBase + i].texID = texID;
      glyphs[glyphBase + i].texW = texW;
      glyphs[glyphBase + i].texH = texH;
    }

    glyphsLeft -= glyphsInThisTexture;
    glyphBase  += glyphsInThisTexture;
  }

  glBindTexture(GL_TEXTURE_2D,saveTextureID);
}

void
Glx::FontCache::genGlyphs(XftFont* font, vector<Glx::Glyph>& glyphs)
{
  FANCYMESG("Glx::FontCache::genGlyphs(XftFont*,vector<Glx::Glyph>&)");

  int loGlyph=32,hiGlyph=126,nGlyphs = hiGlyph-loGlyph+1;
  int maxSize;
  int pad = 3;
  FT_Error ferr;
  FT_Face face = XftLockFace(font);

  if( face->units_per_EM==0 ) {
    _MESG("ERROR: bad font, FT_Face::units_per_EM is ZERO!!!\n");	  
    return;
  }

  FT_Size size = face->size;
  FT_Size_Metrics metrics = size->metrics;
  float dy = face->bbox.yMax - face->bbox.yMin;
  float dx = face->bbox.xMax - face->bbox.xMin;
  float yscale = (float)metrics.y_ppem/face->units_per_EM;
  float xscale = (float)metrics.x_ppem/face->units_per_EM;
  float glyphHeight = dy * yscale;
  float glyphWidth = dx * xscale;
  float lox=1e6,hix=-1e6,deltax;
  float loy=1e6,hiy=-1e6,deltay;

  glyphs.resize(256);

  // Allocate enough textures for all the glyphs in this font
  // [i.e. they may not all fit in, say, a 4096x4096 texture].
  // Glyphs will be tiled with a small pad around each.
  // Texture ID and coords for each glyph will be saved.
  
  allocTextures(face, loGlyph, hiGlyph, glyphWidth, glyphHeight, pad, glyphs );

  int saveTextureID;
  glGetIntegerv(GL_TEXTURE_BINDING_2D,&saveTextureID);

  // for each glyph, use freetype to render the glyph, 
  // then copy the pixels into the texture

  for(unsigned char ch=loGlyph ; ch<=hiGlyph ; ++ch)
  {
    Glx::Glyph& glyph = glyphs[ch];
    ferr = FT_Load_Glyph(face, glyph.index, FT_LOAD_DEFAULT);
    if( ferr )
      continue;

    ferr = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
    if( ferr )
      continue;

    glyph.x  = face->glyph->bitmap_left;
    glyph.y  = face->glyph->bitmap_top;
    glyph.w  = face->glyph->bitmap.width;
    glyph.h  = face->glyph->bitmap.rows;
    glyph.dx = face->glyph->advance.x;
    glyph.dy = face->glyph->advance.y;

    if( glyph.w==0 || glyph.h==0 )
      continue;
    
    glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, glyph.texID);

    glTexSubImage2D(GL_TEXTURE_2D, 0, glyph.xlo, glyph.ylo, glyph.w,glyph.h,
		    GL_ALPHA, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

    glPopClientAttrib();
    
  }
  XftUnlockFace(font);
  glBindTexture(GL_TEXTURE_2D,saveTextureID);
}
#endif
