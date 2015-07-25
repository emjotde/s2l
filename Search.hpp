#pragma once

#include <vowpalwabbit/vw.h>
#include <vowpalwabbit/ezexample.h>

#include <string>
#include <vector>
#include <map>
#include <sstream>

#include "Examples.hpp"
#include "libsearch.h"

class SequenceLabeler : public SearchTask<Sent, vector<int>> {
  public:
  SequenceLabeler(vw* vw_obj)
      : SearchTask<Sent, vector<int> >(*vw_obj) { 
    sch.set_options( Search::AUTO_HAMMING_LOSS | Search::NO_CACHING | Search::IS_LDF );
  }

  void AddConditions(Morfs& morfs, example* prev, char ns) {
    for(size_t i = 0; i < morfs.size(); ++i) {
       ezexample mex(&vw_obj, &morfs.data()[i]);
       mex.add_other_example_ns(*prev, 't', ns);
    }
  }  
  
  void StripConditions(Morfs& morfs) {
    for(size_t i = 0; i < morfs.size(); ++i) {
      ezexample mex(&vw_obj, &morfs.data()[i]);
      --mex;
    }
  }  
  
  void _run(Search::search& sch, Sent& sentence, vector<int>& output) {
    output.clear();
    
    size_t hl = 2;
    
    for(size_t i = 0; i < sentence.size(); i++) {
      // Copy constructor
      Morfs& morfs = sentence[i];
      
      //for(size_t j = 0; j < hl; j++) {
      //  if(i > j) {
      //    int prev = output[i - j - 1];
      //    AddConditions(morfs, &sentence[i - j - 1].data()[prev], 'p' + j);
      //  }
      //}
      
      action p = Search::predictor(sch, i + 1)
        .set_input(morfs.data(), morfs.size())
        .set_oracle(morfs.oracle())
        .set_condition_range(i, hl, 'p')
        .predict();
      
      //for(size_t j = 0; j < hl; j++) {
      //  if(i > j) {
      //    int prev = output[i - j - 1];
      //    StripConditions(morfs);
      //  }
      //}
        
      output.push_back(p);
    }
  }
};

