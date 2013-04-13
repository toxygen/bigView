#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <algorithm>
#include <iterator>
#include <GLX.h>
#include <glxColormap.h>
#include <glxTrackball.h>
#include <assert.h>
#include "debug.h"
#include <Draggers/Slider.h>
#include "adj.h"

using namespace std;

#define CLAMP(_val,_min,_max){ \
  if(_val<_min)_val=_min; \
  if(_val>_max)_val=_max; \
}

const double EPS=1.0e-12;

glx* env=0;
double gmin[3]={MAXFLOAT,MAXFLOAT,MAXFLOAT};
double gmax[3]={-MAXFLOAT,-MAXFLOAT,-MAXFLOAT};

vector<float*> xyz;
int* iblanks=NULL;
float* sptr=NULL;
vector<size_t> off;
Glx::Vector eye;

Glx::Trackball* tb = 0;
GLuint spriteTexture=0;
double dlo=MAXFLOAT,dhi=-MAXFLOAT;

struct Pnt {
  Pnt(void):id(0),d(0.){}
  Pnt(int i) : id(i){}
  Pnt(Pnt& that){
    this->id=that.id;
    this->d=that.d;
  }
  Pnt(const Pnt& that){
    this->id=that.id;
    this->d=that.d;
  }
  int id;
  float d;
};

class PntCompare {
public:
  bool operator()(const Pnt& a1, const Pnt& a2){
    return a1.d > a2.d;
  }
  bool operator()(Pnt& a1, Pnt& a2){
    return a1.d > a2.d;
  }
};

std::vector<Pnt> Pnts;
double qa=1.,qb=0.,qc=0.0,fade=60.,ps=10.,smax=1.;
string userView;

GLuint genSpriteTexture(int N)
{
  GLuint texID;
  float* tex = new float[N*N];
  for(int i=0;i<N;++i){  
    float fi = (float)i/(N-1);
    float di = fabs(fi-0.5);
    for(int j=0;j<N;++j){
      float fj= (float)j/(N-1);
      float dj = fabs(fj-0.5);
      float d=20.*(di*di+dj*dj);
      float factor = exp( -d/2. );
      tex[i*N+j]=factor;
    }
  }

  glActiveTexture(GL_TEXTURE1);
  glGenTextures(1,&texID);
  glBindTexture(GL_TEXTURE_2D,texID);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glTexImage2D(GL_TEXTURE_2D,0,GL_ALPHA,N,N,0,GL_ALPHA,GL_FLOAT,tex);
  assert(glGetError()==GL_NO_ERROR);
  return texID;
}

void distFromEye(Pnt& p)
{
  double dx = xyz[0][p.id]-eye[0];
  double dy = xyz[1][p.id]-eye[1];
  double dz = xyz[2][p.id]-eye[2];
  p.d = sqrt(dx*dx+dy*dy+dz*dz);
  if(p.d<dlo) dlo=p.d;
  if(p.d>dhi) dhi=p.d;
}

int qcomp(const void* p1, const void* p2)
{
  Pnt* pnt1 = (Pnt*)p1;
  Pnt* pnt2 = (Pnt*)p2;
  //return pnt1->d > pnt2->d;  
  return pnt1->d < pnt2->d;  
}

bool needSort=true;
void pre(glx* env, void* user)
{
  Glx::Colormap* cmap = static_cast<Glx::Colormap*>(user);        
  glActiveTexture(GL_TEXTURE0);
  cmap->preRender();
  
  env->getEyePosition(eye);
  if( needSort ){
    dlo=MAXFLOAT;
    dhi=-MAXFLOAT;
    _MESG("std::for_each");
    std::for_each(Pnts.begin(),Pnts.end(),distFromEye);
    //std::sort(Pnts.begin(),Pnts.end(),PntCompare()); 
    _MESG("std::sort");
    qsort(&Pnts[0], Pnts.size(), sizeof(Pnt), qcomp);
    needSort=false;
    _VAR2(dlo,dhi);
    _MESG("std::sort [done]");
  }
  //cmap->setMinmax(dlo,dhi); 
  //cmap->setMinmax(0,200000); 

  /*
  tb->itsNear=10;
  tb->itsFar=100000;
  */

  _FANCYMESG("pre [done]");
}

