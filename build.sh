#!/bin/sh

cc=clang
cflags="-Wall"

if [ ! -d "bin" ]; then
    mkdir bin
fi

rm -rf bin/*

$cc $cflags dol2elf/dol2elf.c -o bin/dol2elf
$cc $cflags chum/chum.c -o bin/chum
$cc $cflags -Icobbler cobbler/fsticuff.c cobbler/main.c -o bin/cbbl

