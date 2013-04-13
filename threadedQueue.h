//////////////////////////////////////////////////////////////////////////
//////////////////////////// ThreadedQueue.h /////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifndef THREADED_QUEUE
#define THREADED_QUEUE

#include <deque>
#include <pthread.h>

/**
 * class ThreadedQueue: provides a thread-safe FIFO queue for passing
 * objects between threads.
 */

template<typename T>
class ThreadedQueue {
public:
  ThreadedQueue();
  ~ThreadedQueue();

  /**
   * Add an object to the back of the queue
   * @param obj: pointer to an object
   */
  void add(T* obj);

  /**
   * Add an object to the back of the queue, if not already in queue
   * @param obj: pointer to an object
   */
  bool addIfUnique(T* obj);

  /**
   * Wait for the obj at the front of the queue
   */
  T* next(void);

  /**
   * Grab the object from the front of the queue, if avail
   */
  T* nextIfReady(void);

  /**
   * see if an object is already in the queue
   */
  bool exists(T& obj);
  bool exists(T* obj);

  /**
   * returns true if the queue is empty
   */
  bool empty(void);

  /**
   * clears all entries from the queue
   */
  void clear(void);

  /**
   * returns the number of entries in the queue
   */
  int  size(void);

  /**
   * wakes all threads waiting on the queue
   */
  void die(void);

protected:
  void qLock();
  void qUnlock();
  pthread_mutex_t itsObjLock;
  pthread_cond_t itsQueueNotEmpty;
  std::deque<T*> itsObjQueue;
  bool done;
};

#ifndef THREADED_QUEUE_TEMPLATES_IN_C
#include "threadedQueue.C"
#endif

#endif
