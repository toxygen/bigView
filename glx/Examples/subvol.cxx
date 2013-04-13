#define GL_GLEXT_VERBOSE 1
#define GL_GLEXT_PROTOTYPES 1

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <GLX.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <glxTrackball.h>
#include <unistd.h>
#include <fcntl.h>
#include <values.h>
#include <assert.h>
#include "slicer.h"

#include "debug.h"
using namespace std;

GLuint itsID=0;
double EPS=1e-12;
int octant=0;
bool userMin=false;
double smin=MAXFLOAT,smax=-MAXFLOAT;
void loadCoords(int id, float glo[3],float ghi[3],float lo[3],float hi[3]);

#if 0
void draw(glx* env, void* user)
{
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glColor4f(.2,.3,.4,.5);
  glBegin(GL_QUADS);
  glVertex3f(0,0,0);
  glVertex3f(1,0,0);
  glVertex3f(1,1,0);
  glVertex3f(0,1,0);  
  glEnd();
}
#else
void draw(glx* env, void* user)
{
  Glx::Trackball* tb = static_cast<Glx::Trackball*>(user);
  static GLfloat xequalzero[] = {1,0,0,0};
  static GLfloat yequalzero[] = {0,1,0,0};
  static GLfloat zequalzero[] = {0,0,1,0};
  double m[16],minv[16];
  glGetDoublev(GL_MODELVIEW_MATRIX,(double *)m);
  int N=60;

  float eye[3]={m[0*4+2],m[1*4+2],m[2*4+2]};
  float gmin[3]={0,0,0},gmax[3]={1,1,1};  
  float lo[3],hi[3];  
  float spacing = (float)1./N;
  float dist = 0.5;
  SlicerState slicer(eye, spacing, dist);
  SlicerState::Slice* slice;

  loadCoords(octant,gmin,gmax,lo,hi);
  int n_slices = slicer.calc_slices(lo, hi, &slice);
  //int n_slices = slicer.calc_slices(gmin, gmax, &slice);

  glBindTexture(GL_TEXTURE_3D,itsID);
  glPushAttrib(GL_LIGHTING_BIT|GL_TEXTURE_BIT);
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);
  glEnable(GL_TEXTURE_GEN_R);
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
  glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
  glTexGenfv(GL_S, GL_EYE_PLANE, xequalzero);
  glTexGenfv(GL_T, GL_EYE_PLANE, yequalzero);
  glTexGenfv(GL_R, GL_EYE_PLANE, zequalzero);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_TEXTURE_3D);
  glDisable(GL_LIGHTING);

  glColor3f(1,1,1);
  for (int s = 0; s < n_slices; s++) {
    glBegin(GL_POLYGON);
    int n_verts = slice[s].n_verts;
    float (*v3p)[3] = &slice[s].vert[0];
    for (int v = 0; v < n_verts; v++)
      glVertex3fv(*v3p++);
    glEnd();
  }
}
#endif

#define LIMIT(v,lo,hi) {v = (v<lo) ? lo : ((v>hi) ? hi : v);}
void 
genFast(float percent, float rgb[4])
{
  const float LVL_0 = 0;
  const float LVL_1 = 0.141176;
  const float LVL_2 = 0.282353;
  const float LVL_3 = 0.427451;
  const float LVL_4 = 0.568627;
  const float LVL_5 = 0.713726;
  const float LVL_6 = 0.854902;
  const float LVL_7 = 1;
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
  rgb[3]= 0.025;
}

