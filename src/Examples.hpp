#pragma once

#include <vowpalwabbit/vw.h>
#include <vowpalwabbit/ezexample.h>

#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>

struct Feature {
  Feature(const std::string& name) : name(name), weight(1.0) {}
  Feature(const std::string& name, double weight) : name(name), weight(weight) {}
  
  bool operator<(const Feature& o) const {
    return name < o.name;
  }
  
  bool operator==(const Feature& o) const {
    return name == o.name;
  }
  
  std::string name;
  double weight;
};

typedef char Namespace;
typedef std::vector<Feature> Features;
typedef std::map<Namespace, Features> FeatureNS;

class Morfs {
  public:
    Morfs() : data_(nullptr), size_(0), oracle_(1) {}
    
    Morfs(const Morfs& o) : data_(o.data_), size_(o.size_), oracle_(o.oracle_) {
      if(data_ != nullptr) {
        data_ = VW::alloc_examples(sizeof(COST_SENSITIVE::label), size_);
        for(size_t a = 0; a < size_; ++a)
          VW::copy_example_data(true, &data_[a], &o.data_[a]);
      }
    }

    example* data() { return data_; };
    size_t size() { return size_; };
    
    example* begin() { return data_; }
    example* end() { return data_ + size_; }

    void reset(size_t size) {
      if(data_ != nullptr) {
        for(size_t a = 0; a < size_; a++)
          VW::dealloc_example(COST_SENSITIVE::cs_label.delete_label, data_[a]);
        free(data_);
        data_ = nullptr;
      }
      
      data_ = VW::alloc_examples(sizeof(COST_SENSITIVE::label), size);                              
      size_ = size;
      oracle_ = size_ + 1;
    }

    example& operator[](size_t i) { return data_[i]; }

    ~Morfs() {
      if(data_ != nullptr) {
        for(size_t a = 0; a < size_; a++)
          VW::dealloc_example(COST_SENSITIVE::cs_label.delete_label, data_[a]);
        free(data_);
      }
    }

    size_t oracle() { return oracle_; }
    size_t setOracle(size_t i) { oracle_ = i; }

  private:
    example* data_;
    size_t size_;
    size_t oracle_;
};

class Tok;

class Lex {
  public:
    Lex(Tok& parent) : oracle_(false), parent_(parent) {};

    const std::string& base() { return base_; }
    const std::string& ctag() { return ctag_; }

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

    bool isOracle() {
       return oracle_;
    }

    const std::string& getBase() {
      return base_;
    }
    
    const std::string& getCtag() {
      return ctag_;
    }
    
    friend inline std::ostream& operator<<(std::ostream &o, const Lex& l) {
      o << ( l.oracle_ ? "*" : " " ) << l.ctag_ << "\t" << l.base_ << std::endl;
      return o;
    }

    FeatureNS& getNamespace() {
      return features_;
    }
    
    static void FF(std::function<void(Lex&)> ff) {
      hooks_.push_back(ff);
    }
    
    void runFF() {
      for(auto& ff: hooks_)
        ff(*this);
    }
    
    Tok& getTok() {
      return parent_;
    }
    
  private:
    FeatureNS features_;
    static std::vector<std::function<void(Lex&)>> hooks_;
    
    std::string base_;
    std::string ctag_;
    bool oracle_;
    Tok& parent_;
};

class Sent;

class Tok {
  public:
    Tok(vw* vwMain, Sent& parent, size_t i)
      : vw_(vwMain), parent_(parent), i_(i) {}
      
    Tok(Tok&& o) 
      : vw_(std::move(o.vw_)), morfs_(std::move(o.morfs_)),
        orth_(std::move(o.orth_)),
        parent_(o.parent_), i_(o.i_)
    {
      // move lex to new parent
      for(auto& lex : o.lex_) {
        lex_.emplace_back(*this);
        lex_.back().base(lex.getBase()); 
        lex_.back().ctag(lex.getCtag());
        if(lex.isOracle())
          lex_.back().oracle();
      }
    }

