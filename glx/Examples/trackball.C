#include <GLX.h>
#include <GL/glu.h>
#include <math.h>
#include <unistd.h>
#include <string>
#include <glxTrackball.h>
#include <unistd.h>

using namespace std;

//#define DEBUG 1
#include "debug.h"
Glx::Trackball* trackball =0;

void
draw(glx* env,void*)
{
  glBegin(GL_LINE_LOOP);
  glVertex3f(1,0,0);
  glVertex3f(0,1,0);
  glVertex3f(0,0,1);
  glEnd();
}

void initGL(glx* env,void* user)
{
  env->addDrawFunc(draw);
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

main(int argc, char** argv)
{
  vector<string> tokens;
  int c,frustumRows=1,frustumCols=1,itsCol=0,itsRow=0;
  while( (c = getopt(argc,argv,"d:r:")) != -1){
    switch( c ){
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
  glx* g = new glx(initGL);
  trackball = new Glx::Trackball(g);
  if( frustumRows != -1 )
    trackball->setSector(itsCol,itsRow,frustumCols,frustumRows);
  g->mainLoop();
}
