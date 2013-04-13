#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
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
#include <stdlib.h>
#include "Timer.h"
#include "debug.h"

/*

using ptrs

===== N = 10 =====
dist:per = 1.42857e+06
sort:per = 1.66667e+06
secs:t = 6e-06
===== N = 100 =====
dist:per = 1.42857e+07
sort:per = 2.85714e+06
secs:t = 3.5e-05
===== N = 1000 =====
dist:per = 1.78571e+07
sort:per = 2.20751e+06
secs:t = 0.000453
===== N = 10000 =====
dist:per = 1.85874e+07
sort:per = 1.49745e+06
secs:t = 0.006678
===== N = 100000 =====
dist:per = 1.6952e+07
sort:per = 983758
secs:t = 0.101651
===== N = 1000000 =====
dist:per = 1.37155e+07
sort:per = 670968
secs:t = 1.49038
===== N = 10000000 =====
dist:per = 1.44294e+07
sort:per = 432900
secs:t = 23.1


using indices

===== N = 10 =====
dist:per = 1.66667e+06
sort:per = 1.25e+06
secs:t = 8e-06
===== N = 100 =====
dist:per = 9.09091e+06
sort:per = 1.06383e+06
secs:t = 9.4e-05
===== N = 1000 =====
dist:per = 1.14943e+07
sort:per = 676590
secs:t = 0.001478
===== N = 10000 =====
dist:per = 1.33869e+07
sort:per = 388319
secs:t = 0.025752
===== N = 100000 =====
dist:per = 1.87547e+07
sort:per = 292446
secs:t = 0.341943
===== N = 1000000 =====
dist:per = 1.69073e+07
sort:per = 218278
secs:t = 4.58131
===== N = 10000000 =====
dist:per = 1.38345e+07
sort:per = 151600
secs:t = 65.9632


structs
ece02:/Examples > ./srt 
===== N = 10 =====
dist:per = 1.66667e+06
sort:per = 1.42857e+06
secs:t = 7e-06
===== N = 100 =====
dist:per = 1.42857e+07
sort:per = 2.77778e+06
secs:t = 3.6e-05
===== N = 1000 =====
dist:per = 2.04082e+07
sort:per = 2.0202e+06
secs:t = 0.000495
===== N = 10000 =====
dist:per = 2.14133e+07
sort:per = 1.60668e+06
secs:t = 0.006224
===== N = 100000 =====
dist:per = 2.1322e+07
sort:per = 1.26775e+06
secs:t = 0.07888
===== N = 1000000 =====
dist:per = 1.64669e+07
sort:per = 1.03429e+06
secs:t = 0.96685
===== N = 10000000 =====
dist:per = 1.43752e+07
sort:per = 874320
secs:t = 11.4375
===== N = 100000000 =====
dist:per = 1.34761e+07
sort:per = 856417
secs:t = 116.766

structs w/qsort()

===== N = 10 =====
dist:per = 1.66667e+06
sort:per = 2e+06
secs:t = 5e-06
===== N = 100 =====
dist:per = 1.66667e+07
sort:per = 6.25e+06
secs:t = 1.6e-05
===== N = 1000 =====
dist:per = 2.12766e+07
sort:per = 3.5461e+06
secs:t = 0.000282
===== N = 10000 =====
dist:per = 2.15054e+07
sort:per = 4.10509e+06
secs:t = 0.002436
===== N = 100000 =====
dist:per = 2.15517e+07
sort:per = 3.19479e+06
secs:t = 0.031301
===== N = 1000000 =====
dist:per = 2.13899e+07
sort:per = 2.60615e+06
secs:t = 0.383708
===== N = 10000000 =====
dist:per = 2.14037e+07
sort:per = 2.20215e+06
secs:t = 4.54102
===== N = 100000000 =====
dist:per = 2.14192e+07
sort:per = 1.93233e+06
secs:t = 51.7511

 */

using namespace std;
Timer timer;


struct Pnt {
  Pnt(void):id(0),d(0){}
  Pnt(int i) : id(i){}
  Pnt(Pnt& that){
    this->id=that.id;
    this->d=that.d;
  }
  Pnt(const Pnt& that){
    this->id=that.id;
    this->d=that.d;
  }
  int id;
  float d;
};

class PntCompare {
public:
  bool operator()(const Pnt& a1, const Pnt& a2){
    return a1.d > a2.d;
  }
  bool operator()(Pnt& a1, Pnt& a2){
    return a1.d > a2.d;
  }
};

void gen(std::vector<Pnt>& pnts, size_t N)
{
  for(size_t i=0;i<N;++i){
    pnts.push_back( Pnt(i) );
  }
}

void distFromEye(Pnt& p)
{
  p.d = drand48();
}

void clean(std::vector<Pnt>& pnts)
{
  pnts.clear();
}

/*
void qsort(void *base, size_t nmemb, size_t size,
	   int(*compar)(const void *, const void *));
*/

int qcomp(const void* p1, const void* p2)
{
  Pnt* pnt1 = (Pnt*)p1;
  Pnt* pnt2 = (Pnt*)p2;
  return pnt1->d > pnt2->d;  
}

int
main(int argc, char** argv)
{
  std::vector<Pnt> pnts;

  srand48( time(0) );
  size_t N=10;
  for(int i=0;i<8;++i){
    gen(pnts,N);
    _FANCYVAR(N);
    timer.reset();  
    timer.start();
    double startt = timer.elapsed();
    std::for_each(pnts.begin(),pnts.end(),distFromEye);
    double endt = timer.elapsed();
    double t = (endt-startt)/1e6; // secs
    double per = (double)N/t;
    _MESGVAR("dist",per);
    startt = timer.elapsed();
    //std::sort(pnts.begin(),pnts.end(),PntCompare()); 
    
    qsort(&pnts[0], pnts.size(), sizeof(Pnt), qcomp);

    endt = timer.elapsed();  
    timer.stop();
    t = (endt-startt)/1e6; // secs
    per = (double)N/t;
    _MESGVAR("sort",per);
    _MESGVAR("secs",t);
    clean(pnts);
    N *= 10;
  }
  return 0;
}
