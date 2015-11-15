#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<endian.h>

typedef struct { 
    uint32_t text_off[7];
    uint32_t data_off[11];
    uint32_t text_addr[7];
    uint32_t data_addr[11];
    uint32_t text_size[7];
    uint32_t data_size[11];
    uint32_t bss_addr;
    uint32_t bss_size;
    uint32_t entry;
    uint32_t pad[7];
} DOL_hdr;

typedef struct {
    uint8_t *data;
    uint16_t max_text;
    uint16_t max_data;
    uint32_t max_off;
    uint32_t max_addr;
    DOL_hdr hdr;
} Read_info;

int
opendol(char *dol, Read_info *dst) {
    FILE *fp;
    int i;
    uint32_t *itr;
    if((fp=fopen(dol, "r")) == NULL) {
        fprintf(stderr, "Error opening file %s\n", dol);
        return 2;
    }

    if(fread(&dst->hdr, sizeof(DOL_hdr), 1, fp) != 1) {
        fprintf(stderr, "Error reading header\n");
        fclose(fp);
        return 2;
    }

    for(itr=(uint32_t*)&dst->hdr; (uint8_t*)itr < ((uint8_t*)(&dst->hdr)+sizeof(DOL_hdr)); ++itr) {
        *itr = be32toh(*itr);
    }

    for(i=0; dst->hdr.text_off[i] && i<7; ++i) {
        if(dst->hdr.text_off[i]+dst->hdr.text_size[i] > dst->max_off)
            dst->max_off = dst->hdr.text_off[i]+dst->hdr.text_size[i];
        if(dst->hdr.text_addr[i]+dst->hdr.text_size[i] > dst->max_addr)
            dst->max_addr = dst->hdr.text_addr[i]+dst->hdr.text_size[i];
    }
    dst->max_text = i;
    for(i=0; dst->hdr.data_off[i] && i<11; ++i) {
        if(dst->hdr.data_off[i]+dst->hdr.data_size[i] > dst->max_off)
            dst->max_off = dst->hdr.data_off[i]+dst->hdr.data_size[i];
        if(dst->hdr.data_addr[i]+dst->hdr.data_size[i] > dst->max_addr)
            dst->max_addr = dst->hdr.data_addr[i]+dst->hdr.data_size[i];
    }
    dst->max_data = i;
    if(dst->hdr.bss_addr + dst->hdr.bss_size > dst->max_addr)
        dst->max_addr = dst->hdr.bss_addr + dst->hdr.bss_size;

    dst->data = malloc(dst->max_off);
    fseek(fp, 0, SEEK_SET);
    if(fread(dst->data, dst->max_off, 1, fp) != dst->max_off) {
        fprintf(stderr, "Error reading file data\n");
        free(dst->data);
        fclose(fp);
        return 2;
    }
    return 0;
}

void
usage(char *name) {
    printf("Usage: %s DOL \n",
            name);
}

int
main(int argc, char **argv) {
    Read_info info = { 0 };
    if(argc < 3)
        return (usage(argv[0]), 0);
}
