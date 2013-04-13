#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <X11/keysym.h>
#include <math.h>
#include <values.h>
#include <GLX.h>
#include <glxTrackball.h>
#include <glxTrackpad.h>
#include <Draggers/Slider.h>

#include <debug.h>
using namespace std;

glx* env = 0;
int frustumRows=-1,frustumCols=-1,itsRow=0,itsCol=0;
double itsScaleFactor=1.0;
double itsXtrans=0.0;
double itsYtrans=0.0;
double startTranslateX,startTranslateY,startScaleY;
double sensitivity=2.0;
const double MIN_SCALE = 1.0e-6;
const double MAX_SCALE = 10000.0;

double* glo=0;
double* ghi=0;
vector< vector<float>* > data;
vector< string > files;
int dim=0;

float qmax=0.0005;

float plo=1.,phi=10.,pq=qmax/2.0;

void
setlo(void* user, float t)
{
  plo=t;
}

void
sethi(void* user, float t)
{
  phi=t;
}

void
setq(void* user, float t)
{
  pq=t;
}

void genSliders(glx* env)
{
  int x=30,dx=60;
  
  Glx::Slider* ls = new Glx::Slider(env,Glx::Slider::Y,x,30);x+=dx;
  ls->setRange(1,plo,100);
  ls->setCallback(setlo,ls);

  Glx::Slider* hs = new Glx::Slider(env,Glx::Slider::Y,x,30);x+=dx;
  hs->setRange(1,phi,100);
  hs->setCallback(sethi,hs);

  Glx::Slider* qs = new Glx::Slider(env,Glx::Slider::Y,x,30);x+=dx;
  qs->setRange(0,pq,qmax);
  qs->setCallback(setq,qs);
}

void drawBbox(double* lo, double* hi)
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

void setColor(int c)
{
  switch(c){
    case 0: glColor3f(1,0,0); break; // red
    case 1: glColor3f(0,1,0); break; // green
    case 2: glColor3f(0,0,1); break; // blue
    case 3: glColor3f(1,1,0); break; // yellow
    case 4: glColor3f(1,0,1); break; // magenta
    case 5: glColor3f(0,1,1); break; // teal
    case 6: glColor3f(1,1,1); break; // white
    case 7: glColor3f(1,0,.5); break; // pink
    case 8: glColor3f(1,.5,0); break; // orange
    case 9: glColor3f(1,.5,.5); break; // salmon
    case 10: glColor3f(.5,1,.5); break; // seafoam
    case 11: glColor3f(.5,0,1); break; // grape
  }
}
void setMesgColor(int c)
{
  switch(c){
    case 0: env->setMessageColor(1,0,0); break; // red
    case 1: env->setMessageColor(0,1,0); break; // green
    case 2: env->setMessageColor(0,0,1); break; // blue
    case 3: env->setMessageColor(1,1,0); break; // yellow
    case 4: env->setMessageColor(1,0,1); break; // magenta
    case 5: env->setMessageColor(0,1,1); break; // teal
    case 6: env->setMessageColor(1,1,1); break; // white
    case 7: env->setMessageColor(1,0,.5); break; // pink
    case 8: env->setMessageColor(1,.5,0); break; // orange
    case 9: env->setMessageColor(1,.5,.5); break; // salmon
    case 10: env->setMessageColor(.5,1,.5); break; // seafoam
    case 11: env->setMessageColor(.5,0,1); break; // grape
  }
}


