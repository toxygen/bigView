#include <iostream>
#include <stdlib.h>
#include <values.h>
#include <GLX.h>

GLuint itsObjID=MAXINT;

void draw(glx*,void*){
  if( itsObjID==MAXINT ){
    itsObjID = glGenLists(1); 
    glNewList(itsObjID, GL_COMPILE);
    glBegin(GL_TRIANGLES);
    glColor3f(1.0,0.0,0.0); glVertex3f(1.0,0.0,0.0);
    glColor3f(0.0,1.0,0.0); glVertex3f(0.0,1.0,0.0);
    glColor3f(0.0,0.0,1.0); glVertex3f(0.0,0.0,1.0);
    glEnd();
    glEndList();
  }
  glDisable(GL_LIGHTING);
  glCallList(itsObjID);
}

int
main(int argc, char** argv)
{
  XtToolkitInitialize();
  XtAppContext app = XtCreateApplicationContext();
  Display* display = XtOpenDisplay(app,0,"a","a", NULL,0, &argc, argv);

  glx* g = new glx(app,display);
  g->setPosition(10,10);
  g->addDrawFunc(draw);

  glx* h = new glx(app,display);
  h->setPosition(610,10);
  h->addDrawFunc(draw);

  g->mainLoop();
}
