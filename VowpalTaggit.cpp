#include <boost/algorithm/string.hpp>

#include "VowpalTaggit.hpp"

#include "Features.hpp"
#include "Examples.hpp"
#include "Search.hpp"

VowpalTaggit::VowpalTaggit() : VowpalTaggit("") {}

VowpalTaggit::VowpalTaggit(const std::string& cli)
: currSent_(nullptr),
  vw_(vwInit(cli)),
  example_(new Examples(vw_)),
  sl_(new SequenceLabeler(vw_)),
  sentencesLearned_(0)
{
  FF::registerFeatures();
}

VowpalTaggit::VowpalTaggit(int argc, char** argv)
: currSent_(nullptr),
  vw_(vwInit(argc, argv)),
  example_(new Examples(vw_)),
  sl_(new SequenceLabeler(vw_)),
  sentencesLearned_(0)
{
  FF::registerFeatures();  
}

VowpalTaggit::~VowpalTaggit() {
  VW::finish(*vw_);
  delete example_;
  delete sl_;
}

vw* VowpalTaggit::vwInit(std::string cli) {
  std::vector<std::string> args;
  boost::split(args, cli, boost::is_any_of(" "));
  int argc = args.size() + 1;
  char* argv[argc];
  argv[0] = const_cast<char*>("bogus");
  for(size_t i = 1; i < argc; i++)
    argv[i] = const_cast<char*>(args[i-1].c_str());
  return vwInit(argc, argv);
}

vw* VowpalTaggit::vwInit(int argc, char** argv) {
  //namespace po = boost::program_options;
  //
  //bool testing = false;
  //bool help;
  //bool noop;
  //
  //std::string finalModel;
  //std::string initialModel;
  //
  //po::options_description general("General options");
  //general.add_options()
  //  ("final_model,f", po::value<std::string>(&finalModel),
  //   "Save final model to file  arg")
  //  //("testing,t", po::value(&testing)->zero_tokens()->default_value(false),
  //  // "Do not train")
  //  ("initial_model,i", po::value<std::string>(&initialModel),
  //   "Load initial model from file  arg")
  //  ("noop,n", po::value(&noop)->zero_tokens()->default_value(false),
  //   "Print this help message and exit")("help,h", po::value(&help)->zero_tokens()->default_value(false),
  //   "Print this help message and exit")
  //;
  //
  //po::options_description cmdline_options("Allowed options");
  //cmdline_options.add(general);
  //po::variables_map vm;
  //
  //try { 
  //  po::store(po::command_line_parser(argc,argv).allow_unregistered().
  //            options(cmdline_options).run(), vm);
  //  po::notify(vm);
  //}
  //catch (std::exception& e) {
  //  std::cout << "Error: " << e.what() << std::endl << std::endl;
  //  
  //  std::cout << "Usage: " + std::string(argv[0]) +  " [options]" << std::endl;
  //  std::cout << cmdline_options << std::endl;
  //  return;
  //}
  //
  //if (help) {
  //  std::cout << "Usage: " + std::string(argv[0]) +  " [options]" << std::endl;
  //  std::cout << cmdline_options << std::endl;
  //  return;
  //}

  std::stringstream vwSS;
  //if(testing)
  //  vwSS << vwTestString();
  //else
    vwSS << vwTrainString();
  //if(!initialModel.empty())
  //  vwSS << " -i " << initialModel; 
  //if(!finalModel.empty())
  //  vwSS << " -f " << finalModel; 
  //if(noop)
  //  vwSS << " --noop";
  
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
  sprintf(buffer, "%ld\t%.4f", sentencesLearned_, loss/examples);
  o << buffer << std::endl;
  
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
VowpalTaggit& VowpalTaggit::orth(const std::string orth) { currSent_->orth(orth); return *this; }
VowpalTaggit& VowpalTaggit::base(const std::string base) { currSent_->base(base); return *this; }
VowpalTaggit& VowpalTaggit::ctag(const std::string ctag) { currSent_->ctag(ctag); return *this; }
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
  
  auto reporter = [](VowpalTaggit& vt) {
    if(vt.sentNum() % 1000 == 0)
      vt.report(std::cerr);
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

