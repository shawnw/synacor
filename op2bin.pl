#!/usr/bin/perl
use warnings;
use strict;

# Read a comma-seperated list of numbers and produce a synacor challenge
# binary file from them.

while (<>) {
	my @nums = split /,/;
	print pack("v*", @nums);
}
	
