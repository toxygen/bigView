#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <X11/keysym.h>
#include <math.h>
#include <values.h>
#include <vector>
#include <GLX.h>
#include <glxTrackpad.h>
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

vector< float* > data;
int dim=0;

void draw(glx* env, void* user)
{
  glDisable(GL_LIGHTING);
  glPointSize(2);

  glBegin(GL_POINTS);
  vector<float*>::iterator i=data.begin();
  for( ; i!=data.end();++i){
    float* v = *i;
    glVertex3fv((float*)v);
  }
  glEnd();
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
  env->addDrawFunc(draw);
}

main(int argc, char** argv)
{
  vector<string> tokens;
  int c;
  bool normalize=true;
  while( (c = getopt(argc,argv,"d:r:")) != -1){
    switch( c ){
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

  ifstream fin(argv[optind]);
  float* fmin=new float[dim];
  float* fmax=new float[dim];
  for(int i=0; i<dim;i++){
    fmin[i] = MAXFLOAT;
    fmax[i] = -MAXFLOAT;
  }
  while( fin ){
    float* fp = new float[dim];
    for(int i=0; i<dim && fin ; ++i ){
      fin >> fp[i];
      if( fp[i]<fmin[i] ) fmin[i]=fp[i];
      if( fp[i]>fmax[i] ) fmax[i]=fp[i];
    }
    if( fin )
      data.push_back( fp );
  }  
  fin.close();
  if( normalize ){    
    vector<float*>::iterator i=data.begin();
    for( ; i!=data.end();++i){
      float* fp = *i;
      for(int i=0; i<dim ;i++){
	float f = (float)(fp[i]-fmin[i])/(float)(fmax[i]-fmin[i]);
	fp[i] = f;
      }
    }
  }
  env = new glx(initGL);
  env->fullscreen(true);
  env->addEventFunc(handleEvent);
  Glx::Trackpad* trackpad = new Glx::Trackpad(env);
  if( frustumRows != -1 )
    trackpad->setSector(itsCol,itsRow,frustumCols,frustumRows);
  env->mainLoop();
}
