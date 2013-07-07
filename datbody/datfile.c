#include "datfile.h"

#define die(x) { fprintf(stderr, x "\n"); exit(1); }
#define perrordie(x) { perror(x); exit(1); }

void ferrordie(FILE *f, const char *m) {
    if(ferror(f)) {
        fprintf(stderr, "Error while ");
        perrordie(m);
    }else if(feof(f)) {
        fprintf(stderr, "EOF while ");
        perrordie(m);
    }else {
        fprintf(stderr, "Unknown error while %s\n", m);
        exit(1);
    }
}

void DAT_readhdr(DAT_hdr *hdr, FILE *dat) {
    int read;

    read = fread(hdr, sizeof(DAT_hdr), 1, dat);
    if(read != 1)
        ferrordie(dat, "reading dat file header");

    fprintf(stdout, "%08x %08x %08x %08x %08x %08x %08x %08x\n", 
                     hdr->d_filesz, hdr->d_entsz, hdr->d_entnum, 
                     hdr->d_nodenum, hdr->d_nodenum2, hdr->d_sp0,
                     hdr->d_sp1, hdr->d_sp2);
    return;
}

int main(int argc, char **argv) {
    FILE *dat = fopen(argv[1], "rb");
    DAT_hdr hdr;
    memset(&hdr, 0, sizeof(hdr));
    DAT_readhdr(&hdr, dat);
}
