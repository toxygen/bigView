#ifndef _CLAMP_H_
#define _CLAMP_H_

template <typename T>
void clamp(T& val, T hi)
{
  if( val>hi ) val=hi;
}

template <typename T>
void limit(T& val, T lo, T hi)
{
  if( val<lo ) val=lo;
  else if( val>hi ) val=hi;
}

#endif
