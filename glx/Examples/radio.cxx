
#include <GLX.h>
#include <Draggers/Check.h>
#include <glxTrackpad.h>
#include <vector>
#include "debug.h"

using namespace std;

vector<Glx::Check*> cbox;

void select(Glx::Check* c)
{
  vector<Glx::Check*>::iterator iter=cbox.begin();
  for(int i=0 ; iter!=cbox.end();++iter,++i){
    Glx::Check* cb = *iter;
    cb->setChecked( cb==c );   
  }
}

void checked(void* user,bool enabled)
{
  Glx::Check* c = static_cast<Glx::Check*>(user);
  select(c);
}

int
main(int argc, char** argv)
{
  int x=30;
  glx* env = new glx();
  for(int i=0;i<5;++i){
    char name[80];
    name[0]='a'+i;
    name[1]='\0';
    Glx::Check* a = new Glx::Check(env,x,30,25,25,name);
    a->setCallback(checked,a);
    x+=30;
    cbox.push_back(a);
  }

  select(cbox[0]);
  Glx::Trackpad* tp = new Glx::Trackpad(env);
  env->mainLoop();
}
