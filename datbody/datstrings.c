// prints the string table at the end of a melee data file
#include<endian.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>

void usage(char* cmd) {
    printf(
    "Usage: %s DATFILE\n"
    "   where DATFILE is a melee data file usually with a .dat or .usd suffix\n",
    cmd);
}

int main(int argc, char** argv) {
    uint32_t i, hdr[5], tsz;
    char* str;
    FILE* fp;
    if(argc < 2) usage(argv[0]);

    if(!(fp = fopen(argv[1], "r"))) {
        fprintf(stderr, "Error attempting to open %s\n", argv[1]);
        return 2;
    }
    if(fread(hdr, sizeof(uint32_t), 5, fp)!=5) {
        fprintf(stderr, "Error attempting to read header\n");
        return 2;
    }
    for(i=0; i<5; ++i) hdr[i] = be32toh(hdr[i]);
    tsz = hdr[0] - (0x20+hdr[1]+(hdr[2]*4)+((hdr[3]+hdr[4])*8));
    str = malloc(tsz);
    fseek(fp, hdr[0]-tsz, SEEK_SET);
    if(fread(str, tsz, 1, fp)!=1) {
        fprintf(stderr, "Error attempting to read strings\n");
        return 2;
    }

    for(i=0; i<tsz; ++i) {
        if(str[i]) putc(str[i], stdout);
        else putc('\n', stdout);
    }

    free(str);
    fclose(fp);
    return 0;
}
