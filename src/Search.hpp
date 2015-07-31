#pragma once

#include <vowpalwabbit/vw.h>
#include <vowpalwabbit/ezexample.h>

#include <string>
#include <vector>
#include <map>
#include <sstream>

#include "StaticData.hpp"
#include "Examples.hpp"
#include "libsearch.h"

typedef std::vector<int> History;

class SequenceLabeler : public SearchTask<Sent, History> {
  public:
    
  SequenceLabeler(vw* vw_obj)
      : SearchTask<Sent, History>(*vw_obj) { 
    sch.set_options( Search::AUTO_HAMMING_LOSS | Search::NO_CACHING | Search::IS_LDF );
  }
  
  void AddConditions(Tok& tok, Lex& prev, char ns) {
    for(size_t i = 0; i < tok.size(); ++i) {
      FeatureNS temp;
      auto& tempF = temp[ns];
      auto& lexF = tok[i].getNamespace()['t'];
      
      for(auto& f : prev.getNamespace()['t']) {
        tempF.emplace_back(f.name);
        if(std::binary_search(lexF.begin(), lexF.end(), f))
          tempF.emplace_back("m^" + f.name);
      }
      tok.nsToExample(&tok.morfs()[i], temp);
    }
  }
  
  void AddConditions(Sent& sentence, History& output, size_t i, size_t hl) {
    for(size_t j = 0; j < hl; j++) {
      if(i > j) {
        int prev = output[i - j - 1];
        AddConditions(sentence[i], sentence[i - j - 1][prev], hns_ + j);
      }
    }
  }
  
  void StripConditions(Tok& tok, char ns) {
    Morfs& morfs = tok.morfs();
    for(size_t i = 0; i < morfs.size(); ++i) {
      example* ex = &morfs[i];      
      if(ex->indices.size() != 0) {
        char curr_ns;
        for(unsigned char *i = ex->indices.begin; i != ex->indices.end; ++i)
          curr_ns = *i;
        
        if(curr_ns == ns) {
          ex->total_sum_feat_sq -= ex->sum_feat_sq[(int)curr_ns];
          ex->sum_feat_sq[(int)curr_ns] = 0;
          ex->num_features -= ex->atomics[(int)curr_ns].size();
          ex->atomics[(int)curr_ns].erase();
        }
        ex->indices.pop();
      }
    }
  }
  
  void StripConditions(Sent& sentence, size_t i, size_t hl) {
    for(int j = hl - 1; j >= 0; j--) {
      if(i > j) 
        StripConditions(sentence[i], hns_ + j);
    }
  }
  
  void _run(Search::search& sch, Sent& sentence, History& output) {
    output.clear();
    
    const size_t history_length = StaticData::Get<size_t>("history-length");
    
    for(size_t i = 0; i < sentence.size(); i++) {
      AddConditions(sentence, output, i, history_length);
      
      Morfs& morfs = sentence[i].morfs();
      action p = Search::predictor(sch, i + 1)
        .set_input(morfs.data(), morfs.size())
        .set_oracle(morfs.oracle())
        .add_condition_range(i, history_length, hns_)
        .predict();
      
      StripConditions(sentence, i, history_length);
      output.push_back(p);
    }
  }
  
  private:
    const char hns_ = '0';
    
    static std::vector<std::function<void(Lex&, History&, Features&)>> hooks_;
};

