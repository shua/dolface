//stat and dirent on windows?
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "fsticuff.h"

// magic number, could be tied to /sys size, but I don't know
// NOTE I don't understand how they calculate offsets, it seems to be
//  0x20 aligned, like file sizes, but some files are pushed for no
//  apparent reason.
const int _initial_off = 0x45d640; 

// swap functions
int swap32(int in) {
    return 
        (in & 0xff) << 24 |
        (in & 0xff00) << 8 |
        (in & 0xff0000) >> 8 |
        (in & 0xff000000) >> 24;
}

// fst enty struct and defines
struct FstEntry {
    int id;
    union {
        unsigned int file_off;
        unsigned int parent_off;
    };
    union {
        unsigned int file_length;
        unsigned int next_off;
        unsigned int num_entries;
    };
};

int _ename(struct FstEntry e) { return swap32(e.id) & 0x00ffffff; }
int _etype(struct FstEntry e) { return (swap32(e.id) & 0xff000000) >> 24; }
struct FstEntry* _esetid(struct FstEntry* dest, int t, int s_idx) {
    dest->id = swap32(((t & 0xff) << 24) | (s_idx & 0x00ffffff));
    return dest;
}
#define FST_TFILE 0
#define FST_TDIR  1

// utility string array struct
struct StrArray {
    int size;
    int cap;
    char* m;
};

int insertStr(struct StrArray* dest, char* str) {
    if(strlen(str) + dest->size >= dest->cap) {
        char* tmp = malloc(2*dest->cap);
        memcpy(tmp, dest->m, dest->size);
        dest->cap *= 2;
        free(dest->m);
        dest->m = tmp;
    }
    int ret = dest->size;
    strcpy(dest->m + dest->size, str);
    dest->size += (strlen(str) + 1);
    return ret;
}

struct StrArray getStrArray(int icap) {
    struct StrArray ret = { .size = 0, .cap = icap, .m = malloc(icap) };
    return ret;
}

// recursive write functions
struct IdxOff {
    int idx;
    unsigned int off;
};

int filter_dot(const struct dirent* d) { return (d->d_name[0] != '.'); }
// for some reason, this is how the original fst is sorted
//  first lowercase, then uppercase, then punctuation
int _fststrcmp(const char* a, const char* b) {
    int i=0,ga=0,gb=0;
    while(a[i] == b[i] && a[i] != '\0' && b[i] != '\0') i++;
    if(a[i] == '\0' || b[i] == '\0') return a[i] - b[i];

    if(a[i] <= '9' && a[i] >= '0') ga=0;
    else if(a[i] <= 'z' && a[i] >= 'a') ga=1;
    else if(a[i] <= 'Z' && a[i] >='A') ga=2;
    else ga=3;
    if(b[i] <= '9' && b[i] >= '0') gb=0;
    else if(b[i] <= 'z' && b[i] >='a') gb=1;
    else if(b[i] <= 'Z' && b[i] >= 'A') gb=2;
    else gb=3;

    if(ga == gb)
        return a[i] - b[i];
    else
        return ga - gb;
}
int fstsort(const struct dirent** a, const struct dirent** b) {
    return _fststrcmp((*a)->d_name, (*b)->d_name);
}

struct IdxOff stat_dir(char* dirname, FILE* fp, struct IdxOff idxoff, unsigned int pidx, struct StrArray* strtab) {
    struct dirent** namelist;
    int namelistsz;
    struct FstEntry entry = {0};
    struct stat s;
    int staterr;

