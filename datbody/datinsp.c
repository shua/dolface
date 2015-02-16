// basically just swaps an array of bytes
#include<endian.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define HDRSZ 0x20

void usage(char* cmd) {
    printf(
    "Usage: %s DATFILE OFF NUM SIZE\n"
    "  OFF is the offset into a melee data file's body\n"
    "  NUM is the number of elements\n"
    "  SIZE is the size of elements in bytes (4,2,or 1)\n",
    cmd);
}

int main(int argc, char** argv) {
    uint8_t* c, *buf;
    int off, num, size;
    FILE* fp;
    if(argc < 5) usage(argv[0]);

    if(!(fp = fopen(argv[1], "r"))) {
        fprintf(stderr, "Error attempting to open %s\n", argv[1]);
        return 2;
    }

    sscanf(argv[2], "%x", &off);
    sscanf(argv[3], "%x", &num);
    sscanf(argv[4], "%x", &size);
    c = buf = malloc(num*size);
    fseek(fp, off+HDRSZ, SEEK_SET);
    num = fread(c, size, num, fp);
    while(num-->0) {
        printf("%08x ", off);
        switch(size) {
            case 4: printf("%8x\n", be32toh(*(uint32_t*)c)); c+=4; off+=4; break;
            case 2: printf("%4x\n", be16toh(*(uint32_t*)c)); c+=2; off+=2; break;
            case 1: printf("%2x\n", *c); c+=1; off+=1; break;
        }
    }

    free(buf);
    fclose(fp);
    return 0;
}
        
