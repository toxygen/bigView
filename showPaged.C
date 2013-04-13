//////////////////////////////////////////////////////////////////////////
////////////////////////////// showPaged.C ///////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <sstream>
#include <GLX.h>
#include <X11/keysym.h>
#include <math.h> // for floor,ceil
#include <unistd.h> // for getopt
#include <vector>
#include <map>
#include <queue>
#include <deque>
#include <assert.h>
#include "defs.h"
#include "ppm.h"
#include "ostr.h"
#include "PageReader.h"
#include "PageManager.h"
#include "glxTrackball.h"
#include "glxTrackpad.h"
#include "timer.h"
#include "Net.h"

#include <GL/glu.h>

using namespace std;

typedef std::pair<Glx::Vector,Glx::Vector> bbox;

const Glx::Vector LOVEC(-MAXFLOAT,-MAXFLOAT,-MAXFLOAT);
const Glx::Vector HIVEC(MAXFLOAT,MAXFLOAT,MAXFLOAT);

//#define SPHEREDEBUG 1

//#define DEBUG 1
#include "pdebug.h"
#include "glerr.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const double ONEDEG=0.017453292519943; // 1 deg in radians

#define M_PI_OVER_180 (M_PI/180.0f)
#define r2d(r) (r*180/M_PI)
#define LIMIT(val,limit){val = (val>limit) ? limit : val;}
#define CLAMP(val,lo,hi){if(val<lo)val=lo;else if(val>hi)val=hi;}

struct Loc {
  string name;
  double lat,lng;
};

std::vector<Loc*> locations;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////// GLOBALS /////////////////////////////////
////////////////////////////////////////////////////////////////////////// 

glx* env = 0;
Glx::Trackball* trackball=0;
Glx::Trackpad* trackpad=0;
Raw::PageManager* pageMgr=0;
Raw::PageReader* readerThread=0;

int levelOfDetail=0;
int imageHeight=0,imageWidth=0;
int sourceFormat=GL_RGB;
int sourceType=GL_UNSIGNED_BYTE;
int maxResident= -1;
bool memFull=false;
int maxLOD=-1;
int maxTexPages=0;
int totalPages=0;
int frustumRows=1,frustumCols=1,itsRow=0,itsCol=0;
bool showDebug=false;
bool signedPixels=false;
bool sphereMode=false;
bool wire=false;
bool minCurvature=true;
bool showLL=false;

unsigned int texMode = GL_LINEAR;

vector<Raw::Page*> pages;
ThreadedQueue<Raw::Page> workQueue;
std::queue< unsigned int, std::deque<unsigned int> > availTex;
GLuint maxLODobjID=0;
vector<int> visiblePages;

//default is MOLA +/- 88
// Note: MOC is +/- 65
// Big Blue is +/1 90
//double maxLat =  88*M_PI_OVER_180;
//double minLat = -88*M_PI_OVER_180;

double maxLat =   90.*M_PI_OVER_180;
double minLat = - 90.*M_PI_OVER_180;
double minLng =  000.*M_PI_OVER_180;
double maxLng =  360.*M_PI_OVER_180;

float mouseX,mouseY;

string filename;
bool showFilename=true;

std::vector<double> dpp; // pre-calc'd degreePerPixel(lod)

Glx::Vector eye,gaze;
int vp[4];
std::vector<bbox> bboxes;
Timer timer;

#ifdef SPHEREDEBUG
bool spheredebug=false;
glx* dbenv=0;
Glx::Trackball* dbtrackball=0;
std::map<int,int> invisible;
std::vector<int> allvisible;
#endif

double clip[4][4];
Glx::Vector viewWindow[4];
double projWidth, projHeight, projDist;

bool barriermode=false;
int ssock=0;

u_int64_t frameno=0;

const int HW2GAP=198;
int hgap=0;
int vgap=0;

//////////////////////////////////////////////////////////////////////////
////////////////////////// FORWARD DECLARATIONS //////////////////////////
//////////////////////////////////////////////////////////////////////////

int    countNumTextures(void);
int    pow2(int exp);
double degPerPixel(int lod);
double dot(double a[4],double b[4]);
void   normalize(double v[3]);
void   cross(double v1[3], double v2[3], double res[3]);
int    calcVisiblePages(glx* env, int lod);
int    calcVisibility(glx* env, int targetLod, int lod, int row, int col,
		      Glx::Vector& eye,double m[16],double p[16],int vp[4],
		      vector<int>& save);
int    calcVisibleSpherePages(glx* env, int lod,
			      Glx::Vector& eye,double m[16],double p[16],
			      int viewport[4],vector<int>& save);
void   calcPageCoords(int index, int& lod, int& col, int& row);
int    calcBitsetIndex(int lod, int col, int row);
void   calcCorners(glx* env, Raw::Coord& lo, Raw::Coord& hi, int lod);
void   setColor(int lod);
void   calcSourceDeltas(int lod, int imageWidth, int imageHeight,
			double& dx, double& dy);
double calcScale(double min[2], double max[2]);
void   adjustLOD(glx* env);
int    adjustSphereLOD(glx* env);
double d2r(double d);
void   drawMaxLODsphere(glx* env, void* user);
void   drawMaxLODflat(glx* env, void* user);
void   drawMaxLOD(glx* env, void* user);
void   drawFlat(glx* env, void* user);
void   drawSphere(glx* env, void* user);
void   draw(glx* env, void* user);
void   genTexture(Raw::Page* page);
void   pageLoaded(Raw::PageReader*, Raw::Page* page, void* user);
void   pixelToWorldCoords(glx* env, int x, int y, float* xf, float* yf);
void   drawLatArc(double lng, double loLat, double hiLat);
void   drawLngArc(double lat, double loLng, double hiLng);
void   idleMouse(glx* env,int x,int y,void* user);
void   initGL(glx* env,void* user);
void   clearTextures(void);
void   handleEvent(glx* env, XEvent *event, void*);
void   tokenize(string input, vector<string>& tokens, string sep=" \t\n");
int    OpenGLKeyword(string str);
string OpenGLKeyword(int value);

//////////////////////////////////////////////////////////////////////////
////////////////////////////////// CODE //////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// count number of textures registered with the graphics system [DEBUG]
int countNumTextures(void)
{
  int count=0;
  for(int i=0; i<pageMgr->numPages();++i){
    Raw::Page* p = pageMgr->getPage(i);
    assert(p);
    count += (p->itsID!=0);
  }
  return count;
}

int pow2(int exp)
{
  int result = 1;
  while( exp-- > 0 ) result *= 2;
  return result;
}

// calc deg/pixel based upon image size
// assumes a global image

double
degPerPixel(int lod)
{  
  int mipWidth, mipHeight;
  pageMgr->mipmapSize(0,mipWidth,mipHeight);
  double deg=(maxLng-minLng)/(double)mipWidth;
  for(int i=0;i<lod;++i)
    deg *= 2.;
  return deg;
}

double 
dot(double a[4],double b[4])
{
  return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
}

void normalize(double v[3]){
  double mag=sqrt(dot(v,v));
  if(mag !=0 ){
    double M = 1./mag;
    v[0] *= M;
    v[1] *= M;
    v[2] *= M;
  }
}

