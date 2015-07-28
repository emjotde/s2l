use strict;
use XML::Twig;

binmode(STDOUT, ":utf8");

my $c = 0;
my $xt = XML::Twig->new(
    start_tag_handlers => {
        'chunk[@type="s"]' => sub { print "bos\n" },
        'tok' => sub { print "tok\n" },
        'lex' => sub { print "lex\n" },
    },
    twig_handlers => {
        'orth' => sub { print "orth\t", $_->trimmed_text(), "\n"; },
        'base' => sub { print "base\t", $_->trimmed_text(), "\n"; },
        'ctag' => sub { print "ctag\t", $_->trimmed_text(), "\n"; },
        'lex'  => sub { print "oracle\n" if($_->{att}->{disamb} == 1); },
        'chunk[@type="s"]' => sub { print "eos\n"; $c++; if($c % 1000 == 0) { print STDERR "$c\n"; }  },
    },
);

$xt->parse(\*STDIN);
