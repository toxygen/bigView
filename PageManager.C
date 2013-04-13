//////////////////////////////////////////////////////////////////////////
///////////////////////////// PageManager.C //////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include "PageManager.h"
#include "PageReader.h"
#include "ostr.h"

using namespace std;

//#define DEBUG 1
#include "debug.h"

bool VERBOSE=false;

Raw::PageManager::PageManager(int width, int height)
{
  FANCYMESG("Raw::PageManager::PageManager");

  if( pthread_mutex_init(&itsPageLock,0) ) 
    throw;

  // internal
  itsMaxLOD = calcMaxLOD(width,height,1);

  itsOffsets.reserve(itsMaxLOD+2);
  itsRows.reserve(itsMaxLOD+2);
  itsCols.reserve(itsMaxLOD+2);
  itsMipWidths.reserve(itsMaxLOD+2);
  itsMipHeights.reserve(itsMaxLOD+2);
  int max=itsOffsets.capacity();
  for(int i = 0 ; i < max ; ++i){
    itsOffsets.push_back(0);
    itsRows.push_back(0);
    itsCols.push_back(0);
    itsMipWidths.push_back(0);
    itsMipHeights.push_back(0);
  }
  
  int offset=0;
  for(int i=0; i<=itsMaxLOD ; ++i ){
    int rows,cols,mipWidth,mipHeight;
    PageReader::mipmapSize(i,width,height,mipWidth,mipHeight);
    PageReader::pageCounts(i,PAGE_SIZE,width,height,cols,rows);

    itsOffsets[i] = offset;
    itsMipWidths[i]=mipWidth;
    itsMipHeights[i]=mipHeight;
    itsRows[i]=rows;
    itsCols[i]=cols;
    cout << "# Level " << i << " : ["
	 << mipWidth << "," << mipHeight << "] => "
	 << "[" << cols << "," << rows << "] = " 
	 << rows*cols << " pages " << endl;
    offset += rows*cols;
  }

  itsPages.reserve(offset);
  max = itsPages.capacity();
  for(int i=0; i<max ; ++i )
    itsPages.push_back(0);

  for(int i=0; i<=itsMaxLOD ; ++i ){
    int rows,cols;
    PageReader::pageCounts(i,PAGE_SIZE,width,height,cols,rows);  
    for(int r=0 ; r < rows ; ++r){
      for(int c=0 ; c < cols ; ++c ){  
	Coord coord(c,r);
	int index = getPageIndex(i,coord);
	itsPages[index]= new Page(coord,0,0,i);
      }
    }
  }
  max=itsPages.capacity();
  for(int i=0; i<max ; ++i )
    assert(itsPages[i]);

  // external
  itsMaxLOD = calcMaxLOD(width,height);
  itsTotalPages = calcTotalPages(width,height);
  VAR(itsMaxLOD);
}

Raw::PageManager::~PageManager(void)
{
  pthread_mutex_destroy(&itsPageLock);
}

int
Raw::PageManager::getPageIndex(int lod, Coord& c)
{
  int index = itsOffsets[lod];
  index += c.second * itsCols[lod] + c.first;
  return index;
}

Raw::Page*
Raw::PageManager::getPage(int lod, Coord& c)
{
  int index=getPageIndex(lod,c);
  //lock();
  Page* p = itsPages[index];
  //unlock();
  return p;
}

Raw::Page* 
Raw::PageManager::getPage(int index)
{
  //lock();
  Page* p = itsPages[index];
  //unlock();
  return p;
}

void
Raw::PageManager::registerPage(Raw::Page* p)
{
  assert(p);

  int index = getPageIndex(p->itsLevelOfDetail,p->itsCoord);
  VAR(index);
  if(itsPages[index]){
    if( itsPages[index]->itsCoord==p->itsCoord &&
	itsPages[index]->itsLevelOfDetail==p->itsLevelOfDetail )
      return;      
  }
  FANCYMESG("Raw::PageManager::registerPage");
  VAR(p->itsCoord);
  VAR(p->itsID);
  VAR(p->itsLevelOfDetail);
  if(itsPages[index]){
    VAR(itsPages[index]->itsCoord);
    VAR(itsPages[index]->itsID);
    VAR(itsPages[index]->itsLevelOfDetail);
    assert(itsPages[index]->itsID==0);
  }
  lock();
  itsPages[index]=p;
  unlock();
}

void 
Raw::PageManager::clrPage(int lod, Coord& c)
{
  int index = itsOffsets[lod];
  index += c.second * itsCols[lod] + c.first;
  lock();
  itsPages[index] = 0;
  unlock();    
}

int 
Raw::PageManager::calcMaxLOD(int width, int height, int absMax)
{
  int rows = (int)(height/PAGE_SIZE) + 1;
  int cols = (int)(width/PAGE_SIZE) + 1;
  int lod=0;
  
  while( (rows>1 || cols>1) && (rows*cols> absMax) ){
    height /= 2;
    width /= 2;
    rows = (int)(height/PAGE_SIZE) + 1;
    cols = (int)(width/PAGE_SIZE) + 1;
    ++lod;
  }
  return lod;
}

void 
Raw::PageManager::pageCounts(int lod, int& pageCols, int& pageRows)
{
  pageCols=itsCols[lod];
  pageRows=itsRows[lod];
}

void 
Raw::PageManager::mipmapSize(int lod, int& mipWidth, int& mipHeight)
{
  mipWidth=itsMipWidths[lod];
  mipHeight=itsMipHeights[lod];
}

int 
Raw::PageManager::calcTotalPages(int width, int height)
{
  int offset=0;
  int maxlod = calcMaxLOD(width,height);
  for(int i=0; i<=maxlod ; ++i ){
    int rows,cols;
    pageCounts(i,cols,rows);
    int pageCount=rows*cols;
    offset += pageCount;
  }
  return offset;
}

int  Raw::PageManager::maxLOD(void){return itsMaxLOD;}
int  Raw::PageManager::begin(int lod){return itsOffsets[lod];}
int  Raw::PageManager::end(int lod){return itsOffsets[lod+1];}
void Raw::PageManager::lock(void){pthread_mutex_lock(&itsPageLock);}
void Raw::PageManager::unlock(void){pthread_mutex_unlock(&itsPageLock);}

ostream& operator<<(ostream& ostr, Raw::Coord& p)
{
  ostr << "[";
  ostr.width(3);
  ostr << p.first;
  ostr << ",";
  ostr.width(3);
  ostr << p.second;
  ostr<< "]";
  return ostr;  
}

ostream& operator<<(ostream& ostr, list< Raw::Page >& l)
{
  list< Raw::Page >::iterator i = l.begin();
  int index=0;
  ostr << "=== loaded pages ===" << endl;
  for( ; i != l.end() ; ++i ){
    Raw::Page page = *i;
    ostr << "\t["<<index++<<"]:"<< page.itsCoord << " ,"
         << page.itsID 
         << endl;
  }
  return ostr;
}