void 
cross(double v1[3], double v2[3], double res[3])
{
  res[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
  res[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
  res[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
}

// int calcVisiblePages(glx*, int)
//////////////////////////////////////////////////////////////////////////
// 
// PURPOSE: calc's the number of pages visible in the graphics window
//          [given a 2D flat projection with scale and translation
//          accessible from a glxTrackpad object]
// 
//  INPUTS: glx* env : viewer
//           int lod : level of detail to calc at
// 
// RETURNS: number of visible pages
// 
// GLOBALS: Raw::PageManager* pageMgr: keeps track of pages, image size
//        
// CALLERS: adjustLOD()
// 
//   NOTES: This relies upon the the assumption that all pages 
//          in the image are contiguous. Given the lo/hi corner indices:
// 
//          numPages = (hi.x-lo.x+1)*(hi.y-lo.y+1)
//
//          Example: lo=[0,0] and hi=[1,1] : 
//          numPages = (1-0+1)*(1-0+1) = 2*2 = 4
//
//            ..................              [physical space]                
//            .       .      hi.              screen boundaries -----
//            .   +--------+   .              page boundaries   .....  
//            .   |   .    |   .                              
//            ....|........|....                                       
//            .   |   .    |   .                              
//            .   |   .    |   .                              
//            .lo +--------+   .                              
//            ..................                              
//                                                            
//////////////////////////////////////////////////////////////////////////

int calcVisiblePages(glx* env, int lod)
{
  Raw::Coord lo,hi;
  int rows,cols;

  // calc the extreme lower left and upper right pages visible 
  // in the view, assume a contiguous tiled set of pages
  calcCorners(env,lo,hi,lod);

  // get the number of pages for the entire image at this LOD
  pageMgr->pageCounts(lod,cols,rows);
  
  // trivially return if the entire image is offscreen
  if(hi.second<0 || lo.second>=rows || hi.first<0 || lo.first>=cols)
    return 0;
  
  // clamp to reasonable offsets into the image
  if(lo.second<0)lo.second=0;
  if(lo.first<0)lo.first=0;
  if(hi.second>=rows)hi.second=rows-1;
  if(hi.first>=cols)hi.first=cols-1;

  // simple contiguous calculation of number of pages
  return (hi.second-lo.second+1)*(hi.first-lo.first+1);
}

// calc the lat/lng bounds of a page, respecting its LOD

void calcLatLng(Raw::Page* p,
		double* loLat, double* hiLat,
		double* loLng, double* hiLng)
{
  double deg = dpp[p->itsLevelOfDetail]*Raw::PAGE_SIZE;
  int col=p->itsCoord.first;
  int row=p->itsCoord.second;
  *loLat = minLat + row * deg;
  *hiLat = *loLat + deg;
  *loLng = minLng + col * deg;
  *hiLng = *loLng + deg;
}

// convert lat/lng to spherical coords
// note test for boundary pages with 'extra' lat/lng
// also note hack for shrinking pages, leaving a gap

void calcCoords(double loLat, double hiLat,
		double loLng, double hiLng,
		double xyz[4][3], bool space=false)
{
  
  if( hiLat > maxLat ) hiLat=maxLat;
  if( hiLng > maxLng ) hiLng=maxLng;
  if( space ){
    loLat+=0.01;
    loLng+=0.01;
    hiLat-=0.01;
    hiLng-=0.01;
  }
  double cLoLat = cos(loLat);
  double cHiLat = cos(hiLat);
  double sLoLat = sin(loLat);
  double sHiLat = sin(hiLat);
  double cLoLng = cos(loLng);
  double cHiLng = cos(hiLng);
  double sLoLng = sin(loLng);
  double sHiLng = sin(hiLng);

  xyz[0][0] = cLoLat * cLoLng;
  xyz[0][1] = cLoLat * sLoLng;
  xyz[0][2] = sLoLat;  
  xyz[1][0] = cHiLat * cLoLng;
  xyz[1][1] = cHiLat * sLoLng;
  xyz[1][2] = sHiLat;  
  xyz[2][0] = cHiLat * cHiLng;
  xyz[2][1] = cHiLat * sHiLng;
  xyz[2][2] = sHiLat;  
  xyz[3][0] = cLoLat * cHiLng;
  xyz[3][1] = cLoLat * sHiLng;
  xyz[3][2] = sLoLat;
}

// run a poiunt though a plane-equation
//
// a plane equation is of the form:
//
// 0 = ax + by + cz + d
//
// a,b,c are the xyz coords of any point on the plane
// d is the negative distance of the plane from the origin
//
// d = -sqrt(a*a + b*b + c*c)

double testPoint( Glx::Vector& p, double eqn[4])
{
  return eqn[0] * p[0] + eqn[1] * p[1] + eqn[2] * p[2] + eqn[3] ;
}

// calcVisibility()
//////////////////////////////////////////////////////////////////////////
// 
// PURPOSE: calculate the visibility of a page on a sphere and 
//          all it's children up to a given level [targetlod]
// 
//  INPUTS:      glx* env:
//          int targetLod: level of refinement to stop at
//                int lod: starting level [typically maxLOD]
//        int lod,row,col: level and coord of page
//       Glx::Vector& eye: eye pos
//           double m[16]: modelview matrix
//           double p[16]: projection matrix
//              int vp[4]: viewport
//      vector<int>& save: list of all visible pages
//
// RETURNS: The number of visible pages at the target LOD
//          parented by this page.
// 
// GLOBALS: Raw::PageManager* pageMgr;
//                 vector<double> dpp; // deg/pixel
//               double minLat,maxLat;
//               double minLng,maxLng;
//
// CALLERS:
//
//   CALLS: calcBitsetIndex();
//           calcVisibility(); // yes, recurses
//
//   NOTES:
// 
//////////////////////////////////////////////////////////////////////////

const int NOERR =0;
const int RCBIG =-1;
const int LLBIG =-2;
const int EMPTY =-3;
const int AWAY  =-4;
const int PROJ  =-5;
const int BHND  =-6;
const int OFF   =-7;

int calcVisibility(glx* env, int targetLod, int lod, int row, int col,
		   Glx::Vector& eye,double m[16],double p[16],int vp[4],
		   vector<int>& save)
{
  int rows,cols;

  // trivially reject if out of bounds for this LOD
  pageMgr->pageCounts(lod,cols,rows);
  if(row>=rows) return RCBIG;
  if(col>=cols) return RCBIG;

  double xyz[4][3],xy[4][3],d[4],eyedot,l[4],r[4],n[4],eye2pt[3];
  int xLo=vp[0];
  int yLo=vp[1];
  int xHi=vp[0]+vp[2];
  int yHi=vp[1]+vp[3];
  Raw::Coord coord(col,row);
  int idx=pageMgr->getPageIndex(lod,coord);

  // calc lat/lng extents of this page
  double deg   = dpp[lod]*Raw::PAGE_SIZE;
  double loLat = minLat + row * deg;
  double hiLat = loLat + deg;
  double loLng = minLng + col * deg;
  double hiLng = loLng + deg;

  // trivailly reject this page if possible 
  if( loLng >= maxLng ) return LLBIG;
  if( loLat >= maxLat ) return LLBIG;
  if( hiLat > maxLat ) hiLat=maxLat;
  if( hiLng > maxLng ) hiLng=maxLng;
  if( loLat==hiLat ) return EMPTY;
  if( loLng==hiLng ) return EMPTY;

  // calc spherical coords
  calcCoords(loLat, hiLat,loLng, hiLng, xyz);

  // reject this page if all four normals
  // point away from the eye, this takes care
  // of pages that are within the viewport 
  // but beyond the limb of the sphere

  // since the sphere is centered on the origin,
  // we can use the vertex as the normal
  // normals pointing towards the eye are 'in'
  // normals pointing away the eye are 'out'

  if( lod==targetLod ){
    bool allOut=true;
    float ed[4]={0,0,0,0};
    for(int i=0; i < 4 && allOut ; ++i ){ 
      eye2pt[0] = xyz[i][0]-eye[0];
      eye2pt[1] = xyz[i][1]-eye[1];
      eye2pt[2] = xyz[i][2]-eye[2];
      ed[i] = dot(xyz[i],eye2pt);
      if( ed[i] < 0. ) 
	allOut=false;
    }
    if( allOut ) return AWAY;
  }

  // reject if out of viewport, conservatively.
  // only reject pages whose bounding box, 
  // [which subsumes all children of that page], 
  // is COMPLETELY outside of any viewing frustum
  // bounding plane

  bbox box=bboxes[idx];
  bool allVertsOut=false;
  for(int j=0;j<4 && ! allVertsOut;++j){

    allVertsOut=true;
    for(int i=0;i<8 && allVertsOut;++i){      
      Glx::Vector v;
      switch( i ){
	case 0: v[0]=box.first[0]; v[1]=box.first[1]; v[2]=box.first[2];break;
	case 1: v[0]=box.second[0];v[1]=box.first[1]; v[2]=box.first[2];break;
	case 2: v[0]=box.first[0]; v[1]=box.second[1];v[2]=box.first[2];break;
	case 3: v[0]=box.second[0];v[1]=box.second[1];v[2]=box.first[2];break;
	case 4: v[0]=box.first[0]; v[1]=box.first[1]; v[2]=box.second[2];break;
	case 5: v[0]=box.second[0];v[1]=box.first[1]; v[2]=box.second[2];break;
	case 6: v[0]=box.first[0]; v[1]=box.second[1];v[2]=box.second[2];break;
	case 7: v[0]=box.second[0];v[1]=box.second[1];v[2]=box.second[2];break;
      } 
      if( testPoint(v,clip[j]) > 0.) 
	allVertsOut=false;
    }
  }

  if( allVertsOut ) return PROJ;    

#ifdef DEBUG 
  allvisible.push_back(idx);
#endif

  // don't recurse if we're at the target [highest] resolution
  if( targetLod==lod ){
    save.push_back(idx);
    return 1;
  } 

  assert(lod>targetLod);
  int N=0;
  int res;

  // here if this page is:
  // - at least partially visible, and
  // - is not the highest resolution
  //
  // therefore, recurse and check 'child' pages for visibility
  
  res = calcVisibility(env,targetLod,lod-1,row*2,col*2,eye,m,p,vp,save);
  if( res>0 ) N+=res;
  if( N>Raw::MAX_TEX_PAGES ) return N;

  res = calcVisibility(env,targetLod,lod-1,row*2+1,col*2,eye,m,p,vp,save);
  if( res>0 ) N+=res;
  if( N>Raw::MAX_TEX_PAGES ) return N;

  res = calcVisibility(env,targetLod,lod-1,row*2,col*2+1,eye,m,p,vp,save);
  if( res>0 ) N+=res;
  if( N>Raw::MAX_TEX_PAGES ) return N;

  res = calcVisibility(env,targetLod,lod-1,row*2+1,col*2+1,eye,m,p,vp,save);
  if( res>0 ) N+=res;

  return N;
}

// int calcVisibleSpherePages(glx*, int, Glx::Vector&,
//                           double[16], double[16],
//                           int[4],vector<int>&)
//////////////////////////////////////////////////////////////////////////
// 
// PURPOSE: calc number of pages visible assuming they lay upon a sphere
// 
//  INPUTS:          glx* env : viewer
//                    int lod : level of detail to calc at
//              double eye[4] : position of the eye
//               double m[16] : current modelview matrix
//               double p[16] : current projection matrix
//                  int vp[4] : current viewport [drawing area dims]
//          vector<int>& save : place to store intermediate results
// 
// RETURNS: number of visible pages
// 
// GLOBALS: Raw::PageManager* pageMgr : keeps track of pages, image size
//                         int maxLOD : max level of detail
//
// CALLERS: adjustSphereLOD()
// 
//   NOTES:
// 
//////////////////////////////////////////////////////////////////////////


int calcVisibleSpherePages(glx* env, int lod,
			   Glx::Vector& eye,double m[16],double p[16],
			   int vp[4],
			   vector<int>& save)
{
  int count=0;
  int rows,cols;
  pageMgr->pageCounts(maxLOD,cols,rows);
  save.clear();

#ifdef SPHEREDEBUG
  if(spheredebug){
    allvisible.clear();
    invisible.clear();
  }

  if(VERBOSE){
    _FANCYMESG("calcVisibleSpherePages");
    _VAR(lod);
    _VAR(eye);
    _VAR4V(vp);
    _VAR4x4("m",m);
    _VAR4x4("p",p);
  }
#endif

  for(int row = 0 ; row < rows ; ++row ){
    for(int col = 0 ; col < cols ; ++col ){
      int res = calcVisibility(env,lod,maxLOD,row,col,eye,m,p,vp,save);
      if( res>0 ) count += res;

#ifdef SPHEREDEBUG
      if(spheredebug)
      {
	Raw::Coord coord(col,row);
	int idx=pageMgr->getPageIndex(lod,coord);
	invisible[idx]=res;
      }
#endif
    }
  }  

  return count;
}

void calcPageCoords(int index, int& lod, int& col, int& row)
{
  int rows,cols;
  bool foundLOD=false;
  int offset=0;
  lod=0;
  while( ! foundLOD ){
    pageMgr->pageCounts(lod,cols,rows);
    int pageCount=rows*cols;
    if( index < offset+pageCount )
      foundLOD=true;
    else {
      offset += pageCount;
      ++lod;
    }
  }
  int diff = index-offset;
  row = (int)(diff/cols);
  col = diff % cols;
}

/////////////////////////////////////////////////
// a page's BitsetIndex is
// - the sum of all pages in all lesser LODs
// - plus the offset into this LOD
/////////////////////////////////////////////////

int calcBitsetIndex(int lod, int col, int row)
{
  int rows,cols;
  int offset=0;

  // account for all LODs below the target LOD

  for(int i=0; i<lod ; ++i ){
    pageMgr->pageCounts(i,cols,rows);
    offset += rows*cols;
  }

  // add offset into this LOD

  pageMgr->pageCounts(lod,cols,rows);
  offset += row*cols + col;
  return offset;
}

// void calcCorners(glx*,Raw::Coord&,Raw::Coord&,int)
//////////////////////////////////////////////////////////////////////////
// 
// PURPOSE: calculates the zero-based coord [pair<int,int>] of the pages
//          that fall upon the corners of the graphics window's view
//          [given a 2D flat projection with scale and translation
//          accessible from a glxTrackpad object]
//     
//  INPUTS:       glx* env : viewer 
//          Raw::Coord& lo : coord of lower-left page
//          Raw::Coord& hi : coord of upper-right page
//                 int lod : level of detail to calc at
// 
// RETURNS: returns results in lo&hi
// 
// GLOBALS: int frustumCols: size of the entire project [in screens]
//          int frustumRows:  "
//          int itsCol: this screen's offset into the screen array
//          int itsRow:  "
//          glxTrackpad* trackpad: mouse translation & scaling
//
// CALLERS: calcVisiblePages(), adjustLOD(), drawFlat()
// 
//   NOTES: This function relies heavily upon two facts:
//          1) The graphics window is using a flat, 2D projection
//          2) each of the pages is rendered as a texture mapped
//             square which is scaled according to the LOD.
//             squareSize is in physical units
// 
//             LOD | squareSize
//             ----------------
//              0  |    1   <= full res
//              1  |    2   <= each square covers 2 physical units
//              2  |    4   <=  "     "      "    4    "       "
// 
//////////////////////////////////////////////////////////////////////////

void calcCorners(glx* env, Raw::Coord& lo, Raw::Coord& hi, int lod)
{
  double aspect = env->aspect();

  // calc midpoint of screen array
  double midcol = (double)(frustumCols-1)/2.0; 
  double midrow = (double)(frustumRows-1)/2.0;

  // calc this screens indices relative to center of screen array
  double col = -midcol+itsCol; 
  double row = -midrow+itsRow;

  // calc lowerleft corner of this screen in physical coords
  double xOff = col*((2.0*aspect)/trackpad->itsScaleFactor); 
  double yOff = row*(2.0/trackpad->itsScaleFactor);

  // calchow far the user has translated the scene
  double xyz[3]={-xOff+trackpad->itsXtrans,-yOff+trackpad->itsYtrans,0.0};
  double s = trackpad->itsScaleFactor;
  int w = env->winWidth();
  int h = env->winHeight();

  // squareSize is in physical units, this ONLY works when a page
  // has a known size of 1 degree
  // 
  // lod | squareSize
  // ----------------
  //  0  |    1   <= full res
  //  1  |    2   <= each square covers 2 physical units
  //  2  |    4   <=  "     "      "    4    "       "
  //

  double squareSize = pow2(lod);

  // calc actual corners in physical space of this window
  double xlo = -aspect/s - xyz[0];
  double ylo = -1.0/s - xyz[1];
  double xhi =  aspect/s - xyz[0];
  double yhi =  1.0/s - xyz[1];

  // calc pages that overlap the corners based upon the LOD
  lo.first = (int)floor(xlo/squareSize);
  lo.second = (int)floor(ylo/squareSize);
  hi.first = (int)floor(xhi/squareSize);
  hi.second = (int)floor(yhi/squareSize);
  //cout << "calcCorners:[" <<lod<<"] : "<< lo << ":" << hi << endl;
}

void 
setErrColor(int err)
{   
  switch( err ){
    case RCBIG:glColor3f(1,0,0);break;
    case LLBIG:glColor3f(0,1,0);break;
    case EMPTY:glColor3f(0,0,1);break;
    case AWAY: glColor3f(1,1,0);break;
    case PROJ: glColor3f(1,0,1);break;
    case BHND: glColor3f(0,1,1);break;
    case OFF:  glColor3f(1,.5,.5);break; // salmon :)
    default:   glColor3f(1,1,1);break;
  }
}
void 
setErrMesgColor(glx* env, int err)
{   
  switch( err ){
    case RCBIG:env->setMessageColor(1,0,0);break;
    case LLBIG:env->setMessageColor(0,1,0);break;
    case EMPTY:env->setMessageColor(0,0,1);break;
    case AWAY: env->setMessageColor(1,1,0);break;
    case PROJ: env->setMessageColor(1,0,1);break;
    case BHND: env->setMessageColor(0,1,1);break;
    case OFF:  env->setMessageColor(1,.5,.5);break; // salmon :)
    default:   env->setMessageColor(1,1,1);break;
  }
}

void 
setColor(int lod)
{    
  switch(lod){
    case 0:glColor3f(0.5,0.5,0.5);break;
    case 1:glColor3f(1,0,0);break;
    case 2:glColor3f(0,1,0);break;
    case 3:glColor3f(0,0,1);break;
    case 4:glColor3f(1,1,0);break;
    case 5:glColor3f(1,0,1);break;
    case 6:glColor3f(0,1,1);break;
    default:break;
  } 
}

double
calcScale(double min[2], double max[2])
{
  const double EPS=1.e-12;
  double dx = fabs(max[0]-min[0]);
  double dy = fabs(max[1]-min[1]);
  double s = (dx > dy) ? dx : dy;
  if( s < EPS )
    s = EPS;
  return(1.0/s);
}

// void adjustLOD(glx*)
//////////////////////////////////////////////////////////////////////////
// 
// PURPOSE: adjust the level of detail so as to draw the most pages at the
//          highest resolution possible given a maximum number of texture
//          pages availible
// 
//  INPUTS: glx* env : viewer
// 
// RETURNS: adjusts the global levelOfDetail
// 
// GLOBALS: int levelOfDetail: level of detail [may be adjusted]
//          int maxTexPages: number of texture pages allowed
//          ThreadedQueue<Raw::Page> workQueue: page load requests go here
//          Raw::PageManager* pageMgr: keeps track of pages, image size
//          std::queue< unsigned int, std::deque<unsigned int> > availTex
//
// CALLERS:
// 
//   NOTES:
// 
//////////////////////////////////////////////////////////////////////////

void 
adjustLOD(glx* env)
{
  Raw::Coord lo,hi;
  int rows,cols;
  int fixLevel=-1;

  int visible = calcVisiblePages(env, levelOfDetail);
  if( visible > Raw::MAX_TEX_PAGES )
  {
    fixLevel=levelOfDetail;
    while( visible > Raw::MAX_TEX_PAGES ){
      ++levelOfDetail;
      visible = calcVisiblePages(env, levelOfDetail);
    }
  } 
  else 
  {
    int candidateLOD=levelOfDetail-1;
    visible = calcVisiblePages(env, candidateLOD);
    if( visible < Raw::MAX_TEX_PAGES && candidateLOD>=0){
      while( visible < Raw::MAX_TEX_PAGES && candidateLOD>0){
	--candidateLOD;
	visible = calcVisiblePages(env, candidateLOD);
      }
      fixLevel=levelOfDetail;
      levelOfDetail=candidateLOD;
    }
  }

  // free up the textures from the previous level
  if( fixLevel != -1 && fixLevel!=maxLOD){
    workQueue.clear();
    int start = pageMgr->begin(fixLevel);
    int end   = pageMgr->end(fixLevel);
    for(int i=start ; i < end ; ++i ){
      Raw::Page* p = pageMgr->getPage(i);
      assert(p);
      if( p->itsID != 0 ){
	availTex.push(p->itsID);
	p->itsID=0;
	delete [] p->itsPixels;
	p->itsPixels=0;
	MESGVAR("FLUSHED",p->itsCoord);
      }
    }
  }

  calcCorners(env,lo,hi,levelOfDetail);
  pageMgr->pageCounts(levelOfDetail,cols,rows);

  // if nothing is visible, do nothing...
  if(hi.second<0 || lo.second>=rows || hi.first<0 || lo.first>=cols) 
    return;

  if(lo.second<0)lo.second=0;
  if(lo.first<0)lo.first=0;
  if(hi.second>=rows)hi.second=rows-1;
  if(hi.first>=cols)hi.first=cols-1;

  workQueue.clear();

  for(int r=0 ; r < rows ; ++r){
    for(int c=0 ; c < cols ; ++c ){
      Raw::Coord coord(c,r);
      Raw::Page* p = pageMgr->getPage(levelOfDetail,coord);
      assert(p);

      // offscreen, release tex if not background
      if( r<lo.second || r>hi.second || c<lo.first || c>hi.first ){
	if( p->itsID!=0 && p->itsLevelOfDetail!=maxLOD){
	  availTex.push(p->itsID);
	  p->itsID=0;
	  delete [] p->itsPixels;
	  p->itsPixels=0;
	  MESGVAR("FLUSHED",p->itsCoord);
	}
	continue;
      }
      
      if( p->itsPixels==0 )
	// pixel memory was purged... ask to have it read again
	workQueue.add( p );	
      else if( p->itsID==0 ){
	// texture memory was released, need to reload pixels
	genTexture(p);
      }
    }
  }
}

int 
adjustSphereLOD(glx* env)
{
  int N,fixLevel=-1;
  int save=levelOfDetail;
  
  N = calcVisibleSpherePages(env,levelOfDetail,eye,
			     trackball->view,trackball->proj,vp,
			     visiblePages);
  bool candidateOK=false;
  int candidateLOD=levelOfDetail;

  if( N > Raw::MAX_TEX_PAGES )
  {
    fixLevel=levelOfDetail;

    while( N > Raw::MAX_TEX_PAGES ){
      ++levelOfDetail;
      N=calcVisibleSpherePages(env,levelOfDetail,eye,
			       trackball->view,trackball->proj,vp,
			       visiblePages);
    }
  } 
  else // N < maxTexPages, i.e. we can increase # visible
  {
    N=Raw::MAX_TEX_PAGES-1; // hack to start loop

    while( N < Raw::MAX_TEX_PAGES && candidateLOD>0){
      --candidateLOD;
      N = calcVisibleSpherePages(env,candidateLOD,eye,
				 trackball->view,trackball->proj,vp,
				 visiblePages);

      if( N<Raw::MAX_TEX_PAGES ) 
	candidateOK=true;
      else
	++candidateLOD; //one too far
    }
    if( candidateOK ){
      fixLevel=levelOfDetail;
      levelOfDetail=candidateLOD;
    }
    assert(levelOfDetail>=0 && levelOfDetail<=maxLOD);
    N = calcVisibleSpherePages(env,levelOfDetail,eye,
			       trackball->view,trackball->proj,vp,
			       visiblePages);
  }
  assert(N<=Raw::MAX_TEX_PAGES);

  // free up the textures from the previous level
  if( fixLevel != -1 && fixLevel!=maxLOD){
    workQueue.clear();
    int start = pageMgr->begin(fixLevel);
    int end   = pageMgr->end(fixLevel);
    for(int i=start ; i < end ; ++i ){
      Raw::Page* p = pageMgr->getPage(i);
      assert(p);
      if( p->itsID != 0 ){
	availTex.push(p->itsID);
	p->itsID=0;
	delete [] p->itsPixels;
	p->itsPixels=0;
	MESGVAR("FLUSHED",p->itsCoord);
      }
    }
  }

  workQueue.clear();
  int rows,cols;
  vector<int>::iterator iter;
  pageMgr->pageCounts(levelOfDetail,cols,rows);
  for(int r=0 ; r < rows ; ++r){
    for(int c=0 ; c < cols ; ++c ){
      Raw::Coord coord(c,r);
      Raw::Page* p = pageMgr->getPage(levelOfDetail,coord);
      assert(p);

      // see if offscreen, release tex if not background
      int index = calcBitsetIndex(levelOfDetail,c,r);
      iter = find(visiblePages.begin(),visiblePages.end(),index);
      if( iter == visiblePages.end() ){
	if( p->itsID!=0 && p->itsLevelOfDetail!=maxLOD){
	  availTex.push(p->itsID);
	  p->itsID=0;
	  delete [] p->itsPixels;
	  p->itsPixels=0;
	  MESGVAR("FLUSHED",p->itsCoord);
	}
	continue;
      }
      
      if( p->itsPixels==0 ){
	// pixel memory was purged... ask to have it read again
	workQueue.add( p );
	MESGVAR("REQ",p->itsCoord);
	VAR(workQueue.size());
      } else if( p->itsID==0 ){
	// texture memory was released, need to reload pixels
	genTexture(p);
      }
    }
  }
  return N;
}

//////////////////////////////////////////////////////////////////////////
///////////////////////////// GLX CALLBACKS //////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double d2r(double d){
  return d*(M_PI/180.0f);
}

void drawMaxLODsphere(glx* env, void* user)
{
  int cols,rows;
  double xyz[4][3];

  glPushMatrix();
  //glScalef(0.99,0.99,0.99);

  pageMgr->pageCounts(maxLOD,cols,rows);
  double deg = dpp[maxLOD]*Raw::PAGE_SIZE;

  glEnable(GL_TEXTURE_2D);
  int start = pageMgr->begin(maxLOD);
  int end   = pageMgr->end(maxLOD);

  for( int i=start ; i < end ; ++i ){
    double s=1.0,t=1.0;

    Raw::Page* p = pageMgr->getPage(i);
    assert(p);
    if( p->itsID==0 ) {
      if( p->itsPixels==0 )
	continue;
      genTexture(p);
      if( p->itsID==0)
	continue;
    }

    int col=p->itsCoord.first;
    int row=p->itsCoord.second;

    double loLat = minLat + row * deg;
    double hiLat = loLat + deg;
    double loLng = minLng + col * deg;
    double hiLng = loLng + deg;
    if( loLng >= maxLng ) continue;
    if( loLat >= maxLat ) continue;

    if( hiLat > maxLat ) {
      t = (maxLat - loLat)/deg;
      hiLat=maxLat;
    }
    if( hiLng >= maxLng ){
      s = (maxLng - loLng)/deg;
      hiLng=maxLng;
    }    
    
    glBindTexture(GL_TEXTURE_2D,p->itsID);

    double latWidth=hiLat-loLat;
    double lngWidth=hiLng-loLng;
    double minDelta=4*M_PI_OVER_180;
    if( minDelta>deg ) minDelta=deg;
    const double EPS =1.0e-8;
    for( double lat=loLat ; lat < hiLat ; lat += minDelta ){
      double nextLat = (lat+minDelta<hiLat) ? (lat+minDelta) : hiLat;
      double tThis = (double)(lat-loLat)/latWidth;
      if( fabs(1.0-tThis)<EPS ) continue;
      double tNext = (double)(nextLat-loLat)/latWidth;
      if( tNext>1 ) tNext=1;
      double tLo = tThis*t;
      double tHi = tNext*t;
      if( tHi>1) tHi=1;
      double cLoLat = cos(lat);
      double sLoLat = sin(lat);
      double cHiLat = cos(nextLat);
      double sHiLat = sin(nextLat);
      
      for( double lng=loLng ; lng < hiLng ; lng += minDelta){
	double nextLng = (lng+minDelta<hiLng) ? lng+minDelta : hiLng;
	double tThis = (double)(lng-loLng)/lngWidth;
	if( fabs(1.0-tThis)<EPS ) continue;
	double tNext = (double)(nextLng-loLng)/lngWidth;
	if( tNext>1.0 ) tNext=1.0;
	double sLo = tThis*s;
	double sHi = tNext*s;
	if( sHi>1) sHi=1;

	double cLoLng = cos(lng);
	double sLoLng = sin(lng);
	double cHiLng = cos(nextLng);
	double sHiLng = sin(nextLng);

	xyz[0][0] = cLoLat * cLoLng;
	xyz[0][1] = cLoLat * sLoLng;
	xyz[0][2] = sLoLat; 
	xyz[1][0] = cHiLat * cLoLng;
	xyz[1][1] = cHiLat * sLoLng;
	xyz[1][2] = sHiLat; 
	xyz[2][0] = cHiLat * cHiLng;
	xyz[2][1] = cHiLat * sHiLng;
	xyz[2][2] = sHiLat; 
	xyz[3][0] = cLoLat * cHiLng;
	xyz[3][1] = cLoLat * sHiLng;
	xyz[3][2] = sLoLat;

	glColor3f(1,1,1);
	glBegin(GL_QUADS);
	glTexCoord2f(sLo,tLo); glVertex3dv(xyz[0]);
	glTexCoord2f(sLo,tHi); glVertex3dv(xyz[1]);
	glTexCoord2f(sHi,tHi); glVertex3dv(xyz[2]);
	glTexCoord2f(sHi,tLo); glVertex3dv(xyz[3]);
	glEnd();
      }
    }
  }
  glPopMatrix();
  glDisable(GL_TEXTURE_2D);
}

void drawMaxLODflat(glx* env, void* user)
{
  glEnable(GL_TEXTURE_2D);
  int start = pageMgr->begin(maxLOD);
  int end   = pageMgr->end(maxLOD);
  double sq = pow2(maxLOD);
  for( int i=start ; i < end ; ++i ){
    Raw::Page* p = pageMgr->getPage(i);
    assert(p);
    if( p->itsID==0 ) {
      if( p->itsPixels==0 )
	continue;
      genTexture(p);
      if( p->itsID==0)
	continue;
    }
    int col=p->itsCoord.first;
    int row=p->itsCoord.second;

    glBindTexture(GL_TEXTURE_2D,p->itsID);
    glColor3f(1,1,1);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex3f(col*sq,    row*sq,   -0.01);
    glTexCoord2f(1,0); glVertex3f(col*sq+sq, row*sq,   -0.01);
    glTexCoord2f(1,1); glVertex3f(col*sq+sq, row*sq+sq,-0.01);
    glTexCoord2f(0,1); glVertex3f(col*sq,    row*sq+sq,-0.01);
    glEnd();
  }
  glDisable(GL_TEXTURE_2D);
}

void drawMaxLOD(glx* env, void* user)
{
  if( maxLODobjID ){
    if( ! sphereMode )
      glCallList( maxLODobjID );
    return;
  }

  maxLODobjID = glGenLists( (GLsizei)1 ); 
  glNewList(maxLODobjID, GL_COMPILE_AND_EXECUTE);
  switch( sphereMode ){
    case true:
      drawMaxLODsphere(env,user);
      break;
    case false:
      drawMaxLODflat(env,user);
      break;      
  }
  glEndList();
}

void
drawFlat(glx* env, void* user)
{
  int rows,cols;
  double squareSize = pow2(levelOfDetail);

  pageMgr->pageCounts(levelOfDetail,cols,rows);

  if(showDebug){
    setColor(levelOfDetail);    
    for(int r=0 ; r < rows ; ++r){
      for(int c=0 ; c < cols ; ++c ){
	glBegin(GL_LINE_LOOP);
	glVertex3f(c*squareSize,           r*squareSize,0.1);
	glVertex3f(c*squareSize+squareSize,r*squareSize,0.1);
	glVertex3f(c*squareSize+squareSize,r*squareSize+squareSize,0.1);
	glVertex3f(c*squareSize,           r*squareSize+squareSize,0.1);
	glEnd();
      }
    }
  }

  drawMaxLOD(env, user);

  glEnable(GL_TEXTURE_2D);

  int start = pageMgr->begin(levelOfDetail);
  int end   = pageMgr->end(levelOfDetail);

  Raw::Coord coord,lo,hi;
  calcCorners(env,lo,hi,levelOfDetail);

  for( int i=start ; i < end ; ++i ){
    int index=i-start;
    coord.second = (int)(index/cols);
    coord.first = index-(coord.second*cols);
    if( coord.first  < lo.first ||
	coord.first  > hi.first ||
	coord.second < lo.second ||
	coord.second > hi.second )
    {
      // not visible
      continue;
    }

    Raw::Page* p = pageMgr->getPage(i);
    assert(p);

    if( p->itsID==0 ) {
      if( p->itsPixels==0 )
	continue;
      genTexture(p);
      if( p->itsID==0)
	continue;
    }
    double sq=pow2(p->itsLevelOfDetail);
    int col=coord.first;
    int row=coord.second;

    glBindTexture(GL_TEXTURE_2D,p->itsID);
    glColor3f(1,1,1);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex2f(col*sq,    row*sq);
    glTexCoord2f(1,0); glVertex2f(col*sq+sq, row*sq);
    glTexCoord2f(1,1); glVertex2f(col*sq+sq, row*sq+sq);
    glTexCoord2f(0,1); glVertex2f(col*sq,    row*sq+sq);
    glEnd();

#if DEBUG
    glDisable(GL_TEXTURE_2D);
    //env->showMessage(ctoa(p->itsCoord),col*sq+sq/2.0f,row*sq+sq/2.0f);
    glEnable(GL_TEXTURE_2D);
#endif
  }

  glDisable(GL_TEXTURE_2D);

#if DEBUG
  double sq=pow2(levelOfDetail);
  double aspect=env->aspect();
  double midcol = (double)(frustumCols-1)/2.0;
  double midrow = (double)(frustumRows-1)/2.0;
  double col = -midcol+itsCol;
  double row = -midrow+itsRow;
  double xOff = col*((2.0*aspect)/trackpad->itsScaleFactor);
  double yOff = row*(2.0/trackpad->itsScaleFactor);
  double xyz[3]={-xOff+trackpad->itsXtrans,-yOff+trackpad->itsYtrans,0.0};
  double s = trackpad->itsScaleFactor;
  double xlo = -aspect/s - xyz[0];
  double ylo = -1.0/s - xyz[1];
  double xhi =  aspect/s - xyz[0];
  double yhi =  1.0/s - xyz[1];
  int loc = (int)floor(xlo/sq);
  int lor = (int)floor(ylo/sq);
  int hic = (int)floor(xhi/sq);
  int hir = (int)floor(yhi/sq);

  glDisable(GL_DEPTH_TEST);
  env->showMessage(itoa(levelOfDetail)+" LOD");
  //env->showMessage(itoa(calcVisiblePages(env, levelOfDetail))+" Pages");
  env->showMessage("[col,row]   : "+itoa(itsCol)+" , "+itoa(itsRow));
  env->showMessage("[lo,hi]     : "+ctoa(lo)+" , "+ctoa(hi));
  env->showMessage("[xOff,yOff] : "+ftoa(xOff)+" , "+ftoa(yOff));
  env->showMessage("[xt,yt]     : "+ftoa(xyz[0])+" , "+ftoa(xyz[1]));
  env->showMessage("[xlo,xhi]   : "+ftoa(xlo)+" , "+ftoa(xhi));
  env->showMessage("[ylo,yhi]   : "+ftoa(ylo)+" , "+ftoa(yhi));
  env->showMessage("lo[c,r]     : "+itoa(loc)+" , "+itoa(lor));
  env->showMessage("hi[c,r]     : "+itoa(hic)+" , "+itoa(hir));
  env->showMessage("mouse[x,y]  : "+ftoa(mouseX)+" , "+ftoa(mouseY));  
  env->showMessage("scale       : "+ftoa(trackpad->itsScaleFactor));
  env->showMessage("trans[x,y]  : "+ftoa(trackpad->itsXtrans)+" , "+ftoa(trackpad->itsYtrans));
  glEnable(GL_DEPTH_TEST);
#endif
}

int calcParentTexCoords(Raw::Page* p, 
			double* slo, double* shi,
			double* tlo, double* thi)
{
  Raw::Coord pcoord=p->itsCoord;
  int rows,cols;
  int col=p->itsCoord.first;
  int row=p->itsCoord.second;
  pageMgr->pageCounts(p->itsLevelOfDetail,cols,rows);

  int l=p->itsLevelOfDetail;
  while(l<maxLOD){
    pcoord.first/=2;
    pcoord.second/=2;
    ++l;
  }
  int pi = pageMgr->getPageIndex(maxLOD, pcoord);
  Raw::Page* pp = pageMgr->getPage(pi); 
  
  double cdeg = dpp[p->itsLevelOfDetail]*Raw::PAGE_SIZE;
  double pdeg = dpp[pp->itsLevelOfDetail]*Raw::PAGE_SIZE;
  
  if(row==rows-1 || col==cols-1 ){
    MESGVAR("C",p->itsCoord);
    MESGVAR("C",p->itsLevelOfDetail);
    MESGVAR("C",cdeg);
    
    MESGVAR("P",pp->itsCoord);
    MESGVAR("P",pp->itsLevelOfDetail);
    MESGVAR("P",pdeg);
  }

  double cLatLo,cLatHi,cLngLo,cLngHi;
  double pLatLo,pLatHi,pLngLo,pLngHi;

  calcLatLng(p, &cLatLo,&cLatHi,&cLngLo,&cLngHi);
  calcLatLng(pp,&pLatLo,&pLatHi,&pLngLo,&pLngHi);

  LIMIT(cLatHi,maxLat);
  LIMIT(pLatHi,maxLat);
  LIMIT(cLngHi,maxLng);
  LIMIT(pLngHi,maxLng);

  // these will be less than 1 if there
  // if the parent page is at the edge
  // and contains black, non-data pixels

  double maxt = (pLatHi - pLatLo)/pdeg;
  double maxs = (pLngHi - pLngLo)/pdeg;

  // these coords reach into the maxLOD texture
  // may be scaled for edge pages

  *slo=maxs*(cLngLo-pLngLo)/(pLngHi-pLngLo);
  *shi=maxs*(cLngHi-pLngLo)/(pLngHi-pLngLo);
  *tlo=maxt*(cLatLo-pLatLo)/(pLatHi-pLatLo);
  *thi=maxt*(cLatHi-pLatLo)/(pLatHi-pLatLo);

  if(row==rows-1 || col==cols-1 ){
    FANCYMESG("child");
    VAR(p->itsCoord);
    VAR(p->itsLevelOfDetail);
    VAR(dpp[p->itsLevelOfDetail])
    VAR2(r2d(cLatLo),r2d(cLatHi));
    VAR2(r2d(cLngLo),r2d(cLngHi));
    VAR2(*slo,*shi);
    VAR2(*tlo,*thi);
    VAR(cdeg);
    VAR2(maxs,maxt);
    
    FANCYMESG("parent");
    VAR(pp->itsCoord);
    VAR(pp->itsLevelOfDetail);
    VAR(dpp[pp->itsLevelOfDetail]);
    VAR2(r2d(pLatLo),r2d(pLatHi));
    VAR2(r2d(pLngLo),r2d(pLngHi));
    VAR(pdeg);
  }
  
  // belt *and* suspenders

  CLAMP(*slo,0.,1.);
  CLAMP(*shi,0.,1.);
  CLAMP(*tlo,0.,1.);
  CLAMP(*thi,0.,1.);
  
  return pp->itsID;
}

void drawSphere(glx* env, void* user)
{
  vector<int>::iterator iter,found;
  int cols,rows;
  double xyz[4][3];
  double pslo,pshi,ptlo,pthi; // tex coords relative to maxLOD
  FANCYMESG("drawSphere");
  drawMaxLOD(env, user);

  pageMgr->pageCounts(levelOfDetail,cols,rows);
  int lvlpages=cols*rows;
  int drawn=0;
  int rejected=0;
  int mipWidth, mipHeight;
  int notReady=0;

  pageMgr->mipmapSize(levelOfDetail,mipWidth,mipHeight);
  double deg = dpp[levelOfDetail]*Raw::PAGE_SIZE;
  VAR(levelOfDetail);
  VAR(mipWidth);
  VAR(mipHeight);
  MESGVAR("dpp[levelOfDetail]",dpp[levelOfDetail]);
  MESGVAR("deg per page",r2d(deg));

  if( ! wire ) glEnable(GL_TEXTURE_2D);
  int start = pageMgr->begin(levelOfDetail);
  int end   = pageMgr->end(levelOfDetail);
  double sq=pow2(levelOfDetail);

  iter = visiblePages.begin();
  for( ; iter != visiblePages.end() ; ++iter ){
    int i=*iter;
    double maxs=1.0,maxt=1.0;
    bool useParentTex=false;
    int parentID=0;

    Raw::Page* p = pageMgr->getPage(i);
    assert(p);

    if( p->itsID==0 ) 
    {
      if( p->itsPixels==0 )
      {
	MESGVAR("NOTREADY[pix=0]",p->itsCoord);
	useParentTex=true;
	parentID = calcParentTexCoords(p,&pslo,&pshi,&ptlo,&pthi);
	notReady++;
	//continue;
      } 
      else // p->itsPixel!=0 i.e. got pixels!
      {
	genTexture(p);
	if( p->itsID==0){
	  MESGVAR("NOTREADY[id=0]",p->itsCoord);
	  useParentTex=true;
	  parentID = calcParentTexCoords(p,&pslo,&pshi,&ptlo,&pthi);
	  notReady++;
	  //continue;
	}
      }
    } // p->itsID==0

    int col=p->itsCoord.first;
    int row=p->itsCoord.second;

    double loLat = minLat + row * deg;
    double hiLat = loLat + deg;
    double loLng = minLng + col * deg;
    double hiLng = loLng + deg;
    if( loLng >= maxLng ) { ++rejected;continue;}
    if( loLat >= maxLat ) { ++rejected;continue;}

    // VERY IMPORTANT for the top/right 'edge' pages
    // The edge pages have both data pixels and black fill pixels. 
    // maxs/maxt are the max tex coords for valid data
    // and are 1 everywher except the the edges

    LIMIT(hiLat,maxLat);
    maxt = (hiLat - loLat)/deg;
    LIMIT(hiLng,maxLng);
    maxs = (hiLng - loLng)/deg;

    if( loLat>=hiLat ) { ++rejected;continue;}
    if( loLng>=hiLng ) { ++rejected;continue;}

    double latWidth=hiLat-loLat;
    double lngWidth=hiLng-loLng;
    ++drawn;

    if( ! minCurvature )
    {
      float sLo = 0;
      float sHi = maxs;
      float tLo = 0;
      float tHi = maxt;
      double cLoLat = cos(loLat);
      double sLoLat = sin(loLat);
      double cHiLat = cos(hiLat);
      double sHiLat = sin(hiLat);
      double cLoLng = cos(loLng);
      double sLoLng = sin(loLng);
      double cHiLng = cos(hiLng);
      double sHiLng = sin(hiLng);

      xyz[0][0] = cLoLat * cLoLng;
      xyz[0][1] = cLoLat * sLoLng;
      xyz[0][2] = sLoLat;      
      xyz[1][0] = cHiLat * cLoLng;
      xyz[1][1] = cHiLat * sLoLng;
      xyz[1][2] = sHiLat;      
      xyz[2][0] = cHiLat * cHiLng;
      xyz[2][1] = cHiLat * sHiLng;
      xyz[2][2] = sHiLat;      
      xyz[3][0] = cLoLat * cHiLng;
      xyz[3][1] = cLoLat * sHiLng;
      xyz[3][2] = sLoLat;

      if( wire ){
	setColor(levelOfDetail); 
	glBegin(GL_LINE_LOOP);
	glVertex3dv(xyz[0]);
	glVertex3dv(xyz[1]);
	glVertex3dv(xyz[2]);
	glVertex3dv(xyz[3]);
	glEnd();
      } else if( useParentTex ){
	glBindTexture(GL_TEXTURE_2D,parentID);
	glColor3f(1,1,1);
	glBegin(GL_QUADS);
	glTexCoord2f(pslo,ptlo); glVertex3dv(xyz[0]);
	glTexCoord2f(pslo,pthi); glVertex3dv(xyz[1]);
	glTexCoord2f(pshi,pthi); glVertex3dv(xyz[2]);
	glTexCoord2f(pshi,ptlo); glVertex3dv(xyz[3]);
	glEnd();	
      } else {
	glBindTexture(GL_TEXTURE_2D,p->itsID);
	glColor3f(1,1,1);
	glBegin(GL_QUADS);
	glTexCoord2f(sLo,tLo); glVertex3dv(xyz[0]);
	glTexCoord2f(sLo,tHi); glVertex3dv(xyz[1]);
	glTexCoord2f(sHi,tHi); glVertex3dv(xyz[2]);
	glTexCoord2f(sHi,tLo); glVertex3dv(xyz[3]);
	glEnd();
      }

      if( showDebug ){
	setColor(levelOfDetail); 
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);

	glBegin(GL_LINE_LOOP);
	glVertex3dv(xyz[0]);
	glVertex3dv(xyz[1]);
	glVertex3dv(xyz[2]);
	glVertex3dv(xyz[3]);
	glEnd();

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
      }
    } 
    else // minCurvature==true
    {
      //double minDelta=4*M_PI_OVER_180;
      double minDelta=2*M_PI_OVER_180;
      LIMIT(minDelta,deg);
      const double EPS =1.0e-8;
      for( double lat=loLat ; lat < hiLat ; lat += minDelta ){
	double nextLat = lat+minDelta;
	LIMIT(nextLat,hiLat);
	double tThis = (double)(lat-loLat)/latWidth;
	double tNext = (double)(nextLat-loLat)/latWidth;
	LIMIT(tThis,1);
	LIMIT(tNext,1);
	double tLo = tThis*maxt;
	double tHi = tNext*maxt;
	LIMIT(tLo,1);
	LIMIT(tHi,1);

	double cLoLat = cos(lat);
	double sLoLat = sin(lat);
	double cHiLat = cos(nextLat);
	double sHiLat = sin(nextLat);

	for( double lng=loLng ; lng < hiLng ; lng += minDelta){
	  double nextLng = lng+minDelta;
	  LIMIT(nextLng,hiLng);	  
	  double sThis = (double)(lng-loLng)/lngWidth;
	  double sNext = (double)(nextLng-loLng)/lngWidth;
	  LIMIT(sThis,1);
	  LIMIT(sNext,1);
	  double sLo = sThis*maxs;
	  double sHi = sNext*maxs;
	  LIMIT(sLo,1);
	  LIMIT(sHi,1);

	  double cLoLng = cos(lng);
	  double sLoLng = sin(lng);
	  double cHiLng = cos(nextLng);
	  double sHiLng = sin(nextLng);

	  xyz[0][0] = cLoLat * cLoLng;
	  xyz[0][1] = cLoLat * sLoLng;
	  xyz[0][2] = sLoLat; 
	  xyz[1][0] = cHiLat * cLoLng;
	  xyz[1][1] = cHiLat * sLoLng;
	  xyz[1][2] = sHiLat; 
	  xyz[2][0] = cHiLat * cHiLng;
	  xyz[2][1] = cHiLat * sHiLng;
	  xyz[2][2] = sHiLat; 
	  xyz[3][0] = cLoLat * cHiLng;
	  xyz[3][1] = cLoLat * sHiLng;
	  xyz[3][2] = sLoLat;

	  if( wire ){
	    setColor(levelOfDetail); 
	    glBegin(GL_LINE_LOOP);
	    glVertex3dv(xyz[0]);
	    glVertex3dv(xyz[1]);
	    glVertex3dv(xyz[2]);
	    glVertex3dv(xyz[3]);
	    glEnd();
	  } else if( useParentTex ){
	    double _slo = pslo + sThis * (pshi-pslo);
	    double _shi = pslo + sNext * (pshi-pslo);
	    double _tlo = ptlo + tThis * (pthi-ptlo);
	    double _thi = ptlo + tNext * (pthi-ptlo);

	    glBindTexture(GL_TEXTURE_2D,parentID);
	    glColor3f(1,1,1);
	    glBegin(GL_QUADS);
	    glTexCoord2f(_slo,_tlo); glVertex3dv(xyz[0]);
	    glTexCoord2f(_slo,_thi); glVertex3dv(xyz[1]);
	    glTexCoord2f(_shi,_thi); glVertex3dv(xyz[2]);
	    glTexCoord2f(_shi,_tlo); glVertex3dv(xyz[3]);
	    /*
	    glTexCoord2f(pslo,ptlo); glVertex3dv(xyz[0]);
	    glTexCoord2f(pslo,pthi); glVertex3dv(xyz[1]);
	    glTexCoord2f(pshi,pthi); glVertex3dv(xyz[2]);
	    glTexCoord2f(pshi,ptlo); glVertex3dv(xyz[3]);
	    */
	    glEnd();	    
	  } else {
	    glBindTexture(GL_TEXTURE_2D,p->itsID);
	    glColor3f(1,1,1);
	    glBegin(GL_QUADS);
	    glTexCoord2f(sLo,tLo); glVertex3dv(xyz[0]);
	    glTexCoord2f(sLo,tHi); glVertex3dv(xyz[1]);
	    glTexCoord2f(sHi,tHi); glVertex3dv(xyz[2]);
	    glTexCoord2f(sHi,tLo); glVertex3dv(xyz[3]);
	    glEnd();
	  }

	  if( showDebug ){
	    setColor(levelOfDetail); 
	    glDisable(GL_TEXTURE_2D);
	    glDisable(GL_DEPTH_TEST);

	    //glPushMatrix();
	    //glScalef(1.01,1.01,1.01);
	    glBegin(GL_LINE_LOOP);
	    glVertex3dv(xyz[0]);
	    glVertex3dv(xyz[1]);
	    glVertex3dv(xyz[2]);
	    glVertex3dv(xyz[3]);
	    glEnd();
	    //glPopMatrix();
	    glEnable(GL_TEXTURE_2D);
	    glEnable(GL_DEPTH_TEST);
	  }

	} // for int lng
      } //for int lat
    } // else minCurvature==true
  }

  glDisable(GL_TEXTURE_2D);

  if( showLL ){
    for(int i=0;i<36;++i){
      glLineWidth(1);
      glColor3f(0.2,0.2,0.2);
      drawLatArc(d2r(i*10),d2r(-90),d2r(90));
    }
    for(int i=-90;i<=90;++i){
      glLineWidth(1);
      glColor3f(0.2,0.2,0.2);
      drawLngArc(d2r(i*10),d2r(0),d2r(360));
    }
  }

  std::vector<Loc*>::iterator liter=locations.begin();
  for(;liter!=locations.end();++liter){
    Loc* loc=*liter;
    double xyz[3];
    double clat = cos(loc->lat);
    double slat = sin(loc->lat);
    double clng = cos(loc->lng);
    double slng = sin(loc->lng);
    glColor3f(1,1,1);
    drawLatArc((loc->lng),(loc->lat-ONEDEG),(loc->lat+ONEDEG));
    drawLngArc((loc->lat),(loc->lng-ONEDEG),(loc->lng+ONEDEG));
    xyz[0] = clat * clng;
    xyz[1] = clat * slng;
    xyz[2] = slat; 
    env->showMessage(loc->name,xyz[0],xyz[1],xyz[2]);
  }

#ifdef SPHEREDEBUG
  if( spheredebug ){
    glDisable(GL_DEPTH_TEST);
    env->showMessage(itoa(levelOfDetail)+" LOD");
    env->showMessage(itoa(lvlpages)+" level pages");
    env->showMessage(itoa(visiblePages.size())+" visible pages");
    env->showMessage(itoa(drawn)+" drawn pages");
    env->showMessage(itoa(rejected)+" rejected pages");
    env->showMessage(itoa(notReady)+" notReady pages");
    env->showMessage(itoa(workQueue.size())+" queue size");
    glEnable(GL_DEPTH_TEST);
  } // spheredebug
#endif

}

void genPlaneEqn(Glx::Vector& p0, 
		 Glx::Vector& p1, 
		 Glx::Vector& p2, 
		 double eqn[4])
{  
  // eye,v1,v2 plane:
  Glx::Vector u=p1-p0;
  Glx::Vector v=p2-p0;
  u.normalize();
  v.normalize();
  Glx::Vector up = Glx::cross(u,v);
  up.normalize();
  double dist = - Glx::dot(p0,up);
  eqn[0]=up[0];
  eqn[1]=up[1];
  eqn[2]=up[2];
  eqn[3]=dist;
}

void 
calcViewParams(glx* env)
{
  Glx::Vector orig,vvec(0,0,-1);

  env->makeCurrent();

  double projHeight = trackball->itsNear*tan(trackball->itsFOV/2.);
  double projWidth = projHeight*env->aspect();
  double projDist=-trackball->itsNear;

  //glGetDoublev(GL_MODELVIEW_MATRIX,(double *)m);
  //glGetDoublev(GL_PROJECTION_MATRIX,(double *)p);
  glGetIntegerv(GL_VIEWPORT, vp);

  //Glx::inv4x4(m,invM);
  double invM[16];
  Glx::inv4x4(trackball->view,invM);
  Glx::xformVec(orig,invM,eye);
  Glx::xformVec(vvec,invM,gaze);
  
  /*
     double glx::Trackball::frustum[6]; // l,r,b,t,near,far
  */
  double l=trackball->frustum[0];
  double r=trackball->frustum[1];
  double b=trackball->frustum[2];
  double t=trackball->frustum[3];
  double n=trackball->frustum[4];
  double f=trackball->frustum[5];

  Glx::Vector v1( l, t,-n);
  Glx::Vector v2( r, t,-n);
  Glx::Vector v3( r, b,-n);
  Glx::Vector v4( l, b,-n);
  /*
  Glx::Vector v1(-projWidth, projHeight,-trackball->itsNear);
  Glx::Vector v2( projWidth, projHeight,-trackball->itsNear);
  Glx::Vector v3( projWidth,-projHeight,-trackball->itsNear);
  Glx::Vector v4(-projWidth,-projHeight,-trackball->itsNear);
  */
  Glx::xformVec(v1,invM,viewWindow[0]);
  Glx::xformVec(v2,invM,viewWindow[1]);
  Glx::xformVec(v3,invM,viewWindow[2]);
  Glx::xformVec(v4,invM,viewWindow[3]);

  VAR4x4("v",trackball->view);
  VAR4x4("invM",invM);
  VAR(eye);
  VAR(gaze);
  VAR(viewWindow[0]);
  VAR(viewWindow[1]);
  VAR(viewWindow[2]);
  VAR(viewWindow[3]);

  int xLo=vp[0];
  int yLo=vp[1];
  int xHi=vp[0]+vp[2];
  int yHi=vp[1]+vp[3];

  genPlaneEqn(eye,viewWindow[0],viewWindow[1],clip[0]);
  genPlaneEqn(eye,viewWindow[1],viewWindow[2],clip[1]);
  genPlaneEqn(eye,viewWindow[2],viewWindow[3],clip[2]);
  genPlaneEqn(eye,viewWindow[3],viewWindow[0],clip[3]);  
}

void ping(glx* env)
{
  char c=' ';
  ssize_t res = send(ssock, &c, sizeof(char),MSG_WAITALL);
  if( res<=0 ){
    _MESG("send() failed server died");
    perror("send() failed server died");
    Net::setDone();
    env->setDone();  
  } 
}

void wait(glx* env, void*)
{
  _MESG("wait");
  ping(env);
  char mesg;
  ssize_t res = recv(ssock,&mesg,sizeof(char),MSG_WAITALL);
  if(res<=0){
    _MESG("recv() failed server died");
    perror("recv() failed: server died");
    Net::setDone();
    env->setDone();    
  }
  _MESG("wait [done]");
}

void draw(glx* env, void* user)
{
  ++frameno;
  glDisable(GL_LIGHTING);
  switch( sphereMode ){
    case true:
      calcViewParams(env);
      adjustSphereLOD(env);
      drawSphere(env,user);
      break;
    case false:  
      adjustLOD(env);
      drawFlat(env,user);
      break;      
  }

  /*
  ostringstream ostr;
  ostr<<"Frame ["<<frameno<<"]";
  env->showMessage(ostr.str());
  ostr.str("");

  ostr<<"GAP ["<<hgap<<","<<vgap<<"]";
  env->showMessage(ostr.str());
  ostr.str("");
  */

  if( showFilename ){
    if( (frustumRows>0 || frustumCols>0) && (itsRow!=0 || itsCol!=0) )
	return;
    env->showMessage(filename);
  }
  if(VERBOSE){
    if( workQueue.size()==0 ) 
      VERBOSE=false;
    else
      env->wakeup();
  }

}

// called by reader thread
void
genTexture(Raw::Page* page)
{
  if(! env ) return;
  if(VERBOSE){
    _MESGVAR("genTexture",page->itsCoord);
  }  
  env->makeCurrent();
  assert(page);
  assert(page->itsPixels);
  assert(page->itsID==0);
  if( memFull ){
    if(VERBOSE){
      _MESG("genTexture: memfull!");
    }
    if( availTex.size()==0 ){
      if(VERBOSE){
	_MESG("genTexture: memFull! but no reusable texture memory");
      }      
      return;
    }
    unsigned int texid = availTex.front();
    assert(texid!=0);
    availTex.pop();
    glBindTexture(GL_TEXTURE_2D,texid);

    int levelOfDetail=0;
    int border=0;   
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texMode);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texMode);   
    glTexImage2D(GL_TEXTURE_2D, levelOfDetail, 
		 readerThread->sourceFormat(), 
		 readerThread->pageWidth(), 
		 readerThread->pageHeight(), 
		 border, 
		 readerThread->sourceFormat(), 
		 readerThread->sourceType(), 
		 page->itsPixels);
    page->itsID=texid;
    return;
  }

  GLuint thisID;
  checkError("before glGenTextures()");
  glGenTextures(1,&thisID);
  if( checkError("glGenTextures()") == GL_NO_ERROR ){
    glBindTexture(GL_TEXTURE_2D,thisID);
    if( checkError("glBindTexture(GL_TEXTURE_2D)") == GL_NO_ERROR )
    {
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texMode);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texMode);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

      int levelOfDetail=0;
      int border=0;

      glTexImage2D(GL_TEXTURE_2D, levelOfDetail, 
		   readerThread->sourceFormat(), 
		   readerThread->pageWidth(), 
		   readerThread->pageHeight(), 
		   border, 
		   readerThread->sourceFormat(), 
		   readerThread->sourceType(), 
		   page->itsPixels);
      if( checkError("glTexImage2D()")==GL_NO_ERROR){
	assert(thisID!=0);
	page->itsID = thisID;
	if( ! memFull ){
	  ++maxResident;
	  if( maxResident >= maxTexPages ){
	    MESGVAR("genTexture: memfull 1st time",maxResident);
	    MESGVAR("genTexture: ",maxTexPages);
	    MESGVAR("genTexture: ",Raw::MAX_TEX_PAGES);
	    MESGVAR("genTexture: ",availTex.size());
	    MESGVAR("genTexture: ",countNumTextures());
	    memFull=true;

	    assert((availTex.size()+countNumTextures())>=maxTexPages);
	    //assert((availTex.size()+countNumTextures())>=Raw::MAX_TEX_PAGES);
	  }
	} // if ! memFull
      } 
      else // glTexImage failed
      {
	plock();
	cerr << "glTexImage2D failed" << endl;
	punlock();
      }
    } 
    else // glBindTexture failed
    {
      plock();
      cerr << "glBindTexture failed" << endl;
      punlock();
    }
  } 
  else // glGenTextures failed
  {
    plock();
    cerr << "glGenTextures failed" << endl;
    punlock();
  }

}

