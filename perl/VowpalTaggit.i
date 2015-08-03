%module VowpalTaggit

%include "std_vector.i"
%include "std_string.i"

namespace std {
    %template(vectori) vector<int>;
    %template(vectord) vector<double>;
};

%{
#include "VowpalTaggit.hpp"
#include "StaticData.hpp"
%}

class StaticData {
  public:
    static StaticData& Init(const std::string&);
    static bool Has(const std::string& key);
    template <typename T>
    static T Get(const std::string& key);

};

%template(GetInt)    StaticData::Get<size_t>;
%template(GetBool)    StaticData::Get<bool>;
%template(GetString) StaticData::Get<std::string>;

class VowpalTaggit {
  public:
    VowpalTaggit();
    VowpalTaggit(const std::string&);
    
    VowpalTaggit& learn();
    
    VowpalTaggit& clearHooks();
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

sub XcesTrainer() {
    my $vt = shift;
    VowpalTaggit::mkTrainer($vt);
    my $xml = XML::Twig->new(
        start_tag_handlers => {
            'chunk[@type="s"]' => sub { $vt->bos(); },
            'tok' => sub { $vt->tok(); },
            'lex' => sub {
                $vt->lex();
                $vt->oracle() if($_->{att}->{disamb} == 1);
            },
        },
        twig_roots => {
            'orth' => sub { $vt->orth($_->trimmed_text()); },
            'base' => sub { $vt->base($_->trimmed_text()); },
            'ctag' => sub { $vt->ctag($_->trimmed_text()); },
            'chunk[@type="s"]' => sub { $vt->eos(); $_->purge(); },
        },
    );
    return $xml;
}

sub XcesPredictor() {
    my $vt = shift;
    $vt->clearHooks();
    
    my $xml = XML::Twig->new(
        pretty_print => 'indented',
        start_tag_handlers => {
            'chunk[@type="s"]' => sub { $vt->bos(); },
            'tok' => sub { $vt->tok(); },
            'lex' => sub { $vt->lex(); },
        },
        twig_handlers => {
            'orth' => sub { $vt->orth($_->trimmed_text()); },
            'base' => sub { $vt->base($_->trimmed_text()); },
            'ctag' => sub { $vt->ctag($_->trimmed_text()); },
            
            'chunk[@type="s"]' => sub {
                my ($twig, $s) = @_;
                $vt->eos();
            
                my @predicts = $vt->ppredict();
            
                # @TODO: Add new alternatives for unknown words
                my @toks = $s->children('tok');
                foreach my $i (0 .. $#toks) {
                  my @lexemes = $toks[$i]->children('lex');
                  foreach my $j (0 .. $#lexemes) {
                    if ($j == $predicts[$i]) {
                      $lexemes[$j]->{att}->{disamb} = 1;
                    }
                  }
                }
                $_->flush();
            },
        },
    );
    return $xml;
}

%}

