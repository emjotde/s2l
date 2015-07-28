#!/usr/bin/perl

use strict;
use Getopt::Long;
use XML::Twig;
use Data::Dumper;
use Getopt::Long;
use FindBin;

binmode(STDIN, ":utf8");
binmode(STDOUT, ":utf8");

my $IN;
my $ME = 0;
my $TAGS = 0;

my $CLS = undef;
my $EM = undef;
my $UNKOUT = undef;
my $UNKIN = undef;
my $WINDOW = 2;
my $LIMIT = 25;
my $RATIO = undef;

my $SIMPLE;

GetOptions(
  'i|input=s' => \$IN,
  'm|metagger' => \$ME,
  'c|classes=s' => \$CLS,
  'e|embeddings=s' => \$EM,
  't|tags' => \$TAGS,
  'w|window=i' => \$WINDOW,
  'unk-out=s' => \$UNKOUT, 
  'unk-in=s' => \$UNKIN,
  'unk-limit=i' => \$LIMIT,
  'unk-ratio=f' => \$RATIO,
  'simple' => \$SIMPLE,
);

my %UNKS;


my %CLASSES;
if ($CLS) {
  open(CLS, "<:utf8", $CLS) or die "";
  while (<CLS>) {
    chomp;
    my ($w, $c) = split(/\s/, $_);
    $CLASSES{lc($w)} = $c;
  }
  close(CLS);  
}

my %EMBEDDINGS;
if ($EM) {
  open(EM, "<:utf8", $EM) or die "";
  while (<EM>) {
    chomp;
    my ($w, @v) = split(/\s/, $_);
    my $i = 0;
    $EMBEDDINGS{lc($w)} = join(" ", map { "v" . $i++ . ":$_" } @v);
  }
  close(EM);  
}

sub procSentence {
  my ($s) = @_;
  
  foreach my $t (@$s) {
    procTok($t, $s);
  }
  
  print "#EOS\n\n";
}

sub procTok {
  my ($t, $s) = @_;
  
    my $word = $t->{orth};
    my @LDFs = LDFeatures($t, $s, $WINDOW);
    
    my @shared;
    if ($ME) {
      push(@shared, MEFeatures($t, $s, $WINDOW));
    }
    if ($TAGS) {
      push(@shared, TAGFeatures($t, $s, $WINDOW));
    }
    if ($CLS) {
      push(@shared, CLSFeatures(\%CLASSES, $t, $s, $WINDOW));
    }
    if ($EM) {
      push(@shared, EMFeatures(\%EMBEDDINGS, $t));
    }
    
    print "shared |s w^", esc($word) , " " ,  join(" ", @shared), "\n" if(not $SIMPLE);
    
    if($SIMPLE) {
      print "$_ |s w^$word\n" foreach(@LDFs) ;
    }
    else {
      print "$_\n" foreach(@LDFs);
    }
    print "\n";
}

sub addUNKs {
  my $t = shift;
  
  my %SEEN;
  $SEEN{$_} = 1 foreach(map { $_->[1] } @{$t->{lexemes}});
  
  my $word = $t->{orth};
  my ($found, @choices) = Unk::getUNKs($word, $LIMIT, 0, $RATIO);
  foreach my $c (@choices) {
    push(@{$t->{lexemes}}, $c) if(not exists($SEEN{$c->[1]}));
  }
}