void
pageLoaded(Raw::PageReader*, Raw::Page* page, void* user)
{
  MESGVAR("REC",page->itsCoord);
  VAR(workQueue.size());
  if(VERBOSE){
    _MESGVAR("pageLoaded",page->itsCoord);
    _MESGVAR("pageLoaded",page->itsID);
    _MESGVAR("pageLoaded",workQueue.size());
    _MESGVAR("pageLoaded",(void*)page->itsPixels);
  }
  
  pageMgr->registerPage(page);
  if( env ) env->wakeup();
}

void
pixelToWorldCoords(glx* env, int x, int y, float* xf, float* yf)
{
  int viewport[4];
  double modelMatrix[16],projMatrix[16],inv[16];
  Glx::Vector v;
  env->makeCurrent();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  trackpad->setProjection(env,trackpad);
  glGetDoublev(GL_MODELVIEW_MATRIX,(double *)modelMatrix);
  glGetDoublev(GL_PROJECTION_MATRIX,(double *)projMatrix);
  glGetIntegerv(GL_VIEWPORT, viewport);
  glPopMatrix();
  Glx::mult4x4(modelMatrix, projMatrix, inv);
  if (! Glx::inv4x4(inv, inv))
    return;
  v[0]=(x - viewport[0]) * 2.0f / (float)viewport[2] - 1.0;
  v[1]=((viewport[3]-y)-viewport[1])*2.0f/(float)viewport[3]-1.0;
  v[2]=-1.0f;
  v[3]=1.0f;
  Glx::xformVec(v,inv,v);
  if( v[3] == 0 ){
    //FAILURE
    *xf=0.0f;
    *yf=0.0f;
  }
  *xf=v[0];
  *yf=v[1];
}


