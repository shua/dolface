-dol2elf- takes an official gamecube dol file,
and turns it back into an elf file. Every document online said to use
a program called doltool, but I have yet to find a live link to this
tool.

-cobbler- fixes your boots
creates a new fst.bin and boot.bin given a game directory

-chum- takes whole dolphin executables, and chops them into smaller, more
easily digestible bits. Useful when I was working on dol2elf.

compile:
I compiled them with a simple 'clang dol2elf -o dol2elf.c' (gcc is fine also).
There's no needed includes other than the std c library.

TODO:
dol2elf
segment .bss into appropriate .sbss and .sbss2 sections
figure out what to do with extab and extabindex

cobbler
-create fst.bin's-
calculate initial offset given sys dir, and write info to boot.bin
