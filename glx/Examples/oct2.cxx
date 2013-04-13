#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <values.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>
#include <assert.h>
#include "debug.h"

using namespace std;

int loadRange(int oct,int glo[3],int ghi[3],int lo[3],int hi[3])
{
  switch( oct ){
    case 0: 
      lo[0]=glo[0];
      lo[1]=glo[1];
      lo[2]=glo[2];
      hi[0]=(int)(0.5 * (glo[0]+ghi[0]));
      hi[1]=(int)(0.5 * (glo[1]+ghi[1]));
      hi[2]=(int)(0.5 * (glo[2]+ghi[2]));
      break;

    case 1:
      lo[0]=(int)(0.5 * (glo[0]+ghi[0]));
      lo[1]=glo[1];
      lo[2]=glo[2];
      hi[0]=ghi[0];
      hi[1]=(int)(0.5 * (glo[1]+ghi[1]));
      hi[2]=(int)(0.5 * (glo[2]+ghi[2]));
      break;

    case 2:
      lo[0]=glo[0];
      lo[1]=(int)(0.5 * (glo[1]+ghi[1]));
      lo[2]=glo[2];
      hi[0]=(int)(0.5 * (glo[0]+ghi[0]));
      hi[1]=ghi[1];
      hi[2]=(int)(0.5 * (glo[2]+ghi[2]));
      break;

    case 3:
      lo[0]=(int)(0.5 * (glo[0]+ghi[0]));
      lo[1]=(int)(0.5 * (glo[1]+ghi[1]));
      lo[2]=glo[2];
      hi[0]=ghi[0];
      hi[1]=ghi[1];
      hi[2]=(int)(0.5 * (glo[2]+ghi[2]));
      break;

    case 4: 
      lo[0]=glo[0];
      lo[1]=glo[1];
      lo[2]=(int)(0.5 * (glo[2]+ghi[2]));
      hi[0]=(int)(0.5 * (glo[0]+ghi[0]));
      hi[1]=(int)(0.5 * (glo[1]+ghi[1]));
      hi[2]=ghi[2];
      break;

    case 5:
      lo[0]=(int)(0.5 * (glo[0]+ghi[0]));
      lo[1]=glo[1];
      lo[2]=(int)(0.5 * (glo[2]+ghi[2]));
      hi[0]=ghi[0];
      hi[1]=(int)(0.5 * (glo[1]+ghi[1]));
      hi[2]=ghi[2];
      break;

    case 6:
      lo[0]=glo[0];
      lo[1]=(int)(0.5 * (glo[1]+ghi[1]));
      lo[2]=(int)(0.5 * (glo[2]+ghi[2]));
      hi[0]=(int)(0.5 * (glo[0]+ghi[0]));
      hi[1]=ghi[1];
      hi[2]=ghi[2];
      break;

    case 7:
      lo[0]=(int)(0.5 * (glo[0]+ghi[0]));
      lo[1]=(int)(0.5 * (glo[1]+ghi[1]));
      lo[2]=(int)(0.5 * (glo[2]+ghi[2]));
      hi[0]=ghi[0];
      hi[1]=ghi[1];
      hi[2]=ghi[2];
      break;
  }
}

void showSubs(string parent, int level,int glo[3], int ghi[3])
{
  for(int octant=0;octant<8;++octant){
    int lo[3],hi[3];
    ostringstream ostr;
    ostr << parent << octant;
    string oct = ostr.str();
    loadRange(octant,glo,ghi,lo,hi);
    if( level==0 ){
      printf("%s: [% 4d,% 4d,% 4d]=>[% 4d,% 4d,% 4d]\n", oct.c_str(),
	     lo[0],lo[1],lo[2],hi[0],hi[1],hi[2]);
    } else
      showSubs(oct,level-1,lo,hi);
  }
}

int
main(int argc, char** argv)
{
  int dims[3]={1024,1024,1024};
  int lo[3]={0,0,0},hi[3]={1024,1024,1024};
  string parent("0");
  showSubs(parent,1,lo,hi);
  return 0;
}