void idleMouse(glx* env,int x,int y,void* user)
{
  pixelToWorldCoords(env,x,y,&mouseX,&mouseY);
  if( env) env->wakeup();
}

void drawLatArc(double lng, double loLat, double hiLat)
{
  double del=0.017453292519943; // 1 deg
  int n=static_cast<int>(ceil(hiLat-loLat)/del);
  double clng = cos(lng);
  double slng = sin(lng);

  glBegin(GL_LINE_STRIP);
  for(int i=0;i<n;++i){
    double lat = loLat + i*del;
    LIMIT(lat,hiLat);
    double clat = cos(lat);
    double slat = sin(lat);
    double xyz[3];
    xyz[0] = clat * clng;
    xyz[1] = clat * slng;
    xyz[2] = slat;  
    glVertex3dv(xyz);
  }
  glEnd();
}

void drawLngArc(double lat, double loLng, double hiLng)
{
  double del=0.017453292519943; // 1 deg
  int n=static_cast<int>(ceil(hiLng-loLng)/del);
  double clat = cos(lat);
  double slat = sin(lat);

  glBegin(GL_LINE_STRIP);
  for(int i=0;i<n;++i){
    double lng = loLng + i*del;
    LIMIT(lng,hiLng);
    double clng = cos(lng);
    double slng = sin(lng);
    double xyz[3];
    xyz[0] = clat * clng;
    xyz[1] = clat * slng;
    xyz[2] = slat;  
    glVertex3dv(xyz);
  }
  glEnd();
}

