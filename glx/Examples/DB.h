// Emacs mode -*-c++-*- //
#ifndef GUI_DATABASE_H
#define GUI_DATABASE_H

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace DB {
  class Mgr {
  public:
    typedef std::map<std::string,std::string>::iterator Iter;

    Mgr(std::string fname) : filename(fname) {
      std::string sep(" \t\n");
      struct stat statBuf;
      int res = stat(filename.c_str(), &statBuf);
      if( res==0 ){
	std::ifstream fin(filename.c_str());
	while( fin ){
	  const int LSIZE=1024;
	  char line[LSIZE];
	  if( fin.getline(line,LSIZE) ){
	    std::string lstr(line);
	    std::vector<std::string> t;
	    tokenize(lstr,t,sep);
	    if( t.size()>1 ){
	      int start = t[0].length();
	      int next = lstr.find_first_not_of(sep,start);	    
	      std::string rest = std::string(lstr,next);
	      db.insert( make_pair(t[0],rest) );
	    }
	  }
	}
	fin.close();
      } else {
	std::cerr<<"file not found:"<<filename<<std::endl;
      }
    }
    ~Mgr(void){dump();}
    void clear(void){db.clear();}
    void clear(std::string key){db.erase(key);}
    void set(std::string key, std::string value){
      db.insert( make_pair(key,value) );      
    }
    void replace(std::string key, std::string value){
      clear(key);
      set(key,value);
    }
    bool get(std::string key, std::vector<std::string>& res){      
      bool found=false;
      Mgr::Iter iter;
      for(iter=db.lower_bound(key);iter !=db.upper_bound(key);++iter){
	res.push_back(iter->second);
	found=true;
      }
      return found;
    }

    void dump(void){
      std::ofstream fout(filename.c_str());
      if( fout ){
	for(Mgr::Iter i=db.begin();i!=db.end();++i)
	  fout<<(*i).first<<" "<<(*i).second<<std::endl;
	fout.close();
      }
    }

    void
    tokenize(std::string input,
	     std::vector<std::string>& tokens,
	     std::string sep){
      std::string cur = input;
      int done=0;
      tokens.erase(tokens.begin(),tokens.end());
      while( ! done ){
	int start = cur.find_first_not_of(sep);
	int end = cur.find_first_of(sep,start+1);
	if( start == -1 || end == -1 ){
	  if( start != -1 )
	    tokens.push_back( std::string( cur, start ) );
	  return;
	}
	tokens.push_back( std::string( cur, start, end-start ) );
	cur = std::string(cur, end+1);
      }
    }
  
    friend std::ostream& operator<<(std::ostream& ostr,Mgr& mgr){
      ostr<<"===["<<mgr.db.size()<<" entries]==="<<std::endl;
      Mgr::Iter i=mgr.db.begin();
      for(;i!=mgr.db.end();++i)
	ostr<<"["<<(*i).first<<"] => "<<(*i).second<<std::endl;
      return ostr<<std::endl;
    }
    std::string filename;
    std::multimap<std::string,std::string> db;
  };

}; //namespace DB


#endif
