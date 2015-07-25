#pragma once

#include <vowpalwabbit/vw.h>
#include <vowpalwabbit/ezexample.h>

#include <string>
#include <vector>
#include <map>
#include <sstream>

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

    friend inline std::ostream& operator<<(std::ostream &o, const Lex& l) {
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
    
    Morfs(const Morfs& o) : data_(o.data_), size_(o.size_), oracle_(o.oracle_) {
      if(data_ != nullptr) {
        data_ = VW::alloc_examples(sizeof(COST_SENSITIVE::label), size_);
        for(size_t a = 0; a < size_; ++a)
          VW::copy_example_data(true, &data_[a], &o.data_[a]);
      }
    }

    example* data() { return data_; };
    size_t size() { return size_; };

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
    
    Lex& back() {
      return lex_.back();
    }

    Tok& done() {
      morfs_.reset(lex_.size());
      for(size_t i = 0; i < morfs_.size(); i++) {
        ezexample ez(vw_, &morfs_[i], false);
        ez(vw_namespace('s'))("w^" + orth_);
        ez(vw_namespace('t'))("t^" + lex_[i].ctag())("b^" + lex_[i].base());
        
        COST_SENSITIVE::wclass default_wclass = { 0., 0, 0., 0. };                                                  
        ez.get()->l.cs.costs.push_back(default_wclass);
         
        if(lex_[i].isOracle())
          morfs_.setOracle(i);
      }
      //std::cerr << "Oracle: " << morfs_.oracle() << std::endl;
      return *this;
    }

    Morfs& morfs() { return morfs_; }

    friend inline std::ostream& operator<<(std::ostream &o, const Tok& t) {
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
    Sent() : vw_(nullptr) {}
    Sent(vw* vwMain) : vw_(vwMain) {}

    ~Sent() {}
    
    Sent& tok() {
      if(!tok_.empty()) 
         tok_.back().done();
      tok_.emplace_back(vw_);
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


