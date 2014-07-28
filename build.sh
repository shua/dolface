#!/bin/sh

cc=clang
cflags="-Wall"

if [ ! -d "bin" ]; then
    mkdir bin
fi

rm -rf bin/*

$cc $cflags dol2elf/dol2elf.c -o bin/dol2elf
$cc $cflags chop/chop.c -o bin/chop
$cc $cflags -Ifsticuff fsticuff/fsticuff.c fsticuff/main.c -o bin/fsticuff

