use strict;
use Getopt::Long;
use lib 'perl';
use VowpalTaggit;

VowpalTaggit::StaticData::Init("--vw-args '-i $ARGV[0]'");
my $vt = new VowpalTaggit::VowpalTaggit();

binmode(STDOUT, ":utf8");
myXcesPredictor($vt)->parse(\*STDIN);

sub myXcesPredictor {
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