void drawBbox(Glx::Vector& lo, Glx::Vector& hi)
{    
  glColor3f(1,1,1);
  glBegin(GL_LINES);
  glVertex3d(lo[0],lo[1],lo[2]);
  glVertex3d(hi[0],lo[1],lo[2]);
  
  glVertex3d(lo[0],hi[1],lo[2]);
  glVertex3d(hi[0],hi[1],lo[2]);
  
  glVertex3d(lo[0],hi[1],hi[2]);
  glVertex3d(hi[0],hi[1],hi[2]);
  
  glVertex3d(lo[0],lo[1],hi[2]);
  glVertex3d(hi[0],lo[1],hi[2]);
  //////////////
  glVertex3d(lo[0],lo[1],lo[2]);
  glVertex3d(lo[0],hi[1],lo[2]);
  
  glVertex3d(hi[0],lo[1],lo[2]);
  glVertex3d(hi[0],hi[1],lo[2]);
  
  glVertex3d(hi[0],lo[1],hi[2]);
  glVertex3d(hi[0],hi[1],hi[2]);
  
  glVertex3d(lo[0],lo[1],hi[2]);
  glVertex3d(lo[0],hi[1],hi[2]);
  //////////////
  glVertex3d(lo[0],lo[1],lo[2]);
  glVertex3d(lo[0],lo[1],hi[2]);
  
  glVertex3d(hi[0],lo[1],lo[2]);
  glVertex3d(hi[0],lo[1],hi[2]);
  
  glVertex3d(hi[0],hi[1],lo[2]);
  glVertex3d(hi[0],hi[1],hi[2]);
  
  glVertex3d(lo[0],hi[1],lo[2]);
  glVertex3d(lo[0],hi[1],hi[2]);
  glEnd();
}

