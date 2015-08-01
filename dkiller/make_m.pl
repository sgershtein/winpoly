#!/usr/bin/perl

use strict;

die "Usage: make_m.pl number_of_points radius center_x center_y\n" unless( $ARGV[0] > 0 && $ARGV[1] > 0);

my $ang = 0;
my $rstep = 3.14159265358979323846*2/$ARGV[0];
my $prec = 1e-10;
my $i;
my $x = $ARGV[2] + 0;
my $y = $ARGV[3] + 0;

print "{\n";

for( $i=0; $i<int($ARGV[0]); $i++ ) {
	my $cos = int( cos( $ang )*$ARGV[1] / $prec ) * $prec;
        my $sin = int( sin( $ang )*$ARGV[1] / $prec ) * $prec;
	printf "%15s %15s\n", $cos+$x, $sin+$y;
        $ang += $rstep;
}

print "}\n\n";

