#!/usr/bin/perl
use strict;
use warnings;

# Prints out PC, registers, stack from a saved state file.

my $pc = <>;
chomp $pc;
my @regs;
for (my $i = 0; $i < 8; $i += 1) {
	my $r = <>;
	push @regs, $r;
}
my @stack;
my $ssize = <>;
for (my $i = 0; $i < $ssize; $i += 1) {
	my $s = <>;
	chomp $s;
	unshift @stack, $s;
}

print "PC = $pc\nREGISTERS:\n";
for (my $i = 0; $i < 8; $i += 1) {
	print " r$i = $regs[$i]";
}
print "\nStack: @stack\n";
