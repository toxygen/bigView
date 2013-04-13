#include <iostream>
#include <fstream>
#include <math.h>
#include <assert.h>
#include "glxColormap.h"

//#define DEBUG 1
#include "debug.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////
////////////////////////// class Glx::Colormap ///////////////////////////
//////////////////////////////////////////////////////////////////////////

namespace Glx {

  Colormap::Colormap(int numEntries) : 
    itsCmap(0),
    itsSize(numEntries),
    itsDefMin(0.0f),itsDefMax(1.0f),
    usersMin(0),usersMax(0),
    itsContouringFlag(false),itsContourCount(16),itsContourType(NONE),
    //itsEnvVal(GL_MODULATE),itsFilterVal(GL_LINEAR),
    itsEnvVal(GL_MODULATE),itsFilterVal(GL_NEAREST)
  {
    FANCYMESG("Colormap::Colormap(int)");
    VAR(numEntries);
    usersMin = &itsDefMin;
    usersMax = &itsDefMax;
    setSize(itsSize);
  }

  Colormap::Colormap(string filename) : 
    itsCmap(0),
    itsDefMin(0.0f),itsDefMax(1.0f),
    usersMin(0),usersMax(0),
    itsContouringFlag(false),itsContourCount(16),itsContourType(NONE),
    //itsEnvVal(GL_MODULATE),itsFilterVal(GL_LINEAR),
    itsEnvVal(GL_MODULATE),itsFilterVal(GL_NEAREST)
  {
    FANCYMESG("Colormap::Colormap(string)");
    usersMin = &itsDefMin;
    usersMax = &itsDefMax;
    ifstream in(filename.c_str());
    if( ! in ){
      cerr << "Error opening file: " << filename << endl;
      cerr << "Generating default colormap" << endl;
      itsSize=256;
      setSize(itsSize);
      return;
    }
    string op;
    in >> op >> itsSize;
    if( op.compare("NUM_CMAP_ENTRIES") ){
      cerr << "Expected: NUM_CMAP_ENTRIES <int>" << endl;
      cerr << "Generating default colormap" << endl;
      itsSize=256;
      setSize(itsSize);
      return;
    }
    itsCmap = new float[itsSize*4];

    for(int i=0;i<itsSize;++i){
      int index = i * 4;
      in >> itsCmap[i*4+0] 
	 >> itsCmap[i*4+1]  
	 >> itsCmap[i*4+2]  
	 >> itsCmap[i*4+3];
    }
    /*
    for(int i=0;i<itsSize;++i){
      cout << "["<<i<<"]:"
	   << itsCmap[i*4+0]<<","
	   << itsCmap[i*4+1]<<","
	   << itsCmap[i*4+2]<<","
	   << itsCmap[i*4+3]<<endl;
    }
    */
    MESG("Colormap::Colormap(): calling update");
    update();
    in.close();
  }

  Colormap::~Colormap(void)
  {
    FANCYMESG("Colormap::~Colormap()");
    ListenerIter iter = itsListeners.begin();
    for( ; iter != itsListeners.end() ; iter++ )
      delete *iter;
    itsListeners.clear();

    usersMin=0;
    usersMax=0;

    std::map<GLXContext,GLuint>::iterator texIter = itsTextureIDs.begin();  
    for( ; texIter != itsTextureIDs.end() ; texIter++){
      GLuint texID = (*texIter).second;
      glDeleteTextures(1,&texID);
      assert( glGetError()==GL_NO_ERROR );
    }

    delete [] itsCmap;
  }

  void 
  Colormap::calcNormRgb(const float percent, float rgb[3])
  {
    int index = (int)(percent * (itsSize-1));
    rgb[0] = itsCmap[index*4+0];
    rgb[1] = itsCmap[index*4+1];
    rgb[2] = itsCmap[index*4+2];
  }

  void 
  Colormap::calcUNormRgb(const float percent, float rgb[3])
  {
    float t = (percent-*usersMin)/(*usersMax-*usersMin);
    int index = (int)(t * (itsSize-1));
    rgb[0] = itsCmap[index*4+0];
    rgb[1] = itsCmap[index*4+1];
    rgb[2] = itsCmap[index*4+2];
  }

