#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/param.h>

#define EI_NIDENT       16
typedef struct {
    unsigned char       e_ident[EI_NIDENT];
    uint16_t            e_type;
    uint16_t            e_machine;
    uint32_t            e_version;
    uint32_t            e_entry;
    uint32_t            e_phoff;
    uint32_t            e_shoff;
    uint32_t            e_flags;
    uint16_t            e_ehsize;
    uint16_t            e_phentsize;
    uint16_t            e_phnum;
    uint16_t            e_shentsize;
    uint16_t            e_shnum;
    uint16_t            e_shstrndx;
} Elf32_Ehdr;


#define EI_CLASS        4
#define EI_DATA         5
#define EI_VERSION      6
#define EI_PAD          7

#define ELFCLASS32      1
#define ELFDATA2MSB     2
#define EV_CURRENT      1

#define ET_EXEC         2
#define EM_PPC          20

typedef struct {
    uint32_t    p_type;
    uint32_t    p_offset;
    uint32_t    p_vaddr;
    uint32_t    p_paddr;
    uint32_t    p_filesz;
    uint32_t    p_memsz;
    uint32_t    p_flags;
    uint32_t    p_align;
} Elf32_Phdr;

#define PT_LOAD 1
#define PF_R    4
#define PF_W    2
#define PF_X    1

typedef struct {
    uint32_t    s_name;
    uint32_t    s_type;
    uint32_t    s_flags;
    uint32_t    s_addr;
    uint32_t    s_offset;
    uint32_t    s_size;
    uint32_t    s_link;
    uint32_t    s_info;
    uint32_t    s_align;
    uint32_t    s_entsize;
} Elf32_Shdr;

#define SHT_NOBITS   8
#define SHT_PROGBITS 1
#define SHT_STRTAB   3
#define SHF_ALLOC    0x2
#define SHF_WRITE    0x1
#define SHF_EXECINSTR 0x4
#define SHF_LINK_ORDER 0x80

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

#define HAVE_BSS 1

#define MAX_TEXT_SEGMENTS 7
#define MAX_DATA_SEGMENTS 11

#define ELF_SEGMENT_ALIGNMENT 0x10000
#define ELF_SECTION_ALIGNMENT 0x10

typedef struct {
    Elf32_Ehdr header;
    Elf32_Phdr pheader[4];
    Elf32_Shdr sheader[19];
    uint32_t dol_off[4];
    uint32_t shstrtabsz;
    char *shstrtab;
    FILE *dol;
} ELF_map;


#ifndef MAX
#define MAX(a, b)       (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b)       (((a) < (b)) ? (a) : (b))
#endif
#define ALIGN(x, a) (((x) + (a) - 1) & ~((a) - 1))

#if BYTE_ORDER == BIG_ENDIAN
#define swap32(x) (x)
#define swap16(x) (x)
#else
static inline uint32_t 
swap32(uint32_t v) {
    return (v >> 24) | ((v >> 8)  & 0x0000FF00) | ((v << 8)  & 0x00FF0000) | (v << 24);
}

static inline uint16_t 
swap16(uint16_t v) {
    return (v >> 8) | (v << 8);
}
#endif /* BIG_ENDIAN */

#define die(x) { fprintf(stderr, x "\n"); exit(1); }
#define perrordie(x) { perror(x); exit(1); }
void ferrordie(FILE *, const char *);

void read_dol_segments(ELF_map *, const char *);
void map_elf(ELF_map *);
void fcpy(FILE *, FILE *, uint32_t, uint32_t, uint32_t);
void write_elf(ELF_map *, const char *);
void print_dol_hdr(const char *);
void usage(const char *p);

int verbosity = 0;

void 
ferrordie(FILE *f, const char *str) {
    if(ferror(f)) {
        fprintf(stderr, "Error while ");
        perrordie(str);
    } else if(feof(f)) {
        fprintf(stderr, "EOF while %s\n", str);
        exit(1);
    } else {
        fprintf(stderr, "Unknown error while %s\n", str);
        exit(1);
    }
}

#define PHI_TEXT 1
#define PHI_DATA 2
#define PHI_BSS  3

