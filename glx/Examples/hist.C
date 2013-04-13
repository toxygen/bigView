#include <fstream>
#include <GLX.h>
#include <Draggers/Histogram.h>
#include <glxTrackpad.h>
#include <vector>
#include <sstream>

using namespace std;

int NONE=-1;
int candidate=NONE,loSelection=NONE,hiSelection=NONE;
std::vector<uint64_t> lVec;

double smin = -0.391,smax=0.0;
vector<double> data;

void draw(glx* env,void*)
{
  string mesg;
  ostringstream ostr;
  ostr << "Candidate:" << candidate;
  mesg = ostr.str();
  env->showMessage(mesg);

  ostr.str("");
  ostr << "Selection:" << loSelection << " => " << hiSelection;
  mesg = ostr.str();
  env->showMessage(mesg);
}

void candidateFunc(glx* env,Glx::Histogram*,int id,void*)
{
  candidate = id;
  env->wakeup();
}
void selectionFunc(glx* env,Glx::Histogram*,int lo,int hi,void*)
{
  loSelection=lo;
  hiSelection=hi;
  env->wakeup();
}

void
initGL(glx* env, void*)
{
  int count=64;
  if( data.size () )
  {       
    lVec.resize(count);
    for(int i=0;i<data.size();++i){
      double t = (double)(data[i]-smin)/(smax-smin);
      int index = (int)(t*(count-1));
      if(index>=(count-1)){count-1;}
      ++lVec[index];
    }
  } 
  else 
  {
    for(int i=0;i<count;++i ){
      float x = -4.0 + 8 * (float)i/(count-1);
      float e = -0.5 * x * x;
      float t = exp(e);
      lVec.push_back((uint64_t)(t * 256));
    }
    //Glx::Histogram* h = new Glx::Histogram(env,lVec);
  }
  Glx::ScalarHistogram* h = new Glx::ScalarHistogram(env,lVec,smin,smax);
  Glx::Trackpad* tp = new Glx::Trackpad(env);

  h->addCandidateFunc(candidateFunc);
  h->addSelectionFunc(selectionFunc);
  env->addDrawFunc(draw);
}

int
main(int argc, char** argv)
{
  if( argc==2 ){
    ifstream fin(argv[1]);
    double sval;
    
    smin=MAXFLOAT;
    smax=-MAXFLOAT;

    while( fin ){
      fin>>sval;

      if( fin ){
	data.push_back(sval);
	if(sval<smin)smin=sval;
	if(sval>smax)smax=sval;
      }

    }
    fin.close(); 
  }
  glx* env = new glx(initGL);
  env->mainLoop();
}
