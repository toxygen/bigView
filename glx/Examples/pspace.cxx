#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <GLX.h>
#include <glxTrackpad.h>

#include "NodeBlock.h"
#include <iTuple.h>

#include "debug.h"

using namespace std;

vector<int> dims;
vector<iTuple> tuples;
set<string> known;

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

string tostring(iTuple& coord)
{
  int N=coord.coord.size()-1;
  string res;
  for(int i=0;i<N;++i)
    res += itoa(coord[i])+"_";
  res += itoa(coord[N]);
  return res;
}

void setColor(void*, iTuple& coord, float color[3])
{  
  string key = tostring(coord);
  set<string>::iterator siter=known.find(key);
  if( siter != known.end()){
    color[0]=1.;
    color[1]=color[2]=0.;
  } else {
    color[0]=color[1]=color[2]=0.1;
  } 
}

iTuple centercoord;
int rows=0,cols=0,plots=0;
int p1=0,p2=6;
int nplots=1; 

void draw(glx* env, void*)
{
  ostringstream ostr;

  ostr<<"params: "
      <<p1<<"["<<iTuple::idims[p1]<<"] vs "
      <<p2<<"["<<iTuple::idims[p2]<<"]";
      
  env->showMessage(ostr.str());
  ostr.str("");

  ostr<<"nPlots = "<<nplots;
  env->showMessage(ostr.str());
  ostr.str("");  
}

void rt2xy(float r, float t, float& x, float& y)
{
  x = r * cos(t);
  y = r * sin(t);
}

void initCoord(iTuple& coord)
{
  vector<int> c;
  c.resize( dims.size() );
  c.assign(dims.size(),0);
  coord=iTuple( c );
}

typedef void(*TupleFunc)(iTuple&,void*);

void showTuple(iTuple& t, void*)
{
  _VAR(t);
}

void genPlot(iTuple& t, void* user)
{
  glx* env=static_cast<glx*>(user);
  int row=plots/cols;
  int col=plots%cols;
  _VAR(t);
  _MESGVAR2("genPlot",row,col);
  float x=col,y=row,w=1.,h=1.;
  Glx::NodeBlock* nb = new Glx::NodeBlock(env,t,p1,p2,x,y,w,h);
  nb->setColorfunc(setColor);
  //nb->setCoord(t);

  ++plots;
}

void
enumerate(TupleFunc f, iTuple& tuple, iTuple& mask, void* u,  int i)
{
  if( i==iTuple::idims.size() ) return;
  if( ! mask[i] ){
    if( i==iTuple::idims.size()-1 )
      f(tuple,u);
    enumerate(f,tuple,mask,u,i+1);
    return;
  }

  for(int j=0;j<iTuple::idims[i];++j){
    tuple[i]=j;
    if( i==iTuple::idims.size()-1 ){
      f(tuple,u);
    }
    enumerate(f,tuple,mask,u,i+1);
  }
}

void initGL(glx* env, void* user)
{
  env->addDrawFunc(draw);
  Glx::Trackpad* tb = new Glx::Trackpad(env);
  float dtheta=(2.*M_PI)/(float)(dims.size()-2); 
  float x=0.,y=0.,w=1.,h=1.;
  //int p1=0,p2=1;

  initCoord(centercoord);
  _VAR(centercoord);
  _VAR(iTuple::idims[p1]);
  _VAR(iTuple::idims[p2]);

  Glx::NodeBlock* nb = new Glx::NodeBlock(env,centercoord,p1,p2,x,y,w,h);
  nb->setColorfunc(setColor);
  nb->setCoord(centercoord);

  iTuple mask,tuple;
  for(int i=0;i<dims.size()-1;++i){  
    mask[i] = ( i==p1 || i==p2 ) ? 0 : 1;
  } 
  for(int i=0;i<dims.size()-1;++i){    
    if( i!=p1 && i!=p2 )
	nplots *= iTuple::idims[i];
  }
  cols=(int)sqrt((double)nplots);
  rows=nplots/cols+1;
  _VAR(nplots);
  _VAR2(rows,cols);
  //enumerate(showTuple,tuple,mask,0,0);
#if 1
  enumerate(genPlot,tuple,mask,env,0);
#else
  // loop over remaining params... 
  for(int i=0;i<dims.size()-2;++i){
    float t = (float)i * dtheta;

    // loop over this cases for this param
    for(int j=1;j<dims[i+2];++j){
      iTuple radiantcoord(centercoord);

      radiantcoord[2+i]=j;

      _VAR(radiantcoord);

      float r = 2*j;
      rt2xy(r,t,x,y);
      nb = new Glx::NodeBlock(env,radiantcoord,p1,p2,x,y,w,h);
      nb->setColorfunc(setColor);
      nb->setCoord(radiantcoord);
    }
  }
#endif

}

int main(int argc, char** argv)
{

  if( argc==1 ){
    cerr<<"usage: pspace <pcoords file>"<<endl;
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
      ostringstream ostr;
      string str;
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
	//int* tuple = new int[dims.size()];
	std::vector<int> tuple;
	for(int i=0;i<dims.size();++i){
	  //tuple[i]=atoi(tokens[i].c_str());
	  tuple.push_back( atoi(tokens[i].c_str()) );
	  str += tokens[i];
	  if( i<dims.size()-1 )str += '_';
	}
	tuples.push_back(tuple);
	_VAR(str);
	known.insert(str);
      }
    }
    
  }
  fin.close();
  iTuple::idims = dims;

  _VAR(dims);


  glx* env = new glx(initGL);
  //return 0;
  env->mainLoop();
  return 1;
}