void 
read_dol_segments(ELF_map *map, const char *dol) {
    int read, i;
    DOL_hdr hdr;

    if(verbosity >= 2) fprintf(stderr, "Reading DOL file...\n");

    map->dol = fopen(dol, "rb");
    if(!map->dol) perrordie("Could not open DOL file");

    read = fread(&hdr, sizeof(hdr), 1, map->dol);
    if(read != 1) ferrordie(map->dol, "reading DOL header");

    map->header.e_entry = hdr.entry;

    if(verbosity >= 2) fprintf(stderr, "Valid DOL header found\n");

    int tsec = 0,dsec = 0;
    for(i=0;i<7;++i) {
        if(hdr.text_size[i] != 0)
            tsec = i+1;
    }
    if(!tsec) perrordie("No text sections");
    for(i=0;i<11;++i) {
        if(hdr.data_size[i] != 0)
            dsec = i+1;
    }
    if(!dsec) perrordie("No data sections");
    if(verbosity >= 2) fprintf(stderr, "%d text sections, and %d data sections in dol\n", tsec, dsec);

    map->pheader[PHI_TEXT].p_flags = swap32(PF_R | PF_X);
    map->dol_off[PHI_TEXT] = swap32(hdr.text_off[0]);
    map->pheader[PHI_TEXT].p_paddr = hdr.text_addr[0];
    map->pheader[PHI_TEXT].p_filesz = swap32(swap32(hdr.text_off[tsec-1]) + swap32(hdr.text_size[tsec-1]) - swap32(hdr.text_off[0]));
    map->pheader[PHI_TEXT].p_memsz = map->pheader[PHI_TEXT].p_filesz;

    map->pheader[PHI_DATA].p_flags = swap32(PF_R | PF_W);
    map->dol_off[PHI_DATA] = swap32(hdr.data_off[0]);
    map->pheader[PHI_DATA].p_paddr = hdr.data_addr[0];
    map->pheader[PHI_DATA].p_filesz = swap32(swap32(hdr.data_off[dsec-1]) + swap32(hdr.data_size[dsec-1]) - swap32(hdr.data_off[0]));
    map->pheader[PHI_DATA].p_memsz = map->pheader[PHI_DATA].p_filesz;

    map->pheader[PHI_BSS].p_flags = swap32(PF_R | PF_W);
    map->dol_off[PHI_BSS] = swap32(0);
    map->pheader[PHI_BSS].p_paddr = hdr.bss_addr;
    map->pheader[PHI_BSS].p_filesz = swap32(0);
    map->pheader[PHI_BSS].p_memsz = hdr.bss_size;

    map->header.e_phnum = swap16(4);

    if(verbosity >= 2) {
        fprintf(stderr, "Program Headers:\n");
        for(i = 0; i<swap16(map->header.e_phnum); ++i) {
            uint32_t flags = swap32(map->pheader[i].p_flags);
            fprintf(stderr, "paddr 0x%08x filesz  0x%08x memsz 0x%08x\n",
                    swap32(map->pheader[i].p_paddr), swap32(map->pheader[i].p_filesz),
                    swap32(map->pheader[i].p_memsz));
            fprintf(stderr, "flags %c%c%c        dol_off 0x%08x\n",
                    (flags & PF_R) ? 'r' : '-', (flags & PF_W) ? 'w' : '-', (flags & PF_X) ? 'x' : '-',
                    map->dol_off[i]);
        }
    }

    if(tsec != 2 || dsec != 8)
        return;

    if(verbosity >= 2)
        fprintf(stderr, "Assuming standard nintendo section layout.\n");

    map->sheader[0].s_type = 0;
    for(i=0;i<tsec;++i) {
        map->sheader[i+1].s_name = i;
        map->sheader[i+1].s_type = swap32(SHT_PROGBITS);
        map->sheader[i+1].s_flags = swap32(SHF_ALLOC | SHF_EXECINSTR);
        map->sheader[i+1].s_addr = hdr.text_addr[i];
        map->sheader[i+1].s_offset = swap32(hdr.text_off[i]) - swap32(hdr.text_off[0]);
        map->sheader[i+1].s_size = hdr.text_size[i];
    }
    for(i=0;i<dsec;++i) {
        map->sheader[i+tsec+1].s_name = i+tsec;
        map->sheader[i+tsec+1].s_type = swap32(SHT_PROGBITS);
        map->sheader[i+tsec+1].s_flags = swap32(SHF_ALLOC | SHF_WRITE);
        map->sheader[i+tsec+1].s_addr = hdr.data_addr[i];
        map->sheader[i+tsec+1].s_offset = swap32(hdr.data_off[i]) - swap32(hdr.data_off[0]);
        map->sheader[i+tsec+1].s_size = hdr.data_size[i];
    }
    i=tsec+dsec+1;
    map->sheader[i].s_name = i-1;
    map->sheader[i].s_type = swap32(SHT_NOBITS);
    map->sheader[i].s_flags = swap32(SHF_ALLOC | SHF_WRITE);
    map->sheader[i].s_addr = hdr.bss_addr;
    map->sheader[i].s_offset = map->sheader[i-1].s_offset;
    map->sheader[i].s_size = hdr.bss_size;
}

