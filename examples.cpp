#include <vowpalwabbit/vw.h>
#include <vowpalwabbit/ezexample.h>

#include <string>
#include <vector>
#include <map>

#include "libsearch.h"


class Lex {
  public:
    Lex() : oracle_(false) {};

    Lex& base(const std::string& base) {
      base_ = base;
      return *this;
    }

    const std::string& base() { return base_; }
    const std::string& ctag() { return ctag_; }

    Lex& ctag(const std::string& ctag) {
      ctag_ = ctag;
      return *this;
    }

    Lex& oracle() {
      oracle_ = true;
      return *this;
    }

    bool isOracle() {
       return oracle_;
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

class Morfs {
  public:
    Morfs() : data_(nullptr), size_(0), oracle_(1) {}
    Morfs(Morfs&& o) : size_(o.size_), oracle_(o.oracle_)  {
      data_ = o.data_;
      o.data_ = nullptr;
      o.size_ = 0;
      o.oracle_ = 1;
    }

    Morfs(const Morfs&) = delete;

    example* data() { return data_; };
    size_t size() { return size_; };

    void reset(size_t size) {
      if(data_ != nullptr) {
         //delete[] data_;
        //data_ = nullptr;
      }
      
      data_ = VW::alloc_examples(sizeof(COST_SENSITIVE::label), size);                              
      COST_SENSITIVE::wclass default_wclass = { 0., 0, 0., 0. };                                                  
      for (size_t a = 0; a < size; a++) {                                                                   
        data_[a].l.cs.costs.push_back(default_wclass);                                         
      }   
      size_ = size;
      oracle_ = size_ + 1;
    }

    example& operator[](size_t i) { return data_[i]; }

    ~Morfs() {
      //if(data_ != nullptr)
         //delete[] data_;
    }

    size_t oracle() { return oracle_; }
    size_t setOracle(size_t i) { oracle_ = i; }

  private:
    example* data_;
    size_t size_;
    size_t oracle_;
};

class Tok {
  public:
    Tok(vw* vwMain) : vw_(vwMain) {}
    Tok(Tok&& o) 
      : vw_(std::move(o.vw_)), morfs_(std::move(o.morfs_)),
        orth_(std::move(o.orth_)), lex_(std::move(o.lex_))
    {}

    Tok(const Tok&) = delete;

    Tok& orth(const std::string& orth) {
      orth_ = orth;
      return *this;
    }

    Lex& lex() {
      lex_.emplace_back();
      return lex_.back();
    }

    Tok& done() {
      morfs_.reset(lex_.size());
      for(size_t i = 0; i < morfs_.size(); i++) {
         ezexample* ez = new ezexample(vw_, &morfs_[i], false);
         (*ez)(vw_namespace('s'))("w^" + orth_);
         (*ez)(vw_namespace('t'))("t^" + lex_[i].ctag())("b^" + lex_[i].base());
         ez->set_label(std::string("1111:") + std::string(lex_[i].isOracle() ? "0" : "1"));
         
         //COST_SENSITIVE::wclass zero = { 0., 1, 0., 0. };                                                  
         //ez.get()->l.cs.costs.push_back(zero);
         
         if(lex_[i].isOracle())
           morfs_.setOracle(i);
         
         // l.cs.costs[0].class_index
         
      }
      std::cerr << "Oracle: " << morfs_.oracle() << std::endl;
      return *this;
    }

    Morfs& morfs() { return morfs_; }

    friend std::ostream& operator<<(std::ostream &o, const Tok& t) {
      o << t.orth_ << std::endl;
      for(auto& l : t.lex_)
        o << "\t" << l;
      return o;
    }

  private:
    std::string orth_;
    std::vector<Lex> lex_;
    vw* vw_;
    Morfs morfs_;
};

class Sent {
  public:
    Sent(vw* vwMain) : vw_(vwMain) {}

    Tok& tok() {
      if(!tok_.empty()) 
         tok_.back().done();
      tok_.emplace_back(vw_);
      return tok_.back();
    }

    Lex& lex() {
      return tok_.back().lex();
    }

    Sent& eos() {
      if(!tok_.empty()) 
         tok_.back().done();
      return *this;
    }

    Morfs& operator[](size_t i) {
       return tok_[i].morfs();
    }

    size_t size() {
      return tok_.size();
    }
    
    friend std::ostream& operator<<(std::ostream &o, const Sent& s) {
      o << "#BOS" << std::endl;
      for(auto& t : s.tok_)
        o << t;
      o << "#EOS" << std::endl;
      return o;
    }

  private:
    std::vector<Tok> tok_;
    vw* vw_;
};

class Examples {
  public:
    Examples(const std::string& vwstring) {
      vw_ = VW::initialize(vwstring);
    }

    ~Examples() {
      VW::finish(*vw_);
    }

    vw* getVw() {
      return vw_;
    }
    
    Sent newSent() {
      return Sent(vw_);
    }

  private:
    vw* vw_;
};

class SequenceLabelerTask : public SearchTask<Sent, vector<uint32_t>> {
  public:
  SequenceLabelerTask(vw* vw_obj)
      : SearchTask<Sent, vector<uint32_t> >(*vw_obj) {  // must run parent constructor!
    sch.set_options( Search::AUTO_HAMMING_LOSS | Search::NO_CACHING | Search::AUTO_CONDITION_FEATURES | Search::IS_LDF );
    sch.set_label_parser( COST_SENSITIVE::cs_label, [](polylabel&l) -> bool { return l.cs.costs.size() == 0; });
    
    HookTask::task_data* d = sch.get_task_data<HookTask::task_data>();
    cerr << "num_actions = " << d->num_actions << endl;
  }

  // using vanilla vw interface
  void _run(Search::search& sch, Sent& sentence, vector<uint32_t>& output) {
    output.clear();
    for (size_t i = 0; i < sentence.size(); i++) {
      Morfs& morfs = sentence[i];
      action p = Search::predictor(sch, i + 1)
        .set_input(morfs.data(), morfs.size())
        .set_oracle(morfs.oracle())
        .set_condition(i, 'p')
        .predict();
        
      output.push_back(p);
    }
  }

};

int main(int argc, char *argv[]) {
  Examples ex("--hash all -b 31 --csoaa_ldf m --search 0 --search_task hook");
  
  Sent s = ex.newSent();

  s.tok().orth("fryzjerstwa");
  s.lex().base("fryzjerstwo").ctag("subst:pl:nom:n");
  s.lex().base("fryzjerstwo").ctag("subst:pl:acc:n");
  s.lex().base("fryzjerstwo").ctag("subst:pl:voc:n");
  s.lex().base("fryzjerstwo").ctag("subst:sg:gen:n").oracle();
  
  s.tok().orth("wiedzą");
  s.lex().base("wiedzieć").ctag("fin:pl:ter:imperf").oracle();
  s.lex().base("wiedza").ctag("subst:sg:inst:f");
  
  s.tok().orth(",").lex().base(",").ctag("interp").oracle();
  s.eos();

  std::cout << s << std::endl;

  SequenceLabelerTask sl(ex.getVw());
  vector<uint32_t> output;
  sl.learn(s, output);
  sl.predict(s, output);
  
  for(auto& p: output)
    std::cout << p << " ";
  std::cout << std::endl;
}
