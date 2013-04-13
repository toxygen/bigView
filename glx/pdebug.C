//////////////////////////////////////////////////////////////////////////
//////////////////////////////// debug.C /////////////////////////////////
//////////////////////////////////////////////////////////////////////////


#include <sstream>
#include <string>
#include <vector>
#include <pthread.h>
using namespace std;

pthread_mutex_t printLock=PTHREAD_MUTEX_INITIALIZER;

static std::vector<pthread_t> tdict;

void plock(void)
{
  pthread_mutex_lock(&printLock);
}

void punlock(void)
{
  pthread_mutex_unlock(&printLock);
}

std::string _ptid_(void)
{
  pthread_t tid = pthread_self();
  int index=0;
  std::vector<pthread_t>::iterator i=find(tdict.begin(),tdict.end(),tid);
  if( i==tdict.end() ){
    tdict.push_back(tid);
    index=tdict.size()-1;
  } else 
    index=i-tdict.begin();
  std::ostringstream ostr;
  ostr << "[";
  ostr << "T: ";
  ostr.width(2);
  ostr << index;
  //ostr.width(12);
  //ostr << std::hex << pthread_self() << std::dec;
  ostr << "]: ";
  return ostr.str();
}


