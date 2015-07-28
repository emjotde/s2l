#pragma once

#include <vector>
#include <memory>

class Examples;
class SequenceLabeler;
class Sent;
class vw;

class VowpalTaggit {
  public:
    VowpalTaggit(const std::string&);
    VowpalTaggit(int argc, char **argv);
    ~VowpalTaggit();
  
    static vw* vwInit(std::string);
    static vw* vwInit(int argc, char **argv);
  
    static std::string vwTrainString() {
      return
        " --quiet --csoaa_ldf mc --search 0 --search_task hook"
        " -b 28 -q t:";
    }
    
    static std::string vwTestString() {
      return
        " --quiet -t ";
    }
    
    VowpalTaggit& addHook(std::function<void(VowpalTaggit&)> hook) {
      hooks_.push_back(hook);
      return *this;
    }
    
    VowpalTaggit& clearHooks() {
      hooks_.clear();
      return *this;
    }
    
    VowpalTaggit& report(std::ostream& o);
  
    VowpalTaggit& learn();
    VowpalTaggit& predict(std::vector<int>& output);
        
    VowpalTaggit& read(std::istream& in) {
      std::string line;
      sentencesLearned_ = 0;
      while(std::getline(in, line))
        readLine(line);
      return *this;
    }
    VowpalTaggit& readLine(const std::string& line);
        
    VowpalTaggit& save(const std::string& predictor);
    
    VowpalTaggit& bos();
    VowpalTaggit& eos();
    VowpalTaggit& tok();
    VowpalTaggit& lex();
    VowpalTaggit& orth(const std::string orth);
    VowpalTaggit& base(const std::string base);
    VowpalTaggit& ctag(const std::string ctag);
    VowpalTaggit& oracle();
    
    Sent& getSent();
    size_t sentNum() {
      return sentencesLearned_;
    }
  
  // convienience functions
  
  VowpalTaggit& operator()(std::function<void(VowpalTaggit&)> hook) { return addHook(hook); }
  VowpalTaggit& operator()(std::istream& in) { return read(in); }
  
  private:
    Sent* currSent_;
    bool testing_;
    vw* vw_;
    
    Examples* example_;
    SequenceLabeler* sl_;
    
    size_t sentencesLearned_;
    std::vector<std::function<void(VowpalTaggit&)>> hooks_;
};

