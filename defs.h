//////////////////////////////////////////////////////////////////////////
///////////////////////////////// defs.h /////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifndef DEFS_H
#define DEFS_H

#include <algorithm> // for std::pair
#include <list>      // for std::list

namespace Raw {

  const int MAX_TEX_PAGES=140; // 140 pages visible at once
  const int PAGE_SIZE=256;
  const int PAGE_SIZE_EXP = 8;
  
  typedef std::pair<int,int> Coord; // col,row
  
  struct Page {
    Page(Coord coord, unsigned int id, unsigned char* pix, int lod) :
      itsCoord(coord),itsID(id),itsPixels(pix),itsLevelOfDetail(lod)
    {
    }
    int operator==(Page& that){
      if( that.itsLevelOfDetail != itsLevelOfDetail ) return 0;
      if( that.itsCoord != itsCoord ) return 0;
      return 1;
    }
    Coord itsCoord;
    unsigned int itsID;
    unsigned char* itsPixels;
    int itsLevelOfDetail;
  };

} // namespace Raw

extern bool VERBOSE;
extern std::ostream& operator<<(std::ostream&, Raw::Coord&);
extern std::ostream& operator<<(std::ostream&, std::list< Raw::Page >&);

#endif