void
draw(glx* env, void* user)
{
  _FANCYMESG("draw");
  _VAR( Pnts.size());

  GLenum err;
  Glx::Colormap* cmap = static_cast<Glx::Colormap*>(user);
  float quadratic[] =  {qa,qb,qc};
  
  glPushAttrib(GL_LIGHTING_BIT|GL_TEXTURE_BIT);

  glColor3f(1,1,1);
  glPointSize(ps);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
  glDisable(GL_LIGHTING);

  glActiveTexture(GL_TEXTURE1);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D,spriteTexture);
  glTexEnvf( GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE );

  glPointParameterfARB( GL_POINT_SIZE_MIN_ARB, 1. );
  glPointParameterfARB( GL_POINT_SIZE_MAX_ARB, ps );
  glPointParameterfvARB( GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic );
  glPointParameterfARB( GL_POINT_FADE_THRESHOLD_SIZE_ARB, fade );

  glEnable( GL_POINT_SPRITE_ARB ); 
  glBegin( GL_POINTS );
  for(int x=0;x<100;++x){
    float xf=(float)x/100.;
    for(int y=0;y<100;++y){
      float yf=(float)y/100.;
      for(int z=0;z<100;++z){
	float zf=(float)z/100.;
	glMultiTexCoord1f(GL_TEXTURE0,z);
	glVertex3f(xf,yf,zf);
      }
    }
  }
#if 0
  vector<Pnt>::iterator piter = Pnts.begin();
  for(int i=0 ; piter != Pnts.end(); ++piter,++i ){
    //if( i % 100 != 0 ) continue;
    Pnt& p = *piter;
    /*
    float t = (p.d)/(dhi);
    if( i>5000 && i<5010 ){_VAR3(p.d,dhi,t);}
    //float t = (p.d-dlo)/(dhi-dlo);
    //cmap->scalar(t);      
    */
    float t = sptr[p.id]/smax;
    //if( t>0.9 ) continue;
    if( i>5000 && i<5010 ){_VAR2(p.d,t);}
    glMultiTexCoord1f(GL_TEXTURE0,t);
    glVertex3f(xyz[0][p.id],xyz[1][p.id],xyz[2][p.id]);
  }
#endif
  glEnd();
  glDisable( GL_POINT_SPRITE_ARB );
  glPopAttrib();
  assert(glGetError()==GL_NO_ERROR);
  _FANCYMESG("draw [done]");
}

void post(glx* env, void* user)
{
  _FANCYMESG("post");
  Glx::Colormap* cmap = static_cast<Glx::Colormap*>(user);
  glActiveTexture(GL_TEXTURE0);
  cmap->postRender();
  
  tb->itsNear=0.01;
  tb->itsFar=1000;
  
  _FANCYMESG("post [done]");
}

void initGL(glx* env, void*)
{  
  new Glx::AdjParam(env,"qa",qa,0.,20.);
  //new Glx::AdjParam(env,"qb",qb,0.,0.05);
  //new Glx::AdjParam(env,"qc",qc,0.,0.005);
  new Glx::AdjParam(env,"qb",qb,0.,20.0);
  new Glx::AdjParam(env,"qc",qc,0.,20.0);
  new Glx::AdjParam(env,"ps",ps,1.,200.);
  new Glx::AdjParam(env,"fade",fade,0.,200.);
  new Glx::AdjParam(env,"smax",smax,0.,10.);
  new Glx::AdjParam(env,"dhi",dhi,100.,100000.);

  tb = new Glx::Trackball( env );

  spriteTexture = genSpriteTexture(32);

  glActiveTexture(GL_TEXTURE0);
  Glx::Colormap* cmap = new Glx::Colormap();
  for(int i=0;i<cmap->getSize();++i){
    float tf = (float)i/(cmap->getSize()-1);
    float* pix = cmap->operator[](i);
    if( tf<0.5 )
      pix[3]=0.;
    else      
      pix[3]=0.5 + tf * 0.5;
    VAR4V(pix);
  }
  cmap->endFill();

  if( userView.length() )
    tb->loadView(userView);

  env->addPreDrawFunc(pre,cmap);
  env->addDrawFunc(draw,cmap);
  env->addPostDrawFunc(post,cmap);

}

static void 
swap32(uint32_t* buf, int numWords)
{
  register int i;
  register uint32_t* src = buf;

  for(i = 0 ; i < numWords ; i++, src++ ){

    *src = ((*src & 0x000000ff) << 24) |
      ((*src & 0x0000ff00) << 8) |
      ((*src & 0x00ff0000) >> 8) |
      ((*src & 0xff000000) >> 24);
  }
}

