use strict;
use Text::LevenshteinXS qw(distance);

package Unk;

use Data::Dumper;

my %UNK_LDF;
my %SUFFIXES;

sub loadUNKs {
  my $UNKIN = shift;
  open(UNKIN, "<:utf8", $UNKIN) or die "Cannot open $UNKIN";
  while (<UNKIN>) {
    chomp;
    my ($w, $b, $t, $tc, $bc) = split(/\t/, $_);
    $UNK_LDF{$w}->{$t}->{COUNT}++;
    $UNK_LDF{$w}->{$t}->{$b}++;

    my $suf = substr($w, length($w)-1);
    $UNK_LDF{TAGS}->{$suf}->{$t}++;
    $SUFFIXES{$suf}->{$w} = 1;
  }
  
  close(UNKIN);      
}

sub getSimilar {
  my ($word, $top) = @_;
  my @D;
  my $last = substr($word, length($word)-1);
  if (exists($SUFFIXES{$last})) {
    foreach my $k (sort keys %{$SUFFIXES{$last}}) {
      push(@D, [$k, Text::LevenshteinXS::distance($word, $k)]);
    }
    @D = sort { $a->[1] <=> $b->[1] or $a->[0] cmp $b->[0] } @D;
    my @topD;
    foreach my $d (@D) {
      push(@topD, $d);
      last if @topD >= $top;
    }
    return @topD;
  }
  return @D;
}


sub getUNKs {
  my $word = shift;
  my $limit = shift;

  my %SEEN;
  my $found = 0;
  my @D = getSimilar($word, 10);
  my @choices;
  foreach my $d (@D) {
    next if not $d;
    my ($w, $l) = @$d;
    $found = 1 if($l == 0);
    my @tags = sort { $a cmp $b } grep { $_ ne "COUNT" } keys %{$UNK_LDF{$w}};
    foreach my $tag (@tags) {
      if ($limit and @choices >= $limit) {
        last;
      }
      push(@choices, [$word, $tag, 0]) if not exists($SEEN{$tag});
      $SEEN{$tag} = 1;
    }
  }

  if(not $found) {
    my $suf = substr($word, length($word)-1);
    my @sorted = sort { $UNK_LDF{TAGS}->{$suf}->{$b} <=> $UNK_LDF{TAGS}->{$suf}->{$a} or $a cmp $b } keys %{$UNK_LDF{TAGS}->{$suf}};
    foreach my $tag (@sorted) {
      #print STDERR "GEN $word $tag\n";
      if ($limit and @choices >= $limit) {
        last;
      }
      push(@choices, [$word, $tag, 0]) if not exists($SEEN{$tag});
      $SEEN{$tag} = 1;
    }
  }
  push(@choices, [$word, "xxx", 0]) if(not exists($SEEN{'xxx'}));
  #print STDERR scalar @choices, "\n";
  return $found, @choices;
}

1;
