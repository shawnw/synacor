#!/usr/bin/perl
use warnings;
use strict;

my @opcodes = (
	["halt", 0],
	["set", 2],
	["push", 1],
	["pop", 1],
	["eq", 3],
	["gt", 3],
	["jmp", 1],
	["jt", 2],
	["jf", 2],
	["add", 3],
	["mult", 3],
	["mod", 3],
	["and", 3],
	["or", 3],
	["not", 2],
	["rmem", 2],
	["wmem", 2],
	["call", 1],
	["ret", 0],
	["out", 1],
	["in", 1],
	["noop", 0]
	);

sub is_register {
	my $v = shift;
	return ($v >= 32768 && $v <= 32775);
}

sub print_value {
	my $v = shift;
	if (is_register $v) {
		print " r", $v - 32768;
	} else {
		print " $v";
	}
}

my $FILE;
my $filename = shift;
open $FILE, "<", $filename or die "Unable to open file $filename: $!\n";
binmode $FILE;

my $addr = 0;
my $word;
while (sysread($FILE, $word, 2) == 2) {
	my $opcode = unpack "v", $word;
	my $opdesc = $opcodes[$opcode];
	die "Unknown opcode $opcode at $addr\n" && next unless defined $opdesc;
	print "$addr: $$opdesc[0]";
	if ($$opdesc[1] > 0) {
		for (1 .. $$opdesc[1]) {
			sysread $FILE, $word, 2;
			$addr += 1;
			my $val = unpack "v", $word;
			if ($$opdesc[0] eq "out") {
				my $c = chr $val;
				print " '$c'" if $c =~ /[[:ascii:]]/ && $c =~ /[[:print:]]/;
				print " '\\n'" if $c eq "\n";
			} else {
				print_value $val;
			}
		}
	}
	$addr += 1;
	print "\n";
}
close $FILE;


