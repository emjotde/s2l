use strict;
use FindBin;
use lib "$FindBin::Bin";
use Data::Dumper;

use Unk;

my $UNKS = undef;
my $LIMIT = 25;
my $FILL = 0;
use Getopt::Long;
GetOptions(
  'u|unks=s' => \$UNKS,
  'l|limit=i' => \$LIMIT,
  'f|fill' => \$FILL,
);

die if not $UNKS;
my ($TPs, $FPs, $FNs) = (0, 0, 0);
my ($TPu, $FPu, $FNu) = (0, 0, 0);

Unk::loadUNKs($UNKS);
print STDERR "Loaded dictionary from $UNKS\n";
binmode(STDIN, ":utf8");
binmode(STDOUT, ":utf8");
binmode(STDERR, ":utf8");

while (<STDIN>) {
    chomp;
    my ($w, $tag, $b) = split(/\t/, $_);
    #print "$w\n";
    #print Dumper([Unk::getSimilar($w, 10)]);
    my ($found, @choices) = Unk::getUNKs($w, $LIMIT, $FILL);
    my ($tp, $fp, $fn) = suffstats($tag, @choices);
    if ($found) {
        #print STDERR "$found $w\t$tag\n" if($fn);
        $TPs += $tp;
        $FPs += $fp;
        $FNs += $fn;
    }
    else {
        #print STDERR "$found $w\t$tag\n" if($fn);
        #print STDERR Dumper([Unk::getSimilar($w, 3)]) if($fn);
        $TPu += $tp;
        $FPu += $fp;
        $FNu += $fn;        
    }
    
}

print "Seen   TP: $TPs, FP: $FPs, FN: $FNs\n"; 
print "Unseen TP: $TPu, FP: $FPu, FN: $FNu\n"; 
print sprintf("Seen   P: %.2f, R: %.2f\n", $TPs/($TPs+$FPs), $TPs/($TPs+$FNs), 1);
print sprintf("Unseen P: %.2f, R: %.2f\n", $TPu/($TPu+$FPu), $TPu/($TPu+$FNu), 1);
print sprintf("P: %.2f, R: %.2f\n", ($TPu+$TPs)/($TPu+$FPu+$TPs+$FPs),
              ($TPu+$TPs)/($TPu+$FNu+$TPs+$FNs), 1);

sub suffstats {
    my $tag = shift;
    my @choices = @_;
    
    my $tp = 0;
    my $fp = 0;
    my $fn = 0;
    
    my $found = 0;
    foreach my $c (@choices) {
        my ($base, $ctag, $cost) = @$c;
        if ($ctag eq $tag) {
            $tp++;
            $found = 1;
        }
        else {
            $fp++;
        }
    }
    $fn++ if (not $found);
    return ($tp, $fp, $fn);
}