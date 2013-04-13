#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <values.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <assert.h>
#include <GLX.h>
#include <glxTrackball.h>
#include <X11/keysym.h>
#include "debug.h"

using namespace std;

const int N=180;

float MINLAT=-90.,MAXLAT=90.;
float MINLON=-180.,MAXLON=180.;
float DLAT  = 5.;
float DLON  = 5.;

int nLat=181;
int nLon=361;
float lolat=-90.,hilat=90.;
float lolon=-180.,hilon=180.;
float dLat=5.,dLon=5.;
float wLat=5.,wLon=5.;
vector<float*> coasts;
vector<int> coastsizes;

vector<float*> tracks;
vector<int> tracksizes;

int ti=0;
vector<float*> data;
int datadims[4]={0};
float dmin=MAXFLOAT,dmax=-MAXFLOAT;
vector<float> fignore;
bool animating=false;

GLuint tex=0;
GLuint sobj=0;
GLuint lobj=0;
GLuint cobj=0;
GLuint tobj=0;
GLuint Cobj=0;
GLuint uobj=0;

vector<int*> candidates;

float d2r(float deg)
{
  return deg * (M_PI/180.);
}

void adv(void)
{
  ti += 1;
  ti = ti % data.size();
  _VAR(ti);
}

void dec(void)
{
  ti -= 1;
  if( ti<0 ) ti = data.size()-1;
  _VAR(ti);
}

int mesg=0;

void clamp(float& val, float lo, float hi)
{
  if( val<lo ){
    if(++mesg<10 )
      cerr << val << " clamped to " << lo << endl;
    val = lo;
  } else if (val>hi ){
    if(++mesg<10 )
      cerr << val << " clamped to " << hi<< endl;
    val = hi;
  }
}

void toRGB(float percent)
{
  const float LVL_0 = 0;
  const float LVL_1 = 0.141176;
  const float LVL_2 = 0.282353;
  const float LVL_3 = 0.427451;
  const float LVL_4 = 0.568627;
  const float LVL_5 = 0.713726;
  const float LVL_6 = 0.854902;
  const float LVL_7 = 1;
  float rgb[4]={0};

  assert(0<=percent && percent <= 1.0 );

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
  glColor3fv(rgb);
}

void lltoxyz(float lat, float lon, float xyz[3])
{
  xyz[0] = cos(d2r(lat)) * cos(d2r(lon));
  xyz[1] = cos(d2r(lat)) * sin(d2r(lon));
  xyz[2] = sin(d2r(lat));
}

float getdata(int i, int ilat, int ilon)
{
  float* fp = data[i];
  return fp[ (nLat - ilat - 1) * nLon + ilon ];
}

void drawVertex(float lat, float lon, float s)
{
  float xyz[3];
  toRGB(s);
  lltoxyz(lat, lon, xyz); 
  glVertex3fv(xyz);
}

void drawQuad(int ilat, int ilon)
{
  float flat = (float)lolat + ilat * dLat;
  float flon = (float)lolon + ilon * dLon;
  float raw  = getdata(ti,ilat,ilon);

  clamp(raw,dmin,dmax);
  
  float s = (raw-dmin)/(dmax-dmin);
  
  drawVertex(flat-wLat, flon-wLon, s);
  drawVertex(flat+wLat, flon-wLon, s);
  drawVertex(flat+wLat, flon+wLon, s);
  drawVertex(flat-wLat, flon+wLon, s);
}

void drawCross(int ilat, int ilon)
{
  float flat,flon;
  flat = (float)lolat + (nLat-ilat-1) * dLat;
  glBegin(GL_LINE_STRIP);
  for(int j= -5; j<=5 ;++j){
    flon = (float)lolon + (ilon+j) * dLon;
    drawVertex(flat,flon,1);
  }
  glEnd();

  flon = (float)lolon + (ilon) * dLon;
  glBegin(GL_LINE_STRIP);
  for(int i= -5; i<=5 ;++i){
    flat = (float)lolat + (nLat-ilat-1+i) * dLat;
    drawVertex(flat,flon,1);
  }
  glEnd();
}

