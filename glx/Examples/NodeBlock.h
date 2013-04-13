#ifndef _NODE_BLOCK_H_
#define _NODE_BLOCK_H_

#include <vector>

#include "Draggers/WorldPalette.h"
#include "iTuple.h"

namespace Glx {

  class NodeBlock : public Glx::WorldPalette {
  public:
    //typedef void (*Colorfunc)(void*,std::vector<int>&, float[3]);
    typedef void (*Colorfunc)(void*,iTuple&, float[3]);
    
    NodeBlock(glx*,iTuple&,int,int,float,float,float,float);

    //NodeBlock(glx*,std::vector<int>&,std::vector<int>&,
    //	      int,int,float,float,float,float);

    void draw(glx*, void*);

    inline void setCoord(iTuple& c){coord=c;}
    //inline void setCoord(std::vector<int>& c){coord=c;}

    inline void setColorfunc( Glx::NodeBlock::Colorfunc cfunc,void* cdata=0){
      colorfunc=cfunc;
      colordata=cdata;
    }
    //static void defColor(void*,std::vector<int>&,float[3]);
    static void defColor(void*,iTuple&,float[3]);

    int P1,P2;
    std::vector<int> dims;
    //std::vector<int> coord;
    iTuple coord;
    Colorfunc colorfunc;
    void* colordata;
  };
};

#endif

