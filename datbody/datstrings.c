// prints the string table at the end of a melee data file
#include<endian.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>

#define HDRSZ 0x20

int printstrt(char* file_s) {
    uint32_t i, hdr[5], tsz;
    char* str;
    FILE* fp;
    if(!(fp = fopen(file_s, "r"))) {
        fprintf(stderr, "Error attempting to open %s\n", file_s);
        return 2;
    }
    if(fread(hdr, sizeof(uint32_t), 5, fp)!=5) {
        fprintf(stderr, "Error attempting to read header\n");
        return 2;
    }
    for(i=0; i<5; ++i) hdr[i] = be32toh(hdr[i]);
    tsz = hdr[0] - (HDRSZ+hdr[1]+(hdr[2]*4)+((hdr[3]+hdr[4])*8));
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

void usage(char* cmd) {
    printf(
    "Usage: %s DATFILE\n"
    "   prints the string table of a melee data file (.dat .usd)\n",
    cmd);
    exit(0);
}

int main(int argc, char** argv) {
    if(argc < 2) usage(argv[0]);
    return printstrt(argv[1]);
}