void draw(glx* env, void* user)
{
  glDisable(GL_LIGHTING);
  
  //glPointSize(1);
  float quadratic[3]={0,0,pq};
  glPointSize(plo);
  /*
  glEnable(GL_POINT_SMOOTH);
  glPointParameterf(GL_POINT_SIZE_MIN,plo);
  glPointParameterf(GL_POINT_SIZE_MAX,20.*plo);
  glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION,quadratic);
  */

  glBegin(GL_POINTS);
  vector< vector<float>* >::iterator diter = data.begin();
  for(int c=0 ; diter != data.end() ;++diter,++c ){
    setColor(c);
    vector<float>& vec = **diter;
    int N=vec.size()/dim;
    for(int i=0;i<N;++i){
      int off=i*dim;
      glVertex3fv(&vec[off]);
    }
    
  }
  glEnd();  
  /*
  diter = data.begin();
  for(int c=0 ; diter != data.end() ;++diter,++c ){
    setColor(c);
    vector<float>& vec = **diter;
    int N=vec.size()/dim;
    glBegin(GL_LINE_STRIP);
    for(int i=0;i<N;++i){
      int off=i*dim;
      glVertex3fv(&vec[off]);
    }
    glEnd();  
    
  }
  */
  drawBbox(glo,ghi);

  for(int i=0;i<files.size();++i){
    setMesgColor(i);
    env->showMessage(files[i]);
  }
  
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

void handleEvent(glx* env, XEvent *event, void*)
{
  KeySym sym;
  XComposeStatus status;
  char buffer[20];
  int buf_len = 20;
  XKeyEvent *kep = (XKeyEvent *)(event);
  XLookupString(kep, buffer, buf_len, &sym, &status);
  switch( sym ){
    case XK_r:
      itsScaleFactor=1.0;
      itsXtrans=0.0;
      itsYtrans=0.0;
      sensitivity=2.0;	
      env->wakeup();
      break;
  }
}

void initGL(glx* env,void* user)
{
  genSliders(env);
  env->addDrawFunc(draw);
}

string replace(string src, string charclass, char dchar)
{
  char dst[80];
  int di=0;
  for(int si=0;si<src.length();++si){
    bool found=false;
    for(int j=0;j<charclass.length()&&!found;++j){
      if( src[si]==charclass[j] )
        found=true;
    }
    if( found )
      dst[di++] = dchar;
    else
      dst[di++] = src[si];
  }
  dst[di++]='\0';
  return string(dst);
}

int
main(int argc, char** argv)
{
  Glx::Trackpad* trackpad=0;
  Glx::Trackball* trackball=0;
  vector<string> tokens;
  int c;
  bool normalize=false;

  if( argc==1 ){
    cerr << "usage:" << argv[0] << " [options] <files>" << endl
	 << "=== [options] ===" << endl
	 << "-d<int> : dimension of points" << endl
	 << "-r<int,int,int,int> : cols,rows,col,row" << endl;
    return 0;
  }
  while( (c = getopt(argc,argv,"nd:r:")) != -1){
    switch( c ){
      case 'n':
	normalize=true;
	break;
      case 'd':
	dim = atoi(optarg);
	break;
      case 'r':
	tokenize(optarg, tokens, string(","));	
	if (tokens.size() != 4) {
	  cerr << "not enough arguments for -r option" << endl;
	  return 0;
	}
	frustumCols=atoi(tokens[0].c_str());
	frustumRows=atoi(tokens[1].c_str());
	itsCol=atoi(tokens[2].c_str());
	itsRow=atoi(tokens[3].c_str());	
	break;
    }
  }
  _VAR(dim);
  glo=new double[dim];
  ghi=new double[dim];
  for(int i=0; i<dim;i++){
    glo[i] = MAXFLOAT;
    ghi[i] = -MAXFLOAT;
  }

  for(int file=optind;file<argc;++file){
    _FANCYVAR(argv[file]);
    files.push_back( argv[file] );
    ifstream fin(argv[file]);
    vector<float>* vecptr = new vector<float>;
    vector<float>& vec = *vecptr;

    while( fin ){
      char buf[256];      
      if( fin.getline(buf,256) ){
	vector<string> args;
	string line(buf);
	string s = replace(line,"(,)",' ');
	tokenize(s,args);
	for(int i=0;i<args.size();++i){
	  float val = atof(args[i].c_str());
	  if( val<glo[i] ) glo[i]=val;
	  if( val>ghi[i] ) ghi[i]=val;
	  vec.push_back( val );
	}
      }
      /*
      for(int i=0; i<dim && fin ; ++i ){
	float val;
	fin >> val;
	if( fin ){
	  if( val<glo[i] ) glo[i]=val;
	  if( val>ghi[i] ) ghi[i]=val;
	  vec.push_back(val);
	}
      }
      */
    }  
    fin.close();
    _VAR(vec.size());

    if( normalize ){
      _MESG("normalizing");
      _VAR2V(glo);
      _VAR2V(ghi);
      int n=vec.size()/dim;
      for(int i=0 ; i<n;++i){
	float* fp = &vec[i*dim];
	for(int d=0; d<dim ;d++){
	  float f = (float)(fp[d]-glo[d])/(float)(ghi[d]-glo[d]);
	  fp[d] = f;
	  VAR3(i,d,fp[d]);
	}
      }
    }

    data.push_back(vecptr);
  }

  if(normalize){
    for(int i=0;i<dim;++i){
      glo[i]=0.;
      ghi[i]=1.;
    }
  }

  _VAR3V(glo);
  _VAR3V(ghi);

  env = new glx(initGL);
  //env->fullscreen(true);
  env->addEventFunc(handleEvent);
  switch( dim ){
    case 2:
      trackpad = new Glx::Trackpad(env);
      if( frustumRows != -1 )
	trackpad->setSector(itsCol,itsRow,frustumCols,frustumRows);
      break;
    case 3:
      trackball = new Glx::Trackball(env);
      if( frustumRows != -1 )
	trackball->setSector(itsCol,itsRow,frustumCols,frustumRows);
      trackball->viewAll(glo,ghi);
      //trackball->itsNear=1e-6;
      //trackball->itsFar=1.;
      
      break;
  }
  env->mainLoop();
  return 0;

}
