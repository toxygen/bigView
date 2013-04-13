// Emacs mode -*-c++-*- //
///////////////////////////////////////////////////////////////////////////////
// Class Glx::Histogram
///////////////////////////////////////////////////////////////////////////////

#ifndef GLX_HISTOGRAM_H
#define GLX_HISTOGRAM_H

#include <Draggers/Palette.h>
#include <string>
#include <vector>

namespace Glx {
  
  //////////////////////////////////////////////////////////////////////////
  ////////////////////// vector<bool> helper methods ///////////////////////
  //////////////////////////////////////////////////////////////////////////

  bool any(std::vector<bool>& bits);
  void init(std::vector<bool>& bits, int size);
  void reset(std::vector<bool>& bits);
  void select(std::vector<bool>& bits, int index);
  void select(std::vector<bool>& bits, int lo, int hi);
  void getSelection(std::vector<bool>& bits, int* lo, int* hi);

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////// class Histogram /////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  /**
   * class Glx::Histogram
   *
   * An 2D dragger for selecting ranges from a histogrammed distribution
   */
  
  class Histogram : public Glx::Palette {
  public:
    enum {MOVE=Glx::Palette::MOVE,
	  RESIZE=Glx::Palette::RESIZE,
	  TRACKPAD,RESET,FIRSTBIN=100};

    /**
     * Glx::Histogram::CandidateFunc is a function prototype. 
     * User functions matching this format can be registered 
     * to be called by a Histogram dragger when the mouse
     * moves over a bin in order to show info about a bin
     */
    
    typedef void (*CandidateFunc)(glx*,Glx::Histogram*,int,void*);
    typedef std::pair<Glx::Histogram::CandidateFunc,void*> CandidatePair;

    /**
     * Glx::Histogram::SelectionFunc is a function prototype. 
     * User functions matching this format can be registered 
     * to be called by a Histogram dragger when the mouse
     * drags and releases over a range of bins
     */
    
    typedef void (*SelectionFunc)(glx*,Glx::Histogram*,int,int,void*);
    typedef std::pair<Glx::Histogram::SelectionFunc,void*> SelectionPair;

    /**
     * CTOR for class Glx::Histogram
     * @param env: glx class in which this will be used
     * @param bins: vector of counts [the histogram]
     * @param x: x location in pixels
     * @param y: y location in pixels
     * @param w: width in pixels
     * @param h: height in pixels
     */
    Histogram(glx* env, std::vector<u_int64_t>& bins,
			  int x=60, int y=60, int w=200, int h=200);

    /**
     * register a user method to be called when mouse moves over a bin
     */
    void addCandidateFunc(Glx::Histogram::CandidateFunc,void* =0);

    /**
     * register a user method to be called when a selection occurs
     */
    void addSelectionFunc(Glx::Histogram::SelectionFunc,void* =0);

    // concrete classes MUST fill all methods from here...

    /**
     * draw the dragger, called by glx
     */
    void draw(glx*,void *);

    /**
     * dragging has finished, called by glx
     */
    void handleMouseUp(glx*,void*);

    /**
     * dragging is occuring, called by glx
     */
    void handleDrag(glx*,int,int,void*);

    /**
     * compete for picking, called by glx
     */
    int  idlePick(glx*,int,int,void*);
    // ... to here

  protected:
    static void mouseDown(glx*, int x, int y, void* user );
    void resetXforms(void);
    int projW(void){return itsW-2*Glx::Palette::HANDLESIZE;}
    int projH(void){return itsH;}
    float aspect(void){return (float)projW()/projH();}
    int getSelected(int mouseX, int mouseY);

    void callCandidateFuncs(int candidate);
    void callSelectionFuncs(int lo, int hi);

    void pixelToWin(int x, double& xf);
    void winToPixel(double xf, int& x);

    std::vector< Glx::Histogram::CandidatePair > itsCandidateFuncs;
    std::vector< Glx::Histogram::SelectionPair > itsSelectionFuncs;

    std::vector<u_int64_t>& itsData;
    std::vector<bool> selectedBins;

    int selectionStart,lastSelectionLo,lastSelectionHi;
    int numBins;
    bool dragging;
    float startOffsetX,startScaleY;
    float saveOffset,saveScale;
    float itsOffset;

  }; // Class Histogram

  //////////////////////////////////////////////////////////////////////////
  ///////////////////////// class ScalarHistogram //////////////////////////
  //////////////////////////////////////////////////////////////////////////

  class ScalarHistogram : public Glx::Histogram {
  public:
    
    enum {MOVE=Glx::Histogram::MOVE,
	  RESIZE=Glx::Histogram::RESIZE,
	  TRACKPAD=Glx::Histogram::TRACKPAD,
	  RESET=Glx::Histogram::RESET,
	  LO,HI};
    enum{TOL=5};

    ScalarHistogram(glx* env, std::vector<u_int64_t>& bins,
		    double smin, double smax,
		    int x=60, int y=60, int w=200, int h=200);

    void draw(glx*,void *);
    int idlePick(glx*,int,int,void*);
    void handleDrag(glx*,int,int,void*);
    void handleMouseUp(glx*,void*);

    void update(double smin, double smax); // new min/max for thumbs
    double thumb(int i){return (i==0||i==1) ? (* thumbs[i].itsValue) : 0.;}

	//protected:
    double scalarMin, scalarMax;
    double lastX; // used for displaying scalar value under mouse
    double lo,hi;

    struct Thumb {
      enum {WIDTH=10,THICKNESS=20};

      ScalarHistogram* itsParent;
      int itsID;
      double *itsValue;

      int  idlePick(glx*,int,int,void*);
      void draw(glx*,void *);

      void calcXDX(double&,double&);
      void clamp(int& val, int min,int max){
	if(val<min)val=min; 
	if(val>max)val=max;
      }
    };
    Glx::ScalarHistogram::Thumb thumbs[2];

  }; // class ScalarHistogram

} // namespace Glx
#endif