  void 
  Colormap::calcUNormRgb(const float percent, float rgb[3],
			 float min, float max)
  {
    float t = (percent-min)/(max-min);
    int index = (int)(t * (itsSize-1));
    rgb[0] = itsCmap[index*4+0];
    rgb[1] = itsCmap[index*4+1];
    rgb[2] = itsCmap[index*4+2];
  }

  void 
  Colormap::calcNormRgba(const float percent, float rgb[4])
  {
    int index = (int)(percent * (itsSize-1));
    rgb[0] = itsCmap[index*4+0];
    rgb[1] = itsCmap[index*4+1];
    rgb[2] = itsCmap[index*4+2];
    rgb[3] = itsCmap[index*4+3];
  }

  void 
  Colormap::calcUNormRgba(const float percent, float rgb[4],
			  float min, float max)
  {
    float t = (percent-min)/(max-min);
    int index = (int)(t * (itsSize-1));
    rgb[0] = itsCmap[index*4+0];
    rgb[1] = itsCmap[index*4+1];
    rgb[2] = itsCmap[index*4+2];
    rgb[3] = itsCmap[index*4+3];
  }

  void 
  Colormap::calcUNormRgba(const float percent, float rgb[4])
  {
    float t = (percent-*usersMin)/(*usersMax-*usersMin);
    int index = (int)(t * (itsSize-1));
    rgb[0] = itsCmap[index*4+0];
    rgb[1] = itsCmap[index*4+1];
    rgb[2] = itsCmap[index*4+2];
    rgb[3] = itsCmap[index*4+3];
  }

  void 
  Colormap::beginFill(void)
  {
    FANCYMESG("Colormap::beginFill()");
  }

  void 
  Colormap::endFill(int callUsers)
  {
    FANCYMESG("Colormap::endFill()");

    int target = itsSize/itsContourCount;

    for(int i=0;itsContouringFlag && itsContourType!=NONE && i<itsSize;i++){
      double t,whole,frac,percent = (float)i/(float)(itsSize-1); 
      switch( itsContourType ){
      
	case STEP:
	  frac = modf(percent*itsContourCount, &whole);
	  if(frac<0.5){
	    itsCmap[i*4+0] *= 0.85;
	    itsCmap[i*4+1] *= 0.85;
	    itsCmap[i*4+2] *= 0.85;
	  }
	  break;
	case SAW:
	  t = percent * itsContourCount;
	  frac = 0.85 + 0.15 * modf(t, &whole);
	  itsCmap[i*4+0] *= frac;
	  itsCmap[i*4+1] *= frac;
	  itsCmap[i*4+2] *= frac;
	  break;
	case NOTCHED:
	  if( target==0 ) break;
	  if( (i % target)==0 ){
	    itsCmap[i*4+0] = 0.;
	    itsCmap[i*4+1] = 0.;
	    itsCmap[i*4+2] = 0.;
	  }
	  break;
	case FUZZY:
	  t = percent * itsContourCount;
	  frac = 0.85 + 0.15 * sin(M_PI*modf(t, &whole));
	  itsCmap[i*4+0] *= frac;
	  itsCmap[i*4+1] *= frac;
	  itsCmap[i*4+2] *= frac;
	  break;
	case PEAKY:
	  t = percent * itsContourCount;
	  frac = 0.85 + 0.15 * (1.0-sin(M_PI*modf(t, &whole)));
	  itsCmap[i*4+0] *= frac;
	  itsCmap[i*4+1] *= frac;
	  itsCmap[i*4+2] *= frac;
	  break;

	case HALFFUZZY:
	  t = percent * itsContourCount;
	  frac = 0.85 + 0.15 * sin(M_PI/2*modf(t, &whole));
	  itsCmap[i*4+0] *= frac;
	  itsCmap[i*4+1] *= frac;
	  itsCmap[i*4+2] *= frac;
	  break;
	case HALFPEAKY:
	  t = percent * itsContourCount;
	  frac = 0.85 + 0.15 * (1.0-sin(M_PI/2*modf(t, &whole)));
	  itsCmap[i*4+0] *= frac;
	  itsCmap[i*4+1] *= frac;
	  itsCmap[i*4+2] *= frac;
	  break;
	case VENETIAN:
	  t = percent * itsContourCount;
	  frac = 0.85 + 0.15 * cos(M_PI*modf(t, &whole));
	  itsCmap[i*4+0] *= frac;
	  itsCmap[i*4+1] *= frac;
	  itsCmap[i*4+2] *= frac;
	  break;
	default:
	  break;
      }
    }

    MESG("Colormap::endFill(): calling update");
    update();
    if( callUsers )
      callListeners();
  }

