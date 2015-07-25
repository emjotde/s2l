#include <boost/program_options.hpp>

#include "Examples.hpp"
#include "Search.hpp"
#include "VowpalTaggit.hpp"

VowpalTaggit::VowpalTaggit(int argc, char** argv)
: currSent_(nullptr),
  vw_(vwInit(argc, argv)),
  example_(new Examples(vw_)),
  sl_(new SequenceLabeler(vw_)),
  sentencesLearned_(0)
{}

VowpalTaggit::~VowpalTaggit() {
  VW::finish(*vw_);
  delete example_;
  delete sl_;
}

vw* VowpalTaggit::vwInit(int argc, char** argv) {
  namespace po = boost::program_options;
  
  bool testing;
  bool help;
  std::string finalModel;
  std::string initialModel;
  
  po::options_description general("General options");
  general.add_options()
    ("final_model,f", po::value<std::string>(&finalModel),
     "Save final model to file  arg")
    ("testing,t", po::value(&testing)->zero_tokens()->default_value(false),
     "Do not train")
    ("initial_model,i", po::value<std::string>(&initialModel),
     "Load initial model from file  arg")
    ("help,h", po::value(&help)->zero_tokens()->default_value(false),
     "Print this help message and exit")
  ;

  po::options_description cmdline_options("Allowed options");
  cmdline_options.add(general);
  po::variables_map vm;
  
  try { 
    po::store(po::command_line_parser(argc,argv).
              options(cmdline_options).run(), vm);
    po::notify(vm);
  }
  catch (std::exception& e) {
    std::cout << "Error: " << e.what() << std::endl << std::endl;
    
    std::cout << "Usage: " + std::string(argv[0]) +  " [options]" << std::endl;
    std::cout << cmdline_options << std::endl;
    exit(0);
  }
  
  if (help) {
    std::cout << "Usage: " + std::string(argv[0]) +  " [options]" << std::endl;
    std::cout << cmdline_options << std::endl;
    exit(0);
  }

  std::stringstream vwSS;
  if(testing)
    vwSS << vwTestString();
  else
    vwSS << vwTrainString();
  if(!initialModel.empty())
    vwSS << " -i " << initialModel; 
  if(!finalModel.empty())
    vwSS << " -f " << finalModel; 
  
  std::cerr << vwSS.str() << std::endl;
  return VW::initialize(vwSS.str());
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
      
      for(auto& hook : hooks_)
        hook(*this);
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

