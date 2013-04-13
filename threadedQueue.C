//////////////////////////////////////////////////////////////////////////
//////////////////////////// ThreadedQueue.C /////////////////////////////
//////////////////////////////////////////////////////////////////////////

#if defined(THREADED_QUEUE) || defined(THREADED_QUEUE_TEMPLATES_IN_C)

#include "threadedQueue.h"

template<typename T>
ThreadedQueue<T>::ThreadedQueue() : done(false)
{
  pthread_mutex_init(&itsObjLock,0);
  pthread_cond_init (&itsQueueNotEmpty,0);
}

template<typename T>
ThreadedQueue<T>::~ThreadedQueue()
{
  pthread_mutex_destroy(&itsObjLock);
  pthread_cond_destroy(&itsQueueNotEmpty);
}

template<typename T>
void ThreadedQueue<T>::add(T* obj)
{
  qLock();
  itsObjQueue.push_back(obj);
  qUnlock();
  pthread_cond_broadcast(&itsQueueNotEmpty);
}

template<typename T>
void ThreadedQueue<T>::die(void)
{
  done=true;
  pthread_cond_broadcast(&itsQueueNotEmpty);
}

// requires T::operator*
// requires T::operator==

template<typename T>
bool ThreadedQueue<T>::addIfUnique(T* obj)
{
  bool found=true;
  typename std::deque<T*>::iterator iter;
  qLock();
  iter = find(itsObjQueue.begin(),itsObjQueue.end(),obj);
  if( iter == itsObjQueue.end() ){
    itsObjQueue.push_back(obj);
    found=false;
  }
  qUnlock();
  if(! found )
    pthread_cond_broadcast(&itsQueueNotEmpty);
  return ! found;
}

template<typename T>
T* ThreadedQueue<T>::nextIfReady(void)
{
  T* res=0;
  qLock();
  if( ! itsObjQueue.empty() ){
    res = itsObjQueue.front();
    itsObjQueue.pop_front();
  }
  qUnlock();
  return res;
}

template<typename T>
T* ThreadedQueue<T>::next(void)
{
  T* res=0;
  qLock();
  while( itsObjQueue.empty() && ! done )
    pthread_cond_wait(&itsQueueNotEmpty,&itsObjLock);

  if( ! itsObjQueue.empty() && ! done ){
    res = itsObjQueue.front();
    itsObjQueue.pop_front();
  }
  qUnlock();
  return res;
}

template<typename T>
bool ThreadedQueue<T>::empty(void)
{
  bool isEmpty;
  qLock();
  isEmpty = itsObjQueue.empty();
  qUnlock();
  return isEmpty;
}

template<typename T>
bool ThreadedQueue<T>::exists(T* obj)
{
  typename std::deque<T*>::iterator iter;
  bool found=false;
  qLock();
  iter = find(itsObjQueue.begin(),itsObjQueue.end(),obj);
  found = iter == itsObjQueue.end() ? false : true;
  qUnlock();
  return found;
}

template<typename T>
bool ThreadedQueue<T>::exists(T& obj)
{
  typename std::deque<T*>::iterator iter;
  bool found=false;
  qLock();
  iter = itsObjQueue.begin();
  for( ; iter != itsObjQueue.end() && ! found; ++iter ){
    T& that = **iter;
    if( obj==that )
      found=true;
  }
  qUnlock();
  return found;
}

template<typename T>
void ThreadedQueue<T>::qLock()
{
  pthread_mutex_lock(&itsObjLock);
}

template<typename T>
void ThreadedQueue<T>::qUnlock()
{
  pthread_mutex_unlock(&itsObjLock);
}

template<typename T>
void ThreadedQueue<T>::clear(void)
{
  qLock();
  itsObjQueue.clear();
  qUnlock();
}

template<typename T>
int ThreadedQueue<T>::size(void)
{
  qLock();
  int size = itsObjQueue.size();
  qUnlock();
  return size;
}

#endif