  void 
  Colormap::preRender(void)
  {
    FANCYMESG("Colormap::preRender()");

    GLXContext ctx = glXGetCurrentContext();

    if( ctx == NULL ){
      MESG("Colormap::preRender(): glXGetCurrentContext() returned NULL!");
      MESG("Colormap::preRender(): texture NOT bound");
      return;
    }

    // save state:
    // - matrix mode
    // - texture matrix
    // - blend mode
    // - blend func src&dst
    glGetFloatv(GL_TEXTURE_MATRIX,saveTexMat);
    glGetIntegerv(GL_MATRIX_MODE,&saveMatMode);
    glGetBooleanv(GL_BLEND,&saveBlendEnabled);
    glGetBooleanv(GL_TEXTURE_1D,&saveTex1Enabled);
    glGetIntegerv(GL_TEXTURE_BINDING_1D,&saveTextureID);
    glGetIntegerv(GL_BLEND_SRC,&saveBlendSrc);
    glGetIntegerv(GL_BLEND_DST,&saveBlendDst);

    map<GLXContext,GLuint>::iterator iter = itsTextureIDs.end();
  
    if( ! itsTextureIDs.empty() ){
      MESG("Some contexts exist");
      iter = itsTextureIDs.find(ctx);
    }
  
    if( iter == itsTextureIDs.end() ){
      MESG("First time we've seen this context");
      VARHEX(ctx);
      MESG("Colormap::preRender(): calling update");
      update();
      iter=itsTextureIDs.find(ctx);
      if(iter == itsTextureIDs.end())
	return;
    } else {
      MESG("We've seen this context before");
      VARHEX(ctx);    
    }
    GLuint id = (*iter).second;

    MESGVAR("about to call glBindTexture() with",id);

    glBindTexture(GL_TEXTURE_1D,id);
    GLenum err;
    GLERROR(err,"glBindTexture(GL_TEXTURE_1D,id)");
    assert( glGetError()==GL_NO_ERROR );

    // GL_INVALID_OPERATION is generated if texture has a
    // dimensionality which doesn't match that of target.

    // GL_INVALID_OPERATION is generated if glBindTexture is
    // executed between the execution of glBegin and the
    // corresponding execution of glEnd.

    glEnable(GL_TEXTURE_1D);
    assert( glGetError()==GL_NO_ERROR );

    // ok, nothing too fancy here
    // manipulate the texture matrix so that
    // users can send scalar values as the 
    // texture coordinate

    float s=1.0f,t=0.0f;
    float range = *usersMax - *usersMin;
    if( range!=0.0 ){
      s = 1.0f/range;
      t = -(*usersMin) * s;
      VAR(s);
      VAR(t);
    }
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glTranslatef(t,0,0);
    glScalef(s,1,1);
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    assert( glGetError()==GL_NO_ERROR );
  }  

  void 
  Colormap::postRender(void)
  {
    FANCYMESG("Colormap::postRender()");

    // restore state:
    glMatrixMode(GL_TEXTURE);
    glLoadMatrixf(saveTexMat);

    if( saveTex1Enabled )
      glEnable(GL_TEXTURE_1D);
    else
      glDisable(GL_TEXTURE_1D);

    glBindTexture(GL_TEXTURE_1D,saveTextureID);

    if( saveBlendEnabled )
      glEnable(GL_BLEND);
    else
      glDisable(GL_BLEND);

    glBlendFunc(saveBlendSrc,saveBlendDst);

    glMatrixMode(saveMatMode);
  } 

  void 
  Colormap::setMinmax(double* min, double* max)
  {
    FANCYMESG("Colormap::setMinmax(double*,double*)");
    VAR2(*min,*max);
    usersMin=min;
    usersMax=max;
  }

  void 
  Colormap::setMinmax(double min, double max)
  {
    FANCYMESG("Colormap::setMinmax(double,double)");
    VAR2(min,max);
    itsDefMin = min;
    itsDefMax = max;
    usersMin = &itsDefMin; 
    usersMax = &itsDefMax;
  }