sub LDFeatures {
  my ($t, $s, $window) = @_;
  my @LDFs;
  
  #my $HAS_UNK = scalar grep { $_->[2] == -1 } @{$t->{lexemes}};  
  my $HAS_UNK = scalar grep { $_->[1] eq "ign" } @{$t->{lexemes}};  
  
  foreach my $l (@{$t->{lexemes}}) {
    my @ldf;
    
    my $FILL = 0;
    if ($l->[2] == -1) {
      $l->[2] = 0;
      $FILL = 1;
    }
    
    my $cost = 1 - $l->[2];
    my $tag = $l->[1];
    my $base = $l->[0];
    
    push(@{$UNKS{$t->{orth}}}, $l) if ($UNKOUT and $HAS_UNK and $tag ne "ign");
    
    my @subtags = split(/\:/, $tag);
    
    push(@ldf, "1111:$cost", "|t");
    #push(@ldf, "is_unk") if($HAS_UNK);
    #push(@ldf, "fillup") if($FILL);
    #push(@ldf, "dic") if($HAS_UNK and not $FILL);
    push(@ldf, "t^" . esc($tag));
    push(@ldf, "b^" . esc($base));
#    push(@ldf, map { "m^$_" } @subtags) if(not $SIMPLE);
    
    #if (not $SIMPLE) {    
    #  push(@ldf, "|m");
    #  
    #  my $i = $t->{i};
    #  my $j = 1;
    #  while ($j <= $window-1 and $i - $j >= 0 and defined($s->[$i - $j])) {
    #    my $tok = $s->[$i - $j];
    #    my @tags = map { $_->[1] } @{$tok->{lexemes}};
    #    foreach my $t (@tags) {
    #      if ($tag eq $t) {
    #        push(@ldf, "pt${j}^match^" . esc($tag));
    #      }
    #      my @subt = split(/\:/, $t);
    #      push(@ldf, map { "pt${j}^match^F^$_" } match(\@subtags, \@subt));
    #    }
    #    $j++;
    #  }
    #  
    #  $j = 1;
    #  while ($j <= $window-1 and $i + $j < @$s and defined($s->[$i + $j])) {
    #    my $tok = $s->[$i + $j];
    #    my @tags = map { $_->[1] } @{$tok->{lexemes}};
    #    foreach my $t (@tags) {
    #      if ($tag eq $t) {
    #        push(@ldf, "nt${j}^match^" . esc($tag));
    #      }
    #      
    #      my @subt = split(/\:/, $t);
    #      push(@ldf, map { "nt${j}^match^F^$_" } match(\@subtags, \@subt));
    #    }
    #    $j++;
    #  }
    #}
    
    push(@LDFs, join(" ", @ldf));
  }
  
  return @LDFs;
}

sub match {
  my ($a1, $a2) = @_;
  my %f;
  foreach my $t1 (@$a1) {
    foreach my $t2 (@$a2) {            
      $f{$t1} = 1 if($t1 eq $t2);
    }
  }
  return keys %f;
}

sub CLSFeatures {
  my ($classes, $t, $s, $window) = @_;
  
  my @features;
  
  push(@features, '|c');
  
  my $w = lc($t->{orth});
  if (exists($classes->{$w})) {
    push(@features, "c^$classes->{$w}");  
  }
  
  my $j = 1;
  my $i = $t->{i};
  while ($j <= $window and $i - $j >= 0 and defined($s->[$i - $j])) {
    my $tok = $s->[$i - $j];
    my $pw = lc($tok->{orth});
    if (exists($classes->{$pw})) {
      push(@features, "cp${j}^$classes->{$pw}");  
    }
    $j++;
  }
  
  $j = 1;
  while ($j <= $window and $i + $j < @$s and defined($s->[$i + $j])) {
    my $tok = $s->[$i + $j];
    my $nw = lc($tok->{orth});
    if (exists($classes->{$nw})) {
      push(@features, "cn${j}^$classes->{$nw}");  
    }
    $j++;
  }
  
  return @features;
}

sub TAGFeatures {
   my ($t, $s, $window) = @_;
  
  my @features;
  
  push(@features, "|g");
  
  my $i = $t->{i};
  my $j = 1;
  while ($j <= $window and $i - $j >= 0 and defined($s->[$i - $j])) {
    my $tok = $s->[$i - $j];
    my @tags = map { $_->[1] } @{$tok->{lexemes}};
    foreach my $t (@tags) {
      push(@features, "pt${j}^" . esc($t), map { "pt${j}^f^$_" } split(/:/, $t) );
    }
    $j++;
  }
  
  $j = 1;
  while ($j <= $window and $i + $j < @$s and defined($s->[$i + $j])) {
    my $tok = $s->[$i + $j];
    my @tags = map { $_->[1] } @{$tok->{lexemes}};
    foreach my $t (@tags) {
      push(@features, "nt${j}^" . esc($t), map { "nt${j}^f^$_" } split(/:/, $t) );
    }
    $j++;
  }
  
  return @features;
}
   
