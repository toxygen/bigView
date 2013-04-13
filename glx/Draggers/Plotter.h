#ifndef GLX_PLOTTER_H
#define GLX_PLOTTER_H

#include <GLX.h>
#include <Draggers/Palette.h>
#include <vector>

namespace Glx {
  class PointPlotter : public Glx::Palette {
  public:
    enum {BORDER=Glx::Palette::HANDLESIZE};
    enum {MOVE=Glx::Palette::MOVE,
	  RESIZE=Glx::Palette::RESIZE,
	  DRAW,BOX,LASSO,FIRSTBIN=100};
    enum {BOXMODE,LASSOMODE};

    PointPlotter(glx* env, std::vector<double>*,std::vector<double>*,
		 int x=60, int y=60, int w=200, int h=200);    
    
    void setData(std::vector<double>*,std::vector<double>*);
    void setSelected( std::vector<bool>* );
    void drawDashed(float,float,float,float);

    void pix2real(float,float,float*,float*);
    void real2pix(float,float,float*,float*);

    void viewHasChanged(glx*);
    void draw(glx*,void *);
    int  idlePick(glx*,int,int,void*);
    void handleDrag(glx*,int,int,void*);
    void handleMouseUp(glx*,void*);
    void keyEvent(int, bool, bool, bool);
    void clear(void);
    void updateRanges(void);
    void updateWinvars(void);

    std::vector<double>* itsXdata;
    std::vector<double>* itsYdata;
    int itsLastXcoord,itsLastYcoord;
    std::vector<float> pntx,pnty;
    std::vector<bool>* selected;
    bool needclear;
    int N;
    int selectionMode;

    int minX,maxX,minY,maxY,plotW,plotH,envW,envH;
    double dmin[2],dmax[2],range[2];
    float startbox[2];
    float lobox[2];
    float hibox[2];
    bool invert;
  };

  class LinePlotter : public Glx::Palette {
  public:
    enum {BORDER=10};

    LinePlotter(glx* env, std::vector<float>&,
		int x=60, int y=60, int w=200, int h=200);    

    void draw(glx*,void *);
    int  idlePick(glx*,int,int,void*);
    
  protected:
    std::vector<float>& itsData;
    int itsLastXcoord,itsLastYcoord;
  };


  class MultiPlotter : public Glx::Palette {
  public:
    enum {BORDER=10};
    
    MultiPlotter(glx* env, 
		 std::vector<std::vector<float>*>&,
		 std::vector<std::string*>&,
		 int x=60, int y=60, int w=200, int h=200);
    
    void draw(glx*,void *);
    int  idlePick(glx*,int,int,void*);
    
    bool setZeroMin(bool v) { itsZeroMin=v; }
  protected:
    std::vector<std::vector<float>*> &itsDataSet;
    std::vector<std::string*> &itsNames;
    float itsLastXcoord,itsLastYcoord;
    bool itsZeroMin;
  };

} // namespace Glx

#endif