  void 
  Colormap::addListener(Colormap::Listener listener, void* user)
  {
    FANCYMESG("Colormap::addListener(Colormap::Listener,void*)");
    Colormap::ListenerPair* res = 
      new Colormap::ListenerPair(listener,user);  
    itsListeners.push_back(res);
  }

  void 
  Colormap::callListeners(void)
  {
    FANCYMESG("Colormap::callListeners()");
    VAR(itsListeners.size());
    Colormap::ListenerIter iter = itsListeners.begin();
    for( ; iter != itsListeners.end() ; ++iter ){
      Colormap::ListenerPair* listenerPair = *iter;
      (* listenerPair->first)(*this, listenerPair->second);
    }
  }

  float* 
  Colormap::operator[](int index)
  {
    if( index<0 || index>=itsSize )
      return 0;
    return &itsCmap[index*4];
  }

  void 
  Colormap::setSize(int size)
  {
    FANCYMESG("Colormap::setSize(int)");
    itsSize = size;
    VAR(itsSize);
    if( itsCmap != 0 ){
      MESG("deleting previous cmap");
      VARHEX(itsCmap);
      delete [] itsCmap;
    }
    itsCmap = new float[itsSize*4];
    setColormap(FAST);
  }

  void 
  Colormap::setTexEnv(GLenum envVal)
  {
    FANCYMESG("Colormap::setTexEnv(GLenum)");
    itsEnvVal=envVal;
    //MESG("Colormap::setTexEnv(): calling update");
    //update();
  }

  void 
  Colormap::setTexFilter(GLenum filterVal)
  {
    FANCYMESG("Colormap::setTexFilter(GLenum)");
    itsFilterVal=filterVal;
    //MESG("Colormap::setTexFilter(): calling update");
    //update();
  }

  void 
  Colormap::update(void)
  {
    FANCYMESG("Colormap::update()");

    GLuint texID;

    GLXContext ctx = glXGetCurrentContext();
    if( ctx == NULL )
      return;

    map<GLXContext,GLuint>::iterator iter = itsTextureIDs.end();
    if( ! itsTextureIDs.empty() ){
      MESG("Some context's exist");
      iter = itsTextureIDs.find(ctx);
    }
    if( iter == itsTextureIDs.end() ){
      MESG("First time we've seen this context");
      VARHEX(ctx);
      MESG("calling glGenTextures");
      glGenTextures(1,&texID);
      MESGVAR("Result of glGenTextures",texID);
      assert( glGetError()==GL_NO_ERROR );
      itsTextureIDs[ctx]=texID;
    } else {
      MESG("We've seen this context before");
      VARHEX(ctx);
      texID = (*iter).second;
    }

    MESGVAR("calling glBindTexture",texID);
    glBindTexture(GL_TEXTURE_1D,texID);
    assert( glGetError()==GL_NO_ERROR );
    glTexParameterf(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_1D,GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexParameterf(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER, itsFilterVal);
    glTexParameterf(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER, itsFilterVal);

    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,itsEnvVal);

    assert( glGetError()==GL_NO_ERROR );
    MESG("Colormap::update(): loading texture");
    VAR(itsSize);
    glTexImage1D(GL_TEXTURE_1D, // target
		 0,             // level of detail 0=base image level
		 GL_RGBA,       // internal format
		 itsSize,       // width
		 0,             // border
		 GL_RGBA,       // format
		 GL_FLOAT,      // type
		 itsCmap        // pixels
		 );
    assert( glGetError()==GL_NO_ERROR );
  }

  void 
  Colormap::setColormap(unsigned cmap)
  {
    FANCYMESG("Colormap::setColormap");
    VAR(cmap);

    beginFill();
    for(int i = 0 ; i < itsSize ; i++){
      float percent = (float)i/(float)(itsSize-1);    
      switch(cmap){
	case Colormap::GREY:
	  genGrey(percent,&itsCmap[i*4]);
	  break;
	case Colormap::HEAT:
	  genBlackBody(percent,&itsCmap[i*4]);
	  break;
	case Colormap::FAST:
	  genFast(percent,&itsCmap[i*4]);
	  break;
	case Colormap::PLOT3D:
	  genPlot3D(percent,&itsCmap[i*4]);
	  break;
	case Colormap::TERRAIN:
	  genTerrain(percent,&itsCmap[i*4]);
	  break;
	case Colormap::DIFF:
	  genDiff(percent,&itsCmap[i*4]);
	  break;
	case Colormap::CREON:
	  genCreon(percent,&itsCmap[i*4]);
	  break;
	case Colormap::CREONREV:
	  genReverseCreon(percent,&itsCmap[i*4]);
	  break;
	case Colormap::STEIN:
	  genStein(percent,&itsCmap[i*4]);
	  break;
	case Colormap::STEIN2:
	  genStein2(percent,&itsCmap[i*4]);
	  break;
      } 
    }
    endFill();
  }

