use strict;
use FindBin qw($Bin);
use lib "$Bin";
use Unk;
use Data::Dumper;

Unk::loadUNKs("data/train01.dic");

binmode(STDIN, ":utf8");
binmode(STDOUT, ":utf8");

my @Q;
my $IGN = 0;
while (<STDIN>) {
    chomp;
    $IGN = 1 if (/^ctag\tign$/);
    if (/^tok|eos$/) {
        annotUNK(\@Q) if($IGN);
        print "$_\n" foreach(@Q);
        @Q = ();
        $IGN = 0;
    }
    push(@Q, $_);
}
print "$_\n" foreach(@Q);

sub annotUNK {
    my $q = shift;
    #print Dumper($q);
    
    my ($orth, $base, $ctag);
    my ($obase, $octag);
    
    my @OUT;
    foreach my $l (@$q) {
        if ($l =~ /^orth\t(\S+)$/) {
            $orth = $1;
        }
        if ($l =~ /^base\t(\S+)$/) {
            $base = $1;
        }
        if ($l =~ /^ctag\t(\S+)$/) {
            $ctag = $1;
        }
        if ($l eq "oracle") {
            $obase = $base;
            $octag = $ctag;
        }
        push(@OUT, $l);
        push(@OUT, "unk") if($l eq "oracle");
        if ($l eq "ctag\tign") {
            pop(@OUT);
            pop(@OUT);
            pop(@OUT);
        }
    }
    
    my ($found, @ldfs) = Unk::getUNKs($orth, 25);
    foreach (@ldfs) {
        my ($base, $ctag, undef) = @$_;
        push(@OUT, "lex", "base\t$base", "ctag\t$ctag", "unk") if($ctag ne $octag);
    }
    
    @$q = @OUT;
    #print Dumper($q);
}
