//////////////////////////////////////////////////////////////////////////
///////////////////////////////// ppm.h //////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifndef PPM_H
#define PPM_H

#include <iostream>
#include <string>
#include <fcntl.h> // for O_RDONLY

namespace PPM {
  
  extern bool verbose;

  enum Format{UNKNOWN,PLAIN_PPM,BINARY_PPM,PLAIN_PGM,BINARY_PGM};
  int ppmHeader(std::string filename, PPM::Format* format, 
		int* cpp, int* bpc,
		int* sizeX, int* sizeY,
		int* imageOffset);

  unsigned char* load(std::string filename, PPM::Format* format, 
		      int* cpp, int* bpc,
		      int* sizeX, int* sizeY);

  // used for 16bit greyscale images like MOLA elevation maps
  unsigned char* load(std::string filename, PPM::Format* format, 
		      int* cpp, int* bpc,
		      int* sizeX, int* sizeY,
		      bool signedPixels,
		      bool forceNormalize,
		      bool swapPixels);

  class Image {
  public:
    Image(std::string filename, int flags=O_RDONLY);
    ~Image(void);

    unsigned char* loadSubimage(int srcCol, int srcRow,
				int dstWidth,int dstHeight);

    void setImageParams(int cpp, int bpc,int w, int h);
    void storeSubimage(int dstCol, int dstRow, // offset into dest image
		       int dstWidth, int dstHeight, // size of write
		       unsigned char* pixels); // pixels to write
    static int pow2(int);

  protected:
    void openFile(void);
    void closeFile(void);

    std::string itsFilename;
    PPM::Format itsFormat;
    int itsComponentsPerPixel; // components per pixel
    int itsBitsPerComponent; // bits per component
    int itsWidth; // width of image
    int itsHeight; // height of image
    int itsImageOffset;
    int itsFile;
    int itsFlags;
  };

  // general purpose pixel utilities 
  void swapShorts(unsigned short*, int);
  void swapWords(unsigned*, int);
}
#endif