  void 
  Colormap::genGrey(float percent, float rgb[4])
  {
    rgb[0]=percent;
    rgb[1]=percent;
    rgb[2]=percent;
    rgb[3]=1.0f;
  }

  void 
  Colormap::genBlackBody(float percent, float rgb[4])
  {
    const float LVL_0 = 0;
    const float LVL_1 = 0.556863;
    const float LVL_2 = 0.929412;

    if( percent <= LVL_1 ){
      float t = (percent-LVL_0)/0.556863;
      if( t<0 ) t = 0.0;
      if( t>1 ) t = 1.0;
      rgb[0] = 0 + t * 1;
      rgb[1] = 0 + t * 0.0901961;
      rgb[2] = 0;
    } else if( percent <= LVL_2 ){
      float t = (percent-LVL_1)/0.372549;
      if( t<0 ) t = 0.0;
      if( t>1 ) t = 1.0;
      rgb[0] = 1;
      rgb[1] = 0.0901961 + t * 0.831373;
      rgb[2] = 0;
    } else {
      float t = (percent-LVL_2)/0.0627451;
      if( t<0 ) t = 0.0;
      if( t>1 ) t = 1.0;
      rgb[0] = 1;
      rgb[1] = 0.921569 + t * 0.0784314;
      rgb[2] = 0 + t * 1;
    }
    rgb[3] = 1.0f;
  }

  void 
  Colormap::genFast(float percent, float rgb[4])
  {
    const float LVL_0 = 0;
    const float LVL_1 = 0.141176;
    const float LVL_2 = 0.282353;
    const float LVL_3 = 0.427451;
    const float LVL_4 = 0.568627;
    const float LVL_5 = 0.713726;
    const float LVL_6 = 0.854902;

    if( percent <= LVL_1 ){
      float t = (percent-LVL_0)/0.141176;
      if( t<0 ) t = 0.0;
      if( t>1 ) t = 1.0;
      rgb[0] = 0;
      rgb[1] = 0;
      rgb[2] = 0 + t * 1;
    } else if( percent <= LVL_2 ){
      float t = (percent-LVL_1)/0.141176;
      if( t<0 ) t = 0.0;
      if( t>1 ) t = 1.0;
      rgb[0] = 0;
      rgb[1] = 0 + t * 1;
      rgb[2] = 1;
    } else if( percent <= LVL_3 ){
      float t = (percent-LVL_2)/0.145098;
      if( t<0 ) t = 0.0;
      if( t>1 ) t = 1.0;
      rgb[0] = 0;
      rgb[1] = 1;
      rgb[2] = 1 + t * -1;
    } else if( percent <= LVL_4 ){
      float t = (percent-LVL_3)/0.141176;
      if( t<0 ) t = 0.0;
      if( t>1 ) t = 1.0;
      rgb[0] = 0 + t * 1;
      rgb[1] = 1;
      rgb[2] = 0;
    } else if( percent <= LVL_5 ){
      float t = (percent-LVL_4)/0.145098;
      if( t<0 ) t = 0.0;
      if( t>1 ) t = 1.0;
      rgb[0] = 1;
      rgb[1] = 1 + t * -1;
      rgb[2] = 0;
    } else if( percent <= LVL_6 ){
      float t = (percent-LVL_5)/0.141176;
      if( t<0 ) t = 0.0;
      if( t>1 ) t = 1.0;
      rgb[0] = 1;
      rgb[1] = 0;
      rgb[2] = 0 + t * 1;
    } else {
      float t = (percent-LVL_6)/0.145098;
      if( t<0 ) t = 0.0;
      if( t>1 ) t = 1.0;
      rgb[0] = 1;
      rgb[1] = 0 + t * 1;
      rgb[2] = 1;
    }
    rgb[3] = 1.0f;
  }