#ifdef SPHEREDEBUG
int crow=0,ccol=0;
void dbdraw(glx* env, void* user)
{
  if(!sphereMode) return;
  double xyz[4][3];

  // eye,v3,v4 plane:
  Glx::Vector u=viewWindow[2]-eye;
  Glx::Vector v=viewWindow[3]-eye;

  u.normalize();
  v.normalize();

  Glx::Vector up = Glx::cross(u,v);
  up.normalize();

  double dist = - Glx::dot(eye,up);
  double r = 1. - dist*dist;

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  
  glBegin(GL_TRIANGLES);
  glNormal3fv(up);
  glVertex3fv(eye);
  glVertex3fv(eye+u);
  glVertex3fv(eye+v);
  glEnd();
  glDisable(GL_LIGHTING);

  glColor3f(1,1,1);
  glLineWidth(4);
  glBegin(GL_LINES);
  glVertex3f(0,0,0);
  glVertex3f(-dist*up[0],-dist*up[1],-dist*up[2]);
  glEnd();
  glLineWidth(1);

  map<int,int>::iterator miter=invisible.begin();
  for( ; miter != invisible.end() ; ++miter ){
    int idx = (*miter).first;
    int err = (*miter).second;
    Raw::Page* p = pageMgr->getPage(idx); 
    double invdeg = dpp[p->itsLevelOfDetail]*Raw::PAGE_SIZE;
    int col=p->itsCoord.first;
    int row=p->itsCoord.second;
    double loLat = minLat + row * invdeg;
    double hiLat = loLat + invdeg;
    double loLng = minLng + col * invdeg;
    double hiLng = loLng + invdeg;
    if( hiLat > maxLat ) hiLat=maxLat;
    if( hiLng > maxLng ) hiLng=maxLng;

    calcCoords(loLat, hiLat, loLng, hiLng, xyz, true);
    
    setErrColor(err); 
    drawLatArc(loLng,loLat,hiLat);
    drawLatArc(hiLng,loLat,hiLat);
    drawLngArc(loLat,loLng,hiLng);
    drawLngArc(hiLat,loLng,hiLng);

    double mid[3];
    for(int i=0;i<4;++i){
      mid[0]+=xyz[i][0];
      mid[1]+=xyz[i][1];
      mid[2]+=xyz[i][2];
    }
    mid[0]/=4.;
    mid[1]/=4.;
    mid[2]/=4.;
  }

  Glx::Vector vvec=gaze-eye;
  vvec.normalize();
  glBegin(GL_LINES);
  glVertex3f(eye[0],eye[1],eye[2]);
  glVertex3f(gaze[0],gaze[1],gaze[2]);
  glEnd();

  glBegin(GL_LINE_LOOP);
  glVertex3fv(viewWindow[0]);
  glVertex3fv(viewWindow[1]);
  glVertex3fv(viewWindow[2]);
  glVertex3fv(viewWindow[3]);
  glEnd();

  glDisable(GL_DEPTH_TEST);

  int rows,cols;
  pageMgr->pageCounts(maxLOD,cols,rows);
  Raw::Coord coord(ccol,crow);
  int idx=pageMgr->getPageIndex(maxLOD,coord);
  bbox box=bboxes[idx];
  drawBbox(box.first, box.second);

  for(int row = 0 ; row < rows ; ++row ){
    for(int col = 0 ; col < cols ; ++col ){
      Raw::Coord coord(col,row);
      int idx=pageMgr->getPageIndex(maxLOD,coord);
      Raw::Page* p = pageMgr->getPage(idx); 

      double invdeg = dpp[p->itsLevelOfDetail]*Raw::PAGE_SIZE;
      int col=p->itsCoord.first;
      int row=p->itsCoord.second;
      double loLat = minLat + row * invdeg;
      double hiLat = loLat + invdeg;
      double loLng = minLng + col * invdeg;
      double hiLng = loLng + invdeg;
      calcCoords(loLat, hiLat, loLng, hiLng, xyz);
      double mid[3];
      for(int i=0;i<4;++i){
	mid[0]+=xyz[i][0];
	mid[1]+=xyz[i][1];
	mid[2]+=xyz[i][2];
      }
      mid[0]/=4.;
      mid[1]/=4.;
      mid[2]/=4.;

      int err=invisible[idx];
      ostringstream ostr;
      ostr<<"["<<col<<","<<row<<"]:";
     
      switch( err ){
	case RCBIG: ostr<<"RCBIG"; break;
	case LLBIG: ostr<<"LLBIG"; break;
	case EMPTY: ostr<<"EMPTY"; break;
	case AWAY:  ostr<<"AWAY";  break; 
	case PROJ:  ostr<<"PROJ";  break; 
	case BHND:  ostr<<"BHND";  break; 
	case OFF:   ostr<<"OFF";   break;  
	default:    ostr<<err;     break;
      }
      string mesg=ostr.str();
      setErrMesgColor(env,err);  
      env->showMessage(mesg); 
      env->showMessage(mesg,mid[0],mid[1],mid[2]);

    }
    env->setMessageColor(1,1,1);
  }

  vector<int>::iterator iter = allvisible.begin();
  for( ; iter != allvisible.end() ; ++iter ){
    int i=*iter;
    Raw::Page* p = pageMgr->getPage(i);
    double deg = dpp[p->itsLevelOfDetail]*Raw::PAGE_SIZE;
    int col=p->itsCoord.first;
    int row=p->itsCoord.second;
    double loLat = minLat + row * deg;
    double hiLat = loLat + deg;
    double loLng = minLng + col * deg;
    double hiLng = loLng + deg;
    calcCoords(loLat, hiLat, loLng, hiLng, xyz);

    glPushMatrix();
    double s=1.0 - 0.05*p->itsLevelOfDetail;
    //glScalef(s,s,s);
    setColor(p->itsLevelOfDetail); 
    glBegin(GL_LINE_LOOP);
    glVertex3dv(xyz[0]);
    glVertex3dv(xyz[1]);
    glVertex3dv(xyz[2]);
    glVertex3dv(xyz[3]);
    glEnd();
    glPopMatrix();
  }
  ostringstream ostr;
  ostr<<"DIST: "<<dist<<endl;
  env->showMessage(ostr.str()); 
  ostr.str("");

  ostr<<"ORIGIN:";
  Glx::Vector orig;
  bool out=false;
  for(int j=0;j<4;++j){
    double r=testPoint(orig,clip[j]);
    if( r<0. ) out=true;
    ostr<<r<<" ";
  }
  if( !out ) ostr<<"[visible]\n";
  else ostr<<"[INvisible]\n";
  env->showMessage(ostr.str()); 

  ostr.str("");
  ostr<<"C,R: ["<<ccol<<","<<crow<<"]\n";
  env->showMessage(ostr.str()); 

  glEnable(GL_DEPTH_TEST);
}

