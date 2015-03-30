// basically just swaps an array of bytes
#include<endian.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define HDRSZ 0x20

void usage(char* cmd) {
    printf(
    "Usage: %s DATFILE OFF NUM\n"
    "  OFF is the offset into a melee data file's body\n"
    "  NUM is the number of elements\n",
    cmd);
}

int main(int argc, char** argv) {
    uint8_t* c, *buf;
    int off, num, val;
    float valf;
    FILE* fp;
    if(argc < 4) usage(argv[0]);

    if(!(fp = fopen(argv[1], "r"))) {
        fprintf(stderr, "Error attempting to open %s\n", argv[1]);
        return 2;
    }

    sscanf(argv[2], "%x", &off);
    sscanf(argv[3], "%x", &num);
    c = buf = malloc(num*4);
    fseek(fp, off+HDRSZ, SEEK_SET);
    num = fread(c, 4, num, fp);
    while(num-->0) {
        printf("%08x ", off);
        val = be32toh(*(uint32_t*)c);
        valf = *(float*)&val;
        printf("%8x", val);
        if(valf >= 100.f || valf <= 0.00000001f)
            printf(" %+10E\n", valf);
        else
            printf(" %+2.8f\n", valf);
        c+=4; off+=4;
    }

    free(buf);
    fclose(fp);
    return 0;
}
        
