#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <pcrecpp.h>
#include <boost/algorithm/string.hpp>  

#include "StaticData.hpp"
#include "Features.hpp"
#include "Examples.hpp"

namespace FF {

// Classes

class Classes {
  public:
    Classes(const std::string& file);
    std::string getClass(const std::string& orth) const;
    
  private:
    std::map<std::string, std::string> classes_;
};

Classes::Classes(const std::string& file) {
  std::ifstream fileStream(file);
  std::string line;
  while(std::getline(fileStream, line)) {
    std::stringstream vs(line);
    std::string word;
    std::string classid;
    vs >> word >> classid;
    classes_[word] = classid;
  }
  std::cerr << "Loaded " << classes_.size() << " class ids" << std::endl;
}

std::string Classes::getClass(const std::string& orth) const {
  auto it = classes_.find(orth);
  if(it != classes_.end()) {
    return it->second;
  }
  return "-1";
}

// Embeddings

class Embeddings {
  public:
    Embeddings(const std::string& file);
    void addEmbeddings(Tok& tok) const;
  
  private:
    std::map<std::string, std::vector<float>> embeddings_;
};

Embeddings::Embeddings(const std::string& file) {
  std::ifstream fileStream(file);
  std::string line;
  while(std::getline(fileStream, line)) {
    std::stringstream vs(line);
    std::string word;
    vs >> word;
    std::vector<float> embedding;
    float weight;
    while(vs >> weight)
      embedding.push_back(weight);
    embeddings_[word] = embedding;
  }
  std::cerr << "Loaded " << embeddings_.size() << " vectors" << std::endl;
}

void Embeddings::addEmbeddings(Tok& tok) const {
  auto it = embeddings_.find(tok.getOrth());
  if(it != embeddings_.end()) {
    size_t i = 0;
    for(auto e : it->second)
      tok.getNamespace()['v'].emplace_back("v" + std::to_string(i++), e);
  }
}

void meFeatures() {
  // normal word
  Tok::FF([](Tok& tok) {
    tok.getNamespace()['s'].emplace_back("w^" + tok.getOrth());
  });

  Tok::FF([](Tok& tok) {
    std::string orth = tok.getOrth();
    boost::to_lower(orth);
    tok.getNamespace()['s'].emplace_back("lc^" + orth);
  });

  // word length
  Tok::FF([](Tok& tok) {
    tok.getNamespace()['s'].emplace_back("length^" + std::to_string(tok.getOrth().size()));
  });

  /*

  pcrecpp::RE_Options options;
  options.set_utf8(true);
  std::map<std::string, pcrecpp::RE> regexes;
  regexes.emplace("ucFirst", pcrecpp::RE(R"(^\p{Lu})", options));
  regexes.emplace("ucAll", pcrecpp::RE(R"(^\p{Lu}+$)", options));
  regexes.emplace("hasNumber", pcrecpp::RE(R"(\d)", options));
  regexes.emplace("isNumber", pcrecpp::RE(R"(^\d+([.,]\d+)(%)?$)", options));
  regexes.emplace("hasLetter", pcrecpp::RE(R"(\p{L})", options));
  regexes.emplace("hasPunct", pcrecpp::RE(R"(\p{P})", options));
  regexes.emplace("hasFix", pcrecpp::RE(R"(-)", options));
  
  // regular expressions
  Tok::FF([=](Tok& tok) {
    std::string orth = tok.getOrth();
    for(auto& r: regexes)
      if(r.second.PartialMatch(orth))
        tok.getNamespace()['s'].emplace_back("r^" + r.first);
  });

  Tok::FF([](Tok& tok) {
    for(size_t i = 1; i < std::min(tok.getOrth().size(), StaticData::Get<size_t>("affix")); i++) {
      // prefixes and suffixes
      tok.getNamespace()['s'].emplace_back("pref^" + tok.getOrth().substr(0, i));
      tok.getNamespace()['s'].emplace_back("suff^" + tok.getOrth().substr(tok.getOrth().size()-i));

      // complements of prefixes and suffixes
      tok.getNamespace()['s'].emplace_back("cpref^" + tok.getOrth().substr(i));
      tok.getNamespace()['s'].emplace_back("csuff^" + tok.getOrth().substr(0, tok.getOrth().size()-i));
    }
  });
  
  // Add prev word and regexes
  Tok::FF([=](Tok& tok) {
    Sent& s = tok.getSent();
    size_t i = tok.getI();
    for(size_t j = 1; j <= StaticData::Get<size_t>("window"); ++j) {
      tok.getNamespace()['s'].emplace_back(
        "pw" + std::to_string(j) + "^"
        + (i >= j ? s[i - j].getOrth() : "<s>" )
      );
      if(i >= j) {
        for(auto& r: regexes)
        if(r.second.PartialMatch(s[i - j].getOrth()))
          tok.getNamespace()['s'].emplace_back("pr" + std::to_string(j) + "^" + r.first);
      }
    }
  });

  // Add next word and regexes
  Tok::FF([=](Tok& tok) {
    Sent& s = tok.getSent();
    size_t i = tok.getI();
    for(size_t j = 1; j <= StaticData::Get<size_t>("window"); ++j) {
      tok.getNamespace()['s'].emplace_back(
        "nw" + std::to_string(j) + "^"
        + (i + j < s.size() ? s[i + j].getOrth() : "</s>" )
      );
      if(i + j < s.size()) {
        for(auto& r: regexes)
        if(r.second.PartialMatch(s[i + j].getOrth()))
          tok.getNamespace()['s'].emplace_back("nr" + std::to_string(j) + "^" + r.first);
      }
    }
  });
  */
}

void ngramFeatures() {
  // BOW features
  Tok::FF([](Tok& tok) {
     Sent& s = tok.getSent();
     for(auto& t : s)
       tok.getNamespace()['s'].emplace_back("bow^" + t.getOrth());
   });
  
  // word ngram features in window
  Tok::FF([=](Tok& tok) {
    Sent& s = tok.getSent();
    size_t i = tok.getI();
    std::string png = tok.getOrth();
    std::string nng = tok.getOrth();
    std::string cng = tok.getOrth();
    for(size_t j = 1; j <= StaticData::Get<size_t>("window"); ++j) {
      if(j <= StaticData::Get<size_t>("window")/2) {
        cng = (i >= j ? s[i - j].getOrth() : "<s>") + "_" + cng +
            "_" + (i + j < s.size() ? s[i + j].getOrth() : "</s>");
        tok.getNamespace()['s'].emplace_back("cng^" + cng);
      }
      png = (i >= j ? s[i - j].getOrth() : "<s>") + "_" + png;
      nng = nng + "_" + (i + j < s.size() ? s[i + j].getOrth() : "</s>");
      tok.getNamespace()['s'].emplace_back("png^" + png);
      tok.getNamespace()['s'].emplace_back("nng^" + nng);
    }
  });
}

void tagFeatures() {
  // Add all tags and bases from preceeding words
  Tok::FF([=](Tok& tok) {
    Sent& s = tok.getSent();
    size_t i = tok.getI();
    for(size_t j = 1; j <= StaticData::Get<size_t>("window"); ++j) {
      if(i >= j) {
        for(auto& lex : s[i - j]) {
          tok.getNamespace()['s'].emplace_back("pot" + std::to_string(j) + "^" + lex.getCtag());    
          tok.getNamespace()['s'].emplace_back("pob" + std::to_string(j) + "^" + lex.getBase());
          std::vector<std::string> frags;
          boost::split(frags, lex.getCtag(), boost::is_any_of(":"));
          for(auto f : frags)
            tok.getNamespace()['s'].emplace_back("pom" + std::to_string(j) + "^" + f);
        }
      }
    }
  });
  
  // Add all tags and bases from following words
  Tok::FF([=](Tok& tok) {
    Sent& s = tok.getSent();
    size_t i = tok.getI();
    for(size_t j = 1; j <= StaticData::Get<size_t>("window"); ++j) {
      if(i + j < s.size()) { 
        for(auto& lex : s[i + j]) {
          tok.getNamespace()['s'].emplace_back("not" + std::to_string(j) + "^" + lex.getCtag());    
          tok.getNamespace()['s'].emplace_back("nob" + std::to_string(j) + "^" + lex.getBase());
          std::vector<std::string> frags;
          boost::split(frags, lex.getCtag(), boost::is_any_of(":"));
          for(auto f : frags)
            tok.getNamespace()['s'].emplace_back("nom" + std::to_string(j) + "^" + f);
        }
      }
    }
  });
}

void lfdFeatures() {
  // Add tag to LDF
  Lex::FF([](Lex& lex) {
    lex.getNamespace()['t'].emplace_back("t^" + lex.getCtag());
  });

  // Add base to LDF
  Lex::FF([](Lex& lex) {
    lex.getNamespace()['t'].emplace_back("b^" + lex.getBase());
  });
  
  // Add tag fragments to LDF
  Lex::FF([](Lex& lex) {
    std::vector<std::string> frags;
    boost::split(frags, lex.getCtag(), boost::is_any_of(":"));
    for(auto f : frags)
      lex.getNamespace()['t'].emplace_back("m^" + f);
  });
  
  // Matches with preceeding tags on ldf level
  Lex::FF([](Lex& lex) {
    Tok& t = lex.getTok();
    Sent& s = t.getSent();
    size_t i = t.getI();
    
    std::set<std::string> frags;
    boost::split(frags, lex.getCtag(), boost::is_any_of(":"));
    
    for(size_t j = 1; j <= StaticData::Get<size_t>("window"); ++j) {
      if(i >= j) {
        for(auto& lex2 : s[i - j]) {
          if(lex.getCtag() == lex2.getCtag()) {
            lex.getNamespace()['t'].emplace_back("pm" + std::to_string(j) + "^ctag");
            lex.getNamespace()['t'].emplace_back("pm" + std::to_string(j) + "^" + lex.getCtag());
          }
          
          std::vector<std::string> frags2;
          boost::split(frags2, lex2.getCtag(), boost::is_any_of(":"));
          for(auto& f2 : frags2)
            if(frags.count(f2))
              lex.getNamespace()['t'].emplace_back("pmf" + std::to_string(j) + "^" + f2);
        }
      }
    }
  });
  
  // Matches with following tags on ldf level
  Lex::FF([](Lex& lex) {
    Tok& t = lex.getTok();
    Sent& s = t.getSent();
    size_t i = t.getI();
    
    std::set<std::string> frags;
    boost::split(frags, lex.getCtag(), boost::is_any_of(":"));
    
    for(size_t j = 1; j <= StaticData::Get<size_t>("window"); ++j) {
      if(i + j < s.size()) {
        for(auto& lex2 : s[i + j]) {
          if(lex.getCtag() == lex2.getCtag()) {
            lex.getNamespace()['t'].emplace_back("nm" + std::to_string(j) + "^ctag");
            lex.getNamespace()['t'].emplace_back("nm" + std::to_string(j) + "^" + lex.getCtag());
          }
          
          std::vector<std::string> frags2;
          boost::split(frags2, lex2.getCtag(), boost::is_any_of(":"));
          for(auto& f2 : frags2)
            if(frags.count(f2))
              lex.getNamespace()['t'].emplace_back("nmf" + std::to_string(j) + "^" + f2);
        }
      }
    }
  });

}

void embeddingFeatures() {
  if(StaticData::Has("embeddings")) {
    std::shared_ptr<Embeddings>
    em(new Embeddings(StaticData::Get<std::string>("embeddings")));
    
    Tok::FF([=](Tok& tok) {
      em->addEmbeddings(tok);
    });
  }
}

void classFeatures() {
  if(StaticData::Has("classes")) {
    std::shared_ptr<Classes>
    cls(new Classes(StaticData::Get<std::string>("classes")));
    
    Tok::FF([=](Tok& tok) {
      tok.getNamespace()['c'].emplace_back("cls^" + cls->getClass(tok.getOrth()));
  
      Sent& s = tok.getSent();
      size_t i = tok.getI();
  
      for(size_t j = 1; j <= StaticData::Get<size_t>("window"); ++j) {
        tok.getNamespace()['c'].emplace_back(
          "clsp" + std::to_string(j) + "^"
          + (i >= j ? cls->getClass(s[i - j].getOrth()) : cls->getClass("<s>") )
        );
      }
  
      for(size_t j = 1; j <= StaticData::Get<size_t>("window"); ++j) {
        tok.getNamespace()['c'].emplace_back(
          "clsn" + std::to_string(j) + "^"
          + (i + j < s.size() ? cls->getClass(s[i + j].getOrth()) : cls->getClass("</s>") )
        );
      }
      
    });
  }
}

void printFeatures() {
  
  auto printNs = [](FeatureNS& fns) {
    for(auto& ns : fns) {
      std::cerr << ns.first << " : ";
      for(auto& f : ns.second) {
        std::cerr << f.name << (f.weight != 1.0 ? ":"
                                + std::to_string(f.weight) : "") << " ";
      }
      std::cerr << std::endl;
    }
  };

  Tok::FF([=](Tok& tok) {
    printNs(tok.getNamespace());
  });
  
  Lex::FF([=](Lex& lex) {
    printNs(lex.getNamespace());
  });
 
}

void registerFeatures() {
  meFeatures();
  //ngramFeatures();
  //tagFeatures();
  lfdFeatures();

  embeddingFeatures();
  classFeatures();

  auto cleanNs = [](FeatureNS& fns) {
    for(auto& ns : fns) {
      std::sort( ns.second.begin(), ns.second.end() );
      ns.second.erase( std::unique( ns.second.begin(), ns.second.end() ), ns.second.end() );
    }
  };
  
  Tok::FF([=](Tok& tok) {
    cleanNs(tok.getNamespace());
  });

  Lex::FF([=](Lex& lex) {
    cleanNs(lex.getNamespace());
  });
  
  if(StaticData::Get<bool>("debug"))
    printFeatures();
}

}
