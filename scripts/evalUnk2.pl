use strict;
use Data::Dumper;

sub zip2 { @_[map { $_, $_ + @_/2 } 0..(@_/2 - 1)] }

my @UNKS = [];
my @UNKWORDS;
while(<STDIN>) {
  chomp;
  if(/^$/) {
    push(@UNKWORDS, $UNKS[-1]->[0]->[0]) if(@UNKS);
    push(@UNKS, []);
    next;
  }
  my ($w, $b, $t, $s) = split(/\t/, $_);
  push(@{$UNKS[-1]}, [$w, $t, $s]);
}

my @G;
my @GW;
open(G, $ARGV[0]);
while(<G>) {
  chomp;
  my ($w, $t, $b) = split(/\t/, $_);
  push(@G, [$w, $t]);
  push(@GW, $w);
}

#print Dumper(\@UNKWORDS);
#print Dumper(\@GW);

use Algorithm::Diff;
my $diff = Algorithm::Diff->new( \@UNKWORDS, \@GW );

my @pairs;
while(  $diff->Next()  ) {
    if($diff->Same()) {
      my @l1 = $diff->Range(1);
      my @l2 = $diff->Range(2);
      push(@pairs, [$l1[$_], $l2[$_]]) foreach(0 .. $#l1);
    }
    else {
      my @l1 = $diff->Range(1);
      push(@pairs, [$l1[$_], -1]) foreach(0 .. $#l1);
    }
}

sub suf {
  my $gtag = shift;
  my $alts = shift;
  my $top  = shift;
  my ($tp, $fp, $fn) = (0,0,0);

  for(my $i = 0; $i < @$alts and $i < $top; $i++) {
    if($gtag eq $alts->[$i]->[1]) { 
      $tp++;
    }
    else {
      $fp++;
    }
  }
  if($tp == 0) {
    $fn++;
  }

  return ($tp, $fp, $fn);
}

my ($TP, $FP, $FN) = (0, 0, 0);
foreach my $p (@pairs) {
  my ($i, $j) = @$p;
  my @unks = @{$UNKS[$i]};
  my $ref  = $G[$j];
 
  my ($tp, $fp, $fn) = suf($ref->[1], \@unks, $ARGV[1]);
  $TP += $tp;
  $FP += $fp;
  $FN += $fn;
  #print Dumper($ref, \@unks);
}

my $P; my $R;
print "$TP $FP $FN\n";
print "Prec: ", $P = $TP/($TP+$FP), "\n";
print "Prec: ", $R = $TP/($TP+$FN), "\n";

print ((2 * $P * $R) / ( $P + $R ), "\n");
