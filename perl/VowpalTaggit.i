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
use XML::Twig;

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

sub XMLTrainer() {
    my $vt = shift;
    VowpalTaggit::mkTrainer($vt);
    my $xml = XML::Twig->new(
        start_tag_handlers => {
            'chunk[@type="s"]' => sub { $vt->bos(); },
            'tok' => sub { $vt->tok(); },
            'lex' => sub { $vt->lex(); },
        },
        twig_handlers => {
            'orth' => sub { $vt->orth($_->trimmed_text()); },
            'base' => sub { $vt->base($_->trimmed_text()); },
            'ctag' => sub { $vt->ctag($_->trimmed_text()); },
            'lex'  => sub { $vt->oracle() if($_->{att}->{disamb} == 1); },
            'chunk[@type="s"]' => sub { $vt->eos(); $_->purge(); },
        },
    );
    return $xml;
}

sub XMLPredictor() {
    my $vt = shift;
    VowpalTaggit::mkPredictor($vt);
    my $xml = XML::Twig->new(
        start_tag_handlers => {
            'chunk[@type="s"]' => sub { $vt->bos(); },
            'tok' => sub { $vt->tok(); },
            'lex' => sub { $vt->lex(); },
        },
        twig_handlers => {
            'orth' => sub { $vt->orth($_->trimmed_text()); },
            'base' => sub { $vt->base($_->trimmed_text()); },
            'ctag' => sub { $vt->ctag($_->trimmed_text()); },
            'lex'  => sub { $vt->oracle() if($_->{att}->{disamb} == 1); },
            'chunk[@type="s"]' => sub { $vt->eos(); $_->purge(); },
        },
    );
    return $xml;
}

%}

