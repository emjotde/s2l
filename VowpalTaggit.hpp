#pragma once

#include <vector>
#include <memory>

class Examples;
class SequenceLabeler;
class Sent;
class vw;

class VowpalTaggit {
  public:
    VowpalTaggit();
    ~VowpalTaggit();
  
    static vw* vwInit();
  
    static std::string vwString() {
      return
        " --hash all --quiet --csoaa_ldf mc --search 0 --search_task hook"
        " --noconstant -b 31 -q t:";
    }
    
    VowpalTaggit& report(std::ostream& o);
  
    VowpalTaggit& learn();
    VowpalTaggit& predict(std::vector<int>& output);
    
    VowpalTaggit& read(const std::string line);
    
    VowpalTaggit& bos();
    VowpalTaggit& eos();
    VowpalTaggit& tok();
    VowpalTaggit& lex();
    VowpalTaggit& orth(const std::string orth);
    VowpalTaggit& base(const std::string base);
    VowpalTaggit& ctag(const std::string ctag);
    VowpalTaggit& oracle();
    
    Sent& getSent();
  
  private:
    Sent* currSent_;
    
    vw* vw_;
    
    Examples* example_;
    SequenceLabeler* sl_;
    
    size_t sentencesLearned_;
};

