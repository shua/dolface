The main program here, dol2elf, takes an official gamecube dol file,
and turns it back into an elf file. Every document online said to use
a program called doltool, but I have yet to find a live link to this
tool, so I made my own.

"Why?" you ask. To make disassembly and reverse engineering easier.

The other program chop just takes any dol file and chops it up into its
sections for further analysis. It was useful for me, but I don't know how
useful it is for other people.

compile:
I compiled them with a simple 'clang dol2elf -o dol2elf.c' (gcc is fine also).
There's no needed includes other than the std c library.

TODO:
segment .bss into appropriate .sbss and .sbss2 sections
figure out what to do with extab and extabindex
