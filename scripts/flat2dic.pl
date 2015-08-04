use strict;

my $DIC = {};

my ($O, $B, $T);
while (<STDIN>) {
    if (/^orth\t(\S+)$/) {
        $O = $1;
    }
    if (/^base\t(\S+)$/) {
        $B = $1;
    }
    if (/^ctag\t(\S+)$/) {
        $T = $1;
        if ($T ne "ign") {
            $DIC->{$O}->{COUNT}++;
            $DIC->{$O}->{$T}->{COUNT}++;
            $DIC->{$O}->{$T}->{$B}->{COUNT}++;
        }
    }
}

foreach my $tok (sort keys %$DIC) {
    next if $tok eq "COUNT";
    foreach my $tag (sort { $DIC->{$tok}->{$b}->{COUNT}
                            <=> $DIC->{$tok}->{$a}->{COUNT} } grep { !/COUNT/ } keys %{$DIC->{$tok}}) {
        foreach my $base (sort keys %{$DIC->{$tok}->{$tag}}) {
            next if $base eq "COUNT";
            print "$tok\t$base\t$tag\t$DIC->{$tok}->{$tag}->{COUNT}\t$DIC->{$tok}->{$tag}->{$base}->{COUNT}\n"
        }
    }
}
