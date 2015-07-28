#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <pcrecpp.h>

#include <boost/algorithm/string.hpp>

#include "Examples.hpp"
#include "VowpalTaggit.hpp"

const size_t WINDOW = 2;
const size_t AFFIX = 4;

class Classes {
  public:
    Classes(const std::string& file) {
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
    
    std::string getClass(const std::string& orth) {
      auto it = classes_.find(orth);
      if(it != classes_.end()) {
        return it->second;
      }
      return "-1";
    }
  
  private:
    std::map<std::string, std::string> classes_;
};


class Embeddings {
  public:
    Embeddings(const std::string& file) {
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
    
    void addEmbeddings(Tok& tok) {
      auto it = embeddings_.find(tok.getOrth());
      if(it != embeddings_.end()) {
        size_t i = 0;
        for(auto e : it->second)
          tok.getNamespace()['v'].emplace("v" + std::to_string(i++), e);
      }
    }
  
  private:
    std::map<std::string, std::vector<float>> embeddings_;
};

void meFeatures() {
  // normal word
  Tok::FF([](Tok& tok) {
    tok.getNamespace()['s'].insert("w^" + tok.getOrth());
  });

  //Tok::FF([](Tok& tok) {
  //  tok.getNamespace()['s'].insert("lc^" + tok.getOrth().tolower());
  //});

  // word length
  Tok::FF([](Tok& tok) {
    tok.getNamespace()['s'].insert("length^" + std::to_string(tok.getOrth().size()));
  });

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
        tok.getNamespace()['s'].insert("r^" + r.first);
  });

  // prefixes and suffixes of length up to 4
  Tok::FF([=](Tok& tok) {
    for(size_t i = 1; i < std::min(tok.getOrth().size(), AFFIX); i++) {
      tok.getNamespace()['s'].insert("pref^" + tok.getOrth().substr(0, i));
      tok.getNamespace()['s'].insert("suff^" + tok.getOrth().substr(tok.getOrth().size()-i));
    }
  });
  
  // Add prev word and regexes
  Tok::FF([=](Tok& tok) {
    Sent& s = tok.getSentence();
    size_t i = tok.getI();
    for(size_t j = 1; j <= WINDOW; ++j) {
      tok.getNamespace()['s'].insert(
        "pw" + std::to_string(j) + "^"
        + (i >= j ? s[i - j].getOrth() : "<s>" )
      );
      if(i >= j) {
        for(auto& r: regexes)
        if(r.second.PartialMatch(s[i - j].getOrth()))
          tok.getNamespace()['s'].insert("pr" + std::to_string(j) + "^" + r.first);
      }
    }
  });

  // Add next word and regexes
  Tok::FF([=](Tok& tok) {
    Sent& s = tok.getSentence();
    size_t i = tok.getI();
    for(size_t j = 1; j <= WINDOW; ++j) {
      tok.getNamespace()['s'].insert(
        "nw" + std::to_string(j) + "^"
        + (i + j < s.size() ? s[i + j].getOrth() : "</s>" )
      );
      if(i + j < s.size()) {
        for(auto& r: regexes)
        if(r.second.PartialMatch(s[i + j].getOrth()))
          tok.getNamespace()['s'].insert("nr" + std::to_string(j) + "^" + r.first);
      }
    }
  });
  
  // BOW features
  Tok::FF([](Tok& tok) {
     Sent& s = tok.getSentence();
     for(auto& t : s)
       tok.getNamespace()['s'].insert("bow^" + t.getOrth());
   });
}

void ngramFeatures() {
  // word ngram features in window
  Tok::FF([=](Tok& tok) {
    Sent& s = tok.getSentence();
    size_t i = tok.getI();
    std::string png = tok.getOrth();
    std::string nng = tok.getOrth();
    std::string cng = tok.getOrth();
    for(size_t j = 1; j <= WINDOW; ++j) {
      if(j <= WINDOW/2) {
        cng = (i >= j ? s[i - j].getOrth() : "<s>") + "_" + cng +
            "_" + (i + j < s.size() ? s[i + j].getOrth() : "</s>");
        tok.getNamespace()['s'].insert("cng^" + cng);
      }
      png = (i >= j ? s[i - j].getOrth() : "<s>") + "_" + png;
      nng = nng + "_" + (i + j < s.size() ? s[i + j].getOrth() : "</s>");
      tok.getNamespace()['s'].insert("png^" + png);
      tok.getNamespace()['s'].insert("nng^" + nng);
    }
  });
}

