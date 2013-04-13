#include <iostream>
#include <GLX.h>
#include "WorldPalette.h"

//#define DEBUG 1
#include "debug.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////// class Glx::WorldPalette /////////////////////////
//////////////////////////////////////////////////////////////////////////

namespace Glx 
{
  WorldPalette::WorldPalette(glx* env, float x, float y, float w, float h): 
    Glx::Draggable2D(env),snapX(0.05),snapY(0.05),
    itsX(x),itsY(y),itsW(w),itsH(h),itsOffX(0),itsOffY(0),
    itsState(Glx::WorldPalette::NORMAL),
    keepaspect(false),bold(false),aspect(w/h),
    itsDragFunc(0),itsDragData(0),
    itsDoneFunc(0),itsDoneData(0)    
  {
    Glx::WorldPalette::configureChanged(env,this);
    env->addConfigureFunc(Glx::WorldPalette::configureChanged, this);
    itsMouseFlags[glx::LEFT]=1;
    itsMouseFlags[glx::MIDDLE]=0;
    itsMouseFlags[glx::RIGHT]=0;
    itsMouseFlags[glx::WHEEL_UP]=0;
    itsMouseFlags[glx::WHEEL_DN]=0;
  }
  
  WorldPalette::~WorldPalette(void)
  {
  }

  void WorldPalette::draw(glx* env,void *user)
  {
    //FANCYMESG("WorldPalette::draw");
    if( ! itsVisibleFlag )
      return;

    int lw;
    int W=bold ? 3 : 1;
    glGetIntegerv(GL_LINE_WIDTH,&lw);
    glPushAttrib(GL_DEPTH_BUFFER_BIT|GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_COLOR_MATERIAL);

    // draw the entire container
    if( itsCurDragHandle == Glx::WorldPalette::MOVE )
      glColor3f(0.5f,0.0f,0.0f);
    else 
      glColor3f(0.0f,0.5f,0.0f);

    glLineWidth(W);
    glBegin(GL_LINE_LOOP);
    glVertex3f(itsX,     itsY,0.0f);
    glVertex3f(itsX,     itsY+itsH,0.0f);
    glVertex3f(itsX+itsW,itsY+itsH,0.0f);
    glVertex3f(itsX+itsW,itsY,0.0f);
    glEnd();

    if( itsState != Glx::WorldPalette::MOVING ){

      // draw the resize handle
      if( itsCurDragHandle == Glx::WorldPalette::RESIZE )
	glColor3f(0.5f,0.0f,0.0f);
      else 
	glColor3f(0.0f,0.5f,0.0f);
      
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();
      glOrtho(0,winw,0,winh,-1,1);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();
      
      glBegin(GL_LINE_LOOP);
      glVertex3f(itsPixelX+itsPixelW-HANDLESIZE, 
		 itsPixelY+itsPixelH,0.1f);
      glVertex3f(itsPixelX+itsPixelW-HANDLESIZE, 
		 itsPixelY+itsPixelH-HANDLESIZE,0.1f);
      glVertex3f(itsPixelX+itsPixelW,            
		 itsPixelY+itsPixelH-HANDLESIZE,0.1f);
      glVertex3f(itsPixelX+itsPixelW,            
		 itsPixelY+itsPixelH,0.1f);
      glEnd();
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
    }

    // Show Usage messages
    glColor3f(1,1,1);
    switch( itsCurDragHandle ){
      case Glx::WorldPalette::MOVE:
	env->showMessage("Left mouse = move");
	break;
      case Glx::WorldPalette::RESIZE:
	env->showMessage("Left mouse = resize");
	break;
    }

    glPopAttrib();
    glLineWidth(lw);
  }

  void WorldPalette::handleMouseUp(glx* env,void* user)
  {
    FANCYMESG("WorldPalette::handleMouseUp");
    unselect();
    itsState = Glx::WorldPalette::NORMAL;
    if(itsDoneFunc)
      itsDoneFunc(this,itsX,itsY,itsW,itsH,itsDoneData);
  }

