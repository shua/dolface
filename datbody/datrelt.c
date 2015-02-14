// prints relocation tables of a melee data fp
#include<endian.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>

#define HDRSZ 0x20

int printrelt(char* file_s) {
    uint32_t i, hdr[3];
    uint32_t* buf;
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
    buf = malloc(hdr[2] * sizeof(uint32_t));
    fseek(fp, 0x20 + hdr[1], SEEK_SET);
    fread(buf, sizeof(uint32_t), hdr[2], fp);
    for(i=0; i<hdr[2]; ++i)
        printf("%08x\n", be32toh(buf[i]));
    
    free(buf);
    fclose(fp);
    return 0;
}

void usage(char* cmd) {
    printf(
    "Usage: %s DATFILE\n"
    "   prints the relocation table of a melee data file (.dat .usd)\n",
    cmd);
    exit(0);
}

int main(int argc, char** argv) {
    if(argc<2) usage(argv[0]);

    return printrelt(argv[1]);
}
