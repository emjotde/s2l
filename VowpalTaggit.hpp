#pragma once

#include <vector>
#include <memory>

class Examples;
class SequenceLabeler;
class Sent;

class VowpalTaggit {
  public:
    VowpalTaggit();
    
    VowpalTaggit& learn();
    
    VowpalTaggit& predict(std::vector<uint32_t>& output);
    
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
    
    Examples* example_;
    SequenceLabeler* sl_; 
};

