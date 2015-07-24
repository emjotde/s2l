#include <iostream>

#include "VowpalTaggit.hpp"

int main(int argc, char *argv[]) {
  VowpalTaggit vt;
  
  vt.bos()
    .tok().orth("test")
      .lex().base("test1").ctag("t:t:t:t")
      .lex().base("test2").ctag("t:t:t:t").oracle()
    .eos().learn();
  
  std::vector<uint32_t> predict;
  
  vt.bos()
    .tok().orth("test")
      .lex().base("test2").ctag("t:t:t:t")
      .lex().base("test1").ctag("t:t:t:t")
    .eos().predict(predict);
  
  std::cout << predict[0] << std::endl;
}
