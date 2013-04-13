#ifndef _TIMER_H_
#define _TIMER_H_
#include <sys/time.h>
class Timer {
public:
  bool running;
  struct timeval start_tv;
  double total;

  Timer() : running(false),total(0) {}
  inline void start(){
    assert(running==false);
    running = true;
    gettimeofday(&start_tv, (struct timezone *) 0);
  }
  inline void stop(){
    assert(running);
    running = false;
    struct timeval stop_tv;
    gettimeofday(&stop_tv, (struct timezone *) 0);
    long dts = stop_tv.tv_sec - start_tv.tv_sec;
    long dtus = stop_tv.tv_usec - start_tv.tv_usec;
    total += (double) dts * 1.0e6 + (double) dtus;
  }
  inline double elapsed(){
    struct timeval now_tv;
    if (running) {
      gettimeofday(&now_tv, (struct timezone *) 0);
      long dts = now_tv.tv_sec - start_tv.tv_sec;
      long dtus = now_tv.tv_usec - start_tv.tv_usec;
      return total + (double) dts * 1.0e6 + (double) dtus;
    }
    else
      return total;
  }
  inline void reset(){total=0;}
};
#endif