void 
map_elf(ELF_map *map) {
    if(verbosity >= 2)
        fprintf(stderr, "Laying out ELF file...\n");

    uint32_t fpos;
    int i;

    unsigned char ident[] = {'\177', 'E', 'L', 'F', ELFCLASS32, ELFDATA2MSB, EV_CURRENT, '\0'};
    memcpy(map->header.e_ident, ident, 7);
    map->header.e_version = swap32(EV_CURRENT);
    map->header.e_type = swap16(ET_EXEC);
    map->header.e_machine = swap16(EM_PPC);
    map->header.e_phoff = swap32(sizeof(Elf32_Ehdr));
    map->header.e_flags = swap32(0x80000000);
    map->header.e_ehsize = swap16(sizeof(Elf32_Ehdr));
    map->header.e_phentsize = swap16(sizeof(Elf32_Phdr));

    fpos = (sizeof(Elf32_Ehdr) + swap16(map->header.e_phnum) * sizeof(Elf32_Phdr));
    for(i=0; i<swap16(map->header.e_phnum); ++i) {
        map->pheader[i].p_type = swap32(PT_LOAD);
        map->pheader[i].p_vaddr = map->pheader[i].p_paddr;
        map->pheader[i].p_align = swap32(ELF_SEGMENT_ALIGNMENT);
        map->pheader[i].p_offset = swap32(swap32(map->pheader[i].p_paddr) - 0x80000000); // just how it seems to work out; hacky? please.
    }
    map->pheader[0].p_offset = swap32(fpos); // mysterious null header that shows up on compiled elfs
    map->pheader[0].p_flags = swap32(PF_R | PF_X);

    if(swap32(map->sheader[1].s_size) == 0)
        return;

    if(verbosity >= 2)
        fprintf(stderr, "Creating Section Header String Table\n");

    char* shstr[12] = {".init", ".text", "extab", "extabindex", ".ctors", ".dtors", ".rodata", ".data", ".sdata", ".sdata2", ".bss", ".shstrtab"};
    int shstrtablen[12];
    int shstrtabndx[12];
    map->shstrtabsz = 1;
    for(i=0;i<12;++i)  {
        shstrtablen[i] = strlen(shstr[i]) + 1;
        shstrtabndx[i] = (i==0) ? 1 : (shstrtabndx[i-1] + shstrtablen[i-1]);
        if(verbosity >= 2)
            fprintf(stderr, "%2d : %2d, %2d : %s\n", i, shstrtablen[i], shstrtabndx[i], shstr[i]);
        map->shstrtabsz += shstrtablen[i];
    }
    map->shstrtab = malloc(map->shstrtabsz * sizeof(char));

    if(verbosity >= 2)
        fprintf(stderr, "length: %2d; size: %ld\n", map->shstrtabsz, map->shstrtabsz * sizeof(char));

    fpos = ALIGN(swap32(map->pheader[swap16(map->header.e_phnum)-1].p_offset) + swap32(map->pheader[swap16(map->header.e_phnum)-1].p_filesz), ELF_SECTION_ALIGNMENT);
    map->sheader[12].s_name = 11;
    map->sheader[12].s_type = swap32(SHT_STRTAB);
    map->sheader[12].s_offset = swap32(fpos);
    map->sheader[12].s_size = swap32(map->shstrtabsz * sizeof(char));
    fpos = ALIGN(fpos + swap32(map->sheader[12].s_size), ELF_SECTION_ALIGNMENT);

    if(verbosity >= 2)
        fprintf(stderr, "Laying out Section Header Data\n");

    map->header.e_shoff = swap32(fpos);
    map->header.e_shnum = swap16(13);
    map->header.e_shentsize = swap16(sizeof(Elf32_Shdr));
    map->header.e_shstrndx = swap16(12);

    for(i=0;i<12;++i) {
        memcpy(map->shstrtab + (shstrtabndx[i] * sizeof(char)), shstr[i], shstrtablen[i]);  // concatenate strings from shstr to map->shstrtab
        map->sheader[i+1].s_name = swap32(shstrtabndx[map->sheader[i+1].s_name]);           // change all section header names from string array index(shstr) to char array index(map->shstrtab)
        map->sheader[i+1].s_align = swap32(4);                                          // assuming alignment for dols is all 4 (2**2). I'm assuming they don't use longs or double words but I could easily be wrong
        if(i<2) {
            map->sheader[i+1].s_offset = swap32(map->sheader[i+1].s_offset + swap32(map->pheader[PHI_TEXT].p_offset));
        } else if(i<11) {
            map->sheader[i+1].s_offset = swap32(map->sheader[i+1].s_offset + swap32(map->pheader[PHI_DATA].p_offset));
        }
    }
}