  void WorldPalette::handleDrag(glx* env,int x,int y,void* user)
  {
    FANCYMESG("WorldPalette::handleDrag");
    double xf,yf;
    y=winh-y;
    if( itsEnv->buttonpressed != glx::LEFT ){
      unselect();
      return;
    } 
    toWorld(env,x,y,&xf,&yf);

    xf = snapX * (int)( (float)(xf+snapX/2.)/snapX);
    yf = snapY * (int)( (float)(yf+snapY/2.)/snapY);

    switch( itsEnv->buttonpressed ){
      case glx::LEFT:
	if( itsCurDragHandle == Glx::WorldPalette::MOVE ) 
	{
	  if( itsState == Glx::WorldPalette::NORMAL )
	  {
	    itsOffX = xf-itsX;
	    itsOffY = yf-itsY;
	  }
	  itsState = Glx::WorldPalette::MOVING;
	  itsX = xf - itsOffX;
	  itsY = yf - itsOffY;
	}
	else if( itsCurDragHandle == Glx::WorldPalette::RESIZE )
	{
	  if( itsState == Glx::WorldPalette::NORMAL )
	  {
	    itsOffX = itsX;
	    itsOffY = itsY;
	  }
	  itsState = Glx::WorldPalette::MOVING;
	  itsW = xf - itsOffX;
	  itsH = yf - itsOffY;

	  if( keepaspect ){
	    itsW = itsH * aspect;
	  } 
	}
	break;
    }
    updateProjection();
    if(itsDragFunc)
      itsDragFunc(this,itsX,itsY,itsW,itsH,itsDragData);
  }

  int WorldPalette::idlePick(glx* env,int x,int y,void* user)
  {
    //FANCYMESG("WorldPalette::idlePick");
    int envH = env->winHeight();
    y=envH-y;
    itsSelectionDist = MAXFLOAT;

    if( x >= (itsPixelX+itsPixelW-HANDLESIZE) && x <= (itsPixelX+itsPixelW) &&
	y >= (itsPixelY+itsPixelH-HANDLESIZE) && y <= (itsPixelY+itsPixelH) )
    {
      itsSelectionDist=0;
      return Glx::WorldPalette::RESIZE;
    } 
    else if( x >= itsPixelX && x <= (itsPixelX+itsPixelW) &&
	     y >= itsPixelY && y <= (itsPixelY+itsPixelH) )
    {
      itsSelectionDist=0;
      return Glx::WorldPalette::MOVE;
    }
    unselect();
    return UNSELECTED;
  }

  void
  WorldPalette::toPixels(glx* env, double x, double y, double* xf, double* yf)
  {
    double px,py,pz;
    env->project(x,y,0,
		 env->modelMatrix(),
		 env->projMatrix(),
		 env->viewport(),&px,&py,&pz);
    *xf = px;
    *yf = py;
  }

  void
  WorldPalette::toWorld(glx* env, double x, double y, double* xf, double* yf)
  {
    double px,py,pz; 
    env->unproject(x,y,0,
		   env->modelMatrix(),
		   env->projMatrix(),
		   env->viewport(),&px,&py,&pz);
    *xf = px;
    *yf = py;
  }

  void
  WorldPalette::updateProjection(void)
  {
    FANCYMESG("WorldPalette::updateProjection");
    double xf,yf,zf;

    itsEnv->project(itsX,itsY,0,
		    itsEnv->modelMatrix(),
		    itsEnv->projMatrix(),
		    itsEnv->viewport(),
		    &itsPixelX,&itsPixelY,&zf);

    itsEnv->project(itsX+itsW,itsY+itsH,0,
		    itsEnv->modelMatrix(),
		    itsEnv->projMatrix(),
		    itsEnv->viewport(),
		    &xf,&yf,&zf);

    itsPixelW=xf-itsPixelX;
    itsPixelH=yf-itsPixelY;
    VAR2(itsPixelX,itsPixelY);
    VAR2(itsPixelW,itsPixelH);
  }

  void
  WorldPalette::viewHasChanged(glx*)
  {
    FANCYMESG("WorldPalette::viewHasChanged");
    updateProjection();
  }

  void
  WorldPalette::configureChanged(glx*,void* user)
  {
    FANCYMESG("WorldPalette::configureChanged");
    Glx::WorldPalette* _this = static_cast<Glx::WorldPalette*>(user);
    _this->winw = _this->itsEnv->winWidth();
    _this->winh = _this->itsEnv->winHeight();
    _this->updateProjection();
  }

