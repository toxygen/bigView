#include <fstream>
#include <assert.h>

using namespace std;

unsigned char* gen(int w, int h)
{
  unsigned char* buf= new unsigned char[w*h*3];
  assert(buf);
  for(int r=0 ; r<h ; ++r ){
    float fr = (float)r/(float)(h-1);
    unsigned char p = (unsigned char)(fr * 255.0f);
    for(int c=0 ; c<w ; ++c ){
      int offset = (r*w + c)*3;
      buf[offset+0] = p;
      buf[offset+1] = p;
      buf[offset+2] = p;
    }
  }
  return buf;
}

main()
{
  unsigned char* pix = gen(128,128);
  ofstream fout ("gen.ppm");
  fout << "P6 128 128 255" << endl;
  fout.write(pix,128*128*3);
  fout.close();
}
