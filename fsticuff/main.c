#include "fsticuff.h"
#include <string.h>
#include <stdio.h>

void usage(int argc, char** argv) {
    printf("Usage: %s DIR|INPUT [OPTION] [OUTPUT]\n", argv[0]);
    printf("Creates gamecube fst file given directory DIR containing extracted iso\n");
    printf("Optional OUTPUT file name (Default is fst.bin[.bak])\n\n");
    printf("  -f    overwrite existing fst.bin (otherwise write fst.bin.bak\n");
    printf("  -p    print INPUT info to stdout\n");
}


#define RF_PRINT 1<<0
#define RF_FORCE 1<<1

int main(int argc, char** argv) {
    char* nargv[argc];
    int nargc = 0;
    int flags = 0;
    for(int i=0; i < argc; i++) {
        if(argv[i][0] == '-') {
            for(int j=1; argv[i][j] != '\0'; j++) {
                switch(argv[i][j]) {
                    case 'p': flags |= RF_PRINT; break;
                    case 'f': flags |= RF_FORCE; break;
                    default: break;
                }
            }
        } else {
            nargv[nargc] = argv[i];
            ++nargc;
        }
    }

    if(nargc < 2) {
        usage(argc, argv);
        return 0;
    }

    if(flags & RF_PRINT) {
        print_fst(nargv[1]);
        return 0;
    }

    while(nargv[1][strlen(nargv[1])-1] == '/')
        nargv[1][strlen(nargv[1])-1] = '\0';
    if(nargc > 2)
        generate_fst(nargv[1], nargv[2]);
    else
        generate_fst(nargv[1], "fst.bin");
    return 0;
}

