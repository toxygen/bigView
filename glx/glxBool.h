#ifndef _GLX_SBOOL_H_
#define _GLX_SBOOL_H_

#include <pthread.h>

namespace Glx {

  class Hold {
  public:
    Hold(pthread_mutex_t *l) : lock(l) {pthread_mutex_lock(lock); }
    ~Hold(void) { pthread_mutex_unlock(lock); }
    pthread_mutex_t *lock;
  };
  
  class sBool {
  public:
    
    sBool(void):val(false){pthread_mutex_init(&olock,0);}
    sBool(bool v):val(v){pthread_mutex_init(&olock,0);}

    ~sBool(void){pthread_mutex_destroy(&olock);}
    
    operator bool(void){
      Hold h(&olock);
      return val;
    }
    bool operator==(bool v){
      Hold h(&olock);
      return val==v;
    }
    bool operator!(void){
      Hold h(&olock);
      return val==false;
    }
    
    pthread_mutex_t olock;
    bool val;
  };

};

#endif
