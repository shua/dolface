// prints relocation tables of a melee data fp
#include<endian.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x)
#endif
#define HDRSZ 0x20

int extenddat(char* file_s, int off, int size) {
    uint32_t i, hdr[8], *offs, *offc, offt;
    uint8_t *buf, *zero;
    FILE *fp;
    char *new_s;

    if(!(fp = fopen(file_s, "r"))) {
        fprintf(stderr, "Error attempting to open %s\n", file_s);
        return 2;
    }

    if(fread(hdr, sizeof(uint32_t), 8, fp)!=8) {
        fprintf(stderr, "Error attempting to read header\n");
        return 2;
    }
    for(i=0; i<8; ++i) hdr[i] = be32toh(hdr[i]);
    buf = malloc(hdr[0]);
    fseek(fp, HDRSZ, SEEK_SET);
    if(fread(buf+HDRSZ, hdr[0]-HDRSZ, 1, fp) != 1) {
        fprintf(stderr, "Error reading file into memory\n");
        return 2;
    }
    fclose(fp);

    /* relocation table */
    offs = malloc(2*hdr[2]);
    for(i=0; i<hdr[2]; ++i) {
        offc = ((uint32_t*)(buf+HDRSZ+hdr[1]))+i;
        offt = be32toh(*offc);
        if(offt >= off) {
            *offc = be32toh(offt+size);
            DBG(printf("%8x->%8x\n", offt, offt+size);)
        }
        offc = (uint32_t*)(buf+HDRSZ+offt);
        offt = be32toh(*offc);
        if(offt >= off) {
            *offc = be32toh(offt+size);
            DBG(printf("%8x->%8x\n", offt, offt+size);)
        }
    }

    /* root nodes */
    for(i=0; i<hdr[3]; ++i) {
        offc = (uint32_t*)(buf+HDRSZ+hdr[1]+(hdr[2]*4)+(i*8));
        offt = be32toh(*offc);
        if(offt >= off) {
            *offc = be32toh(offt+size);
            DBG(printf("%8x->%8x\n", offt, offt+size);)
        }
    }
    for(i=0; i<hdr[4]; ++i) {
        offc = (uint32_t*)(buf+HDRSZ+hdr[1]+(hdr[2]*4)+((hdr[3]+i)*8));
        offt = be32toh(*offc);
        if(offt >= off) {
            *offc = be32toh(offt+size);
            DBG(printf("%8x->%8x\n", offt, offt+size);)
        }
    }
    new_s = malloc(strlen(file_s)+5);
    strcpy(new_s, file_s);
    strcpy(new_s+strlen(file_s), ".new");
    if(!(fp = fopen(new_s, "w"))) {
        fprintf(stderr, "Error attempting to open output file %s\n", new_s);
        return 2;
    }
    free(new_s);

    /* adjust header */
    hdr[0] += size;
    hdr[1] += size;
    for(i=0; i<8; ++i) ((uint32_t*)buf)[i] = be32toh(hdr[i]);
    zero = calloc(1, size);
    /* write beginning of file, then extension, then end */
    fwrite(buf, off+HDRSZ, 1, fp);
    fseek(fp, size, SEEK_CUR);
    fwrite(buf+off+HDRSZ, hdr[0]-size-off-HDRSZ, 1, fp);

    fclose(fp);
    free(zero);
    free(buf);
    return 0;
}

void usage(char* cmd) {
    printf(
    "Usage: %s DATFILE OFF SIZE\n"
    "   inserts SIZE zeros in DATFILE at OFF, and adjusts rest of file accordingly (header size info, and offsets)\n"
    "   SIZE and OFF are expected to be hexadecimal numbers\n",
    cmd);
}

int main(int argc, char** argv) {
    uint32_t off, size;
    if(argc<4) {
        usage(argv[0]);
        return 0;
    }

    if(sscanf(argv[2], "%x", &off) != 1) {
        fprintf(stderr, "invalid offset %s\n", argv[2]);
        return 1;
    }
    if(sscanf(argv[3], "%x", &size) != 1) {
        fprintf(stderr, "invalid size %s\n", argv[3]);
        return 1;
    }
    return extenddat(argv[1], off, size);
}
