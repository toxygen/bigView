#include <iostream>
#include <string>
#include <exception>

#include "ppm.h"

using namespace std;

main(int argc, char** argv)
{
  PPM::Format format;
  int cpp,bpc;
  int sizeX,sizeY;
  int off;
  PPM::verbose=true;
  try {
    PPM::ppmHeader(argv[1],&format,&cpp,&bpc,&sizeX,&sizeY,&off);
  } catch( exception& e ){
    cerr << "PPM::ppmHeader failed\n";
  }
}
