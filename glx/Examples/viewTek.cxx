#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
#include <stdio.h>
#include "str.h"

#include <GLX.h>
#include <glxTrackball.h>
#include <glxVector.h>
#include <Draggers/Check.h>

//#define DEBUG 1
#include "debug.h"

using namespace std;

struct TekSurface {
public:
  
  enum {TRI=3,QUAD=4};

  TekSurface(std::ifstream&,std::vector<std::string>&);

  void draw(glx*);

  void message(std::string);
  void setVisibility(bool enabled){visible=enabled;}

  float min(int index){return varmin[index];}
  float max(int index){return varmax[index];}

  std::vector<std::string> varTokens;
  std::vector<float*> varBuffers;
  std::vector<float> varmin;
  std::vector<float> varmax;
  std::vector<int> indexBuffer;
  int numVars;
  int numNodes;
  int numEle;
  int numTet;
  int numTri;
  int vertsPerElement;
  bool ok;
  bool visible;

  static int line;
  static bool verbose;
};

double gmin[3]={MAXFLOAT,MAXFLOAT,MAXFLOAT};
double gmax[3]={-MAXFLOAT,-MAXFLOAT,-MAXFLOAT};
int TekSurface::line=0;
bool TekSurface::verbose=true;

std::vector<TekSurface*> surfaces;

void
TekSurface::message(string mesg)
{
  if( TekSurface::verbose ){
    cout<<"["<<TekSurface::line<<"]:"<<mesg<<endl;
  }
}

TekSurface::TekSurface(std::ifstream& fin, std::vector<std::string>& v):
  varTokens(v),numVars(0),numNodes(0),numEle(0),numTet(0),numTri(0),
  vertsPerElement(0),ok(false),visible(true)
{
  message("TekSurface::TekSurface [start]");

  char zoneLine[256],indexLine[256];
  vector<string> titleTokens,varList,zoneTokens,nList,eList;

  MESG("============================"); 
  bool done=false,empty=true;
  while( empty && ! done ){
    fin.getline(zoneLine,256);
    if( ! fin ) done=true;
    if( strlen(zoneLine)>0 ) empty=false;
  }
  if( done ) return;
  str::tokenize(zoneLine, zoneTokens, ",");

  if( zoneTokens.size()==0 ){
    string mesg = "["+str::itoa(line)+"]"+": Zone line not found";
    throw std::runtime_error(mesg);
  }

  ++TekSurface::line;
  
  // Find 'N' and 'E'
  str::tokenize(zoneTokens[1], nList, "=");
  str::trim(nList);
  str::tokenize(zoneTokens[2], eList, "=");
  str::trim(eList);
  
  numVars = varTokens.size();
  numNodes = atoi(nList[1].c_str());
  numEle = atoi(eList[1].c_str());
  VAR2(numNodes,numEle);

  if( numVars==0 || numNodes==0 || numEle==0 ) {
    message("parse error: numVars==0 || numNodes==0 || numEle==0");
    throw std::runtime_error("parse error");
  }

  message("allocating varbuffers");
  varBuffers.resize(numVars);
  for(int j = 0 ; j < numVars ; ++j ){
    varBuffers[j] = new float[numNodes];
    if( varBuffers[j]==0 ){
      perror("new");
      message("error allocating memory for varbuffer");
      throw std::runtime_error("memory error");
    }
    varmin.push_back(MAXFLOAT);
    varmax.push_back(-MAXFLOAT);
  }  
  message("loading varbuffers");
  for(int i = 0 ; i<numNodes && fin; ++i ){
    for(int j = 0 ; j < numVars && fin; ++j ){
      fin >> varBuffers[j][i];
      if(varBuffers[j][i]<varmin[j]) varmin[j]=varBuffers[j][i];
      if(varBuffers[j][i]>varmax[j]) varmax[j]=varBuffers[j][i];
    }
    ++TekSurface::line;
  }
  fin.ignore(1024,'\n');
  
  message("finished varbuffers");

  fin.getline(indexLine,256);
  if( ! fin ){
    message("parse error: parse error: no connectivity");
    throw std::runtime_error("parse error");
  }
  ++TekSurface::line;

  str::tokenize(indexLine, eList);
  vertsPerElement=eList.size();
  _VAR(vertsPerElement);
  message("loading connectivity");

  indexBuffer.resize(numEle * vertsPerElement);
  for(int i=0;i<vertsPerElement;++i){
    indexBuffer[i]=atoi(eList[i].c_str())-1; // make zero-based
  }
  for(int i=1;i<numEle && fin;++i){
    for(int j=0;j<vertsPerElement && fin; ++j ){
      fin >> indexBuffer[i*vertsPerElement+j];
      --indexBuffer[i*vertsPerElement+j];// make zero-based
    }
    ++TekSurface::line;
  }
  fin.ignore(1024,'\n');
  ok=true;
  
  message("TekSurface::TekSurface [end]");
}

