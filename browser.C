#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <assert.h>
#include <libgen.h>
#include <ppm.h>
#include <GLX.h>
#include <glxTrackpad.h>

#include "debug.h"

using namespace std;
int MAX=0;

struct imageInfo {
  imageInfo(void): 
    format(PPM::UNKNOWN),cpp(0),bpc(0),sizeX(0),sizeY(0),maxs(0.),maxt(0.),
    row(0),selected(false),xlo(0.),xhi(0.),image(NULL),texid(0){}
  string name;
  string thumbName;
  PPM::Format format;
  int cpp,bpc,sizeX,sizeY;
  float maxs,maxt;
  int row;
  bool selected;
  float xlo,xhi;
  unsigned char* image;
  GLuint texid;
};

std::string viewer;
vector<imageInfo*> thumbnails;
int mouseX,mouseY;

void
pixelToWorldCoords(glx* env,int x, int y, float* xf, float* yf)
{
  ostringstream ostr;
  int winh = env->winHeight();
  double px,py,pz;
  int viewport[4];
  double modelMatrix[16],projMatrix[16],inv[16];

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  env->applyProjections();
  glGetIntegerv(GL_VIEWPORT, viewport);
  glGetDoublev(GL_MODELVIEW_MATRIX,(double *)modelMatrix);
  glGetDoublev(GL_PROJECTION_MATRIX,(double *)projMatrix);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  env->unproject(x,winh-y,0,modelMatrix,projMatrix,viewport,&px,&py,&pz);
  *xf = px;
  *yf = py;
}

void draw(glx* env, void*)
{
  int rowSize = (int)sqrt((float)thumbnails.size());
  float worldX,worldY,dx = 1.0;
  float border = 0.1;
  pixelToWorldCoords(env,mouseX,mouseY,&worldX,&worldY);

  int row=0;
  float xlo=0.;

  for(int i=0;i< thumbnails.size() ; ++i){
    imageInfo* ii = thumbnails[i];

    float yunits = ii->maxt;
    float xunits = ii->maxs;
    float scale = 1./yunits;

    float xhi = xlo + xunits*scale;
    float ylo = row * dx + border;
    float yhi = (row+1) * dx - border;
    float aspect = (float)ii->sizeX/ii->sizeY;

    ii->row=row;
    ii->xlo=xlo;
    ii->xhi=xhi;
    VAR3(ii->row,ii->xlo,ii->xhi);
    glPushAttrib(GL_LIGHTING_BIT|GL_TEXTURE_BIT);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,ii->texid);
    glColor3f(1,1,1);
    glBegin(GL_QUADS);
    glTexCoord2f(0,       0);        glVertex2f(xlo,ylo);
    glTexCoord2f(ii->maxs,0);        glVertex2f(xhi,ylo);
    glTexCoord2f(ii->maxs,ii->maxt); glVertex2f(xhi,yhi);
    glTexCoord2f(0,       ii->maxt); glVertex2f(xlo,yhi);
    glEnd();
    glPopAttrib();

    if( ii->selected ){
      glPushAttrib(GL_LIGHTING_BIT);
      glDisable(GL_LIGHTING);
      glColor3f(1,0,0);
      glBegin(GL_LINE_LOOP);
      glVertex3f(xlo,ylo,0.1);
      glVertex3f(xhi,ylo,0.1);
      glVertex3f(xhi,yhi,0.1);
      glVertex3f(xlo,yhi,0.1);      
      glEnd();
      glPopAttrib();
    }

    xlo = xhi + border;
    if( xlo > 30. ){
      row++;
      xlo=0.;
    }
  }

  for(int i=0;i< thumbnails.size() ; ++i){
    imageInfo* ii = thumbnails[i];
    if( ! ii->selected ) continue;
    ostringstream ostr;
    ostr<<"File: [" << ii->name <<"]";
    env->showMessage( ostr.str()) ;
  }
}