  void 
  Colormap::genPlot3D(float percent, float rgb[4])
  {
    const float LVL_0 = 0;
    const float LVL_1 = 0.2;
    const float LVL_2 = 0.4;
    const float LVL_3 = 0.6;
    const float LVL_4 = 0.8;

    if( percent <= LVL_1 ){
      float t = (percent-LVL_0)/0.2;
      if( t<0 ) t = 0.0;
      if( t>1 ) t = 1.0;
      rgb[0] = 0;
      rgb[1] = 0 + t * 1;
      rgb[2] = 1;
    } else if( percent <= LVL_2 ){
      float t = (percent-LVL_1)/0.2;
      if( t<0 ) t = 0.0;
      if( t>1 ) t = 1.0;
      rgb[0] = 0;
      rgb[1] = 1;
      rgb[2] = 1 + t * -1;
    } else if( percent <= LVL_3 ){
      float t = (percent-LVL_2)/0.2;
      if( t<0 ) t = 0.0;
      if( t>1 ) t = 1.0;
      rgb[0] = 0 + t * 1;
      rgb[1] = 1;
      rgb[2] = 0;
    } else if( percent <= LVL_4 ){
      float t = (percent-LVL_3)/0.2;
      if( t<0 ) t = 0.0;
      if( t>1 ) t = 1.0;
      rgb[0] = 1;
      rgb[1] = 1 + t * -1;
      rgb[2] = 0;
    } else {
      float t = (percent-LVL_4)/0.2;
      if( t<0 ) t = 0.0;
      if( t>1 ) t = 1.0;
      rgb[0] = 1;
      rgb[1] = 0;
      rgb[2] = 0 + t * 1;
    }
    rgb[3]=1.0f;
  }

  void 
  Colormap::genTerrain(float percent, float rgb[4])
  {
    const float LVL_0 = 0;
    const float LVL_1 = 0.333333;
    const float LVL_2 = 0.666667;

    if( percent <= LVL_1 ){
      float t = (percent-LVL_0)/0.333333;
      if( t<0 ) t = 0.0;
      if( t>1 ) t = 1.0;
      rgb[0] = 0.203922 + t * 0.407178;
      rgb[1] = 0.52549 + t * 0.08561;
      rgb[2] = 0.192157 + t * -0.114402;
    } else if( percent <= LVL_2 ){
      float t = (percent-LVL_1)/0.333333;
      if( t<0 ) t = 0.0;
      if( t>1 ) t = 1.0;
      rgb[0] = 0.6111 + t * 0.3579;
      rgb[1] = 0.6111 + t * 0.030475;
      rgb[2] = 0.0777549 + t * -0.0777549;
    } else {
      float t = (percent-LVL_2)/0.333333;
      if( t<0 ) t = 0.0;
      if( t>1 ) t = 1.0;
      rgb[0] = 0.969 + t * 0.031;
      rgb[1] = 0.641575 + t * 0.358425;
      rgb[2] = 0 + t * 1;
    }
    rgb[3] = 1.0f;
  }

  void 
  Colormap::genDiff(float percent, float rgb[4])
  {
    const float LVL_0 = 0;
    const float LVL_1 = 0.25;
    const float LVL_2 = 0.5;
    const float LVL_3 = 0.66666;
    const float LVL_4 = 0.83333;

    if( percent <= LVL_1 ){
      float t = (percent-LVL_0)/0.25;
      rgb[0] = 0;
      rgb[1] = 0;
      rgb[2] = 0.5 + t * 0.5;
    } else if( percent <= LVL_2 ){
      float t = (percent-LVL_1)/0.25;
      rgb[0] = 0 + t * 0.5;
      rgb[1] = 0 + t * 0.5;
      rgb[2] = 1 + t * -0.5;
    } else if( percent <= LVL_3 ){
      float t = (percent-LVL_2)/0.16666666;
      rgb[0] = 0.5 + t * 0.5;
      rgb[1] = 0.5 + t * -0.5;
      rgb[2] = 0.5 + t * -0.5;
    } else if( percent <= LVL_4 ){
      float t = (percent-LVL_3)/0.16666666;
      rgb[0] = 1;
      rgb[1] = 0 + t * 1;
      rgb[2] = 0;
    } else {
      float t = (percent-LVL_4)/0.16666666;
      rgb[0] = 1;
      rgb[1] = 1;
      rgb[2] = 0 + t * 1;
    }
    rgb[3] = 1.0f;
  }