void initGL2(glx* env,void* user)
{
  dbtrackball = new Glx::Trackball(env);
  dbtrackball->itsNear=0.01;
  dbtrackball->itsFar=20.;
  env->background(0,0,0);
  env->addDrawFunc(dbdraw);
}
#endif

void pre(glx* env,void* user)
{
  trackball->setSector(itsCol,itsRow,
		       frustumCols,frustumRows,
		       hgap,vgap);
}

void initGL(glx* env,void* user)
{
  FANCYMESG("initGL");

  if( barriermode )
    env->addPostSwapFunc(wait);

  trackball = new Glx::Trackball(env);
  trackball->enable(sphereMode);
  trackball->itsNear=0.01;
  trackball->itsFar=10.;

  trackpad = new Glx::Trackpad(env);
  trackpad->enable(!sphereMode);

  env->addPreDrawFunc(pre,trackball);

  env->addMouseIdleFunc(idleMouse,trackpad);

  trackball->setSector(itsCol,itsRow,
		       frustumCols,frustumRows,
		       hgap,vgap);
  // 153,159);
  trackpad->setSector(itsCol,itsRow,frustumCols,frustumRows);

  while( ! readerThread->ready() )
    ;

  FANCYMESG("readerThread->ready()");
 
  int rows,cols;
  pageMgr->pageCounts(maxLOD,cols,rows);
  for(int r=0 ; r < rows ; ++r){
    for(int c=0 ; c < cols ; ++c ){
      Raw::Coord coord(c,r);
      Raw::Page* p = pageMgr->getPage(maxLOD,coord);
      assert(p);
      workQueue.add( p );
    }
  }

  while( ! workQueue.empty() )
    ;

  FANCYMESG("workQueue.empty()");

  // ok, how big is this in 'real' space
  // a given square = 2 ^ LOD
  // x [real units] = imagewidth/PAGE_SIZE;
  
  double min[2],max[2];
  double x = (double)imageWidth / (double)Raw::PAGE_SIZE;
  double y = (double)imageHeight / (double)Raw::PAGE_SIZE;
  double dx = -x/2.0;
  double dy = -y/2.0;
  double maxDim = (x > y) ? x : y;
  double scale = 1.0/maxDim;
  scale *= 2.5 * frustumCols;

  trackpad->itsScaleFactor = scale;
  trackpad->itsXtrans = dx;
  trackpad->itsYtrans = dy;
  /*
  cout << " levelOfDetail = "<<levelOfDetail<<endl;
  cout << "             x = "<<x<<endl;
  cout << "             y = "<<y<<endl;  
  cout << "            dx = "<<dx<<endl;
  cout << "            dy = "<<dy<<endl;
  cout << "itsScaleFactor = "<<trackpad->itsScaleFactor<<endl;
  cout << "     itsXtrans = "<<trackpad->itsXtrans<<endl;
  cout << "     itsYtrans = "<<trackpad->itsYtrans<<endl;
  */

  env->background(0,0,0);
  env->addDrawFunc(draw);
}

#if 0
void
clearTextures(void)
{
  env->makeCurrent();
  workQueue.clear();
  while( ! workQueue.empty() )
    ;
  vector<GLuint> texids;
  for(int i=0; i<pageMgr->numPages();++i){
    Raw::Page* p = pageMgr->getPage(i);
    assert(p);
    if( p->itsID!=0 ){
      texids.push_back( p->itsID );
      p->itsID=0;
    }
  }
  while(availTex.size()){
    texids.push_back( availTex.front() );
    availTex.pop();
  }
  glDeleteTextures( texids.size(),(GLuint*)&texids[0]);
  memFull=false;
  maxResident=-1;
  int rows,cols;
  pageMgr->pageCounts(maxLOD,cols,rows);
  for(int r=0 ; r < rows ; ++r){
    for(int c=0 ; c < cols ; ++c ){
      Raw::Coord coord(c,r);
      Raw::Page* p = pageMgr->getPage(maxLOD,coord);
      assert(p);
      workQueue.add( p );
    }
  }
  while( ! workQueue.empty() )
    ;  
}
#endif

#ifdef SPHEREDEBUG
void handleEvent2(glx* env, XEvent *event, void*)
{ 
  KeySym sym;
  XComposeStatus status;
  char buffer[20];
  int buf_len = 20;
  XKeyEvent *kep = (XKeyEvent *)(event);
  XLookupString(kep, buffer, buf_len, &sym, &status);
  double c[3];

  int rows,cols;
  pageMgr->pageCounts(maxLOD,cols,rows);
  switch( sym ){
    case XK_1: 
      crow++;
      crow %= rows; 
      _VAR2(crow,rows);
      break;
    case XK_2: 
      crow--;
      if(crow<0) 
	crow+=rows;      
      _VAR2(crow,rows);
      break;
    case XK_3: 
      ccol++;
      ccol %= cols; 
      _VAR2(ccol,cols);
      break;
    case XK_4: 
      ccol--;
      if(ccol<0)
	ccol+=cols;
      _VAR2(ccol,cols);
      break;
      
    case XK_d:{
      dbtrackball->itsXtrans = 0.;
      dbtrackball->itsYtrans = 0.;
      dbtrackball->itsZtrans = 0.;
      dbtrackball->itsCenter[0]=eye[0];
      dbtrackball->itsCenter[1]=eye[1];
      dbtrackball->itsCenter[2]=eye[2];
    }
      break;
    default:
      break;
  }

  env->wakeup();
}
#endif

void handleEvent(glx* env, XEvent *event, void*)
{ 
  KeySym sym;
  XComposeStatus status;
  char buffer[20];
  int buf_len = 20;
  XKeyEvent *kep = (XKeyEvent *)(event);
  XLookupString(kep, buffer, buf_len, &sym, &status);

  switch( sym ){
#ifdef SPHEREDEBUG
    case XK_d:
      spheredebug = !spheredebug;
      inset = spheredebug ? 100 : 0;
      break;
#endif
    case XK_1:
      hgap-= 2;
      vgap-= 2;
      if( hgap<0 ) hgap=0;
      if( vgap<0 ) vgap=0;
      env->wakeup();
      break;
      
    case XK_2:
      hgap+= 2;
      vgap+= 2;
      env->wakeup();
      break;

    case XK_Q:
      {
	VERBOSE=true;
	vector<int>::iterator iter = visiblePages.begin();
	for( ; iter != visiblePages.end() ; ++iter ){
	  int i=*iter;
	  Raw::Page* p = pageMgr->getPage(i); 
	  if( p->itsID==0 || p->itsPixels==NULL ){
	    _MESGVAR("handleEvent: adding to queue",p->itsCoord);
	    workQueue.add( p );	  
	  }
	}
      }
      break;
    case XK_f:
      showFilename = ! showFilename;
      env->wakeup();
      break;      
      
    case XK_l:
      showLL = ! showLL;
      env->wakeup();
      break;
    case XK_c:
      minCurvature = ! minCurvature;
      env->wakeup();
      break;      
    case XK_w:
      wire = ! wire;
      env->wakeup();
      break;
    case XK_r:
      if( sphereMode )
	trackball->reset();
      else
	trackpad->reset();
      env->wakeup();
      break;
    case XK_m:
      sphereMode = ! sphereMode;
      trackball->enable(sphereMode);
      trackpad->enable(!sphereMode);
      if( maxLODobjID ){
	glDeleteLists(maxLODobjID,1);
	maxLODobjID=0;
      }
      env->wakeup();
      break;
    case XK_S:

      if( ! sphereMode ){
	trackpad->itsScaleFactor =
	  env->aspect()/(env->winWidth()/(float)(2*Raw::PAGE_SIZE));
	adjustLOD(env);
	env->wakeup();
      }
      break;
    case XK_t:
      texMode = (texMode==GL_LINEAR) ? GL_NEAREST : GL_LINEAR;
      //clearTextures();
      env->wakeup();
      break;
    case XK_p:
      showDebug = ! showDebug;
      env->wakeup();
      break;
    case XK_q:
      readerThread->stopReaderThread();
      exit(0);
      break;
    case XK_Up:
      {
	int pageCols,pageRows;
	pageMgr->pageCounts(levelOfDetail,pageCols,pageRows);
	if( pageCols>1 || pageRows>1 ){
	  levelOfDetail++;
	  env->wakeup();
	}
      }
      break;
    case XK_Down:
      levelOfDetail--;
      if(levelOfDetail<0)levelOfDetail=0;
      env->wakeup();
      break;
  }
}

