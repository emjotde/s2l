#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>

#include "StaticData.hpp"
#include "VowpalTaggit.hpp"

int main(int argc, char **argv) {
  
  StaticData::Init(argc, argv);
  
  VowpalTaggit vt;
  
  if(StaticData::Has("train")) {
    mkTrainer(vt);
    size_t passes = StaticData::Get<size_t>("passes");
    std::string trainFile = StaticData::Get<std::string>("train");
    
    for(size_t i = 0; i < passes; ++i) {
      std::cerr << "Starting pass " << (i+1) << "/" << passes << std::endl;
      std::ifstream train(trainFile.c_str());
      vt(train);
      
      if(StaticData::Has("final-model") &&
         StaticData::Get<bool>("save-per-pass") && passes > 1) {
        std::string modelFile = StaticData::Get<std::string>("final-model");
        std::string perPassModelFile = modelFile + "." + std::to_string(i+1);
        vt.save(perPassModelFile);
      }
    }
    
    if(StaticData::Has("final-model")) {
      std::string modelFile = StaticData::Get<std::string>("final-model");
      vt.save(modelFile);
    }
  }
  
  if(StaticData::Has("test")) {
    std::string testFile = StaticData::Get<std::string>("test");
    mkPredictor(vt);
    std::ifstream test(testFile.c_str());
    vt(test);
  }
  
  
  return 0;
}
