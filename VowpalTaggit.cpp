#include "Search.hpp"
#include "VowpalTaggit.hpp"

VowpalTaggit::VowpalTaggit()
: currSent_(nullptr),
  vw_(vwInit()),
  example_(new Examples(vw_)),
  sl_(new SequenceLabeler(vw_)),
  sentencesLearned_(0)
{}

VowpalTaggit::~VowpalTaggit() {
  VW::finish(*vw_);
  delete example_;
  delete sl_;
}

vw* VowpalTaggit::vwInit() {
  return VW::initialize(vwString());
}

VowpalTaggit& VowpalTaggit::report(std::ostream& o) {
  double loss = vw_->sd->sum_loss;
  double examples = vw_->sd->weighted_examples;
  o << sentencesLearned_ << "\t";
  o << loss/examples << std::endl;
  return *this;
}

VowpalTaggit& VowpalTaggit::learn() {
  std::vector<int> output;
  sl_->learn(*currSent_, output);
  sentencesLearned_++;
}

VowpalTaggit& VowpalTaggit::predict(std::vector<int>& output) {
  sl_->predict(*currSent_, output);
}

VowpalTaggit& VowpalTaggit::read(const std::string line) {
  std::stringstream tokens(line);
  std::string action;
  std::string param;
  if(tokens >> action) {
    if(action == "bos") {
      bos();
    } else if(action == "tok") {
      tok();
    } else if(action == "orth") {
      tokens >> param;
      orth(param);
    } else if(action == "lex") {
      lex();
    } else if(action == "base") {
      tokens >> param;
      base(param);
    } else if(action == "ctag") {
      tokens >> param;
      ctag(param);
    } else if(action == "oracle") {
      oracle();
    } else if(action == "eos") {
      eos().learn();
      
      if(sentencesLearned_ % 1000 == 0)
        report(std::cerr);
    }
  }
  return *this;
}

VowpalTaggit& VowpalTaggit::bos() {
  if(currSent_ != nullptr)
    delete currSent_;
  currSent_ = example_->newSent(); return *this;
}
VowpalTaggit& VowpalTaggit::eos() { currSent_->eos(); return *this; }
VowpalTaggit& VowpalTaggit::tok() { currSent_->tok(); return *this; }
VowpalTaggit& VowpalTaggit::lex() { currSent_->lex(); return *this; }
VowpalTaggit& VowpalTaggit::orth(const std::string orth) { currSent_->orth(orth); return *this; }
VowpalTaggit& VowpalTaggit::base(const std::string base) { currSent_->base(base); return *this; }
VowpalTaggit& VowpalTaggit::ctag(const std::string ctag) { currSent_->ctag(ctag); return *this; }
VowpalTaggit& VowpalTaggit::oracle() { currSent_->oracle(); return *this; }

Sent& VowpalTaggit::getSent() {
  return *currSent_;
}

