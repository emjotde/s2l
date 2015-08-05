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
  my @D = getSimilar($word, 5);
  my @choices;
  foreach my $d (@D) {
    next if not $d;
    my ($w, $l) = @$d;
    $found = 1 if($l == 0);
    my @tags = sort { $a cmp $b } grep { $_ ne "COUNT" } keys %{$UNK_LDF{$w}};
    TOP: foreach my $tag (@tags) {
      my @bases = sort { $a cmp $b } grep { $_ ne "COUNT" } keys %{$UNK_LDF{$w}->{$tag}};
      foreach my $base (@bases) {
        push(@choices, [$word, $tag, 0]) if not exists($SEEN{$tag});
        $SEEN{$tag} = 1;
        if ($limit and @choices >= $limit) {
          last TOP;
        }
      }
    }
    #last if($found);
  }

  if(not $found) {
    my $suf = substr($word, length($word)-1);
    my @sorted = sort { $UNK_LDF{TAGS}->{$suf}->{$b} <=> $UNK_LDF{TAGS}->{$suf}->{$a} or $a cmp $b } keys %{$UNK_LDF{TAGS}->{$suf}};
    foreach my $tag (@sorted) {
      #print STDERR "GEN $word $tag\n";
      push(@choices, [$word, $tag, 0]) if not exists($SEEN{$tag});
      $SEEN{$tag} = 1;
      if ($limit and @choices >= $limit) {
        last;
      }
    }
  }
  push(@choices, [$word, "xxx", 0]) if(not exists($SEEN{'xxx'}));
  return $found, @choices;
}

1;

__DATA__

sub getUNKs2 {
  my $word = shift;
  my $limit = shift;
  my $fill = 1;
  my $ratio = shift;
  
  my $found = 0;
  my @choices;

  my %SEEN;
  if (not defined($ratio) or rand() > $ratio) {
    if (exists($UNK_LDF{$word}) or exists($UNK_LDF{lc($word)})) {
      $found = 1;
      my $cost = 0;
      my $word = exists($UNK_LDF{$word}) ? $word : lc($word);
      my @tags = sort { $a cmp $b } grep { $_ ne "COUNT" } keys %{$UNK_LDF{$word}};
      TOP: foreach my $tag (@tags) {
        my @bases = sort { $a cmp $b } grep { $_ ne "COUNT" } keys %{$UNK_LDF{$word}->{$tag}};
        foreach my $base (@bases) {
          push(@choices, [$base, $tag, $cost]) if not exists($SEEN{$tag});
          $SEEN{$tag} = 1;
          #if ($limit and @choices >= $limit) {
            #  last TOP;
         #}
        }  
      }
    }
  }
  if (not $found or $fill) {
    
    my $otherword = $word;
    for(my $i = length($word); $i >= 0 and @choices < $limit; $i--) {
      my $suf = substr($word, length($word)-$i);
      if(exists($SUFFIXES{$suf})) {
         foreach my $otherword (keys %{$SUFFIXES{$suf}}) {
         last if @choices >= $limit;
         #print STDERR "$word $w\n" if($word =~ /\d/);
         #($otherword) = sort keys %{$SUFFIXES{$suf}};

##################         
   if (exists($UNK_LDF{$otherword}) or exists($UNK_LDF{lc($otherword)})) {
      my $cost = 0;
      $otherword = exists($UNK_LDF{$otherword}) ? $otherword : lc($otherword);
      my @tags = sort { $a cmp $b } grep { $_ ne "COUNT" } keys %{$UNK_LDF{$otherword}};
      TOP: foreach my $tag (@tags) {
        my @bases = sort { $a cmp $b } grep { $_ ne "COUNT" } keys %{$UNK_LDF{$otherword}->{$tag}};
        foreach my $base (@bases) {
          push(@choices, [$word, $tag, $cost]) if not exists($SEEN{$tag});
          $SEEN{$tag} = 1;
          if ($limit and @choices >= $limit) {
              last TOP;
          }
        }
      }
    }}
#################
      
      }
    }


    push(@choices, [$word, "xxx", -1]) if not exists($SEEN{'xxx'});

    if($word eq $otherword) {
    my $base = $word;
    my $cost = -1;
    my @sorted = sort { $UNK_LDF{TAGS}->{$b} <=> $UNK_LDF{TAGS}->{$a} or $a cmp $b }
      keys %{$UNK_LDF{TAGS}};
    foreach my $tag (@sorted) {
      push(@choices, [$word, $tag, $cost]) if not exists($SEEN{$tag});
      if ($limit and @choices >= $limit) {
        last;
      }
    }
    }
    #print Dumper(\@choices);
    #print "Choices ", $found, " " , scalar @choices, "\n";
  }

  return $found, @choices;
}


1;