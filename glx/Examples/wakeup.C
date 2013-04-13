#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <string>
#include <vector>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <GLX.h>
#include <glxTrackball.h>
#define DEBUG 1
#include "debug.h"
using namespace std;

char* commands[] = {
  "q",
  "?",
  "rate",
  "send"
};

enum {Q,HELP,RATE,SEND};

glx* genv=0;

void
sendWakeup(glx* env, int count, unsigned long usec)
{
  for(int i=0;i<count;++i){
    env->wakeup();
    usleep(usec);
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

void*
runShell(void* user)
{
  FANCYMESG("runShell [start]");

  vector<string> tokens;
  int rate=1,count;
  const unsigned long USEC_PER_SEC=1000000;
  unsigned long usec=USEC_PER_SEC;

  bool done=false;
  while( ! done ){

    char *line = readline ("wakeup> ");
    int com=-1;
    
    for(int i=0;i< sizeof(commands)/sizeof(char*)&&com==-1; ++i)
      if( ! strncmp(line,commands[i],strlen(commands[i]) ) )
	com=i;
    
    switch( com ){
      case Q:
	if( genv ) genv->setDone();
	done=true;
	break;

      case SEND:
	tokens.clear();
	tokenize(line, tokens);	
	if( tokens.size()==1 ){
	  cerr << "Count = " << count << endl;
	  break;
	}
	count = atoi(tokens[1].c_str());
	sendWakeup(genv,count,usec);
	break;

      case RATE:
	tokens.clear();
	tokenize(line, tokens);	
	if( tokens.size()==1 ){
	  cerr << "Rate = " << rate << endl;
	  break;
	}
	rate = atoi(tokens[1].c_str());
	usec = USEC_PER_SEC / rate;
	break;

      case HELP:
      default:
	for(int i=0;i< sizeof(commands)/sizeof(char*); ++i)
	  cout << "\t" << commands[i] << endl;
	break;
    }

    add_history(line);
  }
  FANCYMESG("runShell [ end ]");
  return 0;
}

const float MAX=3.0;
const float MIN=-MAX;
const float INC=(MAX-MIN)/100.0f;
float f(float u, float v)
{
  //return exp( sin(u*v) - cos(3.0f*u + 2.0f*v)) / 10.0f;
  //return sin(v) * cos(u) - 1.0f/sin(v) + 1.0f/cos(u);
  return sin(u*v);
}

void cross(float v1[3], float v2[3], float res[3])
{
  res[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
  res[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
  res[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
}

void
sendColor(float val, float min, float max)
{
  const float LVL_0 = 0;
  const float LVL_1 = 0.556863;
  const float LVL_2 = 0.929412;
  const float LVL_3 = 0.992157;
  float percent = (val-min)/(max-min);
  unsigned char rgb[3];
  if( percent <= LVL_1 ){
    float t = (percent-LVL_0)/0.556863;
    if( t>1 )t=1;
    if( t<0 )t=0;
    rgb[0] = (unsigned char)(255.0*(0 + t*1));
    rgb[1] = (unsigned char)(255.0*(0 + t*0.0901961));
    rgb[2] = 0;
  } else if( percent <= LVL_2 ){
    float t = (percent-LVL_1)/0.372549;
    if( t>1 )t=1;
    if( t<0 )t=0;
    rgb[0] = 255;
    rgb[1] = (unsigned char)(255.0*(0.0901961 + t*0.831373));
    rgb[2] = 0;
  } else {
    float t = (percent-LVL_2)/0.0627451;
    if( t>1 )t=1;
    if( t<0 )t=0;
    rgb[0] = 255;
    rgb[1] = (unsigned char)(255.0*(0.921569 + t*0.0784314));
    rgb[2] = (unsigned char)(255.0*(0 + t * 1));
  }
  glColor3ubv(rgb);
}

void sendNormal(float u, float v)
{
  float delta = INC/20.0f;
  float o[3] = {u,v,f(u,v)};
  float l[3] = {u+delta,v,f(u+delta,v)};
  float r[3] = {u,v+delta,f(u,v+delta)};
  float left[3] = {l[0]-o[0],l[1]-o[1],l[2]-o[2]};
  float right[3] = {r[0]- o[0],r[1]- o[1],r[2]- o[2]};
  float forward[3];

  cross(right,left,forward);

  float lb[3] = {u-delta,v,f(u-delta,v)};
  float rb[3] = {u,v-delta,f(u,v-delta)};
  float leftb[3] = {lb[0] - o[0],lb[1] - o[1],lb[2] - o[2]};
  float rightb[3] = {rb[0] - o[0],lb[1] - o[1],lb[2] - o[2]};
  float back[3];

  cross(rightb,leftb,back);

  float ave[3] = {-1.0f * (forward[0] + back[0]),
                  -1.0f * (forward[1] + back[1]),
                  -1.0f * (forward[2] + back[2])
  };
  float mag = sqrt( ave[0]*ave[0] + ave[1]*ave[1] + ave[2]*ave[2] );
  ave[0] /= mag;
  ave[1] /= mag;
  ave[2] /= mag;
  glNormal3fv(ave);
}

int drawCount=0;
void draw(glx* env, void*)
{
  int N=100;
  float t = (drawCount % N) /(float)(N-1);
  for(float u=MIN;u<=MAX;u+=INC){
    glBegin(GL_TRIANGLE_STRIP);
    for(float v=MIN;v<=MAX;v+=INC){
      float z = t*f(u,v);
      sendColor(z,-1,1);
      sendNormal(u,v);
      glVertex3f(u,v,z);
      
      float zNext = t*f(u+INC,v);
      sendColor(zNext,-1,1);
      sendNormal(u+INC,v);
      glVertex3f(u+INC,v,zNext);
    }
    glEnd();
  }

  ostringstream ostr;
  ostr << "Draw["<<drawCount++<<"]";
  env->showMessage(ostr.str());
}

void init(glx* env,void*)
{
  double gmin[3]={MIN,MIN,f(MIN,MIN)};
  double gmax[3]={MAX,MAX,f(MAX,MAX)};
  env->addDrawFunc(draw);
  Glx::Trackball* tb = new Glx::Trackball(env);
  tb->viewAll(gmin,gmax);
}

void* runGraphics(void* user)
{
  FANCYMESG("runGraphics [start]");
  pair<XtAppContext,Display*>* gi = 
    static_cast< pair<XtAppContext,Display*>* >(user);
  XtAppContext xapp = gi->first;
  Display* xdpy = gi->second;  
  glx* env = new glx(xapp,xdpy,init);
  genv = env; // hack
  env->mainLoop();
  FANCYMESG("runGraphics [ end ]");
}

int
main(int  argc,char** argv)
{  
  int res;
  pthread_t pt;
  vector<pthread_t> threads;
  vector<pthread_t>::iterator titer;

  assert( XInitThreads() );
  XtToolkitInitialize();
  assert( XtToolkitThreadInitialize() );
  XtAppContext xapp = XtCreateApplicationContext();
  Display* xdpy = XtOpenDisplay(xapp,0,"a","a",NULL,0,&argc, argv);
  pair<XtAppContext,Display*>* gi = new pair<XtAppContext,Display*>(xapp,xdpy);

  assert( pthread_create(&pt,0,runShell,NULL) == 0);
  threads.push_back(pt);

  assert( pthread_create(&pt,0,runGraphics,gi) == 0);
  threads.push_back(pt);

  titer = threads.begin();
  for( ; titer != threads.end() ; ++titer ){
    pthread_join(*titer,NULL);  
  }
}