void loadCoords(int id, float glo[3],float ghi[3],float lo[3],float hi[3])
{
  switch( id ){
    case 0: 
      lo[0]=glo[0];
      lo[1]=glo[1];
      lo[2]=glo[2];
      hi[0]=0.5 * (glo[0]+ghi[0]);
      hi[1]=0.5 * (glo[1]+ghi[1]);
      hi[2]=0.5 * (glo[2]+ghi[2]);
      break;

    case 1:
      lo[0]=0.5 * (glo[0]+ghi[0]);
      lo[1]=glo[1];
      lo[2]=glo[2];
      hi[0]=ghi[0];
      hi[1]=0.5 * (glo[1]+ghi[1]);
      hi[2]=0.5 * (glo[2]+ghi[2]);
      break;

    case 2:
      lo[0]=glo[0];
      lo[1]=0.5 * (glo[1]+ghi[1]);
      lo[2]=glo[2];
      hi[0]=0.5 * (glo[0]+ghi[0]);
      hi[1]=ghi[1];
      hi[2]=0.5 * (glo[2]+ghi[2]);
      break;

    case 3:
      lo[0]=0.5 * (glo[0]+ghi[0]);
      lo[1]=0.5 * (glo[1]+ghi[1]);
      lo[2]=glo[2];
      hi[0]=ghi[0];
      hi[1]=ghi[1];
      hi[2]=0.5 * (glo[2]+ghi[2]);
      break;

    case 4: 
      lo[0]=glo[0];
      lo[1]=glo[1];
      lo[2]=0.5 * (glo[2]+ghi[2]);
      hi[0]=0.5 * (glo[0]+ghi[0]);
      hi[1]=0.5 * (glo[1]+ghi[1]);
      hi[2]=ghi[2];
      break;

    case 5:
      lo[0]=0.5 * (glo[0]+ghi[0]);
      lo[1]=glo[1];
      lo[2]=0.5 * (glo[2]+ghi[2]);
      hi[0]=ghi[0];
      hi[1]=0.5 * (glo[1]+ghi[1]);
      hi[2]=ghi[2];
      break;

    case 6:
      lo[0]=glo[0];
      lo[1]=0.5 * (glo[1]+ghi[1]);
      lo[2]=0.5 * (glo[2]+ghi[2]);
      hi[0]=0.5 * (glo[0]+ghi[0]);
      hi[1]=ghi[1];
      hi[2]=ghi[2];
      break;

    case 7:
      lo[0]=0.5 * (glo[0]+ghi[0]);
      lo[1]=0.5 * (glo[1]+ghi[1]);
      lo[2]=0.5 * (glo[2]+ghi[2]);
      hi[0]=ghi[0];
      hi[1]=ghi[1];
      hi[2]=ghi[2];
      break;
  }
}

int loadRange(int oct,int glo[3],int ghi[3],int lo[3],int hi[3])
{
  switch( oct ){
    case 0: 
      lo[0]=glo[0];
      lo[1]=glo[1];
      lo[2]=glo[2];
      hi[0]=(int)(0.5 * (glo[0]+ghi[0]));
      hi[1]=(int)(0.5 * (glo[1]+ghi[1]));
      hi[2]=(int)(0.5 * (glo[2]+ghi[2]));
      break;

    case 1:
      lo[0]=(int)(0.5 * (glo[0]+ghi[0]));
      lo[1]=glo[1];
      lo[2]=glo[2];
      hi[0]=ghi[0];
      hi[1]=(int)(0.5 * (glo[1]+ghi[1]));
      hi[2]=(int)(0.5 * (glo[2]+ghi[2]));
      break;

    case 2:
      lo[0]=glo[0];
      lo[1]=(int)(0.5 * (glo[1]+ghi[1]));
      lo[2]=glo[2];
      hi[0]=(int)(0.5 * (glo[0]+ghi[0]));
      hi[1]=ghi[1];
      hi[2]=(int)(0.5 * (glo[2]+ghi[2]));
      break;

    case 3:
      lo[0]=(int)(0.5 * (glo[0]+ghi[0]));
      lo[1]=(int)(0.5 * (glo[1]+ghi[1]));
      lo[2]=glo[2];
      hi[0]=ghi[0];
      hi[1]=ghi[1];
      hi[2]=(int)(0.5 * (glo[2]+ghi[2]));
      break;

    case 4: 
      lo[0]=glo[0];
      lo[1]=glo[1];
      lo[2]=(int)(0.5 * (glo[2]+ghi[2]));
      hi[0]=(int)(0.5 * (glo[0]+ghi[0]));
      hi[1]=(int)(0.5 * (glo[1]+ghi[1]));
      hi[2]=ghi[2];
      break;

    case 5:
      lo[0]=(int)(0.5 * (glo[0]+ghi[0]));
      lo[1]=glo[1];
      lo[2]=(int)(0.5 * (glo[2]+ghi[2]));
      hi[0]=ghi[0];
      hi[1]=(int)(0.5 * (glo[1]+ghi[1]));
      hi[2]=ghi[2];
      break;

    case 6:
      lo[0]=glo[0];
      lo[1]=(int)(0.5 * (glo[1]+ghi[1]));
      lo[2]=(int)(0.5 * (glo[2]+ghi[2]));
      hi[0]=(int)(0.5 * (glo[0]+ghi[0]));
      hi[1]=ghi[1];
      hi[2]=ghi[2];
      break;

    case 7:
      lo[0]=(int)(0.5 * (glo[0]+ghi[0]));
      lo[1]=(int)(0.5 * (glo[1]+ghi[1]));
      lo[2]=(int)(0.5 * (glo[2]+ghi[2]));
      hi[0]=ghi[0];
      hi[1]=ghi[1];
      hi[2]=ghi[2];
      break;
  }
}

