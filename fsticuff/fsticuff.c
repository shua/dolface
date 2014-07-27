
//stat and dirent on windows?
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "fsticuff.h"

struct FstEntry {
    unsigned char type;
    unsigned char str_off[3];
    union {
        unsigned int file_off;
        unsigned int parent_off;
    };
    union {
        unsigned int file_length;
        unsigned int next_offset;
        unsigned int num_entries;
    };
};
#define FST_TFILE 0
#define FST_TDIR  1

int swap32(int in) {
    return 
        (in & 0xff) << 24 |
        (in & 0xff00) << 8 |
        (in & 0xff0000) >> 8 |
        (in & 0xff000000) >> 24;
}

int swap24c(unsigned char* in) {
    return (in[0] << 16 | in[1] << 8 | in[2]);
}

int stat_dir(char* dir, FILE* fp);
int stat_file();

int generate_fst(char* gamedir, char* fileout) {
    return 0;
}

void print_fst(char* fst) {
    // sanity checks on file, in case someone gives a real, non-fst file
    struct stat s;
    int staterr;
    if((staterr = stat(fst,&s)) != 0) {
        printf("Error: failed to stat file %s : %d", fst, staterr);
        return;
    }
    if(!(s.st_mode & S_IFREG)) {
        printf("Error: %s is not an fst file", fst);
        return;
    }
    FILE* fp = fopen(fst, "r");
    if(!fp) {
        printf("Error: Opening file %s", fst);
        return;
    }
    struct FstEntry entry = {0};
    fread(&entry, 12, 1, fp);
    if(entry.type != FST_TDIR 
            || entry.str_off[0] & entry.str_off[1] & entry.str_off[2] != 0
            || entry.file_off != 0) {
        printf("Error: %s is not an fst file (incorrect root)", fst);
        return;
    }

    int entries = swap32(entry.num_entries);
    long strtab_off = entries * 12;
    long strtab_sz = s.st_size - strtab_off;
    char* strtab = malloc(strtab_sz + 1);

    fseek(fp, strtab_off, SEEK_SET);
    if(fread(strtab, strtab_sz, 1, fp) != 1) {
        printf("Error: Reading string table: %ld %ld %d", strtab_off, strtab_sz, entries);
        free(strtab);
        fclose(fp);
        return;
    }
    printf("strtab[0] : %s\n", strtab);

    rewind(fp);
    fread(&entry, 12, 1, fp);
    printf("root %d; %x %x\n", swap32(entry.num_entries), swap24c(entry.str_off), swap32(entry.parent_off));
    for(int i = 1; i < entries; i++) {
        fread(&entry, 12, 1, fp);
        printf( "%s %08x %08x %s\n", 
                (entry.type == FST_TDIR)?"d":"f",
                swap32(entry.file_off),
                swap32(entry.file_length),
                strtab + swap24c(entry.str_off));
    }

    free(strtab);
    fclose(fp);
}
