#ifndef GLX_COLORMAP_H
#define GLX_COLORMAP_H

#include <string>
#include <vector>
#include <map>
#include <GL/gl.h>
#include <GL/glx.h>

#define GLERROR(errVar,mesg) {                                          \
  errVar = glGetError();                                                \
  switch( errVar){                                                      \
    case GL_NO_ERROR:                                                   \
      break;                                                            \
    case GL_INVALID_ENUM:                                               \
      std::cout << ":ERROR IN " << __FILE__ << ":line# " << __LINE__ << ":"; \
      std::cout << "glError() = ";                                      \
      std::cout << "GL_INVALID_ENUM" << std::endl;                      \
      std::cout << "GlError(" << errVar << "):" << mesg << std::endl;   \
      std::cout << std::flush; \
      break;                                                            \
    case GL_INVALID_VALUE:                                              \
      std::cout << ":ERROR IN " << __FILE__ << ":line# " << __LINE__ << ":"; \
      std::cout << "glError() = ";                                      \
      std::cout << "GL_INVALID_VALUE" << std::endl;                     \
      std::cout << "GlError(" << errVar << "):" << mesg << std::endl;   \
      std::cout << std::flush; \
      break;                                                            \
    case GL_INVALID_OPERATION:                                          \
      std::cout << ":ERROR IN " << __FILE__ << ":line# " << __LINE__ << ":"; \
      std::cout << "glError() = ";                                      \
      std::cout << "GL_INVALID_OPERATION" << std::endl;                 \
      std::cout << "GlError(" << errVar << "):" << mesg << std::endl;   \
      std::cout << std::flush; \
      break;                                                            \
    case GL_STACK_OVERFLOW:                                             \
      std::cout << ":ERROR IN " << __FILE__ << ":line# " << __LINE__ << ":"; \
      std::cout << "glError() = ";                                      \
      std::cout << "GL_STACK_OVERFLOW"    << std::endl;                 \
      std::cout << "GlError(" << errVar << "):" << mesg << std::endl;   \
      std::cout << std::flush; \
      break;                                                            \
    case GL_STACK_UNDERFLOW:                                            \
      std::cout << ":ERROR IN " << __FILE__ << ":line# " << __LINE__ << ":"; \
      std::cout << "glError() = ";                                      \
      std::cout << "GL_STACK_UNDERFLOW"   << std::endl;                 \
      std::cout << "GlError(" << errVar << "):" << mesg << std::endl;   \
      std::cout << std::flush; \
      break;                                                            \
    case GL_OUT_OF_MEMORY:                                              \
      std::cout << ":ERROR IN " << __FILE__ << ":line# " << __LINE__ << ":"; \
      std::cout << "glError() = ";                                      \
      std::cout << "GL_OUT_OF_MEMORY"     << std::endl;                 \
      std::cout << "GlError(" << errVar << "):" << mesg << std::endl;   \
      std::cout << std::flush; \
      break;                                                            \
    default:                                                            \
      std::cout << "Unknown OpenGL error : "<<errVar<<std::endl;        \
      std::cout << std::flush; \
      break;                                                            \
  }                                                                     \
}

namespace Glx {

  class Colormap {
  public:
    enum {GREY,HEAT,FAST,PLOT3D,TERRAIN,DIFF,CREON,CREONREV,STEIN,STEIN2};
    enum {NONE,STEP,SAW,NOTCHED,FUZZY,PEAKY,HALFFUZZY,HALFPEAKY,VENETIAN};

    Colormap(int numEntries=256);
    Colormap(std::string filename);

    virtual ~Colormap(void);
    
    void setTexEnv(GLenum);
    void setTexFilter(GLenum);

    void setColormap(unsigned map);
    void setContouring(bool enabled){itsContouringFlag=enabled;}
    void setContourCount(int count){itsContourCount=count;}
    void setContourType(int type){itsContourType=type;}

    void setMinmax(double* min, double* max);
    void setMinmax(double min, double max);
    void getMinmax(double* min, double* max){
      *min = *usersMin;
      *max = *usersMax;
    }
    
    void calcNormRgb(const float percent, float rgb[3]);
    void calcUNormRgb(const float percent, float rgb[3]);
    void calcUNormRgb(const float percent, float rgb[3],
		       float min, float max);

    void calcNormRgba(const float percent, float rgb[4]);
    void calcUNormRgba(const float percent, float rgb[4]); // call setMinMax
    void calcUNormRgba(const float percent, float rgb[4],
		       float min, float max);

    inline void scalar(float percent){
      glTexCoord1f(percent);
    }
    inline void scalar(double percent){
      glTexCoord1d(percent);
    }

    void preRender(void);
    void postRender(void);
    void beginFill(void);
    void endFill(int callUsers=1);

    typedef void (*Listener)(Colormap&,void*);
    typedef std::pair<Colormap::Listener,void*> ListenerPair;
    typedef std::vector<Colormap::ListenerPair*>::iterator ListenerIter;

    void addListener(Colormap::Listener listener, void* userData);
    void callListeners();

    float* operator[](int index);

    int  getSize(void){return itsSize;}
    void setSize(int newSize);

    void update(void);

    static void genGrey(float percent, float rgb[4]);
    static void genBlackBody(float percent, float rgb[4]);
    static void genFast(float percent, float rgb[4]);
    static void genPlot3D(float percent, float rgb[4]);
    static void genTerrain(float percent, float rgb[4]);
    static void genDiff(float percent, float rgb[4]);
    static void genCreon(float percent, float rgb[4]);
    static void genReverseCreon(float percent, float rgb[4]);
    static void genStein(float percent, float rgb[4]);
    static void genStein2(float percent, float rgb[4]);

    GLuint getID(GLXContext ctx) {
      GLuint res=0;
      std::map<GLXContext,GLuint>::iterator i=itsTextureIDs.begin();  
      for( ; i != itsTextureIDs.end() && res==0 ; i++)
	if( (*i).first == ctx )
	  res = (*i).second;
      return res;
    }

    //protected:
    float* itsCmap; 
    int itsSize;
    double itsDefMin; // default min
    double itsDefMax; // default max
    double* usersMin; // ptr to min scalar value
    double* usersMax; // ptr to max scalar value
    std::vector< Colormap::ListenerPair* > itsListeners;
    std::map<GLXContext,GLuint> itsTextureIDs;
    bool itsContouringFlag;
    int  itsContourCount;
    int  itsContourType;

    float saveTexMat[16];
    GLint saveMatMode;
    GLboolean saveBlendEnabled;
    GLboolean saveTex1Enabled;
    GLint saveTextureID;
    GLint saveBlendSrc;
    GLint saveBlendDst;
    GLenum itsEnvVal;
    GLenum itsFilterVal;
  };

} // namespace Glx

#endif