GLfloat *
loadtex3doctant(char* fname, int oct, int *texwid, int *texht, int *texdepth)
{
  float maxf=1.;
  int lo[3]={0,0,0},hi[3]={0,0,0},dims[4];
  off_t offsets[3];
  int fd = open(fname,O_RDONLY);
  if( fd == -1 ){
    perror("open");
    return 0;
  }
  if( read(fd,dims,4*sizeof(int)) != 4 * sizeof(int)){
    perror("read");
    return 0;
  }

  loadRange(oct,lo,dims,lo,hi);

  int dst[3]={hi[0]-lo[0],hi[1]-lo[1],hi[2]-lo[2]};

  printf("dim: [%d,%d,%d] [src]\n",dims[0],dims[1],dims[2]);
  printf("dim: [%d,%d,%d] [dst]\n",dst[0],dst[1],dst[2]);
  printf(" lo: [%d,%d,%d]\n",lo[0],lo[1],lo[2]);
  printf(" hi: [%d,%d,%d]\n",hi[0],hi[1],hi[2]);

  off_t base = lseek(fd, 0, SEEK_CUR);
  off_t srcline = dims[0];
  off_t srcslab = srcline * dims[1];
  off_t srcblock = srcslab * dims[2];

  off_t dstline = dst[0];
  off_t dstslab = dstline * dst[1];
  off_t dstblock = dstslab * dst[2];

  GLfloat *data = new GLfloat[ dstblock ]; // raw scalar data
  GLfloat *udata = new GLfloat[ 4 * dstblock ]; // RGBA
  assert( data );  
  assert( udata ); 
  
  bool first=true;
  for(int u=lo[2];u<hi[2];++u){
    off_t su = u * srcslab;
    off_t du = (u-lo[2]) * dstslab;
    for(int v=lo[1];v<hi[1];++v){
      off_t sv = v * srcline;
      off_t dv = (v-lo[1]) * dstline;
      off_t src = su + sv + lo[0];
      off_t dst = du + dv + lo[0];

      off_t offset = base + src*sizeof(float);
      off_t res = lseek(fd, offset, SEEK_SET);
      if( first ){
	_VAR2(u,v);
	_VAR2(src,dst);
	_VAR(offset);
	first=false;
      }
      if( res != offset ){
	perror("lseek failed, file read unsuccessful");
	_VAR2(res,offset);
	delete [] data;
	delete [] udata;
	return NULL;
      }
      off_t readsize = dstline*sizeof(float);
      res = read(fd, &data[dst], readsize);
      if( res != readsize ){
	perror("read failed");
	_VAR(lseek(fd, 0, SEEK_CUR));
	_VAR2(readsize,res);
	_VAR2(srcline,srcslab);
	_VAR2(dstline,dstslab);
	return 0;
      }
    }
  }

  close(fd);

  for(int i=0;i<dstblock && ! userMin ;++i){
    if( data[i]<smin ) smin = data[i];
    if( data[i]>smax ) smax = data[i];
  }
  printf("min/max: [%f,%f]\n",smin,smax);

  for(int i=0;i<dstblock;++i){
    int index = i*4;
    float v = (float)(data[i]-smin)/(smax-smin);
    float rgba[4]={0};
    genFast(v,rgba);

    udata[index+0] = rgba[0] * maxf;
    udata[index+1] = rgba[1] * maxf;
    udata[index+2] = rgba[2] * maxf;
    udata[index+3] = rgba[3] * maxf;
  }
  delete [] data;
  *texwid   = dst[0];
  *texht    = dst[1];
  *texdepth = dst[2];
  return udata;
}