sub EMFeatures {
  my ($embeddings, $t) = @_;
  
  my @features;

  push(@features, "|v");
  
  my $w = $t->{orth};
  $w = lc($w);
  if (exists($embeddings->{$w})) {
    push(@features, $embeddings->{$w});
  }
  
  return @features;
} 

sub MEFeatures {
  my ($t, $s, $window) = @_;
  
  my @features;
  
  my $w = $t->{orth};
  my $l = length($w);
  
  push(@features, '|r');
  
  push(@features, esc(lc("lc^$w")));
  push(@features, "length^$l");
  push(@features, "uc_first") if ($w =~ /^\p{Lu}/);
  push(@features, "uc_all") if ($w =~ /^\p{Lu}+$/);
  push(@features, "has_number") if ($w =~ /\d/);
  push(@features, "is_number") if ($w =~ /^\d+([.,]\d+)(%)?$/);
  push(@features, "has_letter") if ($w =~ /\p{L}/);
  push(@features, "has_punct") if ($w =~ /\p{p}/);
  push(@features, "has_fix") if ($w =~ /-/);
  
  for my $j (1 .. 4) {
    if($l > $j) {
      push(@features, "pref^" . esc(substr(lc($w), 0, $j)));
      push(@features, "suf^" . esc(substr(lc($w), $l - $j)));
    }
  }
  
  my $j = 1;
  my $i = $t->{i};
  while ($j <= $window and $i - $j >= 0 and defined($s->[$i - $j])) {
    my $tok = $s->[$i - $j];
    push(@features, "pw${j}^". esc($tok->{orth})); 
    $j++;
  }
  
  $j = 1;
  while ($j <= $window and $i + $j < @$s and defined($s->[$i + $j])) {
    my $tok = $s->[$i + $j];
    push(@features, "pn${j}^". esc($tok->{orth})); 
    $j++;
  }
  
  push(@features, "|b");
  for(@$s) {
    push(@features, "bow^" . esc($_->{orth}));
  }
  
  return @features;
}

sub esc {
  my $str = shift;
  $str =~ s/:/_/g;
  return $str;
}

my $COUNT = 0;

my $twig = XML::Twig->new(   
    twig_handlers => {
      chunk => sub {
        my($twig, $s) = @_;
        if ($s->{att}->{type} eq "s") {
          my @s;
          my @toks = $s->children('tok');
          my $i = 0;
          foreach my $t (@toks) {
            my %tok;
            $tok{orth} = $t->first_child('orth')->trimmed_text();
            $tok{lexemes} = [];
            my @lexemes = $t->children('lex');
            foreach my $lex (@lexemes) {
              my $chosen = exists($lex->{att}->{disamb}) ? 1 : 0;
              my $base = $lex->first_child('base')->trimmed_text();
              my $ctag = $lex->first_child('ctag')->trimmed_text();
              
              push(@{$tok{lexemes}}, [$base, $ctag, $chosen]);
            }
            $tok{i} = $i;
            $i++;
            
            push(@s, \%tok);
          }
          procSentence(\@s);
          $COUNT++;
          if ($COUNT % 100 == 0) {
            print STDERR ".";
          }
          if ($COUNT % 10000 == 0) {
            print STDERR "[$COUNT]\n";
          }
        }
        $s->purge();
      }
    }
);
$twig->parsefile($IN);

if ($UNKOUT) {
  open(UNK, ">:utf8", $UNKOUT) or die "Could not open $UNKOUT";
  foreach my $unk (sort keys %UNKS) {
    foreach my $l (@{$UNKS{$unk}}) {
      my ($base, $tag) = @$l;
      print UNK "$unk\t$base\t$tag\n";
    }
  }
  close(UNK);
}

