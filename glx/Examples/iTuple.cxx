#include <iostream>
#include <assert.h>
#include <iTuple.h>

//#define DEBUG 1
#include "pdebug.h"
using namespace std;

std::vector<int> iTuple::idims; // for operator size_t()

// static 
iTuple iTuple::end(void)
{
  iTuple res;
  for(int i=0;i<idims.size();++i){
    res[i]=idims[i]-1;
    if(res[i]<0)res[i]=0;
  }
  return res;
}

iTuple::iTuple(void) 
{
  coord.resize(idims.size());
  coord.assign(idims.size(),0);
}

iTuple::iTuple(vector<int>& ivec) 
{
  FANCYMESG("iTuple::iTuple(vector<int>&)");
  VAR(ivec.size());
  coord = ivec;
}

iTuple::iTuple(const iTuple& that) 
{
  FANCYMESG("iTuple::iTuple(const iTuple&)");
  VAR(that.coord.size());
  *this=that;
}

iTuple::~iTuple(void){}

iTuple& iTuple::operator=(const iTuple& other) 
{
  FANCYMESG("iTuple::operator=(const iTuple&)");
  coord=other.coord;
  return *this;
}

bool iTuple::operator==(const iTuple& other) 
{
  for(int i=0;i<coord.size();++i){
    if( coord[i] != other.coord[i] )
      return false;
  }
  return true;
}

iTuple operator-(const iTuple& t1, const iTuple& t2)
{
  iTuple t(t1);
  for(int j=0;j<t1.coord.size();++j){
    t[j] -= t2[j];
  }
  return t;
}

int& iTuple::operator[](int i)
{
  assert(i>=0 && i<coord.size());
  return coord[i];
}
int const& iTuple::operator[](int i) const
{
  if( i<0 || i==coord.size() ){
    VAR2(i,coord.size());
    assert(i>=0 && i<coord.size());
  }
  return coord[i];
}

iTuple::operator size_t() const
{
  size_t off=0;
  for(int i=0;i<iTuple::idims.size();++i){
    size_t sum=coord[i];
    for(int j=i+1;j<iTuple::idims.size();j++){
      sum *= idims[j];
    }
    off += sum;
  }
  return off;
}

iTuple index2tuple(size_t off)
{
  iTuple res;

  for(int i=0;i<iTuple::idims.size();++i){
    size_t size=1;
    for(int j=i+1;j<iTuple::idims.size();j++)
      size *= iTuple::idims[j];
    int digit = (int)(off/size);
    res[i]=digit;
    off  -= digit * size;
  }
  return res;
}

bool operator<( const iTuple& a, const iTuple& b)
{
  FANCYMESG("iTuple::operator<(const iTuple&, const iTuple&)");
  if(a.coord.size()==0 ) return false;
  if(b.coord.size()==0 ) return true;
  for(int i=0;i<a.coord.size();++i){
    if( a[i]>b[i] ) return false;
    if( a[i]<b[i] ) return true;
  }
  return false;
}

bool operator>( const iTuple& a, const iTuple& b)
{
  return b < a;
}

ostream& operator<<(ostream& ostr, const iTuple& t)
{
  ostr <<"[";
  for(int i=0;i<t.coord.size();++i){
    ostr.width(2);
    ostr << t.coord[i];
    if( i<t.coord.size()-1)
      ostr << ",";
  }
  ostr << "]";
  return ostr;
}

int iTuple::card(const iTuple& lo, const iTuple& hi)
{
  size_t hoff=hi;
  size_t loff=lo;
  return hoff-loff+1;
}
