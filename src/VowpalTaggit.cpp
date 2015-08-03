#include <boost/algorithm/string.hpp>
#include <boost/timer.hpp>

#include "StaticData.hpp"
#include "VowpalTaggit.hpp"
#include "Features.hpp"
#include "Examples.hpp"
#include "Search.hpp"

VowpalTaggit::VowpalTaggit()
: currSent_(nullptr), vw_(vwInit()),
  example_(new Examples(vw_)), sl_(new SequenceLabeler(vw_)),
  sentencesLearned_(0)
{
  FF::registerFeatures();
}

VowpalTaggit::~VowpalTaggit() {
  VW::finish(*vw_);
  delete example_;
  delete sl_;
}

vw* VowpalTaggit::vwInit() { 
  std::stringstream vwSS;
  std::string vwString;
  if(StaticData::Has("vw-override")) {
    vwSS << StaticData::Get<std::string>("vw-override");
  }
  else {
    if(StaticData::Has("vw-args"))
      vwString = StaticData::Get<std::string>("vw-args");
    if(StaticData::Has("train"))
      vwSS << vwTrainString(vwString);
    else
      vwSS << vwTestString(vwString);
  }
  std::cerr << vwSS.str() << std::endl;
  return VW::initialize(vwSS.str());
}

VowpalTaggit& VowpalTaggit::addHook(std::function<void(VowpalTaggit&)> hook) {
  hooks_.push_back(hook);
  return *this;
}

VowpalTaggit& VowpalTaggit::clearHooks() {
  hooks_.clear();
  return *this;
}

VowpalTaggit& VowpalTaggit::report(std::ostream& o) {
  double loss = vw_->sd->sum_loss;
  double examples = vw_->sd->weighted_examples;
  char buffer [100];
  sprintf(buffer, "%*ld\t%.4f", 6, sentencesLearned_, loss/examples);
  o << buffer;
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

VowpalTaggit& VowpalTaggit::read(std::istream& in) {
  std::string line;
  sentencesLearned_ = 0;
  while(std::getline(in, line))
    readLine(line);
  return *this;
}

VowpalTaggit& VowpalTaggit::readLine(const std::string& line) {
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
      eos();
    }
  }
  return *this;
}

VowpalTaggit& VowpalTaggit::save(const std::string& predictor) {
  VW::save_predictor(*vw_, predictor);
}

//VowpalTaggit& VowpalTaggit::load(const std::string& predictor) {
//  VW::load_predictor(*vw_, predictor);
//}

VowpalTaggit& VowpalTaggit::bos() {
  if(currSent_ != nullptr)
    delete currSent_;
  currSent_ = example_->newSent();
  return *this;
}

VowpalTaggit& VowpalTaggit::eos() {
  currSent_->eos();
  for(auto& hook : hooks_)
    hook(*this);
  return *this;
}

VowpalTaggit& VowpalTaggit::tok() { currSent_->tok(); return *this; }
VowpalTaggit& VowpalTaggit::lex() { currSent_->lex(); return *this; }
VowpalTaggit& VowpalTaggit::orth(const std::string& orth) { currSent_->orth(orth); return *this; }
VowpalTaggit& VowpalTaggit::base(const std::string& base) { currSent_->base(base); return *this; }
VowpalTaggit& VowpalTaggit::ctag(const std::string& ctag) { currSent_->ctag(ctag); return *this; }
VowpalTaggit& VowpalTaggit::oracle() { currSent_->oracle(); return *this; }

Sent& VowpalTaggit::getSent() {
  return *currSent_;
}

size_t VowpalTaggit::sentNum() {
  return sentencesLearned_;
}

void mkTrainer(VowpalTaggit& vt) {
    // Hooks take VowpalTaggit& as a parameter
  
  auto learner = [](VowpalTaggit& vt) {
    vt.learn();
  };
  
  std::shared_ptr<boost::timer> timer(new boost::timer());
  auto reporter = [=](VowpalTaggit& vt) {
    if(vt.sentNum() % 1000 == 0) {
      std::stringstream ss;
      vt.report(ss);
      char buffer [100];
      sprintf(buffer, "%*.1f", 6, timer->elapsed());
      ss << "\t" << buffer << " s";
      std::cerr << ss.str() << std::endl;
    }
  };
  
  auto saver = [](VowpalTaggit& vt) {
    if(vt.sentNum() % 10000 == 0) {
      char buffer[100];
      sprintf(buffer, "model.weights.%ld", vt.sentNum());
      std::cerr << "Saving model to " << buffer << std::endl;
      vt.save(buffer);
    }
  };
  
  vt.clearHooks();
  vt(learner)(reporter);
}

void mkPredictor(VowpalTaggit& vt) {
  size_t c = 0;
  auto predictor = [&](VowpalTaggit& vt) {
    std::vector<int> output;
    vt.predict(output);
    for(auto i : output)
      std::cout << i << " ";
    std::cout << std::endl;
    c++;
    if(c % 1000 == 0)
      std::cerr << c << std::endl;
  };
  
  vt.clearHooks();
  vt(predictor);
}

