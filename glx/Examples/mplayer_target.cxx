#include <iostream>
#include <GLX.h>
#include <glxTrackball.h>
#include "debug.h"

using namespace std;

int x,y,w,h;

void initGL(glx* env, void* user)
{
  env->showAxis(false);
}

int main(int argc, char** argv)
{
  bool gotsize=false;
  int c,res;

  while( (c = getopt(argc,argv,"g:")) != -1){
    switch( c ){
      case 'g':
	res=sscanf(optarg,"%dx%d+%d+%d",&w,&h,&x,&y);	
	gotsize=res==4 ? true : false;
	VAR4(w,h,x,y);
        break;
    }
  }
  if( ! gotsize){
    cout<<"usage: "<<argv[0]<<" -g<int>x<int>+<int>+<int>"<<endl;
    return 0;
  }
  glx* env = new glx(initGL,NULL,x,y,w,h,glx::NoBorder);    
  cout<<std::hex<<env->getWindow()<<std::dec<<endl;
  env->mainLoop();
  return 1;
}
