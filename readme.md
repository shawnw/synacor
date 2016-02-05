My virtual machine, related tooling, and utility solver programs for the Synacor Challenge programming problem
(https://challenge.synacor.com). Written in a mix of C++ and perl.

Don't peek until you've done it yourself. No cheating!

vm.cc: Source code for the virtual machine that runs the included challenge.bin binary.
disassem.pl and assem.pl: Dissassembler and assembler respectively for the Synacor architecture.
op2bin.pl: Assembles numeric opcodes and arguments to a binary. Not very useful.
dumpstate.pl: The VM in debug mode can dump a snapshop of the binary and environment. This displays the stack,
registers, etc. from one of those snapshots.
stripstate.pl: This removes the enviroment leaving just a normal binary.
solver.cc, solver2_reference.cc, solver2.cc, solver3.cc: Utilities to help solve some of the problems encountered in the program the VM runs.


