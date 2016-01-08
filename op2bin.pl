#!/usr/bin/perl
use warnings;
use strict;

# Read a comma-seperated list of numbers and produce a synacor challenge
# binary file from them. Used with the example in arch-spec.
# usage:
# perl op2bin.pl < test.op > test.bin

binmode STDOUT;
while (<>) {
	my @nums = split /,/;
	print pack("v*", @nums);
}
	