#define BLOCK (1024*1024)

void 
fcpy(FILE *dst, FILE *src, uint32_t dst_off, uint32_t src_off, uint32_t size) {
    int left = size;
    int read;
    int written;
    int block;
    void *blockbuf;

    if(fseek(src, src_off, SEEK_SET) < 0)
        ferrordie(src, "reading DOL segment data");
    if(fseek(dst, dst_off, SEEK_SET) < 0)
        ferrordie(dst, "writing ELF segment data");

    blockbuf = malloc(MIN(BLOCK, left));

    while(left) {
        block = MIN(BLOCK, left);
        read = fread(blockbuf, 1, block, src);
        if(read != block) {
            free(blockbuf);
            ferrordie(src, "reading DOL segment data");
        }
        written = fwrite(blockbuf, 1, block, dst);
        if(written != block) {
            free(blockbuf);
            ferrordie(dst, "writing ELF segment data");
        }
        left -= block;
        if(verbosity >= 2) {
            int totalwritten = size - left;
            int percent = 100 * totalwritten / size;
            fprintf(stderr, "%d %% Written %d / %d \n", percent, totalwritten, size);
        }
    }
    free(blockbuf);
}

void 
write_elf(ELF_map *map, const char *elf) {
    FILE *elff;
    int written;
    int i;

    if(verbosity >= 2)
        fprintf(stderr, "Writing ELF file...\n");

    elff = fopen(elf, "wb");
    if(!elff)
        perrordie("Could not open ELF file for writing");

    written = fwrite(&map->header, sizeof(Elf32_Ehdr), 1, elff);
    if(written != 1)
        ferrordie(elff, "writing ELF file header");

    if(verbosity >= 2)
        fprintf(stderr, "Writing program headers...\n");
    written = fwrite(&map->pheader[0], sizeof(Elf32_Phdr) * swap16(map->header.e_phnum), 1, elff);
    if(written != 1)
        ferrordie(elff, "writing ELF program headers");

    for(i=0; i<swap16(map->header.e_phnum); ++i) {
        if(verbosity >= 2)
            fprintf(stderr, "Writing segment %d...\n", i);
        fcpy(elff, map->dol, swap32(map->pheader[i].p_offset), map->dol_off[i],
                swap32(map->pheader[i].p_filesz));
    }

    if(swap32(map->sheader[1].s_size) != 0) {
        if(verbosity >= 2)
            fprintf(stderr, "Writing section header string table...\n");

        if(fseek(elff,swap32(map->sheader[12].s_offset), SEEK_SET) < 0)
            ferrordie(elff, "positioning section header string table");
        written = fwrite(map->shstrtab, sizeof(char) * map->shstrtabsz, 1, elff);
        if(written != 1)
            ferrordie(elff, "writing section header string table");

        if(verbosity >= 2)
            fprintf(stderr, "Writing ELF section headers...\n");

        if(fseek(elff, swap32(map->header.e_shoff), SEEK_SET) < 0)
            ferrordie(elff, "positioning section header table");
        written = fwrite(&map->sheader[0], sizeof(Elf32_Shdr) * swap16(map->header.e_shnum), 1, elff);
        if(written != 1)
            ferrordie(elff, "writing ELF section headers");
    }

    if(verbosity >= 2)
        fprintf(stderr, "All done!\n");

    fclose(map->dol);
    fclose(elff);
}