void idle(glx* env,int x, int y,void* user)
{
  float worldX,worldY;

  mouseX=x;
  mouseY=y;

  pixelToWorldCoords(env,mouseX,mouseY,&worldX,&worldY);

  int row=(int)floor(worldY);
  for(int i=0;i< thumbnails.size() ; ++i){
    imageInfo* ii = thumbnails[i];
    ii->selected=false;
    if(ii->row != row ) continue;
    if(worldX<ii->xlo ) continue;
    if(worldX>ii->xhi ) continue;
    ii->selected=true;
  }

  env->wakeup();
}

void view(glx* env,int x, int y,void* user)
{
  for(int i=0;i< thumbnails.size() ; ++i){
    imageInfo* ii = thumbnails[i];
    if( ii->selected ){
      string pagedName = string(get_current_dir_name()) +
	string("/")+ thumbnails[i]->name + string(".ppm.paged");      
      string command = viewer + " " + pagedName + string("&");
      system(command.c_str());
    }
  }
}

void init(glx* env, void*)
{
  env->makeCurrent();
  vector<imageInfo*>::iterator iter = thumbnails.begin();
  for( ; iter != thumbnails.end() ; ++iter ){
    imageInfo* image = *iter;

    cout<<"name: ["<<image->name<<"] "
	<<"["<<image->sizeX<<"x"<<image->sizeY<<"]";

    glGenTextures(1,&image->texid);
    assert(glGetError()==GL_NO_ERROR);

    glBindTexture(GL_TEXTURE_2D,image->texid);
    assert(glGetError()==GL_NO_ERROR);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    assert(glGetError()==GL_NO_ERROR);
    int type,frmt;
    switch(image->cpp){
      case 1: frmt = GL_LUMINANCE; cout<<"[LUM]";break;
      case 3: frmt = GL_RGB; cout<<"[RGB]";break;
      default: assert(0);
    }
    switch(image->bpc){
      case 8: type=GL_UNSIGNED_BYTE; cout<<"[BYTE]";break;
      case 16: type=GL_UNSIGNED_SHORT; cout<<"[SHORT]";break;
      default: assert(0);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, frmt,
		 image->sizeX, image->sizeY, 0, frmt, type, image->image);

    cout<<endl;

    assert(glGetError()==GL_NO_ERROR);
  }  
  Glx::Trackpad* tp = new Glx::Trackpad(env);
  env->addDrawFunc(draw);
  env->addMouseIdleFunc(idle);
  env->addMouseDownFunc(view);

  int rowSize = (int)sqrt((float)thumbnails.size());
  double min[2],max[2];
  double x = rowSize;
  double y = rowSize + 1;
  double dx = -x/2.0;
  double dy = -y/2.0;
  double maxDim = (x > y) ? x : y;
  double scale = 1.0/maxDim;

  tp->itsScaleFactor = scale;
  tp->itsXtrans = dx;
  tp->itsYtrans = dy;

}

class Image_compare {
public:
  bool operator()(imageInfo* a, imageInfo* b){
    return a->sizeX/a->sizeY > b->sizeX/b->sizeY;
  }
};