    namelistsz = scandir(dirname, &namelist, filter_dot, fstsort);
    if(namelistsz < 0) {
        perror("scandir");
        idxoff.idx = -1;
        return idxoff;
    }
    for (int n = 0; n < namelistsz; n++)  {
        char path[strlen(dirname) + strlen(namelist[n]->d_name) + 2];
        strcpy(path, dirname);
        strcat(path, "/");
        strcat(path, namelist[n]->d_name);

        if((staterr = stat(path, &s)) != 0) {
            printf("Error: could not stat %s: %d", path, staterr);
            free(namelist[n]);
            continue;
        }

        if(s.st_mode & S_IFDIR) {
            struct IdxOff tmp = idxoff;
            idxoff.idx++;
            long curpos = ftell(fp);
            int name = insertStr(strtab, namelist[n]->d_name);
            
            _esetid(&entry, FST_TDIR, name);
            entry.parent_off = swap32(pidx);
            fseek(fp, 12, SEEK_CUR);
            
            idxoff = stat_dir(path, fp, idxoff, tmp.idx, strtab);
            if(idxoff.idx < 0) {
                while(n < namelistsz)
                    free(namelist[n++]);
                free(namelist);
                return idxoff;
            }
            
            entry.next_off = swap32(idxoff.idx);
            long nextpos = ftell(fp);
            fseek(fp, curpos, SEEK_SET);
            fwrite(&entry, 12, 1, fp);
            fseek(fp, nextpos, SEEK_SET);

        } else if(s.st_mode & S_IFREG) {
            int size = (s.st_size / 0x20 + (((s.st_size % 0x20) == 0)?0:1)) * 0x20;
            int name = insertStr(strtab, namelist[n]->d_name);
            
            _esetid(&entry, FST_TFILE, name);
            entry.file_off = swap32(idxoff.off);
            entry.file_length = swap32(size);

            fwrite(&entry, 12, 1, fp);
            
            idxoff.off += size;
            idxoff.idx++;
        } else {
            printf("Warning: %s is some kind of funky", path);
        }

        free(namelist[n]);
    }
    free(namelist);
    return idxoff;
}

// main functions
int generate_fst(char* gamedir, char* fileout) {
    struct StrArray strtab = getStrArray(256);
    FILE* fp = fopen(fileout, "w");
    fseek(fp, 12, SEEK_SET);
    struct IdxOff entries = { 1, _initial_off };
    entries = stat_dir(gamedir, fp, entries, 0, &strtab);

    fwrite(strtab.m, strtab.size, 1, fp);
    free(strtab.m);

    struct FstEntry root = { 0, 0, swap32(entries.idx) };
    _esetid(&root, FST_TDIR, 0);
    fseek(fp, 0, SEEK_SET);
    fwrite(&root, 12, 1, fp);

    fclose(fp);
    return 0;
}

void print_fst(char* fst) {
    // sanity checks on file, in case someone gives a real, non-fst file
    struct stat s;
    int staterr;
    if((staterr = stat(fst,&s)) != 0) {
        printf("Error: failed to stat file %s : %d\n", fst, staterr);
        return;
    }
    if(!(s.st_mode & S_IFREG)) {
        printf("Error: %s is not an fst file\n", fst);
        return;
    }
    FILE* fp = fopen(fst, "r");
    if(!fp) {
        printf("Error: Opening file %s\n", fst);
        return;
    }
    struct FstEntry entry = {0};
    fread(&entry, 12, 1, fp);
    if(_etype(entry) != FST_TDIR 
            || _ename(entry) != 0
            || entry.file_off != 0) {
        printf("Error: %s is not an fst file (incorrect root)\n", fst);
        return;
    }

    int entries = swap32(entry.num_entries);
    long strtab_off = entries * 12;
    long strtab_sz = s.st_size - strtab_off;
    char* strtab = malloc(strtab_sz + 1);

    fseek(fp, strtab_off, SEEK_SET);
    if(fread(strtab, strtab_sz, 1, fp) != 1) {
        printf("Error: Reading string table: %ld %ld %d\n", strtab_off, strtab_sz, entries);
        free(strtab);
        fclose(fp);
        return;
    }

    rewind(fp);
    fread(&entry, 12, 1, fp);
    for(int i = 1; i < entries; i++) {
        fread(&entry, 12, 1, fp);
        printf( "%s %08x %08x %s\n", 
                (_etype(entry) == FST_TDIR)?"d":"f",
                swap32(entry.file_off),
                swap32(entry.file_length),
                strtab + _ename(entry));
    }

    free(strtab);
    fclose(fp);
}
