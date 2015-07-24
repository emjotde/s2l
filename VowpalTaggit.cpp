#include "Search.hpp"
#include "VowpalTaggit.hpp"


VowpalTaggit::VowpalTaggit()
: currSent_(nullptr),
  example_(new Examples("--hash all -b 24 --csoaa_ldf mc --quiet --search 0 --search_task hook")),
  sl_(new SequenceLabeler(example_->getVw()))
{}

VowpalTaggit& VowpalTaggit::learn() {
  std::vector<uint32_t> output;
  sl_->learn(*currSent_, output);
}

VowpalTaggit& VowpalTaggit::predict(std::vector<uint32_t>& output) {
  sl_->predict(*currSent_, output);
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