void
tokenize(string input, vector<string>& tokens, string sep)
{
  string cur = input;
  int done=0;
  tokens.clear();
  while( ! done ){
    int start = cur.find_first_not_of(sep);
    int end = cur.find_first_of(sep,start+1);
    if( start == -1 || end == -1 ){
      if( start != -1 )
        tokens.push_back( string( cur, start ) );
      return;
    }
    tokens.push_back( string( cur, start, end-start ) );
    cur = string(cur, end+1);
  }
}

int OpenGLKeyword(string str)
{
  // component size...
  if( ! str.compare("GL_BYTE") )           return GL_BYTE;
  if( ! str.compare("GL_UNSIGNED_BYTE") )  return GL_UNSIGNED_BYTE;
  if( ! str.compare("GL_SHORT") )          return GL_SHORT;
  if( ! str.compare("GL_UNSIGNED_SHORT") ) return GL_UNSIGNED_SHORT;
  if( ! str.compare("GL_INT") )            return GL_INT;
  if( ! str.compare("GL_UNSIGNED_INT") )   return GL_UNSIGNED_INT;
  if( ! str.compare("GL_FLOAT") )          return GL_FLOAT;
  if( ! str.compare("GL_DOUBLE") )         return GL_DOUBLE;

  // component format
  if( ! str.compare("GL_RGB") )       return GL_RGB;
  if( ! str.compare("GL_BGR,") )      return GL_BGR;
  if( ! str.compare("GL_RGBA") )      return GL_RGBA;
  if( ! str.compare("GL_BGRA") )      return GL_BGRA;
  if( ! str.compare("GL_LUMINANCE") ) return GL_LUMINANCE;
}

string OpenGLKeyword(int value)
{
  switch(value){
    case GL_BYTE:           return string("GL_BYTE"); break;
    case GL_UNSIGNED_BYTE:  return string("GL_UNSIGNED_BYTE"); break;
    case GL_SHORT:          return string("GL_SHORT"); break;
    case GL_UNSIGNED_SHORT: return string("GL_UNSIGNED_SHORT"); break;
    case GL_INT:            return string("GL_INT"); break;
    case GL_UNSIGNED_INT:   return string("GL_UNSIGNED_INT"); break;
    case GL_FLOAT:          return string("GL_FLOAT"); break;
    case GL_RGB:            return string("GL_RGB"); break;
    case GL_LUMINANCE:      return string("GL_LUMINANCE"); break;
  }
}

void calcBbox(int lod, int row, int col, bbox& box)
{
  double xyz[4][3];  
  int rows,cols;

  pageMgr->pageCounts(lod,cols,rows);
  if( row>=rows || col>=cols ) return;

  Raw::Coord coord(col,row);
  int idx=pageMgr->getPageIndex(lod,coord);
  double deg = dpp[lod]*Raw::PAGE_SIZE;
  double loLat = minLat + row * deg;
  double hiLat = loLat + deg;
  double loLng = minLng + col * deg;
  double hiLng = loLng + deg;
  if( hiLat > maxLat ) hiLat=maxLat;
  if( hiLng > maxLng ) hiLng=maxLng; 
  bbox localbox=make_pair(HIVEC,LOVEC);
  
  if( lod==0 )
  {
    calcCoords(loLat, hiLat, loLng, hiLng, xyz);
    for(int i=0;i<4;++i){
      if( xyz[i][0]<localbox.first[0] )  localbox.first[0] = xyz[i][0];
      if( xyz[i][1]<localbox.first[1] )  localbox.first[1] = xyz[i][1];
      if( xyz[i][2]<localbox.first[2] )  localbox.first[2] = xyz[i][2];
      if( xyz[i][0]>localbox.second[0] )  localbox.second[0] = xyz[i][0];
      if( xyz[i][1]>localbox.second[1] )  localbox.second[1] = xyz[i][1];
      if( xyz[i][2]>localbox.second[2] )  localbox.second[2] = xyz[i][2];
    }
  } 
  else 
  {
    calcBbox(lod-1,row*2,col*2,    localbox);
    calcBbox(lod-1,row*2+1,col*2,  localbox);
    calcBbox(lod-1,row*2,col*2+1,  localbox);
    calcBbox(lod-1,row*2+1,col*2+1,localbox); 
  }  

  bboxes[idx]=localbox;
  if( localbox.first[0]<box.first[0] ) box.first[0] = localbox.first[0];
  if( localbox.first[1]<box.first[1] ) box.first[1] = localbox.first[1];
  if( localbox.first[2]<box.first[2] ) box.first[2] = localbox.first[2];
  if( localbox.second[0]>box.second[0] ) box.second[0] = localbox.second[0];
  if( localbox.second[1]>box.second[1] ) box.second[1] = localbox.second[1];
  if( localbox.second[2]>box.second[2] ) box.second[2] = localbox.second[2];
}

void init(int sock, void*)
{
  _MESG("socket init'd");
  ssock=sock;
}
void service(int sock, void*){}

int
main(int argc, char** argv)
{
  int c;
  int offset=0;
  vector<string> tokens;
  bool fs=false;
  int border=glx::FullBorder;
  string host,posfile;
  int port=0;

  if( argc==1 ){
    cerr << "usage: showPages <options> -w<int> -h<int> <paged rawfile>"<<endl;
    cerr << "       showPages <paged ppmfile> "<< endl;
    cerr << " -l <int>           : min/max lat" <<endl;
    cerr << " -s                 : signed pixels" <<endl;
    cerr << " -F                 : fullscreen\n";
    cerr << " -H <host>          : host [barrier mode]\n";
    cerr << " -P <int>           : port [barrier mode]\n";
    cerr << " -f <string>        : GL_RGB, etc" <<endl;
    cerr << " -t <string>        : GL_UNSIGNED_BYTE, etc" <<endl;
    cerr << " -r int,int,int,int : rows,cols,row,col"<<endl;
    return 0;
  }

  while( (c = getopt(argc,argv,"FH:P:msl:f:t:r:w:h:p:")) != -1){
    switch( c ){
      case 'p':
	posfile=optarg;
	break;
      case 'H':
	host=optarg;
	break;
      case 'P':
	port=atoi(optarg);
	break;
      case 'F':
	fs=true;
	border=glx::NoBorder;
	break;
#if 0
      case 'm':
	pdsFileManager* mgr = new pdsFileManager("/scratch/mars/");
	cout << "reading MOC database..." << flush;
	mocDatabase* mocDB = new mocDatabase(mgr);
	cout << "done" << endl;
	for(mocFile* f=mocDB->begin() ; !mocDB->done() ; f=mocDB->next() ){
	  if( ! f->isWideAngle() )
	    footprints.push_back( new MocFootprint(f) );
	}
	break;
#endif
      case 'l':
	maxLat = atof(optarg)*M_PI_OVER_180;
	minLat = -maxLat;
	break;      
      case 's':
	signedPixels=true;
	break;
      case 'f': // format
	sourceFormat = OpenGLKeyword(optarg);
	break;
      case 't':
	sourceType = OpenGLKeyword(optarg);
	break;
      case 'r':
	tokenize(optarg, tokens, string(","));	
	if (tokens.size() != 4) {
	  cerr << "not enough arguments for -r option" << endl;
	  exit(1);
	}
	frustumRows=atoi(tokens[0].c_str());
	frustumCols=atoi(tokens[1].c_str());
	itsCol=atoi(tokens[2].c_str());
	itsRow=atoi(tokens[3].c_str());
	hgap=vgap=HW2GAP;
	break;
      case 'w':
	imageWidth=atoi(optarg);
	break;
      case 'h':
	imageHeight=atoi(optarg);
	break;
    }
  }
  if( imageWidth==0 && imageHeight==0 ){
    PPM::Format format;
    int cpp,bpc;
    int off;
    try {
      PPM::ppmHeader(argv[optind],&format,&cpp,&bpc,
		     &imageWidth,&imageHeight,&off);
      offset=off;
    } catch( exception& e ){throw;}
    sourceFormat = (cpp==1)?GL_LUMINANCE:GL_RGB;
    switch(bpc){
      case 8:
	sourceType = signedPixels ? GL_BYTE : GL_UNSIGNED_BYTE;
	break;
      case 16:
	sourceType = signedPixels ? GL_SHORT : GL_UNSIGNED_SHORT;
	break;
      default:
	assert(0);
    }
  }
  pageMgr = new Raw::PageManager(imageWidth,imageHeight);

  int rows,cols;
  bool ok=false;
  maxLOD = pageMgr->maxLOD();
  while( ! ok ){
    pageMgr->pageCounts(maxLOD,cols,rows);
    int lvlPages = rows*cols;
    if(lvlPages<40){
      ok=true; 
    } else
      ++maxLOD;
  }

  for(int i=0;i<=maxLOD;++i){
    dpp.push_back(degPerPixel(i));
    VAR2(i,dpp[i]);
  }

  levelOfDetail = maxLOD;
  maxTexPages = Raw::MAX_TEX_PAGES+rows*cols;

  cout << "maxTexPages = " << maxTexPages << endl;
  cout << "Max LOD = " << levelOfDetail << endl;
  cout << endl;

  double dx,dy;
  pageMgr->pageCounts(levelOfDetail,cols,rows);
  totalPages = pageMgr->totalPages();
  visiblePages.reserve(totalPages);
  for(int i=0;i<totalPages;i++) visiblePages.push_back(0);

  _MESG("calculating bboxes");
  bboxes.resize(pageMgr->numPages());
  timer.start();
  double t0=timer.elapsed();
  for(int lod=maxLOD;lod>=0;--lod){
    int rows,cols;
    pageMgr->pageCounts(lod,cols,rows);
    for(int row = 0 ; row < rows ; ++row ){
      for(int col = 0 ; col < cols ; ++col ){
	bbox box;
	calcBbox(lod,row,col,box);
      }
    }
  }
  _MESG("calculating bboxes [done]");

  _MESG("starting reader thread");
  try {
    readerThread = 
      new Raw::PageReader(workQueue,argv[optind],Raw::PAGE_SIZE,true,
			  imageWidth,imageHeight,
			  sourceFormat,sourceType,offset);
  } catch( exception& e ){return 0;}

  filename = argv[optind];

  readerThread->addListener(pageLoaded);

  _MESG("starting reader thread [done]");

  if( host.length()>0 && port !=0 ){
    _MESG("barrier mode enabled");
    barriermode=true;
    Net::runTcpClient(host,port,init,service,NULL);
  }

  env = new glx(initGL,0L,10,10,500,500,border,fs);
  env->showAxis(false);
  env->addEventFunc(handleEvent);

#ifdef SPHEREDEBUG 
  dbenv = new glx(env->getApp(), env->getDisplay(), initGL2);
  dbenv->addEventFunc(handleEvent2);
#endif

  if( posfile.length() ){
    ifstream fin(posfile.c_str());
    if( ! fin )
    {
      perror("error opening posfile");
    } 
    else while( fin )
    {
      char line[256];
      if( fin.getline(line,256) ){
	double lat,lng;
	string name;
	istringstream istr(line);
	istr>>lat>>lng;
	while( istr ){
	  string word;
	  istr>>word;
	  if( istr ) name += " " + word;
	}
	_VAR3(lat,lng,name);
	
	Loc* loc = new Loc;
	loc->name=name;
	loc->lat=d2r(lat);
	loc->lng=d2r(lng);
	locations.push_back(loc);
      }
    }
    fin.close();
  }

  env->mainLoop();
  delete readerThread;
  delete env;
}
