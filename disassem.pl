#!/usr/bin/perl -s
use warnings;
use strict;
use feature qw/switch/;
no warnings qw/experimental/;
use vars qw/$a $b $x/;

# Turn a Synacor Challenge binary image file into pseudo-assembly to
# assist with debugging.
# Usage:
# perl disassem.pl [OPTIONS] challenge.bin challenge.asm
#
# Options: -a to prefix each line with its address
#          -b to append a comment with the raw numeric values of each line
#          -x to use base 16 for printing instruction arguments.

my $filename = shift;
open my $FILE, "<:raw:bytes", $filename
	or die "Unable to open file $filename: $!\n";

$filename = shift;
open my $OUT, ">", $filename
	or die "Unable to open file $filename: $!\n";

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
		print $OUT " r", $v - 32768;
	} elsif ($x) {
		printf $OUT " 0x%X", $v;
	} else {
		print $OUT " ", $v;
	}
}

my $addr = 0;
my $word;

while (read($FILE, $word, 2) == 2) {
	printf $OUT "0x%04X: ", $addr if $a;
	$addr += 1;

	my $opcode = unpack "v", $word;
	if ($opcode >= scalar @opcodes) {
		print $OUT ".word $opcode\n";
		next;
	}

	my $opdesc = $opcodes[$opcode];
	my @raw = ($opcode);
	print $OUT $$opdesc[0];
	if ($$opdesc[1] > 0) {
		for (1 .. $$opdesc[1]) {
			die "Unexpected end of file\n" unless read($FILE, $word, 2) == 2;
			$addr += 1;
			my $val = unpack "v", $word;
			push @raw, $val;
			if ($$opdesc[0] eq "out") {
				for (chr $val) {
					print $OUT " '\\''" when $_ eq "'";
					print $OUT " '$_'" when /[[:ascii:]]/ && /[[:print:]]/; 
					print $OUT " '\\n'" when $_ eq "\n";
					default { print_value $val; }
				}
			} elsif ($$opdesc[0] =~ /jmp|jt|jf|rmem/) {
				if ($_ == 1) {
					print_value $val;
				} else {
					if (is_register $val) {
						print_value $val;
					} else {
						printf $OUT " 0x%04X", $val;
					}
				}
			} elsif ($$opdesc[0] =~ /wmem|call/) {
				if ($_ == 1) {
					if (is_register $val) {
						print_value $val;
					} else {
						printf $OUT " 0x%04X", $val;
					}
				} else {
					print_value $val;
				}
			} else {
				print_value $val;
			}
		}
	}
	print $OUT " # @raw" if $b;
	print $OUT "\n";
}
close $FILE;
close $OUT;