  void 
  Colormap::genCreon(float percent, float rgb[4])
  {
    const float LVL_0 = 0;
    const float LVL_1 = 0.5;

    if( percent <= LVL_1 ){
      float t = (percent-LVL_0)/0.5;
      rgb[0] = 1 + t * -0.5;
      rgb[1] = 0 + t * 0.5;
      rgb[2] = 0 + t * 0.5;
    } else {
      float t = (percent-LVL_1)/0.5;
      rgb[0] = 0.5 + t * -0.5;
      rgb[1] = 0.5 + t * -0.5;
      rgb[2] = 0.5 + t * 0.5;
    }
  }
  void 
  Colormap::genReverseCreon(float percent, float rgb[4])
  {
    const float LVL_0 = 0;
    const float LVL_1 = 0.498039;
    const float LVL_2 = 1;
    if( percent <= LVL_1 ){
      float t = (percent-LVL_0)/0.498039;
      rgb[0] = 0 + t * 0.5;
      rgb[1] = 0 + t * 0.5;
      rgb[2] = 1 + t * -0.5;
    } else {
      float t = (percent-LVL_1)/0.501961;
      rgb[0] = 0.5 + t * 0.5;
      rgb[1] = 0.5 + t * -0.5;
      rgb[2] = 0.5 + t * -0.5;
    }
  }
  void 
  Colormap::genStein(float percent, float rgb[4])
  {
    const float LVL_0 = 0;
    const float LVL_1 = 0.49;
    const float LVL_2 = 0.50;
    const float LVL_3 = 0.51;
    const float LVL_4 = 1;
    if( percent <= LVL_1 ){
      float t = (percent-LVL_0)/(LVL_1-LVL_0);
      rgb[0] = 1;
      rgb[1] = 0 + t * 0.470588;
      rgb[2] = 0 + t * 0.470588;
    } else if( percent <= LVL_2 ){
      float t = (percent-LVL_1)/(LVL_2-LVL_1);
      rgb[0] = 1 + t * -1;
      rgb[1] = 0.470588 + t * -0.470588;
      rgb[2] = 0.470588 + t * -0.470588;
    } else if( percent <= LVL_3 ){
      float t = (percent-LVL_2)/(LVL_3-LVL_2);
      rgb[0] = 0 + t * 0.470588;
      rgb[1] = 0 + t * 0.462745;
      rgb[2] = 0 + t * 1;
    } else {
      float t = (percent-LVL_3)/(LVL_4-LVL_3);
      rgb[0] = 0.470588 + t * -0.470588;
      rgb[1] = 0.462745 + t * -0.462745;
      rgb[2] = 1;
    }
  }
  void 
  Colormap::genStein2(float percent, float rgb[4])
  {
    const float LVL_0 = 0;
    const float LVL_1 = 0.49;
    const float LVL_2 = 0.50;
    const float LVL_3 = 0.51;
    const float LVL_4 = 1;
    if( percent <= LVL_1 ){
      float t = (percent-LVL_0)/(LVL_1-LVL_0);
      rgb[0] = 1;
      rgb[1] = 0.996078 + t * -0.996078;
      rgb[2] = 0 + t * 0.0274506;
    } else if( percent <= LVL_2 ){
      float t = (percent-LVL_1)/(LVL_2-LVL_1);
      rgb[0] = 1 + t * -1;
      rgb[1] = 0;
      rgb[2] = 0.0274506 + t * -0.0274506;
    } else if( percent <= LVL_3 ){
      float t = (percent-LVL_2)/(LVL_3-LVL_2);
      rgb[0] = 0;
      rgb[1] = 0 + t * 0.0196071;
      rgb[2] = 0 + t * 1;
    } else {
      float t = (percent-LVL_3)/(LVL_4-LVL_3);
      rgb[0] = 0 + t * 0.0392157;
      rgb[1] = 0.0196071 + t * 0.980393;
      rgb[2] = 1 + t * -1;
    }    
  }
} // namespace Glx
