
#include <GLX.h>
#include <Draggers/Check.h>
#include <glxTrackpad.h>
#include <vector>

using namespace std;

vector<Glx::Check*> cbox;

void checked(void* user,bool enabled)
{
  Glx::Check* c = static_cast<Glx::Check*>(user);
  cout << c->getLabel() << " " << enabled << endl;
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
  }

  Glx::Trackpad* tp = new Glx::Trackpad(env);
  env->mainLoop();
}