  void WorldPalette::setSize(float w,float h,bool calluser)
  {
    itsW=w;
    itsH=h;
    aspect = (float)itsW/itsH;
    updateProjection();
    callFuncs(itsUpdateFuncs);
    if( calluser){
      if( itsDragFunc ) itsDragFunc(this,itsX,itsY,itsW,itsH,itsDragData);
      if( itsDoneFunc ) itsDoneFunc(this,itsX,itsY,itsW,itsH,itsDragData);
    }
  }

  void WorldPalette::setPos(float x, float y, bool calluser)
  {
    itsX=x;
    itsY=y;
    updateProjection();
    callFuncs(itsUpdateFuncs);
    if( calluser){
      if( itsDragFunc ) itsDragFunc(this,itsX,itsY,itsW,itsH,itsDragData);
      if( itsDoneFunc ) itsDoneFunc(this,itsX,itsY,itsW,itsH,itsDragData);
    }
  }

  void WorldPalette::moveleft(bool calluser)
  {
    itsX -= snapX;
    updateProjection();
    callFuncs(itsUpdateFuncs);
    if( calluser){
      if( itsDragFunc ) itsDragFunc(this,itsX,itsY,itsW,itsH,itsDragData);
      if( itsDoneFunc ) itsDoneFunc(this,itsX,itsY,itsW,itsH,itsDragData);
    }
  }
  void WorldPalette::moveright(bool calluser)
  {
    itsX += snapX;
    updateProjection();
    callFuncs(itsUpdateFuncs);
    if( calluser){
      if( itsDragFunc ) itsDragFunc(this,itsX,itsY,itsW,itsH,itsDragData);
      if( itsDoneFunc ) itsDoneFunc(this,itsX,itsY,itsW,itsH,itsDragData);
    }
  }
  void WorldPalette::moveup(bool calluser)
  {
    itsY += snapY;
    updateProjection();
    callFuncs(itsUpdateFuncs);
    if( calluser){
      if( itsDragFunc ) itsDragFunc(this,itsX,itsY,itsW,itsH,itsDragData);
      if( itsDoneFunc ) itsDoneFunc(this,itsX,itsY,itsW,itsH,itsDragData);
    }
  }
  void WorldPalette::movedown(bool calluser)
  {
    itsY -= snapY;
    updateProjection();
    callFuncs(itsUpdateFuncs);
    if( calluser){
      if( itsDragFunc ) itsDragFunc(this,itsX,itsY,itsW,itsH,itsDragData);
      if( itsDoneFunc ) itsDoneFunc(this,itsX,itsY,itsW,itsH,itsDragData);
    }
  }

  void WorldPalette::pageleft(bool calluser)
  {
    itsX -= itsW;
    updateProjection();
    callFuncs(itsUpdateFuncs);
    if( calluser){
      if( itsDragFunc ) itsDragFunc(this,itsX,itsY,itsW,itsH,itsDragData);
      if( itsDoneFunc ) itsDoneFunc(this,itsX,itsY,itsW,itsH,itsDragData);
    }
  }
  void WorldPalette::pageright(bool calluser)
  {
    itsX += itsW;
    updateProjection();
    callFuncs(itsUpdateFuncs);
    if( calluser){
      if( itsDragFunc ) itsDragFunc(this,itsX,itsY,itsW,itsH,itsDragData);
      if( itsDoneFunc ) itsDoneFunc(this,itsX,itsY,itsW,itsH,itsDragData);
    }
  }
  void WorldPalette::pageup(bool calluser)
  {
    itsY += itsH;
    updateProjection();
    callFuncs(itsUpdateFuncs);
    if( calluser){
      if( itsDragFunc ) itsDragFunc(this,itsX,itsY,itsW,itsH,itsDragData);
      if( itsDoneFunc ) itsDoneFunc(this,itsX,itsY,itsW,itsH,itsDragData);
    }
  }
  void WorldPalette::pagedown(bool calluser)
  {
    itsY -= itsH;
    updateProjection();
    callFuncs(itsUpdateFuncs);
    if( calluser){
      if( itsDragFunc ) itsDragFunc(this,itsX,itsY,itsW,itsH,itsDragData);
      if( itsDoneFunc ) itsDoneFunc(this,itsX,itsY,itsW,itsH,itsDragData);
    }
  }

}; // namespace Glx