void initGL(glx* env, void* user)
{  
  int w,h,d;
  char* fname = static_cast<char*>(user);
  GLfloat *udata = loadtex3doctant(fname,octant,&w,&h,&d);
  if( udata==0 ){
    exit(0);
  }
  env->showAxis(0);

  glGenTextures(1,&itsID);
  glBindTexture(GL_TEXTURE_3D,itsID);

  glTexParameterf(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R, GL_CLAMP);

  glTexParameterf(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);  
  glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glTexImage3D(GL_TEXTURE_3D,0,GL_RGBA,
	       w,h,d,0,GL_RGBA,GL_FLOAT,udata);

  Glx::Trackball* tb = new Glx::Trackball(env);
  double lo[3]={0,0,0},hi[3]={1,1,1};
  //tb->viewAll(lo,hi);
  tb->loadView("dump.view");
  env->addDrawFunc(draw,tb);
  env->background(1,1,1);
  env->background(0.3);
}

static string imageFile;

unsigned int PW=0,PH=0;
unsigned char* pix=0;
float* alpha=0;

void dump(glx* env)
{
  int w = env->winWidth();
  int h = env->winHeight();
  int d = 4;
  int size = w * h * d; // RGBA
  int dims[3]={w,h,d};

  if( PW!=w || PH!=h || pix==0 ){
    delete [] pix;
    delete [] alpha;
    pix = new unsigned char[w*h*d];
    alpha = new float[w*h*1];
    PW=w;PH=h;
  }
  glReadBuffer(GL_FRONT);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_ROW_LENGTH,w); 
  glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,pix);
  glReadPixels(0,0,w,h,GL_ALPHA,GL_FLOAT,alpha);
  vector<unsigned char> found;
  for(int i=0;i<w*h;++i){
    vector<unsigned char>::iterator iter = found.begin();
    bool add=true;
    for( ; iter != found.end() && add; ++iter )
      if( fabs((double)pix[i*4+3]-*iter)<1 )
	add=false;
    if( add )
      found.push_back( pix[i*4+3] );  
  }

  cout <<"# of alpha values found: "<<found.size()<<endl;

  //std::string fname = env->nextFilename("dump","rgba");
  std::string fname = imageFile + ".rgba";
  cout << "=== " << fname << " ===" << endl;
  cout << w << " x " << h << " x " << d << " = " <<size<<endl;

  ofstream fout;
  fout.open(fname.c_str());
  fout.write((char*)&dims[0],3*sizeof(int));
  fout.write((char*)pix,w*h*d*sizeof(unsigned char));
  fout.close();

  fname = imageFile + ".alpha";
  cout << "=== " << fname << " ===" << endl;
  cout << w << " x " << h << " x " << 1 << " = " <<size<<endl;

  dims[2]=1; // [X,Y,1]
  fout.open(fname.c_str());
  fout.write((char*)&dims[0],3*sizeof(int));
  fout.write((char*)alpha,w*h*1*sizeof(float));
  fout.close();
}

static int HACK=0;
void
save(glx* env, void* user)
{
  if( ++HACK==3 ){
    dump(env);
    exit(0);
  } else
    env->wakeup();
}

int main(int argc, char** argv)
{
  int c;
  bool dumpAndExit=false;
  
  if( argc==1 ){
    cerr << "usage: vol [options] <[2^N x 2^N x 2^N] plot3d scalar file>\n"
	 << "=== [options] ===\n"
	 << "-o <int>   : octant [0..7]\n"
	 << "-l <float> : scalar min\n"
	 << "-h <float> : scalar max\n"
	 << "\n";
    return 0;
  }

  char options[] = {"do:l:h:"};
  while( (c = getopt(argc,argv,options)) != -1){
    switch( c ){
      case 'd':	
	dumpAndExit=true;
	break;
      case 'o':
	octant=atoi(optarg);
	_VAR(octant);
	break;
      case 'l':
	userMin=true;
	smin = atof(optarg);
	break;
      case 'h':
	userMin=true;
	smax = atof(optarg);
	break;
    } 
  }
  glx* env = new glx(initGL,argv[optind]);
  if(dumpAndExit){
    ostringstream ostr;
    ostr << argv[optind] <<"."<<octant;
    imageFile = ostr.str();
    env->addPostDrawFunc(save);
  }
  env->mainLoop();
  return 1;
}
