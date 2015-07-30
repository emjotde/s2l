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
    VowpalTaggit(const std::string&);
    VowpalTaggit(int argc, char **argv);
    ~VowpalTaggit();
  
    static vw* vwInit();
  
    static std::string vwTrainString() {
      return
        " --quiet --csoaa_ldf mc --search 0 --search_task hook"
        " -b 26 -q t:";
    }
    
    static std::string vwTestString() {
      return
        " --quiet -t ";
    }
    
    VowpalTaggit& addHook(std::function<void(VowpalTaggit&)> hook);
    VowpalTaggit& clearHooks();
    
    VowpalTaggit& report(std::ostream& o);
  
    VowpalTaggit& learn();
    VowpalTaggit& predict(std::vector<int>& output);
        
    VowpalTaggit& read(std::istream& in);
    VowpalTaggit& readLine(const std::string& line);
    VowpalTaggit& save(const std::string& predictor);
    
    VowpalTaggit& bos();
    VowpalTaggit& eos();
    VowpalTaggit& tok();
    VowpalTaggit& lex();
    VowpalTaggit& orth(const std::string& orth);
    VowpalTaggit& base(const std::string& base);
    VowpalTaggit& ctag(const std::string& ctag);
    VowpalTaggit& oracle();
    
    Sent& getSent();
    size_t sentNum();
  
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

void mkTrainer(VowpalTaggit&);
void mkPredictor(VowpalTaggit&);
