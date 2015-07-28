#include "Examples.hpp"

class CESWriter {
  public:
    CESWriter(std::ostream& out) : out_(out) {}
    
    void start() {
      out_ << R"(<?xml version="1.0" encoding="UTF-8"?>)" << std::endl
           << R"(<!DOCTYPE cesAna SYSTEM "xcesAnaIPI.dtd">)" << std::endl
           << R"(<cesAna xmlns:xlink="http://www.w3.org/1999/xlink" version="1.0" type="lex disamb">)" << std::endl
           << R"(<chunkList>)" << std::endl
           << R"( <chunk id="ch1" type="p">)" << std::endl;
    }
    
    void end() {
        out_ << R"( </chunk>)" << std::endl
           << R"(</chunkList>)" << std::endl
           << R"(</cesAna>)" << std::endl;
    }
    
    void operator()(VowpalTaggit& vt) {
      std::vector<int> output;
      vt.predict(output);
      
      out_ << R"(  <chunk type="s">)" << std::endl;
      size_t i = 0;
      for(auto& tok : vt.getSent()) {
        out_ << R"(   <tok>)" << std::endl;
        out_ << R"(    <orth>)" << tok.getOrth() << R"(</orth>)" << std::endl;
        size_t j = 0;
        for(auto& lex : tok) {
          out_ << ((j == output[i]) ? R"(     <lex disamb="1">)" : R"(     <lex>)")
               << R"(<base>)" << lex.getBase() << R"(</base>)"
               << R"(<ctag>)" << lex.getCtag() << R"(</ctag>)"
               << R"(</lex>)"
               << std::endl;
          j++;
        }
        out_ << R"(   </tok>)" << std::endl;
        i++;
        if(i % 1000 == 0)
          std::cerr << i << std::endl;
      }
      out_ << R"(  </chunk>)" << std::endl;
    }
    
  private:
    std::ostream& out_;
};
