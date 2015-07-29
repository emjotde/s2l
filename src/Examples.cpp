#include "Examples.hpp"

std::vector<std::function<void(Lex&)>> Lex::hooks_;
std::vector<std::function<void(Tok&)>> Tok::hooks_;
