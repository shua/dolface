#include "fsticuff.h"
#include <stdio.h>

void usage(int argc, char** argv) {
    printf("Usage: %s DIR|INPUT [OPTION] [OUTPUT]\n", argv[0]);
    printf("Creates gamecube fst file given directory DIR containing extracted iso\n");
    printf("Optional OUTPUT file name (Default is fst.bin[.bak])\n\n");
    printf("  -f,   overwrite existing fst.bin (otherwise write fst.bin.bak\n");
}

int main(int argc, char** argv) {
    if(argc < 2) {
        usage(argc, argv);
        return 0;
    } else if(argc < 3) {
//        return generate_fst(argv[1], "fst.bin");
        print_fst(argv[1]);
        return 0;
    } else {
        return generate_fst(argv[1], "fst.bin");
    }
}

