#ifndef _STR_H_
#define _STR_H_

#include <iostream>
#include <string>
#include <vector>

namespace str {
  void tokenize(std::string, std::vector<std::string>&, std::string=" \t\n");  

  std::string trim(std::string);
  void trim(std::vector<std::string>&);

  std::string lower(std::string);
  void lower(std::vector<std::string>&);

  std::string replace(std::string, char, char);
  std::string replace(std::string, std::string, char);
  std::string strip(std::string, char);
  std::string strip(std::string, std::string);

  std::string itoa(int);

};

#endif
