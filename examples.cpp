#include <iostream>
#include <sstream>

#include "VowpalTaggit.hpp"

//vw = pyvw.vw("--search 0 --hash all -b 31 --csoaa_ldf mc --quiet --search_task hook -q t: -q m: --ngram t2 --ngram m2 --ngram g2 --ngram c2")


int main(int argc, char **argv) {
  std::ios_base::sync_with_stdio(false);
  
  auto learner = [](VowpalTaggit& vt) {
    vt.learn();
  };
  
  auto reporter = [](VowpalTaggit& vt) {
    if(vt.sentNum() % 1000 == 0)
      vt.report(std::cerr);
  };
  
  auto saver = [](VowpalTaggit& vt) {
    if(vt.sentNum() % 10000 == 0) {
      char buffer[100];
      sprintf(buffer, "model.weights.%ld", vt.sentNum());
      std::cerr << "Saving model to " << buffer << std::endl;
      vt.save(buffer);
    }
  };
  
  VowpalTaggit(argc, argv)
    (learner)(reporter)(saver)
    (std::cin)
  ;
}