void tagFeatures() {
  // Add all tags and bases from preceeding words
  Tok::FF([=](Tok& tok) {
    Sent& s = tok.getSentence();
    size_t i = tok.getI();
    for(size_t j = 1; j <= WINDOW; ++j) {
      if(i >= j) {
        for(auto& lex : s[i - j]) {
          tok.getNamespace()['s'].insert("pot" + std::to_string(j) + "^" + lex.getCtag());    
          tok.getNamespace()['s'].insert("pob" + std::to_string(j) + "^" + lex.getBase());
          std::vector<std::string> frags;
          boost::split(frags, lex.getCtag(), boost::is_any_of(":"));
          for(auto f : frags)
            tok.getNamespace()['s'].insert("pom" + std::to_string(j) + "^" + f);
        }
      }
    }
  });
  
  // Add all tags and bases from following words
  Tok::FF([=](Tok& tok) {
    Sent& s = tok.getSentence();
    size_t i = tok.getI();
    for(size_t j = 1; j <= WINDOW; ++j) {
      if(i + j < s.size()) { 
        for(auto& lex : s[i + j]) {
          tok.getNamespace()['s'].insert("not" + std::to_string(j) + "^" + lex.getCtag());    
          tok.getNamespace()['s'].insert("nob" + std::to_string(j) + "^" + lex.getBase());
          std::vector<std::string> frags;
          boost::split(frags, lex.getCtag(), boost::is_any_of(":"));
          for(auto f : frags)
            tok.getNamespace()['s'].insert("nom" + std::to_string(j) + "^" + f);
        }
      }
    }
  });
}

void lfdFeatures() {
  // Add tag to LDF
  Lex::FF([](Lex& lex) {
    lex.getNamespace()['t'].insert("t^" + lex.getCtag());
  });

  // Add base to LDF
  Lex::FF([](Lex& lex) {
    lex.getNamespace()['t'].insert("b^" + lex.getBase());
  });
  
  // Add tag fragments to LDF
  Lex::FF([](Lex& lex) {
    std::vector<std::string> frags;
    boost::split(frags, lex.getCtag(), boost::is_any_of(":"));
    for(auto f : frags)
      lex.getNamespace()['t'].insert("m^" + f);
  });
  
  // Matches with preceeding tags on ldf level
  Lex::FF([](Lex& lex) {
    Tok& t = lex.getTok();
    Sent& s = t.getSentence();
    size_t i = t.getI();
    
    std::set<std::string> frags;
    boost::split(frags, lex.getCtag(), boost::is_any_of(":"));
    
    for(size_t j = 1; j <= WINDOW; ++j) {
      if(i >= j) {
        for(auto& lex2 : s[i - j]) {
          if(lex.getCtag() == lex2.getCtag()) {
            lex.getNamespace()['t'].insert("pm" + std::to_string(j) + "^ctag");
            lex.getNamespace()['t'].insert("pm" + std::to_string(j) + "^" + lex.getCtag());
          }
          
          std::vector<std::string> frags2;
          boost::split(frags2, lex2.getCtag(), boost::is_any_of(":"));
          for(auto& f2 : frags2)
            if(frags.count(f2))
              lex.getNamespace()['t'].insert("pmf" + std::to_string(j) + "^" + f2);
        }
      }
    }
  });
  
  // Matches with following tags on ldf level
  Lex::FF([](Lex& lex) {
    Tok& t = lex.getTok();
    Sent& s = t.getSentence();
    size_t i = t.getI();
    
    std::set<std::string> frags;
    boost::split(frags, lex.getCtag(), boost::is_any_of(":"));
    
    for(size_t j = 1; j <= WINDOW; ++j) {
      if(i + j < s.size()) {
        for(auto& lex2 : s[i + j]) {
          if(lex.getCtag() == lex2.getCtag()) {
            lex.getNamespace()['t'].insert("nm" + std::to_string(j) + "^ctag");
            lex.getNamespace()['t'].insert("nm" + std::to_string(j) + "^" + lex.getCtag());
          }
          
          std::vector<std::string> frags2;
          boost::split(frags2, lex2.getCtag(), boost::is_any_of(":"));
          for(auto& f2 : frags2)
            if(frags.count(f2))
              lex.getNamespace()['t'].insert("nmf" + std::to_string(j) + "^" + f2);
        }
      }
    }
  });

}

