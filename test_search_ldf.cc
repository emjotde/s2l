#include <stdio.h>
#include <stdlib.h> // for system
#include <vowpalwabbit/vw.h>
#include <vowpalwabbit/ezexample.h>
#include <vowpalwabbit/search_sequencetask.h>
#include "libsearch.h"


#include <string>
#include <vector>
#include <map>

class Lex {
  public:
    Lex() : oracle_(false) {};
    
    Lex& base(const std::string& base) {
      base_ = base;
      return *this;
    }

    Lex& ctag(const std::string& ctag) {
      ctag_ = ctag;
      return *this;
    }

    Lex& oracle() {
      oracle_ = true;
      return *this;
    }
    
    friend std::ostream& operator<<(std::ostream &o, const Lex& l) {
      o << ( l.oracle_ ? "*" : " " ) << l.ctag_ << "\t" << l.base_ << std::endl;
      return o;
    }

  private:
    std::string base_;
    std::string ctag_;
    bool oracle_;
};

class Tok {
  public:
    Tok& orth(const std::string& orth) {
      orth_ = orth;
      return *this;
    }
    
    Lex& lex() {
      lex_.push_back(Lex());
      return lex_.back();
    }

    friend std::ostream& operator<<(std::ostream &o, const Tok& t) {
      o << t.orth_ << std::endl;
      for(auto& l : t.lex_)
        o << "\t" << l;
      return o;
    }
    
  private:
    std::string orth_;
    std::vector<Lex> lex_;
};

class Sent {
  public:
    Tok& tok() {
      tok_.push_back(Tok());
      return tok_.back();
    }
    
    Lex& lex() {
      return tok_.back().lex();
    }
    
    friend std::ostream& operator<<(std::ostream &o, const Sent& s) {
      for(auto& t : s.tok_)
        o << t;
      return o;
    }
    
  private:
    std::vector<Tok> tok_;
};

struct wt {
  string word;
  uint32_t tag;
  wt(string w, uint32_t t) : word(w), tag(t) {}
};

class SequenceLabelerTask : public SearchTask< vector<wt>, vector<uint32_t> > {
  public:
  SequenceLabelerTask(vw& vw_obj)
      : SearchTask< vector<wt>, vector<uint32_t> >(vw_obj) {  // must run parent constructor!
    sch.set_options( Search::AUTO_HAMMING_LOSS | Search::AUTO_CONDITION_FEATURES );
    HookTask::task_data* d = sch.get_task_data<HookTask::task_data>();
    cerr << "num_actions = " << d->num_actions << endl;
  }

  // using vanilla vw interface
  void _run(Search::search& sch, vector<wt>& input_example, vector<uint32_t>& output) {
    output.clear();
    for (size_t i = 0; i < input_example.size(); i++) {
      example* ex = VW::read_example(vw_obj, "1 |w " + input_example[i].word);
      
      action p  = Search::predictor(sch, i+1)
        .set_input(*ex)
        .set_oracle(input_example[i].tag)
        .set_condition(i, 'p')
        .predict();
      
      VW::finish_example(vw_obj, ex);
      output.push_back(p);
    }
  }

  // using ezexample
  void _run2(Search::search& sch, vector<wt> & input_example, vector<uint32_t> & output) {
    output.clear();
    for (size_t i=0; i<input_example.size(); i++) {
      ezexample ex(&vw_obj);
      ex(vw_namespace('w'))(input_example[i].word);  // add the feature
      action p  = Search::predictor(sch, i+1).set_input(*ex.get()).set_oracle(input_example[i].tag).set_condition(i, 'p').predict();
      output.push_back(p);
    }
  }

};

void run(vw& vw_obj) {
  // we put this in its own scope so that its destructor on
  // SequenceLabelerTask gets called *before* VW::finish gets called;
  // otherwise we'll get a segfault :(. i'm not sure what to do about
  // this :(.
  SequenceLabelerTask task(vw_obj);
  vector<wt> data;
  vector<uint32_t> output;
  uint32_t DET = 1, NOUN = 2, VERB = 3, ADJ = 4;
  data.push_back( wt("the", DET) );
  data.push_back( wt("monster", NOUN) );
  data.push_back( wt("ate", VERB) );
  data.push_back( wt("a", DET) );
  data.push_back( wt("big", ADJ) );
  data.push_back( wt("sandwich", NOUN) );
  task.learn(data, output);
  task.learn(data, output);
  task.learn(data, output);
  task.predict(data, output);
  cerr << "output = [";
  for (size_t i=0; i<output.size(); i++) cerr << " " << output[i];
  cerr << " ]" << endl;
  cerr << "should have printed: 1 2 3 1 4 2" << endl;
}


void train() {
  // initialize VW as usual, but use 'hook' as the search_task
  vw& vw_obj = *VW::initialize("--search 4 --quiet --search_task hook --ring_size 1024 -f my_model");
  run(vw_obj);
  VW::finish(vw_obj);
}

void predict() {
  vw& vw_obj = *VW::initialize("--quiet -t --ring_size 1024 -i my_model");
  run(vw_obj);
  VW::finish(vw_obj);
}

int main(int argc, char *argv[]) {
  
  Sent s;
  s.tok().orth("fryzjerstwa");
  s.lex().base("fryzjerstwo").ctag("subst:pl:nom:n");
  s.lex().base("fryzjerstwo").ctag("subst:pl:acc:n");
  s.lex().base("fryzjerstwo").ctag("subst:pl:voc:n");
  s.lex().base("fryzjerstwo").ctag("subst:sg:gen:n").oracle();
  
  s.tok().orth("wiedzą");
  s.lex().base("wiedzieć").ctag("fin:pl:ter:imperf").oracle();
  s.lex().base("wiedza").ctag("subst:sg:inst:f");
  
  s.tok().orth(",").lex().base(",").ctag("interp").oracle();
  
  std::cout << s << std::endl;
  
  //train();
  //predict();
}