int main(int argc, char** argv)
{
  string dname(".thumbs");

  struct stat stat_buf;
  struct dirent *entry;
  DIR *dir;
  int res;
  vector<string> entries;
  int filter=0;

  int c;
  while( (c = getopt(argc,argv,"d:f:")) != -1 )
  {
    switch( c ){

      case 'f':
	filter=atoi(optarg);
	break;	
      case 'd':
	dname=optarg;
	break;
    }
  }
  
  char* v = getenv("VIEWER");
  if( ! v ){
    cerr << "No VIEWER env variable set!" << endl;
    cerr << "setenv VIEWER bigView!" << endl;
    return 1;
  }
  viewer = v;

  res = stat(dname.c_str(),&stat_buf);
  if( res != 0 ){
    cerr << "no .thumbs dir found!" << endl;
    return 1;
  }
  dir = opendir(dname.c_str());
  if( dir == 0 ){
    perror("opendir");
    return 2;
  }
  entry = readdir(dir);  
  while( entry ){
    if( strcmp(".",entry -> d_name) &&
	strcmp("..",entry -> d_name) )
    {
      entries.push_back( dname+string("/")+string(entry->d_name) );
    }
    entry = readdir(dir);
  }
  closedir(dir);
  cout << "# entries: " << entries.size() << endl;

  // get max size of thumbnails
  vector<string>::iterator iter = entries.begin();
  for( ; iter != entries.end() ;++iter  ){
    PPM::Format format;
    int cpp, bpc, sizeX, sizeY, off;
    try {
      int res=PPM::ppmHeader(*iter,&format,&cpp,&bpc,&sizeX,&sizeY,&off);
      if( res != 1 ){
	cerr<<"error reading header: ["<<*iter<<"]\n";
	return 0;
      }      
      if( sizeX>MAX ) MAX=sizeX;
      if( sizeY>MAX ) MAX=sizeY;
    } catch( std::exception& e ){
      cerr<<"EXCEPT:"<<e.what()<<endl;
      return 0;
    }

  }
  VAR(MAX);

  // load thumbnails
  for( iter = entries.begin() ; iter != entries.end() ;++iter  ){
    string full(*iter);
    int len = (*iter).length();
    FANCYVAR(full);

    char tmp[256];
    strncpy(tmp,full.c_str(),256);
    string base( basename(tmp) );
    string paged(base.substr(0,base.length()-5)); // trim 'T.ppm'
    VAR(base);
    VAR(paged);

    string pagedName = string(get_current_dir_name()) +
      string("/")+ paged + string(".ppm.paged");      
    _VAR(paged);

    bool skip=false;
    if( filter>0 ){      
      PPM::Format format;
      int cpp, bpc, sizeX, sizeY, off;
      int res=PPM::ppmHeader(pagedName,&format,&cpp,&bpc,&sizeX,&sizeY,&off);
      if( sizeX<filter ) {
	_MESGVAR2("file too small, skipping",paged,sizeX);
	skip=true;
      }
    }
    if( skip ) continue;

    imageInfo* info = new imageInfo;
    info->thumbName = (*iter);
    info->name = paged;
    VAR(info->name);

    info->image = 
      PPM::load(*iter, &info->format,
		&info->cpp,&info->bpc,
		&info->sizeX,&info->sizeY);

    info->maxs=(float)info->sizeX/MAX;
    info->maxt=(float)info->sizeY/MAX;
    VAR2(info->maxs,info->maxt);
    VAR2(info->sizeX,info->sizeY);

    unsigned char* src = info->image;
    int componentSize = info->cpp;
    int bitsPerComponent = info->bpc;
    int bytesPerPixel = componentSize * (bitsPerComponent/8);
    size_t bufSize=MAX*MAX*bytesPerPixel;
    unsigned char* dst = new unsigned char[bufSize];
    assert(dst);
    VAR(componentSize);
    VAR(bitsPerComponent);
    VAR(bytesPerPixel);

    memset(dst,0,bufSize);

    for( int row=0 ; row<info->sizeY ; ++row){
      MESGVAR("copy",row);
      unsigned char* srcP = &src[row*info->sizeX*bytesPerPixel];
      unsigned char* dstP = &dst[row*MAX*bytesPerPixel];
      memcpy(dstP,srcP,info->sizeX*bytesPerPixel);
    }
    delete [] info->image;
    info->image = dst;
    info->sizeX = MAX;
    info->sizeY = MAX;
    thumbnails.push_back(info);
  }
  sort(thumbnails.begin(),thumbnails.end(),Image_compare());

  glx* env = new glx(init);
  env->showAxis(false);
  env->mainLoop();
}