void
print_dol_hdr(const char* dol_file) {
    int i;
    DOL_hdr hdr;
    FILE *fp;
    char *tnames[7] = {".init", ".text", "_text2", "_text3", "_text4", "_text5", "_text6"};
    char *dnames[11] = {"extab", "extabindex", ".ctors", ".dtors", ".rodata", ".data", ".sdata", ".sdata2", "_data8", "_data9", "_data10"};

    if(!(fp=fopen(dol_file, "r"))) perrordie("Couldn't open DOL file for reading\n");
    if(fread(&hdr, sizeof(DOL_hdr), 1, fp) != 1) perrordie("Couldn't read header from DOL file");
    fclose(fp);

    printf("name       | %8s %8s %8s\n", "off", "addr", "size");
    for(i=0; i<7; ++i) if(hdr.text_off[i]) printf("%10s | %8x %8x %8x\n", tnames[i], swap32(hdr.text_off[i]), swap32(hdr.text_addr[i]), swap32(hdr.text_size[i]));
    printf("-----------|\n");
    for(i=0; i<11; ++i) if(hdr.data_off[i]) printf("%10s | %8x %8x %8x\n", dnames[i], swap32(hdr.data_off[i]), swap32(hdr.data_addr[i]), swap32(hdr.data_size[i]));
    printf("-----------|\n");
    printf("      .bss |          %8x %8x\n", swap32(hdr.bss_addr), swap32(hdr.bss_size));
    printf("     entry |          %8x\n", swap32(hdr.entry));
}

void 
usage(const char *p) {
    int i=strlen(p);
    while(i!=0 && p[i-1] != '/') { i--; }
    const char* name = p+i;
    printf("Usage: %s [-h] [-v] [-i] [--] dol-file elf-file\n", name);
    printf(" Convert a DOL file to an ELF file (by segments)\n");
    printf(" Options:\n");
    printf("  -h    Show this help\n");
    printf("  -i    Print dol section information\n"); 
    printf("  -v    Be more verbose (twice for even more)\n");
}

int 
main(int argc, char** argv) {
    char **arg;
    int info;
    const char *dol_file;
    const char *elf_file;

    ELF_map map;

    if(argc < 2) {
        usage(argv[0]);
        return 1;
    }
    arg = &argv[1];
    argc--;

    while(argc && (*arg)[0] == '-') {
        switch((*arg)[1]) {
            case 'h': usage(argv[0]); return 1;
            case 'v': ++verbosity; break;
            case 'i': info=1; break;
            case '-': ++arg; --argc; goto filenames;
            default:
                      fprintf(stderr, "Unrecognized option %s\n", *arg);
                      usage(argv[0]);
                      return 1;
        }
        arg++;
        argc--;
    }

filenames:
    if(!info && argc < 2) argc = 0;
    elf_file=dol_file=0;
    switch(argc) {
        case 2: elf_file = arg[1];
        case 1: dol_file = arg[0]; break;
        default:
                usage(argv[0]);
                exit(1);
    }

    memset(&map, 0, sizeof(map));

    if(info) print_dol_hdr(dol_file);
    if(argc > 2) {
        read_dol_segments(&map, dol_file);
        map_elf(&map);
        write_elf(&map, elf_file);
    }
    return 0;
}
