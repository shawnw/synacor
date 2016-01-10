#!/usr/bin/perl -s
use warnings;
use strict;
use feature qw/switch/;
no warnings qw/experimental/;
use vars qw/$a/;

# Turn a Synacor Challenge binary image file into pseudo-assembly to
# assist with debugging.
# Usage:
# perl disassem.pl challenge.bin > challenge.asm

my @opcodes = (
	["halt", 0], # op 0
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
	["noop", 0] # op 21
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
open $FILE, "<:raw:bytes", $filename or die "Unable to open file $filename: $!\n";

my $addr = 0;
my $word;
while (read($FILE, $word, 2) == 2) {
	my $opcode = unpack "v", $word;
	my $opdesc = $opcodes[$opcode];
	warn "Unknown opcode $opcode at $addr\n" && next unless defined $opdesc;
	print "$addr: " if $a;
	print "$$opdesc[0]";
	if ($$opdesc[1] > 0) {
		for (1 .. $$opdesc[1]) {
			read $FILE, $word, 2;
			$addr += 1;
			my $val = unpack "v", $word;
			if ($$opdesc[0] eq "out") {
				given (chr $val) {
					print " '$_'" when /[[:ascii:]]/ && /[[:print:]]/; 
					print " '\\n'" when $_ eq "\n";
					default { print_value $val; }
				}
			} else {
				print_value $val;
			}
		}
	}
	$addr += 1;
	print "\n";
}
close $FILE;


