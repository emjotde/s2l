#include "Examples.hpp"

std::vector<std::function<void(Lex&)>> Lex::hooks_;
std::vector<std::function<void(Tok&)>> Tok::hooks_;

std::map<std::string, size_t> Lex::fragments_;