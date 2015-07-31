#include <vector>
#include <boost/algorithm/string.hpp>

#include "StaticData.hpp"

StaticData StaticData::instance_;

StaticData& StaticData::Init(const std::string& initString) {
  std::vector<std::string> args;
  boost::split(args, initString, boost::is_any_of(" "));
  int argc = args.size() + 1;
  char* argv[argc];
  argv[0] = const_cast<char*>("bogus");
  for(size_t i = 1; i < argc; i++)
    argv[i] = const_cast<char*>(args[i-1].c_str());
  return Init(argc, argv);
}

StaticData& StaticData::Init(int argc, char** argv) {
  return Instance().NonStaticInit(argc, argv);
}

StaticData& StaticData::NonStaticInit(int argc, char** argv) {
  
  po::options_description general("General options");
  general.add_options()
    ("vw_args", po::value<std::string>(),
     "Options passed on to Vowpal Wabbit instance")
    ("train", po::value<std::string>(),
     "Path to training data")
    ("test", po::value<std::string>(),
     "Path to test data")
    ("model", po::value<std::string>(),
     "Path to final model")
    ("passes,p", po::value<size_t>()->default_value(1),
     "Number of training passes")
    ("help,h", po::value<bool>()->zero_tokens()->default_value(false),
     "Print this help message and exit")
  ;

  po::options_description search("Search options");
  search.add_options()
    ("history_length", po::value<size_t>()->default_value(3),
     "History length for Learning to Search algorithm")
  ;

  po::options_description features("Feature options");
  features.add_options()
    ("classes", po::value<std::string>(),
     "Path to word class dictionary")
    ("embeddings", po::value<std::string>(),
     "Path to word embedding dictionary")
    ("window,w", po::value<size_t>()->default_value(2),
     "Window size for window based features")
    ("affix,a", po::value<size_t>()->default_value(5),
     "Maximum number of characters in prefixes and suffixes")
    ("debug,d", po::value<bool>()->zero_tokens()->default_value(false),
     "Print all features to stderr")
  ;

  po::options_description cmdline_options("Allowed options");
  cmdline_options.add(general);
  cmdline_options.add(search);
  cmdline_options.add(features);
  
  try { 
    po::store(po::command_line_parser(argc,argv).options(cmdline_options).run(), vm_);
    po::notify(vm_);
  }
  catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl << std::endl;
    
    std::cerr << "Usage: " + std::string(argv[0]) +  " [options]" << std::endl;
    std::cerr << cmdline_options << std::endl;
    exit(0);
  }
  
  if (Get<bool>("help")) {
    std::cerr << "Usage: " + std::string(argv[0]) +  " [options]" << std::endl;
    std::cerr << cmdline_options << std::endl;
    exit(0);
  }
  
  return *this;
}
