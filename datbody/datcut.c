#include "datcut.h"
#include <stdio.h>
#include <endian.h>

int relocDump(char* datz, char* out) {
    FILE* dat = fopen(datz, "r");
    DatHeader_buf hdr;
    fread(&hdr, sizeof(DatHeader_buf), 1, dat);
    hdr.filesz = be32toh(hdr.filesz);
    hdr.datasz = be32toh(hdr.datasz);
    hdr.reltnum = be32toh(hdr.reltnum);
    hdr.rootnum = be32toh(hdr.rootnum);
    hdr.srootnum = be32toh(hdr.srootnum);
    int relt[hdr.reltnum];
    fseek(dat, hdr.datasz + 8*(hdr.rootnum + hdr.srootnum), SEEK_CUR);
    if(fread(relt, 4, hdr.reltnum, dat) != hdr.reltnum) {
        fprintf(stderr, "couldn't read the entire table");
        return 0;
    }
    fclose(dat);

    int outn = strlen(out);
    char outfile[outn + 32];
    strncpy(outfile, out, outn);
    if(outfile[outn-1] != '/')
        outfile[outn++] = '/';
    strcpy(outfile+outn, "relt.bin");
    FILE* fp = fopen(outfile, "w");
    fwrite(relt, 4, hdr.reltnum, fp);
}
