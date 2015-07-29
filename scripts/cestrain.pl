use strict;
use Getopt::Long;
use lib 'perl';
use VowpalTaggit;

my $PASSES = 1;
my $TRAIN_FILE = undef;
my $TEST_FILE = undef;

GetOptions(
  "train=s" => \$TRAIN_FILE,
  "test=s" => \$TEST_FILE,
  "p|passes=i" => \$PASSES,  
);

my $vt = new VowpalTaggit::VowpalTaggit();
for my $p (1 .. $PASSES) {
    print STDERR "Training pass $p of $PASSES\n";
    $vt->XMLTrainer()->parsefile($TRAIN_FILE);
}

if ($TEST_FILE) {
    binmode(STDOUT, ":utf8");
    $vt->XMLPredictor()->parsefile($TEST_FILE);
}
