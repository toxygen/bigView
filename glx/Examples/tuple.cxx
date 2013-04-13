#include <iostream>
#include "iTuple.h"

#include "debug.h"
using namespace std;

typedef void(*TupleFunc)(iTuple&,void*);

void showTuple(iTuple& t, void*)
{
  _VAR(t);
}

void
enumerate(TupleFunc f, iTuple& tuple, void* u,  int i)
{
  if( i==iTuple::idims.size() ) return;
  for(int j=0;j<iTuple::idims[i];++j){
    tuple[i]=j;
    if( i==iTuple::idims.size()-1 ){
      f(tuple,u);
    }
    enumerate(f,tuple,u,i+1);
  }
}

void
enumerate(TupleFunc f, iTuple& tuple, iTuple& mask, void* u,  int i)
{
  MESGVAR2("enum",tuple,mask);
  VAR(i);

  if( i==iTuple::idims.size() ) return;
  VAR(mask[i]);

  if( ! mask[i] ){
    if( i==iTuple::idims.size()-1 ){
      f(tuple,u);
    }
    enumerate(f,tuple,mask,u,i+1);
    return;
  }

  for(int j=0;j<iTuple::idims[i];++j){
    tuple[i]=j;
    if( i==iTuple::idims.size()-1 ){
      f(tuple,u);
    }
    enumerate(f,tuple,mask,u,i+1);
  }
}

void
enumerate(TupleFunc f, void* u=0)
{
  iTuple tuple;
  enumerate(f,tuple,u,0);
}

void
enumerate(TupleFunc f, iTuple& mask, void* u=0)
{
  iTuple tuple;
  enumerate(f,tuple,mask,u,0);
}

void testAllMasks(void)
{
  iTuple mask;//(iTuple::idims);
  iTuple tuple;

  tuple[0]=1;tuple[1]=2;tuple[2]=3;

  mask[0]=0;mask[1]=0;mask[2]=0;
  enumerate(showTuple,tuple,mask,0,0);

  _MESG("-------------------");

  tuple[0]=1;tuple[1]=2;tuple[2]=3;
  mask[0]=0;mask[1]=0;mask[2]=1;
  enumerate(showTuple,tuple,mask,0,0);

  _MESG("-------------------");
  tuple[0]=1;tuple[1]=2;tuple[2]=3;
  mask[0]=0;mask[1]=1;mask[2]=0;
  enumerate(showTuple,tuple,mask,0,0);

  _MESG("-------------------");
  tuple[0]=1;tuple[1]=2;tuple[2]=3;
  mask[0]=1;mask[1]=0;mask[2]=0;
  enumerate(showTuple,tuple,mask,0,0);

  _MESG("-------------------");
  tuple[0]=1;tuple[1]=2;tuple[2]=3;
  mask[0]=0;mask[1]=1;mask[2]=1;
  enumerate(showTuple,tuple,mask,0,0);

  _MESG("-------------------");
  tuple[0]=1;tuple[1]=2;tuple[2]=3;
  mask[0]=1;mask[1]=0;mask[2]=1;
  enumerate(showTuple,tuple,mask,0,0);

  _MESG("-------------------");
  tuple[0]=1;tuple[1]=2;tuple[2]=3;
  mask[0]=1;mask[1]=1;mask[2]=0;
  enumerate(showTuple,tuple,mask,0,0);

  _MESG("-------------------");
  tuple[0]=1;tuple[1]=2;tuple[2]=3;
  mask[0]=1;mask[1]=1;mask[2]=1;
  enumerate(showTuple,tuple,mask,0,0);

  _MESG("-------------------");

}

int
main(int,char**)
{
  vector<int> dims;
  vector<int> v;

  dims.push_back(2);
  dims.push_back(3);
  dims.push_back(4);

  v.push_back(1);
  v.push_back(1);
  v.push_back(1);

  iTuple::idims=dims;
  iTuple t(v),mask;

  testAllMasks();
  /*
  mask[0]=1;
  mask[1]=0;
  mask[2]=1;

  _VAR(mask);

  enumerate(showTuple);
  _MESG("-------------------");
  enumerate(showTuple,mask);

  _VAR(iTuple::idims);
  _VAR(t);
  */

}