int
main(int argc, char** argv)
{
#if 0
  int c,readSize=4; // p3d
  bool blanked=false,multi=false,swap=false;

  if( argc == 1 ){
    cerr << "usage: mesh scalar" << endl;
    return 0;
  }
  
  while( (c = getopt(argc, argv, "smiv:")) != -1 ){
    switch( c ){
      case 's':
	swap=true;
	break;
      case 'v':
	userView = optarg;
	break;
      case 'i':
	blanked=true;
	break;
      case 'm':
	multi=true;
	break;
      default: break;
    }
  }

  vector<int*> dims;

  // read mesh
  int fd = open(argv[optind],O_RDONLY);
  if( fd==-1 ){
    perror("open() failed");
    return 0;
  }
  
  size_t res=0,nodes=0,ask=0;
  uint32_t nZones=1;

  if( multi ){
    res = read(fd,&nZones,sizeof(int));
    if( swap ) swap32(&nZones,1);
    if( res != sizeof(int) ){
      perror("read() failed");
      return 0;
    }
    _VAR(nZones);
  }

  for(uint32_t i=0;i<nZones;++i){
    uint32_t d[3];
    res = read(fd,d,3*sizeof(int));
    if( res != 3*sizeof(int)){
      perror("read() dims failed");
      return 0;
    }
    if( swap ) swap32(d,3);
    int* dp=new int[3];
    memcpy(dp,d,3*sizeof(int));
    dims.push_back( dp );
    off.push_back( nodes );
    
    _VAR4(i,d[0],d[1],d[2]);
    nodes += d[0]*d[1]*d[2];
  }
  off.push_back( nodes );
  _VAR(nodes);
  float* x = new float[nodes];
  if( x==NULL ){
    perror("new() [x] failed");
    return 0;
  }
  xyz.push_back(x);
  
  float* y = new float[nodes];
  if( y==NULL ){
    perror("new() [y] failed");
    return 0;
  }
  xyz.push_back(y);
  
  float* z = new float[nodes];
  if( z==NULL ){
    perror("new() [z] failed");
    return 0;
  }
  xyz.push_back(z);

  if( blanked ){
    iblanks = new int[nodes];
    if( iblanks==NULL ){
      perror("new() [iblanks] failed");
      return 0;
    }
  }

  for(int i=0;i<nZones;++i){
    _MESGVAR("reading zone",i);
    uint32_t N=off[i+1]-off[i];
    ask = N*sizeof(float);
    res = read(fd,&x[ off[i] ], ask );
    if( res != ask ){
      perror("read() [x] failed");
      return 0;
    }
    if( swap ) swap32( (uint32_t*)&x[off[i]],N);

    res = read(fd,&y[ off[i] ], ask );
    if( res != ask ){
      perror("read() [y] failed");
      return 0;
    }
    if( swap ) swap32( (uint32_t*)&y[off[i]],N);
    res = read(fd,&z[ off[i] ], ask );
    if( res != ask ){
      perror("read() [z] failed");
      return 0;
    }
    if( swap ) swap32( (uint32_t*)&z[off[i]],N);
    if( blanked ){
      res = read(fd,&iblanks[ off[i] ], ask );
      if( res != ask ){
	perror("read() [z] failed");
	return 0;
      }
      if( swap ) swap32( (uint32_t*)&iblanks[off[i]],N);
    }
  }
  close(fd);

  Pnts.resize(nodes);
  for(int i=0;i<nodes;++i){
    Pnts[i].id=i;
  }

  // read scalar
  fd = open(argv[optind+1],O_RDONLY);
  if( fd==-1 ){
    perror("open() failed");
    return 0;
  }
  
  if( multi ){
    uint32_t nz;
    res = read(fd,&nz,sizeof(int));
    if( res != sizeof(int) ){
      perror("read() failed");
      return 0;
    }
    if( swap ) swap32(&nz,1);
    assert(nz==nZones);
    _VAR(nZones);
  }

  for(uint32_t i=0;i<nZones;++i){
    uint32_t d[4];
    res = read(fd,d,4*sizeof(int));
    if( res != 4*sizeof(int)){
      perror("read() dims failed");
      return 0;
    }
    if( swap ) swap32(d,4);
  }
  sptr = new float[nodes];
  if( sptr==NULL ){
    perror("new() [s] failed");
    return 0;
  }

  for(int i=0;i<nZones;++i){
    uint32_t N=off[i+1]-off[i];
    ask = N*sizeof(float);
    res = read(fd,&sptr[off[i]], ask);
    if( res != ask ){
      perror("read() [x] failed");
      return 0;
    }
    if( swap ) swap32( (uint32_t*)&sptr[off[i]],N);
  }
  close(fd);
#endif

  env = new glx(initGL);
  env->mainLoop();
}
