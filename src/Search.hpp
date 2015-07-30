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
    //bla();
  }

  
  //void bla() {
  //  size_t hl = StaticData::Get<size_t>("history_length");    
  //  
  //  // Copy all ldf features from history up to history_length
  //  hooks_.push_back(
  //    [=](Lex& lex, History& h, Features& features) {
  //      size_t i = lex.getTok().getI();
  //      Sent& s  = lex.getTok().getSent();
  //      
  //      std::set<Feature> lexF(lex.getNamespace()['t'].begin(), lex.getNamespace()['t'].end());
  //      
  //      for(int j = 0; j < h.size() && j < hl; j++) {
  //        size_t k = h.size() - j - 1;
  //        Lex& prevLex = s[k][h[k]];
  //        for(auto& f: prevLex.getNamespace()['t']) {
  //          features.emplace_back("h" + std::to_string(j) + "^" + f.name);
  //          if(lexF.count(f))
  //            features.emplace_back("mh" + std::to_string(j) + "^" + f.name);
  //        }
  //      }
  //    }
  //  );

    
    // Copy all ldf features from history beginning at history_length
    //SequenceLabeler::FF(
    //  [](Lex& lex, History& h, Features& features) {
    //    size_t i = lex.getTok().getI();
    //    Sent& s  = lex.getTok().getSent();
    //    size_t hl = StaticData::Get<size_t>("history_length");
    //    for(int j = hl; j < h.size(); j++) {
    //      size_t k = h.size() - j - 1;
    //      Lex& prevLex = sent[k][h[k]];
    //      for(auto& f: prevLex.getNamespace()['t'])
    //        features.insert("h" + std::to_string(j) + "^" + f.name);
    //    }
    //  }
    //);
  //}
  
  //void AddConditionalFeatures(Tok& tok, History& history) {
  //  for(size_t i = 0; i < tok.size(); ++i) {
  //    FeatureNS temp;
  //    for(auto& f: hooks_)
  //      f(tok[i], history, temp[hns_]);
  //    tok.nsToExample(&tok.morfs()[i], temp);
  //  }
  //}
  
  void AddConditions(Tok& tok, Lex& prev, char ns) {
    for(size_t i = 0; i < tok.size(); ++i) {
      FeatureNS temp;
      auto& tempF = temp[ns];
      auto& lexF = tok[i].getNamespace()['t'];
      
      for(auto& f : prev.getNamespace()['t']) {
        tempF.emplace_back(f.name);
        if(std::binary_search(lexF.begin(), lexF.end(), f))
          tempF.emplace_back(f.name + "=" + f.name);
        //for(auto& g: tok[i].getNamespace()['t'])
        // temp[ns].insert(g.name + "*" + f.name);
      }
      tok.nsToExample(&tok.morfs()[i], temp);
    }
  }
  
  void AddConditions(Sent& sentence, std::vector<int>& output,
                     size_t i, size_t hl) {
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
  //void StripConditions(Sent& sentence, size_t i, size_t hl) {
  //  StripConditions(sentence[i], hns_);
  //}
  
  void _run(Search::search& sch, Sent& sentence, History& output) {
    output.clear();
    
    const size_t history_length = StaticData::Get<size_t>("history_length");
    
    for(size_t i = 0; i < sentence.size(); i++) {
      //AddConditionalFeatures(sentence[i], output);
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

