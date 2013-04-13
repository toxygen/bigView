#include <iostream>
#include <fstream>
#include <GLX.h>
#include <glxTrackpad.h>
#include <Draggers/Slider.h>

#include "debug.h"

using namespace std;

vector<int> dims;
vector<int> coords;

double gmin[2]={0,0},gmax[2]={1,1};
float alphaLo=0,alpha=0.05,alphaHi=0.1;

string replace(string src, char schar, char dchar)
{
  char dst[80];
  int di=0;
  for(int si=0;si<src.length();++si){
    if( src[si]==schar )      
      dst[di++] = dchar;
    else
      dst[di++] = src[si];
  }
  dst[di++]='\0';
  return string(dst);
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

string strip(string src, char ch)
{
  char dst[80];
  int di=0;
  for(int si=0;si<src.length();++si){
    if( src[si]!=ch )
      dst[di++] = src[si];
  }
  dst[di++]='\0';
  return string(dst);
}

string strip(string src, string charclass)
{  
  char dst[80];
  int di=0;
  for(int si=0;si<src.length();++si){
    bool found=false;
    for(int j=0;j<charclass.length()&&!found;++j){
      if( src[si]==charclass[j] )
	found=true;      
    }
    if( ! found )
      dst[di++] = src[si];
  }
  dst[di++]='\0';
  return string(dst);
}

void
tokenize(string input, vector<string>& tokens, string sep=" \t\n")
{
  string cur = input;
  int done=0;
  tokens.erase(tokens.begin(),tokens.end());
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

void draw(glx* env, void*)
{
  float pad=0.05;
  float lo=0.+pad,hi=1.-pad;
  float w=(hi-lo);
  float dx=w/(float)(dims.size()-1);
  float z=0,dz=0.05;

  glDisable(GL_LIGHTING);

  glColor3f(0,1,0);
  glBegin(GL_LINES);
  for(int i=0;i<dims.size();++i){
    float x=lo+dx*(float)i;
    glVertex3f(x,lo,z);
    glVertex3f(x,hi,z);

    float dy=w/(float)(dims[i]-1);
    for(int j=0;j<dims[i];++j){
      float y=lo+dy*(float)j;
      glVertex3f(x-dx/5.,y,z);
      glVertex3f(x+dx/5.,y,z);
    }
  }
  for(int i=0;i<dims.size();++i){
    float x=lo+dx*(float)i;
    glVertex3f(x,lo,z);
    glVertex3f(x,hi,z);
  }
  glEnd();
  z-=dz;
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColor4f(1,1,1,alpha);
  for(int i=0;i<coords.size();i+=dims.size()){
    glBegin(GL_LINE_STRIP);    
    for(int j=0;j<dims.size();++j){
      float dy=w/(float)(dims[j]-1);
      float x=lo+dx*(float)j;
      float y=lo+dy*(float)coords[i+j];
      glVertex3f(x,y,z);
    }
    glEnd();    
  }
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
}

void
setAlpha(void* user, float t)
{
  glx* env = static_cast<glx*>(user);
  alpha=t;
  env->wakeup();
}

void initGL(glx* env, void* user)
{
  env->addDrawFunc(draw);
  env->showAxis(false);
  Glx::Trackpad* tp = new Glx::Trackpad(env);
  tp->viewAll(gmin,gmax);

  Glx::Slider* a = new Glx::Slider(env);
  a->setRange(alphaLo,alpha,alphaHi);
  a->setCallback(setAlpha,env);
}

int main(int argc, char** argv)
{
  if( argc==1 ){
    cerr<<"usage: pcoords <pcoords file>"<<endl;
    return 0;
  }
  
  ifstream fin(argv[optind]);
  if( ! fin ){
    perror("open");
    return 0;
  }

  bool first=true;
  while( fin ){
    char buf[256];
    if( fin.getline(buf,256) ){
      vector<string> tokens;
      string line = strip( string(buf),"[]");
      line = replace( line,',',' ');
      _VAR(line);
      tokenize(line,tokens);

      // first is overall PSPACE dims
      if( first ){
	for(int i=0;i<tokens.size();++i)
	  dims.push_back( atoi(tokens[i].c_str()) );
	first=false;
      } else {
	for(int i=0;i<tokens.size();++i)
	  coords.push_back( atoi(tokens[i].c_str()) );
      }
    }
    
  }
  fin.close();
  glx* env = new glx(initGL);
  env->mainLoop();
  return 1;
}
