#include <iostream>
#include <assert.h>
#include "ppm.h"

using namespace std;

int
main(int, char**)
{
  int cpp=3,bpc=8,w=512,h=512;
  int halfW=w/2,halfH=h/2;
  unsigned char maxValue = (unsigned char)PPM::Image::pow2(bpc)-1;
  
  PPM::Image* i = new PPM::Image("test.ppm",O_WRONLY);
  i->setImageParams(cpp,bpc,512,512);
  
  unsigned char* buf = new unsigned char[cpp*halfW*halfH];
  cout << "BUFFER SIZE:" << cpp*halfW*halfH << endl;
  assert(buf);
  
  // red
  for(int i=0;i<halfW*halfH ; ++i ){
    int row = i / halfH;
    int offset=i * cpp;
    buf[offset+0] = row;
    buf[offset+1] = 0;
    buf[offset+2] = 0;
  }
  i->storeSubimage(0,0,halfW,halfH,buf);

  // green
  for(int i=0;i<halfW*halfH ; ++i ){
    int row = i / halfH;
    int offset=i * cpp;
    buf[offset+0] = 0;
    buf[offset+1] = row;
    buf[offset+2] = 0;
  }
  i->storeSubimage(halfW,0,halfW,halfH,buf);

  // blue
  for(int i=0;i<halfW*halfH ; ++i ){
    int row = i / halfH;
    int offset=i * cpp;
    buf[offset+0] = 0;
    buf[offset+1] = 0;
    buf[offset+2] = row;
  }
  i->storeSubimage(0,halfH,halfW,halfH,buf);

  // grey
  for(int i=0;i<halfW*halfH ; ++i ){
    int row = i / halfH;
    int offset=i * cpp;
    buf[offset+0] = row;
    buf[offset+1] = row;
    buf[offset+2] = row;
  }
  i->storeSubimage(halfW,halfH,halfW,halfH,buf);
  delete i;
  return 0;
}
