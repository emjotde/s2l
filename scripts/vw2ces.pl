#!/usr/bin/perl

use strict;
use Getopt::Long;
use XML::Twig;
use Data::Dumper;
use Getopt::Long;
use FindBin;
use lib "$FindBin::Bin";
use Unk;

binmode(STDIN, ":utf8");
binmode(STDOUT, ":utf8");

my $IN;
my $PREDICT;
my $UNKIN;
GetOptions(
  'i|input=s' => \$IN,
  'unk-in=s' => \$UNKIN,
);

Unk::loadUNKs($UNKIN) if $UNKIN;

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
            if ($bestIndex >= @lexemes) {
              my $tag = $lexemes[0]->first_child('ctag')->trimmed_text();
              if (@lexemes == 1 and $tag eq "ign") {
                my $word = $t->first_child('orth')->trimmed_text();
                my ($found, @choices) = Unk::getUNKs($word, 25, 0);
                if (@choices) {
                  #print $bestIndex, "\n";
                  #print Dumper(@choices);
                  my ($base, $tag) = @{$choices[$bestIndex]};
                  $lexemes[0]->set_att('disamb', 1);
                  $lexemes[0]->first_child('base')->set_text($base);
                  $lexemes[0]->first_child('ctag')->set_text($tag); 
                }
                #$newlex->paste('last_child', $t);
              }
            }
            
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
