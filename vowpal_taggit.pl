use strict;
use FindBin qw($Bin);
use lib $Bin;
use VowpalTaggit;
use Data::Dumper;
use XML::Twig;

#my $vt = new VowpalTaggit::VowpalTaggit();
binmode(STDOUT, ":utf8");

my $c = 0;
my $xt = XML::Twig->new(
    start_tag_handlers => {
        'chunk[@type="s"]' => sub { print "bos\n" },
        'tok' => sub { print "tok\n" },
        'lex' => sub { print "lex\n" },
    },
    twig_handlers => {
        'orth' => sub { print "orth\t", $_->trimmed_text(), "\n"; },
        'base' => sub { print "base\t", $_->trimmed_text(), "\n"; },
        'ctag' => sub { print "ctag\t", $_->trimmed_text(), "\n"; },
        'lex'  => sub { print "oracle\n" if($_->{att}->{disamb} == 1); },
        'chunk[@type="s"]' => sub { print "eos\n"; $c++; if($c % 1000 == 0) { print STDERR "$c\n"; }  },
    },
);

#my $xt = XML::Twig->new(
#    start_tag_handlers => {
#        'chunk[@type="s"]' => sub { $vt->bos() },
#        'tok' => sub { $vt->tok() },
#        'lex' => sub { $vt->lex() },
#    },
#    twig_handlers => {
#        'orth' => sub { $vt->orth($_->trimmed_text()); },
#        'base' => sub { $vt->base($_->trimmed_text()); },
#        'ctag' => sub { $vt->ctag($_->trimmed_text()); },
#        'lex'  => sub {
#                        $vt->oracle() if(exists($_->{att}->{disamb})
#                                         and $_->{att}->{disamb} == 1);
#                      },
#        'chunk[@type="s"]' => sub { $vt->eos()->learn(); },
#    },
#);
$xt->parse(\*STDIN);

