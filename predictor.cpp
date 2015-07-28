#include <iostream>
#include <sstream>

#include "Examples.hpp"
#include "VowpalTaggit.hpp"

//vw = pyvw.vw("--search 0 --hash all -b 31 --csoaa_ldf mc --quiet --search_task hook -q t: -q m: --ngram t2 --ngram m2 --ngram g2 --ngram c2")

int main(int argc, char **argv) {
  std::ios_base::sync_with_stdio(false);
  
  // Hooks take VowpalTaggit& as a parameter
  
  size_t c = 0;
  auto predictor = [&](VowpalTaggit& vt) {
    std::vector<int> output;
    vt.predict(output);
    for(auto i : output)
      std::cout << i << " ";
    std::cout << std::endl;
    c++;
    if(c % 1000 == 0)
      std::cerr << c << std::endl;
  };
    
  VowpalTaggit(argc, argv)(predictor)(std::cin);
}