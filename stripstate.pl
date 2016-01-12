#!/usr/bin/perl
use strict;
use warnings;

# Removes pc,registers,etc from start of a saved state file, leaving
# just the memory image.
# Usage: stripstate.pl SAVE.BIN output.img

my ($infile, $outfile) = @ARGV;

open my $IN, "<:raw:bytes", $infile or die "Unable to open $infile: $!\n";
open my $OUT, ">:raw:bytes", $outfile or die "Unable to open $outfile: $!\n";

my $c;
do {
	$c = getc $IN;
} while ($c ne "~");

my $block;
while (read($IN, $block, 2048) > 0) {
	print $OUT $block;
}
