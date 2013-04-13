#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <sys/types.h>
#include "debug.h"

#define DIM 3

typedef enum { UNMARKED, MARKED } mtype;

typedef unsigned char byte; 

struct TriStructure {
  int  vtx[3];                                /* limits compnums to +-32767  */
  int  Comp:16;                               /* 2 because mtype is an enum  */
  mtype mark:2, mark2:2, :0;                  /* need 1 sign bit, 0 is pad   */
};

typedef u_int64_t INT64;

struct TinyHexStructure {          /* bare-bones version of a cartesian cell */
  INT64       name;
  char        ref[DIM];
  byte        flagByte;
};

typedef struct TinyHexStructure tsTinyHex;

struct MPITinyHexXchangeStructure {
  tsTinyHex tinyHex;
  int       rcvrOlapIndex;
};

int
main(int argc, char** argv)
{  
  printf("sizeof mtype = %d\n",sizeof(mtype));
  printf("sizeof TriStructure = %d\n",sizeof(struct TriStructure));
  printf("offsetof(TriStructure,vtx) = %d\n",offsetof(struct TriStructure,vtx));
  printf("offsetof(TriStructure,Comp) = %d\n",offsetof(struct TriStructure,vtx)+3*sizeof(int));
  printf("offsetof(TriStructure,mark) = %d\n",offsetof(struct TriStructure,vtx)+3*sizeof(int)+2);
  printf("offsetof(TriStructure,mark2) = %d\n",offsetof(struct TriStructure,vtx)+3*sizeof(int)+2);

  printf("offsetof(MPITinyHexXchangeStructure,tinyHex) = %d\n",offsetof(struct MPITinyHexXchangeStructure,tinyHex));
  printf("offsetof(MPITinyHexXchangeStructure,tinyHex.name) = %d\n",offsetof(struct MPITinyHexXchangeStructure,tinyHex.name));
  printf("offsetof(MPITinyHexXchangeStructure,tinyHex.ref) = %d\n",offsetof(struct MPITinyHexXchangeStructure,tinyHex.ref));
  printf("offsetof(MPITinyHexXchangeStructure,tinyHex.flagByte) = %d\n",offsetof(struct MPITinyHexXchangeStructure,tinyHex.flagByte));
  printf("offsetof(MPITinyHexXchangeStructure,rcvrOlapIndex) = %d\n",offsetof(struct MPITinyHexXchangeStructure,rcvrOlapIndex));

  struct TriStructure tri;

  tri.vtx[0]=11;
  tri.vtx[1]=12;
  tri.vtx[2]=13;
  tri.Comp=1;
  tri.mark=MARKED;
  tri.mark2=MARKED;

  _DUMPHEX(&tri,sizeof(tri));
  _DUMPBIN(&tri,sizeof(tri));

  return 0;
}
