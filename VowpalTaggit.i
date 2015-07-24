// File : gd.i
%module VowpalTaggit

#undef seed


%include "std_vector.i"
%include "std_string.i"

namespace std {
    %template(vectori) vector<int>;
    %template(vectord) vector<double>;
};

%{
#include "VowpalTaggit.hpp"
%}


class VowpalTaggit {
  public:
    VowpalTaggit();
    
    VowpalTaggit& learn();
    
    VowpalTaggit& predict(std::vector<int>& output);
    
    VowpalTaggit& bos();
    VowpalTaggit& eos();

    VowpalTaggit& tok();
    VowpalTaggit& lex();
    VowpalTaggit& orth(const std::string orth);
    VowpalTaggit& base(const std::string base);
    VowpalTaggit& ctag(const std::string ctag);
    VowpalTaggit& oracle();
    
    Sent& getSent();
};

