use strict;

package Unk;

my %UNK_LDF;

sub loadUNKs {
  my $UNKIN = shift;
  open(UNKIN, "<:utf8", $UNKIN) or die "Cannot open $UNKIN";
  while (<UNKIN>) {
    chomp;
    my ($w, $b, $t, $tc, $bc) = split(/\t/, $_);
    #$UNK_LDF{$w}->{$t}->{COUNT} = $tc;
    #$UNK_LDF{$w}->{$t}->{$b} = $bc;
    #$UNK_LDF{TAGS}->{$t} += $tc;
    $UNK_LDF{$w}->{$t}->{COUNT}++;
    $UNK_LDF{$w}->{$t}->{$b}++;
    $UNK_LDF{TAGS}->{$t}++;
  }
  close(UNKIN);      
}

sub getUNKs {
  my $word = shift;
  my $limit = shift;
  my $fill = shift;
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
          push(@choices, [$base, $tag, $cost]);
          $SEEN{$tag} = 1;
          if ($limit and @choices >= $limit) {
              last TOP;
          }
        }  
      }
    }
  }
  if (not $found or $fill) {
    my $base = $word;
    my $cost = -1;
    my @sorted = sort { $UNK_LDF{TAGS}->{$b} <=> $UNK_LDF{TAGS}->{$a} or $a cmp $b }
      keys %{$UNK_LDF{TAGS}};
    foreach my $tag (@sorted) {
      push(@choices, [$base, $tag, $cost]) if not exists($SEEN{$tag});
      if ($limit and @choices >= $limit) {
        last;
      }
    }
  }
  return $found, @choices;
}


1;