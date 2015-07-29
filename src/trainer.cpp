#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>

#include "VowpalTaggit.hpp"

int main(int argc, char **argv) {
  namespace po = boost::program_options;

  bool help;
  size_t passes;
  std::string trainFile;
  std::string testFile;
  
  po::options_description general("Trainer options");
  general.add_options()
    ("train", po::value<std::string>(&trainFile),
     "Path to training data")
    ("test", po::value<std::string>(&testFile),
     "Path to test data")
    ("passes,p", po::value<size_t>(&passes)->default_value(1),
     "Number of training passes")
    ("help,h", po::value(&help)->zero_tokens()->default_value(false),
     "Print this help message and exit")
  ;

  po::options_description cmdline_options("Allowed options");
  cmdline_options.add(general);
  po::variables_map vm;
  
  try { 
    po::store(po::command_line_parser(argc,argv).allow_unregistered().
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

  VowpalTaggit vt(argc, argv);
  
  if(!trainFile.empty()) {
    mkTrainer(vt);
    for(size_t i = 0; i < passes; ++i) {
      std::cerr << "Starting pass " << (i+1) << "/" << passes << std::endl;
      std::ifstream train(trainFile.c_str());
      vt(train);
    }
  }
  
  if(!testFile.empty()) {
    mkPredictor(vt);
    std::ifstream test(testFile.c_str());
    vt(test);
  }
}