    Tok(const Tok&) = delete;

    Tok& orth(const std::string& orth) {
      orth_ = orth;
      return *this;
    }

    Lex& lex() {
      lex_.emplace_back(*this);
      return lex_.back();
    }
    
    Lex& back() {
      return lex_.back();
    }
    
    const std::string& getOrth() {
      return orth_;
    }

    std::vector<Lex>::iterator begin() {
      return lex_.begin();
    }
    
    std::vector<Lex>::iterator end() {
      return lex_.end();
    }
    
    Lex& operator[](size_t i) {
       return lex_[i];
    }
    
    size_t size() {
      return lex_.size();
    }
    
    void nsToExample(example* ex, FeatureNS& features) {
      ezexample ez(vw_, ex, false);
      for(auto& ns : features) {
        ez(vw_namespace(ns.first));
        for(auto& f : ns.second) ez(f.name, f.weight);
      }
    }
    
    FeatureNS& getNamespace() {
      return features_;
    }

    void initExampleWeight(example* ex) {
      COST_SENSITIVE::wclass default_wclass = { 0., 0, 0., 0. };
      ex->l.cs.costs.push_back(default_wclass);
    }
    
    Tok& done() {
      runFF();
      for(auto& l : lex_)
        l.runFF();
      
      morfs_.reset(lex_.size());
      for(size_t i = 0; i < morfs_.size(); i++) {
        example* ex = &morfs_[i];
        initExampleWeight(ex);
          
        nsToExample(ex, getNamespace());
        nsToExample(ex, lex_[i].getNamespace());
        
        if(lex_[i].isOracle())
          morfs_.setOracle(i);
      }
      return *this;
    }

    Morfs& morfs() { return morfs_; }

    friend inline std::ostream& operator<<(std::ostream &o, const Tok& t) {
      o << t.orth_ << std::endl;
      for(auto& l : t.lex_)
        o << "\t" << l;
      return o;
    }

    static void FF(std::function<void(Tok&)> ff) {
      hooks_.push_back(ff);
    }
    
    void runFF() {
      for(auto& ff: hooks_)
        ff(*this);
    }
    
    Sent& getSent() {
      return parent_;
    }
    
    size_t getI() {
      return i_;
    }
    
    void setI(size_t i) {
      i_ = i;
    }
    
  private:
    FeatureNS features_;
    static std::vector<std::function<void(Tok&)>> hooks_;
      
    std::string orth_;
    std::vector<Lex> lex_;
    vw* vw_;
    Sent& parent_;
    size_t i_;
    Morfs morfs_;
};

class Sent {
  public:
    Sent() : vw_(nullptr) {}
    Sent(vw* vwMain) : vw_(vwMain) {}

    ~Sent() {}
    
    Sent& tok() {
      //if(!tok_.empty()) 
      //   tok_.back().done();
      tok_.emplace_back(vw_, *this, size());
      return *this;
    }
    
    Sent& lex() {
      tok_.back().lex();
      return *this;
    }
    
    Sent& orth(const std::string orth) {
      tok_.back().orth(orth);
      return *this;
    }

    Sent& base(const std::string base) {
      tok_.back().back().base(base);
      return *this;
    }

    Sent& ctag(const std::string ctag) {
      tok_.back().back().ctag(ctag);
      return *this;
    }
    
    Sent& oracle() {
      tok_.back().back().oracle();
      return *this;
    }

    Sent& eos() {
      for(auto& t: tok_)
        t.done();
      return *this;
    }
    
    std::vector<Tok>::iterator begin() {
      return tok_.begin();
    }
    
    std::vector<Tok>::iterator end() {
      return tok_.end();
    }

    Tok& operator[](size_t i) {
       return tok_[i];
    }

    size_t size() {
      return tok_.size();
    }
    
    friend inline std::ostream& operator<<(std::ostream &o, const Sent& s) {
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
    Examples(vw* vwp) : vw_(vwp) {}

    Sent* newSent() {
      return new Sent(vw_);
    }

  private:
    vw* vw_;
};


