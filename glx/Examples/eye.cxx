#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <values.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>
#include <assert.h>
#include "debug.h"
#include <GLX.h>
#include <glxTrackball.h>
#include <glxVector.h>
#include "debug.h"

using namespace std;

void showMatrix(glx* env, string label, double mat[16])
{
  ostringstream ostr;
  ostr << "===== " << label << " =====";
  env->showMessage(ostr.str());
  ostr.str("");  

  for(int i=0; i<4; i++){
    for(int j=0; j< 4; j++){      
      ostr.width(14);    
      ostr.fill(' ');
      ostr << mat[i*4+j];
    }
    env->showMessage(ostr.str());  
    ostr.str("");  
  }
}

void draw(glx* env, void* user)
{
  ostringstream ostr;
  double inv[16];
  Glx::Trackball* tb=static_cast<Glx::Trackball*>(user);
  Glx::inv4x4(tb->view,inv);
  Glx::Vector eye;
  Glx::xformVec(eye,inv,eye);
  
  ostr << "Eye pos: " << eye;
  env->showMessage(ostr.str());
  ostr.str("");  

  eye.normalize();
  eye *= -1;
  ostr << "Eye vec: " << eye;
  env->showMessage(ostr.str());

  showMatrix(env,"view",tb->view);
  showMatrix(env,"inv",inv);
  showMatrix(env,"proj",tb->proj);

}

void initGL(glx* env, void* user)
{
  Glx::Trackball* tb = new Glx::Trackball(env);
  env->addDrawFunc(draw,tb);
}

int
main(int argc, char** argv)
{
  glx* env = new glx(initGL);
  env->mainLoop();

  return 0;
}
