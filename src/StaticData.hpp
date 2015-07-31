#pragma once

#include <boost/program_options.hpp>

namespace po = boost::program_options;

class StaticData {
  public:
    static StaticData& Init(const std::string&);
    static StaticData& Init(int argc, char** argv);
    
    static StaticData& Instance() {
      return instance_;
    }
    
    static bool Has(const std::string& key) {
      return instance_.vm_.count(key) > 0;
    }
    
    template <typename T>
    static T Get(const std::string& key) {
      return instance_.vm_[key].as<T>();
    }
    
    static void PrintConfig() {
      PrintConfig(std::cerr);
    }
    
    static void PrintConfig(std::ostream& out) {
      for(auto& entry: instance_.vm_) {
        out << entry.first << " = ";
        try { out << entry.second.as<std::string>(); } catch(...) { }
        try { out << entry.second.as<size_t>(); } catch(...) { }
        try { out << entry.second.as<bool>(); } catch(...) { }
        out << std::endl;
      }
    }
    
  private:
    StaticData& NonStaticInit(int argc, char** argv);
    
    static StaticData instance_;
    po::variables_map vm_;
};

