use strict;
use FindBin qw($Bin);
use lib $Bin;
use VowpalTaggit;
use Data::Dumper;

my $vt = new VowpalTaggit::VowpalTaggit();

$vt->bos()
        ->tok()->orth("test")
        ->lex()->base("test1")->ctag("test2")->oracle()
        ->lex()->base("test2")->ctag("test2")
    ->eos();
    
$vt->learn();

my $v = new VowpalTaggit::vectori();
$vt->predict($v);

print $v->get(0), "\n";