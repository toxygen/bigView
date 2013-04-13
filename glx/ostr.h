#ifndef _OSTR_H_
#define _OSTR_H_

#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <set>
#include <iomanip>
#include <deque>

template <typename T>
std::ostream& operator<<(std::ostream& ostr, std::vector<T>& v)
{
  if(v.empty())return ostr<<"[empty]\n";
  ostr << "===== [vector size=" << v.size() << "] =====" << std::endl;
  typename std::vector<T>::iterator iter = v.begin();
  for(int i=0 ; iter != v.end() ; ++iter,++i ){
    ostr << "["<<i<<"]: " << *iter << std::endl;
  }
  return ostr<<std::endl;
}

template <typename T>
std::ostream& operator<<(std::ostream& ostr, std::set<T>& s )
{
  if(s.empty())return ostr<<"[empty]\n";
  ostr << "===== [set size=" << s.size() << "] =====" << std::endl;
  typename std::set<T>::iterator iter = s.begin();
  for( ; iter != s.end() ; ++iter )
    ostr << *iter << std::endl;
  return ostr;
}

template <typename S,typename T>
std::ostream& operator<<(std::ostream& ostr, std::map<S,T>& m )
{
  if(m.empty())return ostr<<"[empty]\n";
  ostr << "===== [map size=" << m.size() << "] =====" << std::endl;
  typename std::map<S,T>::iterator iter = m.begin();
  for( ; iter != m.end() ; ++iter )
    ostr << "["<<(*iter).first << "] => ["<<(*iter).second <<"]"<< std::endl;
  return ostr;
}

template <typename S,typename T>
std::ostream& operator<<(std::ostream& ostr, std::multimap<S,T>& m )
{
  if(m.empty())return ostr<<"[empty]\n";
  ostr << "===== [multimap size=" << m.size() << "] =====" << std::endl;
  typename std::multimap<S,T>::iterator iter = m.begin();
  for( ; iter != m.end() ; ++iter )
    ostr << "["<<(*iter).first << "] => ["<<(*iter).second <<"]"<< std::endl;
  return ostr;
}

template <typename T>
std::ostream& operator<<(std::ostream& ostr, std::pair<T,T>& p )
{
  return ostr<<"["<<p.first<<","<<p.second<<"]";
}

template <typename S, typename T>
std::ostream& operator<<(std::ostream& ostr, std::pair<S,T>& p )
{
  return ostr<<"["<<p.first<<","<<p.second<<"]";
}

template <typename T>
std::ostream& operator<<(std::ostream& ostr, std::deque<T>& d )
{
  if(d.empty())return ostr<<"[empty]\n";
  ostr << "===== [deque size=" << d.size() << "] =====" << std::endl;
  typename std::deque<T>::iterator iter = d.begin();
  for( ; iter != d.end() ; ++iter )
    ostr << *iter << std::endl;
  return ostr;
}

inline std::string itoa(int val)
{
  std::ostringstream ostr;  
  ostr<<val;
  return ostr.str();
}

inline std::string ftoa(double val)
{
  /*
  std::ostringstream ostr;  
  ostr.precision(12);
  ostr<<val;
  return ostr.str();
  */

  std::ostringstream ostr;  
  ostr.precision(12);
  ostr.width(18);
  ostr.fill(' ');
  //ostr.setf(std::ios::right);
  ostr<<val;
  return ostr.str();

  
}

inline std::string utoa(u_int64_t val)
{
  std::ostringstream ostr;  
  ostr<<val;
  return ostr.str();
}

#endif

