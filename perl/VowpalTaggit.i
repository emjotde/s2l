%module VowpalTaggit

%include "std_vector.i"
%include "std_string.i"

namespace std {
    %template(vectori) vector<int>;
    %template(vectord) vector<double>;
};

%{
#include "VowpalTaggit.hpp"
%}


class VowpalTaggit {
  public:
    VowpalTaggit();
    VowpalTaggit(const std::string&);
    
    VowpalTaggit& learn();
    
    VowpalTaggit& predict(std::vector<int>& output);
    
    VowpalTaggit& bos();
    VowpalTaggit& eos();

    VowpalTaggit& tok();
    VowpalTaggit& lex();
    VowpalTaggit& orth(const std::string orth);
    VowpalTaggit& base(const std::string base);
    VowpalTaggit& ctag(const std::string ctag);
    VowpalTaggit& oracle();
    
    VowpalTaggit& readLine(const std::string& line);
    VowpalTaggit& save(const std::string& predictor);
    
    size_t sentNum();
};

void mkTrainer(VowpalTaggit&);
void mkPredictor(VowpalTaggit&);

%perlcode %{
sub ppredict {
    my $self = shift;
    my @out;
    my $v = new VowpalTaggit::vectori();
    $self->predict($v);
    for(my $i = 0; $i < $v->size(); $i++) {
        push(@out, $v->get($i));
    }
    return @out;
}
%}