void draw(glx* env, void*)
{
  float lat,lon,xyz[3];

  if( ! sobj ) {
    sobj = glGenLists(1);
    glNewList(sobj, GL_COMPILE);

    glPushAttrib(GL_LIGHTING_BIT);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glColor3f(1,1,1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glScalef(0.999,0.999,0.999);
    
    for( lat = MINLAT; lat <= (MAXLAT-DLAT) ; lat += DLAT ){
      glBegin(GL_QUAD_STRIP);
      for( lon = MINLON; lon<= MAXLON ;lon += DLON ){      
	lltoxyz(lat,lon,xyz);
	glNormal3fv(xyz);
	glVertex3fv(xyz);
	lltoxyz(lat+DLAT,lon,xyz);
	glNormal3fv(xyz);
	glVertex3fv(xyz);      
      }
      glEnd();
    }
    glPopMatrix();
    glPopAttrib();
    glEndList();
  }
    
  if( ! lobj ) {
    lobj = glGenLists(1);
    glNewList(lobj, GL_COMPILE);

    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glColor3f(0.6,0.6,0.6);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glScalef(1.003,1.003,1.003);

    for( lat = MINLAT; lat<= MAXLAT ;lat += DLAT ){
      glBegin(GL_LINE_STRIP);
      for( lon = MINLON; lon<= MAXLON ;lon += DLON ){
	lltoxyz(lat,lon,xyz);
	glVertex3fv(xyz);
      }
      glEnd();
    }
    for( lon = MINLON; lon<= MAXLON ;lon += DLON ){
      glBegin(GL_LINE_STRIP);
      for( lat = MINLAT; lat<= MAXLAT ;lat += DLAT ){
	lltoxyz(lat,lon,xyz);
	glVertex3fv(xyz);
      }
      glEnd();
    }
    glPopMatrix();
    glPopAttrib();
    glEndList();
  }  

  if( ! cobj && coasts.size() ){

    cobj = glGenLists(1);
    glNewList(cobj, GL_COMPILE);

    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glColor3f(1,1,1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glScalef(1.003,1.003,1.003);

    vector<float*>::iterator siter = coasts.begin();
    vector<int>::iterator liter = coastsizes.begin();
    
    for( ; siter != coasts.end() ; ++siter , ++liter ){
      int len = *liter;
      float* fp = *siter;
      glBegin(GL_LINE_STRIP);
      for(int i=0;i<len;++i)
	glVertex3fv( &fp[i * 3] );
      glEnd();
    }
    glPopMatrix();
    glPopAttrib();
    glEndList();    
  }

  if( !Cobj && candidates.size() ){
    Cobj = glGenLists(1);
    glNewList(Cobj, GL_COMPILE);

    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glColor3f(1,1,1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glScalef(1.003,1.003,1.003);

    vector<int*>::iterator citer = candidates.begin();
    for( ; citer != candidates.end() ; ++citer ){
      int* iptr = *citer;
      int ilat=iptr[0];
      int ilon=iptr[1];

      drawCross(ilat,ilon);      
    }
    glPopMatrix();
    glPopAttrib();
    glEndList();   
  }

  //if( ! uobj && data.size() ) {
  if( data.size() ) {

    //uobj = glGenLists(1);
    //glNewList(uobj, GL_COMPILE_AND_EXECUTE);

    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glColor3f(1,1,1);

    glBegin(GL_QUADS);
    for(int i=0;i<nLat; ++i){
      for(int j=0;j<nLon; ++j){
	drawQuad(i,j);
      }
    }
    glEnd();

    /*
      DONT DELETE
    for(int i=0;i<nLat; ++i){
      float lat = lolat + i * dLat;
      glBegin(GL_LINE_STRIP);
      for(int j=0;j<nLon; ++j){
	float lon = lolon + j * dLon;
	lltoxyz(lat,lon,xyz);
	glVertex3fv(xyz);
      }
      glEnd();
    }
    for(int j=0;j<nLon; ++j){
      float lon = lolon + j * dLon;
      glBegin(GL_LINE_STRIP);
      for(int i=0;i<nLat; ++i){
	float lat = lolat + i * dLat;
	lltoxyz(lat,lon,xyz);
	glVertex3fv(xyz);
      }
      glEnd();
    }
    */

    glPopAttrib();
    //glEndList();
  }  

  if( ! tobj && tracks.size() ){

    tobj = glGenLists(1);
    glNewList(tobj, GL_COMPILE);

    int lw=1;
    glGetIntegerv(GL_LINE_WIDTH,&lw);
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glColor3f(1,1,1);
    glLineWidth(3);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glScalef(1.003,1.003,1.003);

    vector<float*>::iterator siter = tracks.begin();
    vector<int>::iterator liter = tracksizes.begin();
    
    for( ; siter != tracks.end() ; ++siter , ++liter ){
      int len = *liter;
      float* fp = *siter;
      glBegin(GL_LINE_STRIP);
      for(int i=0;i<len;++i)
	glVertex3fv( &fp[i * 3] );
      glEnd();
    }
    glPopMatrix();
    glPopAttrib();
    glLineWidth(lw);
    glEndList();    
  }

  if( sobj ) glCallList(sobj);
  if( lobj ) glCallList(lobj);
  if( cobj ) glCallList(cobj);
  if( Cobj ) glCallList(Cobj);
  if( uobj ) glCallList(uobj);
  if( tobj ) glCallList(tobj);

  if( env->getAnimation() )
    adv();
}

void processKey(glx* env,XEvent *event,void*)
{
  KeySym ks = XLookupKeysym((XKeyEvent*)event,0);
  XKeyEvent *kep = (XKeyEvent *)(event);
  int ctlDown    = kep->state & ControlMask;
  int shiftDown  = kep->state & ShiftMask;
  int altDown    = kep->state & Mod1Mask;
  bool changed=false;

  switch( ks ){

    case XK_p:
      if( shiftDown ) 
	dec();
      else
	adv();
      changed=true;
      break;
  }
  if( changed )
    env->wakeup();
}

void initGL(glx* env, void* user)
{
  env->addDrawFunc(draw);
  Glx::Trackball* tb = new Glx::Trackball(env);
  env->addEventFunc(processKey);
}

void
tokenize(string input, vector<string>& tokens, string sep=" \t\n")
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

int parse(string fname, vector<float*>& geom, vector<int>& counts)
{
  _FANCYMESG("parse");
  _VAR(fname);

  vector<float> strip;
  ifstream fin(fname.c_str());
  if( ! fin ){
    perror("open");
    return 2;
  }

  bool needStart=true,needClose=false;

  while( fin ){
    char buf[80];
    vector<string> args;
    if( fin.getline(buf,80) ){
      tokenize(buf, args);
      if( args.size()==0 )
	continue;

      if( args.size() == 3 )
      {
        if( needClose ){
	  float* fptr = new float[ strip.size() ];
	  assert(fptr);
	  memcpy(fptr, &strip[0], strip.size()*sizeof(float) );
	  geom.push_back(fptr);
	  counts.push_back( strip.size()/3 );
	  strip.clear();
          needClose=false;
        }
        needStart=true;
      }
      else
      {
        if( needStart )
        {	  
          needStart=false;
          needClose=true;
        }
        // LAT/LON => X/Y
	float lat=atof(args[0].c_str());
	float lon=atof(args[1].c_str());
	float xyz[3];
	lltoxyz(lat,lon,xyz);

	strip.push_back(xyz[0]);
	strip.push_back(xyz[1]);
	strip.push_back(xyz[2]);
      }
    }
  }
  if( needClose ){
    float* fptr = new float[ strip.size() ];
    assert(fptr);
    memcpy(fptr, &strip[0], strip.size()*sizeof(float)  );
    geom.push_back(fptr);
    counts.push_back( strip.size()/3 );
    strip.clear();
    needClose=false;
  }

  fin.close();

}

float* load(string fname, int dims[4])
{
  char buf[80];
  int fd = open(fname.c_str(),O_RDONLY);
  if( fd==-1 ){
    perror("open");
    return 0;
  }
  if( read(fd,dims,4*sizeof(int)) != 4*sizeof(int)){
    perror("read");
    close(fd);
    return 0;
  }
  int N=dims[0]*dims[1]*dims[2];
  float* fp = new float[N];
  if( read(fd,fp,N*sizeof(float)) != N*sizeof(float)){
    perror("read");
    close(fd);
    return 0;
  }  
  close(fd);

  cout << "Finding minmax..." << flush;

  int mesg=0;
  for(int i=0;i<N;++i){
    bool ok=true;
    float EPS = 1e-12;
    vector<float>::iterator fiter = fignore.begin();
    for( ; fiter != fignore.end() && ok ; ++fiter ){
      if( fabs(fp[i] - *fiter)< EPS )
	ok=false;
    }
    if( ok ) {
      if( fp[i]<dmin ) dmin = fp[i];
      if( fp[i]>dmax ) dmax = fp[i];
    } else {
      if( ++mesg <10 )
	cerr << "zeroing: " << fp[i]<< endl;
      fp[i]=0;
    }
  }
  cout << "done!" << endl;
  _VAR2(dmin,dmax);

  /*
  cout << "Normalizing..." << flush;
  float nmin=MAXFLOAT,nmax=-MAXFLOAT;
  for(int i=0;i<N;++i){
    fp[i]=(fp[i]-dmin)/(dmax-dmin);
    if( fp[i]<nmin ) nmin = fp[i];
    if( fp[i]>nmax ) nmax = fp[i];
  }  
  _VAR2(nmin,nmax);
  */

  return fp;
}

int
main(int argc, char** argv)
{ 
  int c;
  string coastfile,trackfile,datafile;
  vector<string> args;
  
  if( argc==1 ){
    cerr << "usage: latlon [opt] <file> ..." << endl
	 << "=== [options] ===" << endl
	 << "-c <file>              : coastlines " << endl
	 << "-t <file>              : tracklines " << endl
	 << "-l \"[lat|lon] <value>\" : lo lat/lon " << endl
	 << "-h \"[lat|lon] <value>\" : hi lat/lon " << endl
	 << "-I <float>             : bad data value" << endl;
    cerr << "Example:" <<endl
	 << "latlon -c /scratch/ev/coast/COAST0.LNS \\\n"
	 << "\t-t francis.track -l\"-20 -20\" -h\"20 20\" -d1\n";
  }
  
  while( (c = getopt(argc,argv,"c:t:l:h:I:C:")) != -1){
    switch( c ){

      case 'c':
	coastfile=optarg;
	break;
      case 't':
	trackfile=optarg;
	break;

      case 'I':
	fignore.push_back(atof(optarg));
	break;

      case 'C':
	tokenize(optarg,args);
	if( args.size()==2 ){
	  int ix=atoi(args[0].c_str());
	  int jx=atoi(args[1].c_str());
	  int* iptr = new int[2];
	  iptr[0]=ix;
	  iptr[1]=jx;
	  candidates.push_back(iptr);
	}
	break;

      case 'l':
	tokenize(optarg,args);
	if( args.size()==2 ){
	  lolat=atof(args[0].c_str());
	  lolon=atof(args[1].c_str());
	}
	break;	
      case 'h':
	tokenize(optarg,args);
	if( args.size()==2 ){
	  hilat=atof(args[0].c_str());
	  hilon=atof(args[1].c_str());
	}
	break;	
    }
  }

  if( coastfile.length() )
    parse(coastfile,coasts,coastsizes);

  if( trackfile.length() )
    parse(trackfile,tracks,tracksizes);

  for(int i=optind;i<argc;++i){
    _VAR(argv[i]);
    float* fp = load(argv[i],datadims);
    if( i==optind ){
      nLat = datadims[1];
      nLon = datadims[0];   
      dLat = (float)(hilat-lolat)/(nLat-1);
      dLon = (float)(hilon-lolon)/(nLon-1);
      wLat = 0.5 * dLat;
      wLon = 0.5 * dLon;
    }
    data.push_back(fp);
  }
  /*
  if( datafile.length() ){
    data = load(datafile,datadims);
    nLat = datadims[1];
    nLon = datadims[0];   
    dLat = (float)(hilat-lolat)/(nLat-1);
    dLon = (float)(hilon-lolon)/(nLon-1);
    wLat = 0.5 * dLat;
    wLon = 0.5 * dLon;
    _VAR2(nLat,nLon);
    _VAR2(dLat,dLon);
    _VAR2(wLat,wLon);
  }
  */

  glx* env = new glx(initGL);
  env->mainLoop();
  return 0;
}
