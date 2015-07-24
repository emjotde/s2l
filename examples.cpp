#include <iostream>
#include <sstream>

#include "VowpalTaggit.hpp"

int main(int argc, char *argv[]) {
  VowpalTaggit vt;
  
  std::ios_base::sync_with_stdio(false);
  std::string line;
  while(std::getline(std::cin, line))
    vt.read(line);
    
  //vt.save()
}