void printNs(FeatureNS& fns) {
  for(auto& ns : fns) {
    std::cerr << ns.first << " : ";
    for(auto& f : ns.second) {
      std::cerr << f.name << (f.weight != 1.0 ? ":" + std::to_string(f.weight) : "") << " ";
    }
    std::cerr << std::endl;
  }
}
void registerFeatures() {
  meFeatures();
  ngramFeatures();
  tagFeatures();
  lfdFeatures();

  // For debugging 
  //Tok::FF([=](Tok& tok) {
  //  printNs(tok.getNamespace());
  //});
  //
  //Lex::FF([=](Lex& lex) {
  //  printNs(lex.getNamespace());
  //});
}

int main(int argc, char **argv) {
  std::ios_base::sync_with_stdio(false);
  
  //Embeddings em("data/pl.vectors");
  //Tok::FF([&](Tok& tok) {
  //  em.addEmbeddings(tok);
  //});

  Classes cls("data/pl.classes");
  Tok::FF([&](Tok& tok) {
    tok.getNamespace()['c'].insert("cls^" + cls.getClass(tok.getOrth()));
  });
  
  // Add prev class
  Tok::FF([&](Tok& tok) {
    Sent& s = tok.getSentence();
    size_t i = tok.getI();
    for(size_t j = 1; j <= WINDOW; ++j) {
      tok.getNamespace()['c'].insert(
        "clsp" + std::to_string(j) + "^"
        + (i >= j ? cls.getClass(s[i - j].getOrth()) : cls.getClass("<s>") )
      );
    }
  });
  
  // Add next class
  Tok::FF([&](Tok& tok) {
    Sent& s = tok.getSentence();
    size_t i = tok.getI();
    for(size_t j = 1; j <= WINDOW; ++j) {
      tok.getNamespace()['c'].insert(
        "clsn" + std::to_string(j) + "^"
        + (i + j < s.size() ? cls.getClass(s[i + j].getOrth()) : cls.getClass("</s>") )
      );
    }
  });
  
  registerFeatures();
  
  
  // Hooks take VowpalTaggit& as a parameter
  
  auto learner = [](VowpalTaggit& vt) {
    vt.learn();
  };
  
  auto reporter = [](VowpalTaggit& vt) {
    if(vt.sentNum() % 1000 == 0)
      vt.report(std::cerr);
  };
  
  auto saver = [](VowpalTaggit& vt) {
    if(vt.sentNum() % 10000 == 0) {
      char buffer[100];
      sprintf(buffer, "model.weights.%ld", vt.sentNum());
      std::cerr << "Saving model to " << buffer << std::endl;
      vt.save(buffer);
    }
  };
    
  VowpalTaggit vt(argc, argv);
  vt(learner)(reporter)(std::cin);
  
  size_t c = 0;
  auto predictor = [&](VowpalTaggit& vt) {
    std::vector<int> output;
    vt.predict(output);
    for(auto i : output)
      std::cout << i << " ";
    std::cout << std::endl;
    c++;
    if(c % 1000 == 0)
      std::cerr << c << std::endl;
  };
  
  std::ifstream test("data/test01.flat");
  vt.clearHooks();
  vt(predictor)(test);
}
