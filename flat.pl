use strict;

my $i = 0;
my @costs;
while(<STDIN>) {
  chomp;
  if(not $_) {
    if(@costs) {
      my $besti = 0;
      my $small = $costs[0];
      foreach my $i (0 .. $#costs) {
        if($costs[$i] < $small) { $besti = $i; $small = $costs[$i]; }
      }
      print "$besti\n";
    }
    @costs = ();
  }
  else {
     /1111:(\S+)/;
     push(@costs, $1);
  }
}
