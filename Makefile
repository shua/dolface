CFLAGS=-Wall

all: bin/dol2elf bin/chum bin/cbbl ohdis

bin/dol2elf: dol2elf/dol2elf.c
	$(CC) $(CFLAGS) dol2elf/dol2elf.c -o bin/dol2elf

bin/chum: chum/chum.c
	$(CC) $(CFLAGS) chum/chum.c -o bin/chum

bin/cbbl: cobbler/fsticuff.c cobbler/main.c
	$(CC) $(CFLAGS) cobbler/fsticuff.c cobbler/main.c -o bin/cbbl

ohdis: bin/datstrings bin/datrelt bin/datbody bin/datinsp bin/insp.sh

bin/insp.sh: datbody/insp.sh
	cp datbody/insp.sh bin/insp.sh

bin/datstrings: datbody/datstrings.c
	$(CC) $(CFLAGS) datbody/datstrings.c -o bin/datstrings

bin/datrelt: datbody/datrelt.c
	$(CC) $(CFLAGS) datbody/datrelt.c -o bin/datrelt

bin/datbody: datbody/datbody.c datbody/structs.h
	$(CC) $(CFLAGS) -Wno-missing-braces datbody/datbody.c -o bin/datbody

bin/datinsp: datbody/datinsp.c
	$(CC) $(CFLAGS) datbody/datinsp.c -o bin/datinsp