void
genFast(float percent)
{
  const float LVL_0 = 0;
  const float LVL_1 = 0.141176;
  const float LVL_2 = 0.282353;
  const float LVL_3 = 0.427451;
  const float LVL_4 = 0.568627;
  const float LVL_5 = 0.713726;
  const float LVL_6 = 0.854902;
  const float LVL_7 = 1;
  float rgb[4];
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

void TekSurface::draw(glx*)
{
  if( ! visible ) return;
  switch( vertsPerElement ){
    case TekSurface::TRI:
      glBegin(GL_TRIANGLES);
      break;
    case TekSurface::QUAD:
      glBegin(GL_QUADS);
      break;
  }
  for(int i=0;i<numEle * vertsPerElement;++i){
    float t = (varBuffers[3][ indexBuffer[i] ]-varmin[3]);
    t /= (varmax[3]-varmin[3]);    
    genFast(t);
    glVertex3f( varBuffers[0][ indexBuffer[i] ],
		varBuffers[1][ indexBuffer[i] ],
		varBuffers[2][ indexBuffer[i] ] );
    
  }
  glEnd();
}

void merge(double gmin[3],double gmax[3],
	   double lmin[3],double lmax[3])
{
  for(int i=0;i<3;++i){
    if(lmin[i]<gmin[i]) gmin[i]=lmin[i];
    if(lmax[i]>gmax[i]) gmax[i]=lmax[i];
  }
}

void draw(glx* env, void*)
{
  for(int i=0;i<surfaces.size();++i){
    surfaces[i]->draw(env);
  }
}

void checked(void* user, bool enabled)
{
  pair<glx*,TekSurface*>* cd = static_cast<pair<glx*,TekSurface*> *>(user);
  cd->second->setVisibility(enabled);
  cd->first->wakeup();
}

void initGL(glx* env, void* user)
{
  env->addDrawFunc(draw);
  Glx::Trackball* tb = new Glx::Trackball(env);

  int y=30;
  for(int i=0;i<surfaces.size();++i){
    double lmin[3]={surfaces[i]->min(0),
		   surfaces[i]->min(1),
		   surfaces[i]->min(2)};
    double lmax[3]={surfaces[i]->max(0),
		   surfaces[i]->max(1),
		   surfaces[i]->max(2)};
    merge(gmin,gmax,lmin,lmax);

    char name[256];
    sprintf(name,"surf#%02d",i);

    Glx::Check* cb = new Glx::Check(env,30,y,25,25,name);
    cb->setChecked(true);
    pair<glx*,TekSurface*> * cd = new pair<glx*,TekSurface*>(env,surfaces[i]);
    cb->setCallback(checked,cd);
    
    y+=30;    
  }
  tb->viewAll(gmin,gmax);
}

int 
main(int argc, char** argv)
{
  if( argc==1 ){
    cerr << "usage: viewTek <tekfile> <tekfile> ..."<< endl;
    return 0;
  }
  try 
  {
    for(int i=optind;i<argc;++i){
      char titleLine[256],varLine[256];
      vector<string> varList,varTokens;

      _VAR(argv[i]);

      ifstream fin( argv[i] );
      if( ! fin ){
	perror("open");
	throw std::runtime_error("error opening file \'"+string(argv[i])+"\'");
      }
      TekSurface::verbose=false;
      TekSurface::line=0;

      // title line, ignore
      fin.getline(titleLine,256);

      ++TekSurface::line;

      fin.getline(varLine,256);
      str::tokenize(varLine, varList, "=");
      string vars = varList[1];
      str::tokenize(vars, varTokens, ",");
      str::trim(varTokens);
      str::lower(varTokens);

      ++TekSurface::line;

      while( fin ){
	TekSurface* surf = new TekSurface(fin,varTokens);
	if( surf->ok )
	  surfaces.push_back(surf);
      }
      
      fin.close();
    }
  } 
  catch (std::exception& e)
  {
    cout<<"exception: " << e.what() << endl;
  }

  _VAR(surfaces.size());

  if( surfaces.size()){
    glx* env = new glx(initGL);
    env->mainLoop();
  }
  return 0;
}
