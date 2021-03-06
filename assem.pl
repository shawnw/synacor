#!/usr/bin/perl
use strict;
use warnings;
use feature qw/switch/;
no warnings qw/experimental/;

# Assembler for synacor bytecode. Usage: assem.pl IMAGEFILE ASMFILE

# name => [opcode, arg spec...]
# arg spec: "r" for register, "a" for address, "n" for number, "l" for label.
my %insns = (
	"halt" => [0],
	"set" => [1, "r", "rn"],
	"push" => [2, "rn"],
	"pop" => [3, "r"],
	"eq" => [4, "r", "rn", "rn"],
	"gt" => [5, "r", "rn", "rn"],
	"jmp" => [6, "ral"],
	"jt" => [7, "r", "ral"],
	"jf" => [8, "r", "ral"],
	"add" => [9, "r", "rn", "rn"],
	"mult" => [10, "r", "rn", "rn"],
	"mod" => [11, "r", "rn", "rn"],
	"and" => [12, "r", "rn", "rn"],
	"or" => [13, "r", "rn", "rn"],
	"not" => [14, "r", "rn"],
	"rmem" => [15, "r", "al"],
	"wmem" => [16, "al", "rn"],
	"call" => [17, "ral"],
	"ret" =>  [18],
	"out" => [19, "rn"],
	"in" => [20, "r"],
	"noop" => [21]
	);

sub is_register {
	return $_[0] =~ /^r[0-7]$/;
}

sub is_label {
	return $_[0] =~ /^[A-Z]\w+$/;
}
	
sub is_char_literal {
	my $c = shift;
	return $c =~ /^'.'$/ || $c =~ /^'\\.'$/ || $c =~ /^''$/;
}

sub is_number {
	return $_[0] =~ /^\d+$/ || $_[0] =~ /^0x[[:xdigit:]]+$/;
}

sub parse_char_literal {
	for ($_[0]) {
		when (/^'(.)'$/) {
			return ord $1;
		}
		when (/^''$/) {
			return ord " ";
		}
		when (/^'(\\.)'$/) {
			return eval "ord(\"$1\")";
		}
	}
}

sub parse_number {
	if ($_[0] =~ /^0x/) {
		return hex $_[0];
	} else {
		return $_[0];
  }
}

my %labels;

my $addr = 0;
my @parsed = ();

my ($outfile, $infile) = @ARGV;
open my $IN, "<", $infile or die "Unable to open $infile: $!\n";

while (<$IN>) {
	chomp;
	s/\s*#.*$//; # Eat comments
	next if length $_ == 0;
	for ($_) {
		when (/^([A-Z]\w*):$/) {
			die "Duplicate label $1\n" if exists $labels{$1};
			$labels{$1} = $addr;
		}
		when (/^\.word (\d+|0x[[:xdigit:]]+)$/ ) {
			push @parsed, [parse_number $1];
			$addr += 1;
		}
		when (/^([a-z]+)(?:\s+(.+))?/) {
			my $name = $1;
			die "Unknown instruction $name at line $.\n" unless exists $insns{$1};
			my $insn = $insns{$1};
			my $argstr = $2;
			my @args;
			if (defined $argstr) {
				$argstr =~ s/(?<!')' '/''/g;
				@args = split / /, $argstr;
			}
			my $lena = scalar @args;
			my $lene = $#{$insn};
			die "Wrong number of arguments for instruction $name at line ${.}. Got $lena, expected $lene.\n" unless $lena == $lene;
			if ($lena > 0) {
				for (1..$lena) {
					my $arg = $args[$_ - 1];
					my $valid = 0;
					for ($$insn[$_]) {
						$valid = 1 when /r/ && is_register $arg;
						$valid = 1 when /l/ && is_label $arg;
						$valid = 1 when /a/ && is_number $arg;
						$valid = 1 when /n/ && (is_number $arg or is_char_literal $arg);
					}
					warn "Invalid argument for instruction $name at line ${.}.\n" unless $valid;
				}
			}
			my @binary = map {
				if (is_char_literal $_) {
					parse_char_literal $_;
				} elsif (/^r(\d)$/) {
					$1 - 32768;
				} elsif (/^0x/) {
					hex $_;
				} else {
					$_;
				}
			} @args;
			unshift @binary, $$insn[0];
			push @parsed, \@binary;
			$addr += scalar @binary;
		}
		default { die "Invalid line $.\n"; }
	}
}
close $IN;

open my $OUT, ">:raw:bytes", $outfile or die "Unable to open $outfile: $!\n";
for (@parsed) {
	print $OUT pack("v*", map {
		if (is_label $_) {
			die "Undefined label $_\n" unless exists $labels{$_};
			$labels{$_};
		} else {
			$_;
		}
	} @$_);
}
close $OUT;
