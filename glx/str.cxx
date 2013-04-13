#include <sstream>
#include "str.h"
#include "debug.h"

using namespace std;

void
str::tokenize(string input, vector<string>& tokens, string sep)
{
  FANCYMESG("tokenize");
  string cur = input;
  int done=0;
  tokens.erase(tokens.begin(),tokens.end());
  while( ! done ){
    int start = cur.find_first_not_of(sep);
    int end = cur.find_first_of(sep,start+1);

    VAR(string( cur, start, end-start ));

    if( start == -1 || end == -1 ){
      MESG("start or end == -1");
      if( start != -1 )
        tokens.push_back( string( cur, start ) );
      return;
    }
    VAR2(start,end);
    tokens.push_back( string( cur, start, end-start ) );
    cur = string(cur, end+1);
  }
}

std::string 
str::itoa(int i)
{
  ostringstream buffer;
  buffer << i;
  return buffer.str();
}

std::string
str::trim(std::string src)
{
  string ws(" \t\n");
  string::size_type nStart = src.find_first_not_of(ws);
  string::size_type nEnd = src.find_first_of(ws,nStart+1);
  return src.substr(nStart, nEnd-nStart);
}

void 
str::trim(std::vector<std::string>& svec)
{
  vector<string>::iterator iter = svec.begin();  
  for( ; iter != svec.end() ; ++iter ){
    string s = trim(*iter);
    *iter = s;
  }
}

std::string
str::lower(std::string src)
{
  for(unsigned i=0; i<src.length() ; ++i )
    src[i]=tolower(src[i]);
  return src;
}

void 
str::lower(vector<string>& svec)
{
  vector<string>::iterator iter = svec.begin();  
  for( ; iter != svec.end() ; ++iter )
    *iter = lower( *iter );
}

string 
str::replace(string src, char schar, char dchar)
{
  char dst[80];
  int di=0;
  for(unsigned si=0;si<src.length();++si){
    if( src[si]==schar )      
      dst[di++] = dchar;
    else
      dst[di++] = src[si];
  }
  dst[di++]='\0';
  return string(dst);
}

string 
str::replace(string src, string charclass, char dchar)
{
  char dst[80];
  int di=0;
  for(unsigned si=0;si<src.length();++si){
    bool found=false;
    for(int j=0;j<charclass.length()&&!found;++j){
      if( src[si]==charclass[j] )
	found=true;    
    }
    if( found )      
      dst[di++] = dchar;
    else
      dst[di++] = src[si];
  }
  dst[di++]='\0';
  return string(dst);
}

string 
str::strip(string src, char ch)
{
  char dst[80];
  int di=0;
  for(unsigned si=0;si<src.length();++si){
    if( src[si]!=ch )
      dst[di++] = src[si];
  }
  dst[di++]='\0';
  return string(dst);
}

string 
str::strip(string src, string charclass)
{  
  char dst[80];
  int di=0;
  for(unsigned si=0;si<src.length();++si){
    bool found=false;
    for(unsigned j=0;j<charclass.length()&&!found;++j){
      if( src[si]==charclass[j] )
	found=true;      
    }
    if( ! found )
      dst[di++] = src[si];
  }
  dst[di++]='\0';
  return string(dst);
}
