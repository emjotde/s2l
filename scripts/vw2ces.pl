#!/usr/bin/perl

use strict;
use Getopt::Long;
use XML::Twig;
use Data::Dumper;
use Getopt::Long;
use FindBin;
use lib "$FindBin::Bin";

binmode(STDIN, ":utf8");
binmode(STDOUT, ":utf8");

my $IN;
my $PREDICT;
GetOptions(
  'i|input=s' => \$IN,
);

my @PRED;
while (<STDIN>) {
  chomp;
  push(@PRED, grep { $_ ne "" } split(/\s/, $_));
}

print STDERR "Disambiguating '$IN'\n";

my $COUNT = 0;
my $twig = XML::Twig->new(   
    pretty_print => 'indented',
    twig_handlers => {
      chunk => sub {
        my($twig, $s) = @_;
        if ($s->{att}->{type} eq "s") {
          my @s;
          my @toks = $s->children('tok');
          foreach my $t (@toks) {
            my @lexemes = $t->children('lex');
            my $bestIndex = $PRED[$COUNT];
            my $i = 0;
            foreach my $lex (@lexemes) {
              if ($i == $bestIndex) {
                $lex->{att}->{disamb} = 1;
              }
              $i++;
            }
            $COUNT++;
            if ($COUNT % 10000 == 0) {
              print STDERR "$COUNT iterations...\n";
            }
          }
          $s->flush();
        }
      }
    }
);
$twig->parsefile($IN);
