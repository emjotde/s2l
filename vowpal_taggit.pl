use strict;
use FindBin qw($Bin);
use lib $Bin;
use VowpalTaggit;
use Data::Dumper;
use Xml::Twig;

my $vt = new VowpalTaggit::VowpalTaggit();

my $xt = XML::Twig->new(
    start_tag_handlers => {
        'chunk[@type="s"]' => sub { $vt->bos() },
        'tok' => sub { $vt->tok() },
        'lex' => sub { $vt->lex() },
    },
    twig_handlers => {
        'orth' => sub { $vt->orth($_->trimmed_text()); },
        'base' => sub { $vt->base($_->trimmed_text()); },
        'ctag' => sub { $vt->ctag($_->trimmed_text()); },
        'lex'  => sub {
                        $vt->oracle() if(exists($_->{att}->{disamb})
                                         and $_->{att}->{disamb} == 1);
                      },
        'chunk[@type="s"]' => sub { $vt->eos()->learn(); },
    },
);
$xt->parsefile('doc.xml');

#$vt->bos()
#      ->tok()->orth("test")
#        ->lex()->base("test1")->ctag("test2")->oracle()
#        ->lex()->base("test2")->ctag("test2")
#      ->tok()->orth("test")
#        ->lex()->base("test2")->ctag("test2")
#        ->lex()->base("test1")->ctag("test2")->oracle()
#    ->eos();
#    
#$vt->learn();
#
#$vt->bos()
#      ->tok()->orth("test")
#        ->lex()->base("test1")->ctag("test2")
#        ->lex()->base("test2")->ctag("test2")
#      ->tok()->orth("test")
#        ->lex()->base("test2")->ctag("test2")
#        ->lex()->base("test1")->ctag("test2")
#    ->eos();
#
#my @out = $vt->ppredict();
#print Dumper(\@out);