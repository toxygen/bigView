//////////////////////////////////////////////////////////////////////////
////////////////////////////// PageReader.h //////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifndef PAGE_READER_H
#define PAGE_READER_H

#include <vector>
#include <string>
#include <fstream>
#include <unistd.h>
#include "defs.h" // for Coord, KeyedPage
#include "threadedQueue.h"

namespace Raw {

  /**
   * class Raw::PageReader
   *
   * A class to read pages in an image pyramid from a
   * [possibly paged] image file. Reads are requested and
   * a user-supplied method is called when the read
   * has finished.
   *
   * The image is broken up into 256x256 tiles or 'pages' which
   * can be [written to]/[read from] disk very quickly and easily
   * stored into texture memory for viewing. 

   * Additionally, the image is down successively sampled at 
   * 1/2, 1/4, 1/8, etc scales all the way down until the 
   * scaled image fits into one 256x256 page. 
   *
   * Based upon visibility, an appropriate level of detail is
   * chosen, visible pages are determined, and these pages are 
   * then loaded/unloaded into main memory and texture memory
   * as needed. Finally, polygons are drawn [one for each page] 
   * displaying the portion of the image visible in the viewer.
   *
   */

  class PageReader {
  public:
  
    typedef void(*ListenerFunc)(PageReader*, Raw::Page*, void*);
    typedef std::pair<PageReader::ListenerFunc,void*> ListenerPair;

    /**
     * CTOR for class PageReader. Tries to open file. 
     * Starts a thread for reading. Throws exception if file not opened, 
     * or thread fails.
     * @param workQueue : thread safe queue for page requests
     * @param filename : fullpath name of paged image file
     * @param pageSize : size of pages
     * @param paged : file is actually paged
     * @param sourceWidth : width of source image, paged=false
     * @param sourceHeight : height of source image, paged=false
     * @param sourceFormat : pixel type: GL_LUMINANCE, GL_RGB, etc
     * @param sourceType : type of component: GL_BYTE, GL_FLOAT, etc
     * @param offset : offset to first pixel in file
     */
    PageReader(ThreadedQueue<Raw::Page>& workQueue,
	       std::string filename, 
	       int pageSize=Raw::PAGE_SIZE,
	       bool paged=true,
	       int sourceWidth=0, int sourceHeight=0,
	       int sourceFormat=0, int sourceType=0,
	       int offset=0);
  
    /**
     * DTOR for class PageReader. Closes file, stops thread.
     */
    ~PageReader(void);
  
    /** 
     * register a callback to be called when a page request has been honored
     * @param func : void(Raw::PageReader*, Raw::Page*, void*);
     * @param user : user data passed back to callback
     */
    void addListener(PageReader::ListenerFunc func, void* user=0);

    /**
     * returns true when the reader thread is ready to start reading
     */
    bool ready(void){return itsReadyFlag;}

    /**
     * queue up a request for a page to be loaded
     * @param page : page to be read
     * @param lod : level of detail
     */
    void schedulePageRead(Raw::Page page, int lod=0);

    /**
     * rows of pages needed for LOD=0 in this image pyramid
     */
    int numPageRows(void){return itsPageRows;}

    /**
     * cols of pages needed for LOD=0 in this image pyramid
     */
    int numPageCols(void){return itsPageCols;}

    /**
     * format of the pixel: GL_RGB, GL_LUMINANCE, etc
     */
    int sourceFormat(void){return itsSourceFormat;}

    /**
     * type of the pixel's components: GL_BYTE, GL_UNSIGNED_SHORT, etc
     */
    int sourceType(void){return itsSourceType;}

    /**
     * width of a page [probably 256]
     */
    int pageWidth(void){return itsPageDim;}

    /**
     * height of a page [probably 256]
     */
    int pageHeight(void){return itsPageDim;}

    /**
     * size of a scaled image in a given level of a pyramid
     * @param lod : level of detail
     * @param imageWidth : width of original image
     * @param imageHeight : height of original image
     * @param mipWidth : width of scaled image [result]
     * @param mipHeight : height of scaled image [result]
     */
    static void mipmapSize(int lod,
			   int imageWidth, int imageHeight,
			   int& mipWidth, int& mipHeight);

    /**
     * number of pages in a given level of the pyramid
     * @param lod : level of detail
     * @param pageDim : width/height of a page
     * @param imageWidth : width of original image
     * @param imageHeight : height of original image
     * @param pageCols : number of columns [result]
     * @param pageRows : number of rows [result]
     */

    static void pageCounts(int lod, int pageDim,
			   int imageWidth, int imageHeight,
			   int& pageCols, int& pageRows);

    /**
     * number of levels in the pyramid, given image and page sizes
     * @param imageWidth : width of original image
     * @param imageHeight : height of original image
     * @param pageCols : number of columns [result]
     * @param pageRows : number of rows [result]
     */
    static int maxLOD(int pageDim, int imageWidth, int imageHeight);

    /**
     * returns 2 ^ exp
     */
    static int pow2(int exp);

    /**
     * sets flags to quitting, wakes up thread if sleeping
     */
    void stopReaderThread(void);

  protected:

    void startReaderThread(void); 
    static void *work(void * arg);
    void callListeners(Page* page);

    unsigned char* load(Raw::Page& c);
    unsigned char* loadUnpaged(Raw::Coord& coord);

    bool isReaderDone(void);
    void openFile(void); 
    void closeFile(void);

    ThreadedQueue<Raw::Page>& itsWorkQueue;
    std::string itsFilename;
    bool itsPagedFlag;
    int itsFile;
    int itsOffset;
    pthread_t itsThreadID;
    bool itsReadyFlag;
    pthread_mutex_t itsDoneLock;
    bool itsDoneFlag;
    std::vector< PageReader::ListenerPair > itsListeners;

    int itsSourceWidth;
    int itsSourceHeight;
    int itsSourceFormat; // GL_LUMINANCE, GL_RGB, must agreed with below
    int itsSourceType;   // GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, etc

    int itsTotalPages;
    int itsPageRows;
    int itsPageCols;
    int itsPageDim;
    int itsMaxLOD;
    int itsPageDimExp;
  };
}

#endif
