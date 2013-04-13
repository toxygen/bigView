#include <GLX.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>     
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>

using namespace std;
const int fullscreen=1000;
const int maxWin=330;
const int numSectors=fullscreen/maxWin;
const int winW=fullscreen/numSectors,winH=fullscreen/numSectors;

glx* g=0;

const float MAX=3.0;
const float MIN=-MAX;
const float INC=(MAX-MIN)/700.0f;
int row=0,col=0;
int rows=1,cols=1;
GLuint itsObjID=0;

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

void showObj(void)
{
  float u,v;
  glColor3f(0,1,0);
  GLenum type = GL_TRIANGLE_STRIP;
  for(u=MIN;u<=MAX;u+=INC){
    glBegin(type);
    for(v=MIN;v<=MAX;v+=INC){
      float z = f(u,v);
      sendColor(z,-1,1);
      sendNormal(u,v);
      glVertex3f(u,v,z);
      
      float zNext = f(u+INC,v);
      sendColor(zNext,-1,1);
      sendNormal(u+INC,v);
      glVertex3f(u+INC,v,zNext);
    }
    glEnd();
  }
}

void 
draw(glx* GLX, void*)
{
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  //glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);
  if( itsObjID == 0 ){
    itsObjID = glGenLists( (GLsizei)1 ); 
    glNewList(itsObjID, GL_COMPILE);
    showObj();
    glEndList();
  }
  glCallList( itsObjID );
}

void 
initGL(glx*, void*)
{
}

int
main(int argc, char** argv)
{  
  if( argc==5 ){
    rows=atoi(argv[1]);
    cols=atoi(argv[2]);
    row=atoi(argv[3]);
    col=atoi(argv[4]);
  }
  XtToolkitInitialize();
  XtAppContext app = XtCreateApplicationContext();
  Display* display = XtOpenDisplay(app,0,"a","a", NULL,0, &argc, argv);
  g = new glx(app,display);
  g->addDrawFunc(draw);
  g->setSector(row*cols+col,rows);
  g->mainLoop();
  return 0;
}
