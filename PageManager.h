//////////////////////////////////////////////////////////////////////////
///////////////////////////// PageManager.h //////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifndef PAGE_MGR_H
#define PAGE_MGR_H

#include <iostream>
#include <vector>
#include <pthread.h>
#include "defs.h"


namespace Raw {

  /**
   * class Raw::PageManager
   *
   * A class to provide management of pages in an image pyramid.
   * Store and retrieve pages read from a paged image file.
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

  class PageManager {
  public:

    /** 
     * CTOR for class PageManager
     * @param width: width of image
     * @param height: height of image
     */
    PageManager(int width, int height);
    ~PageManager(void);

    /**
     * starting page for a given LOD
     * @param lod : level of detail
     */
    int begin(int lod);

    /**
     * just past last page for a given LOD
     * @param lod : level of detail
     */
    int end(int lod);
  
    /**
     * total number of pages needed for this image pyramid [actual]
     */
    int numPages(void){return itsPages.capacity();}

    /**
     * index into entire page array for a page given coord and LOD
     * @param lod : level of detail
     * @param coord : row/col of page in this LOD
     */
    int getPageIndex(int lod, Coord& coord);

    /** 
     * retrieve pages based on index
     * @param index : index into entire page array
     */
    Raw::Page* getPage(int index); 

    /** 
     * retrieve pages based on coord/LOD
     * @param lod : level of detail
     * @param coord : row/col of page in this LOD
     */
    Raw::Page* getPage(int lod, Coord& coord);

    /**
     * inform manager that a page has been loaded, copies pointer
     * @param p : ptr to newly loaded page
     */
    void  registerPage(Raw::Page* p);

    /**
     * inform manager that a page has been unloaded, zeros pointer
     * @param lod : level of detail
     * @param coord : row/col of page in this LOD
     */
    void clrPage(int lod, Coord& coord);

    /**
     * number of LODs required for entire image pyramid for this image
     */
    int  maxLOD(void);
    
    /**
     * number of pages in a given level of the pyramid
     * @param lod : level of detail
     * @param pageCols : number of columns
     * @param pageRows : number of rows
     */
    void pageCounts(int lod, int& pageCols, int& pageRows);

    /**
     * size of the scaled image in a given level of the pyramid
     * @param lod : level of detail
     * @param mipWidth : width of scaled image
     * @param mipHeight : height of scaled image
     */
    void mipmapSize(int lod, int& mipWidth, int& mipHeight);

    /**
     * total number of pages needed for this image pyramid [theoretical]
     */
    int  totalPages(void){return itsTotalPages;}

  protected:
    int  calcMaxLOD(int, int, int absMax=MAX_TEX_PAGES);
    int  calcTotalPages(int width, int height);

    void lock(void);
    void unlock(void);

    // precalc'd for all LODs
    std::vector<Raw::Page*> itsPages; 
    std::vector<int> itsOffsets; 
    std::vector<int> itsRows;  
    std::vector<int> itsCols;  
    std::vector<int> itsMipWidths;
    std::vector<int> itsMipHeights;

    int itsMaxLOD;
    int itsTotalPages;
    pthread_mutex_t itsPageLock;
  };
}

#endif
