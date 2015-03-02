#include<endian.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "gxenums.h"

#define LEN(a) (sizeof(a) / sizeof(a[0]))

enum {
 AtWord = 1,
 AtHalf,
 AtByte,
 AtFloat,
 AtOff,
 AtSub,
 AtLast
};
enum { PHEX, PDEC, PBIT };
enum { SFixed, SList, SCust };

typedef struct {
    char* type;
    uint32_t off;
} DatOffset;

struct DatStructVar_t;
typedef union {
    int i;
    unsigned int ui;
    float f;
    void* v;
    struct DatStructVar_t* sv;
} DatExtra;

typedef struct DatStructVar_t {
    unsigned int type;
    char*        name;
    DatExtra     extra;
} DatStructVar;
typedef void (*DatPrintFun)(DatStructVar*,uint8_t*,void*);
typedef struct {
    char* name;
    unsigned int size;
    DatStructVar* vars;
    DatPrintFun print;
} DatStruct;

typedef struct {
    char* pre;
    char* suf;
    int type;
} DatRootFmt;

typedef struct {
    uint32_t o;
    int      t;
    int      v;
} offtype;

#include "structs.h"

int  cmpot(const void*, const void*);
void definit();
int  insot(offtype*, size_t, offtype);
void pdatomic(DatStructVar*, uint8_t*);
int  pfbody(char*, char*, char*);
int  pfonly(char*, char*);
int  pfoffs(char*);
int  pfrelt(char*);
int  pfroot(char*);
int  pfstrings(char*); 
int  pfstruct(char*, uint32_t, uint32_t);
int  sortuot(offtype*, size_t);
int  travdv(uint8_t*, offtype*, DatStructVar*);
void usage();

char* exename;

void definit() {
    for(DatStruct* dp = ddefs; dp < ddefs + LEN(ddefs); ++dp) {
        if(!dp->vars) continue;
        DatStructVar* dvp = dp->vars;
        while(dvp->type) {
            switch (dvp->type) {
            case AtByte: dp->size += 1; break;
            case AtHalf: dp->size += 2; break;
            case AtSub:  dp->size += dvp->extra.sv->type; break;
            default: dp->size += 4; break;
            }
            ++dvp;
        }
    }
}

int pfbody(char* file_s, char* type_s, char* off_s) {
    uint32_t i, type = 0, off;
    if(!type_s) {
        fprintf(stderr, "Missing: TYPE\n");
        usage();
    }
    if(!off_s) {
        fprintf(stderr, "Missing: OFF\n");
        usage();
    }

    if(sscanf(off_s, "%x", &off) != 1) return 3;
    for(i=1; i < LEN(ddefs); i++) {
        if(!strcmp(type_s, ddefs[i].name)) {
            type = i;
            break;
        }
    }
    return pfstruct(file_s, type, off+0x20);
}

int pfonly(char* file_s, char* type_s) {
    uint32_t i, j, hdr[8], type = DatLast;
    uint32_t *body, *root, *strt;
    uint8_t* buf;
    FILE* file;
    offtype* ots;

    file = fopen(file_s, "r");
    if(!file) return 2;

    if(type_s) {
        for(i=1; i < LEN(ddefs); i++) {
            if(!strcmp(type_s, ddefs[i].name)) {
                type = i;
                break;
            }
        }
    }

    fread(hdr, sizeof(uint32_t), 8, file);
    for(i=0; i<5; ++i) hdr[i] = be32toh(hdr[i]);
    buf = malloc(hdr[0]);
    fread(buf, sizeof(uint32_t), hdr[0]-0x20, file);
    fclose(file);
    ots = calloc(hdr[2]*2, sizeof(offtype));

    i=j=0;
    body = (uint32_t*)(buf);
    root = (uint32_t*)(buf+hdr[1]+(hdr[2]*sizeof(uint32_t)));
    strt = (uint32_t*)(buf+hdr[1]+(hdr[2]*sizeof(uint32_t))+((hdr[3]+hdr[4])*ddefs[DatRoot].size));
    for(;root!=strt; root+=2) {
        if(!root[0]) continue;
        ots[j].o = be32toh(root[0]);
        ots[j].t = ROOTTYPE(root, (char*)strt);
        ++j;
    }
    for(;i<j && j<hdr[2]; ++i) {
        if(ots[i].v) continue;
        j+=travdv(buf+ots[i].o, ots+j, ddefs[ots[i].t].vars);
        ots[i].v=1;
        if((j+1)==hdr[2]) {
            j = sortuot(ots, j);
            i=0;
        }
    }
    for(i=0; i<j; ++i) {
        if(type != DatLast && type != ots[i].t) continue;
        printf("-- %08x %s --\n", ots[i].o, ddefs[ots[i].t].name);
        ddefs[ots[i].t].print(ddefs[ots[i].t].vars, buf+ots[i].o, 0);
    }

    free(ots);
    free(buf);
    return 0;
}

int pfoffs(char* file_s) {
    uint32_t i, j, hdr[8], u;
    uint32_t *body, *relt, *root, *strt;
    uint8_t* buf;
    FILE* file;
    offtype* ots;

    file = fopen(file_s, "r");
    if(!file) return 2;

    fread(hdr, sizeof(uint32_t), 8, file);
    for(i=0; i<5; ++i) hdr[i] = be32toh(hdr[i]);
    buf = malloc(hdr[0]);
    fread(buf, sizeof(uint32_t), hdr[0]-0x20, file);
    fclose(file);
    ots = calloc(hdr[2]*2, sizeof(offtype));

    // construct base array from reloc table
    j = hdr[2]*2;
    relt = (uint32_t*)(buf+hdr[1]);
    for(i=0; i<hdr[2]; ++i) {
        body = (uint32_t*)(buf+be32toh(relt[i]));
        ots[i].o = be32toh(*body);
        ots[i].t = DatNULL;
    }
    // u becomes the end of the unknowns, and the sorted array
    u = j = sortuot(ots, j);

    body = (uint32_t*)(buf);
    root = (uint32_t*)(buf+hdr[1]+(hdr[2]*sizeof(uint32_t)));
    strt = (uint32_t*)(buf+hdr[1]+(hdr[2]*sizeof(uint32_t))+((hdr[3]+hdr[4])*ddefs[DatRoot].size));
    for(;root!=strt; root+=2) {
        if(!root[0]) continue;
        ots[j].o = be32toh(root[0]);
        ots[j].t = ROOTTYPE(root, (char*)strt);
        ots[j].v = 0;
        ++j;
    }
    // start i at the beginning of the unsorted/unvisited offtypes
    for(i=u;i<j && j<2*hdr[2]; ++i) {
        if(ots[i].t == DatNULL) continue;
        j+=travdv(buf+ots[i].o, ots+j, ddefs[ots[i].t].vars);
        ots[i].v = 1;

        // if you can insert it, move the last one to where you are
        // if not, move to the next unsorted
        if(insot(ots, u, ots[i])) 
            ots[i--] = ots[--j];
    }
    // last unique sort, not really necessary because you can "| sort -u", but what reason is there to leave the output unsorted?
    j = sortuot(ots, j);

    for(i=0; i<j; ++i) {
        if(ots[i].t < 0) continue;
        printf("%08x %s\n", ots[i].o, ddefs[ots[i].t].name);
    }

    free(ots);
    free(buf);
    return 0;
}

int cmpot(const void* a, const void* b) { 
    return  (((offtype*)a)->o > ((offtype*)b)->o) - 
        (((offtype*)a)->o < ((offtype*)b)->o);
}

int insot(offtype* ots, size_t otsi, offtype val) {
    offtype* match;
    if((match = bsearch(&val, ots, sizeof(offtype), otsi, cmpot))) {
        if(match->t != DatNULL && match->t != val.t)
            fprintf(stderr, "%08x is double-defined: %s, %s\n", match->o, ddefs[match->t].name, ddefs[val.t].name);
        *match = val;
        return 1;
    }
    return 0;
}

int travdv(uint8_t* buf, offtype* ots, DatStructVar* dvp) {
    int i;
    if(!dvp) return 0;
    for(i=0; dvp->type; ++dvp) {
        switch(dvp->type) {
            case AtByte: buf+=1; break;
            case AtHalf: buf+=2; break;
            case AtOff: 
                         if(*(uint32_t*)buf) {
                             ots[i].o = be32toh(*(uint32_t*)buf);
                             ots[i].t = dvp->extra.ui;
                             ots[i].v = 0;
                             ++i;
                         }
            case AtFloat:
            case AtWord: buf+=4; break;
            case AtSub: 
                         i += travdv(buf, ots+i, dvp->extra.sv); 
                         buf += dvp->extra.sv->type;
                         break;
        }
    }
    return i;
}

int  sortuot(offtype* ots, size_t nmemb) {
    int dup = 0, i;
    qsort(ots, nmemb, sizeof(offtype), cmpot);
    for(i=0; i<(nmemb-1); ++i) {
        if(cmpot(ots+i, ots+i+1) == 0) {
            ots[i].o = 0xFFFFFFFF;
            ots[i+1].v |= ots[i].v;
            ++dup;
        }
    }
    if(dup==0) return nmemb;
    qsort(ots, nmemb, sizeof(offtype), cmpot);
    return nmemb-dup;
}

int pfroot(char* file_s) {
    uint32_t i, hdr[8], root;
    char* strt;
    uint8_t* buf;
    FILE* file;

    file = fopen(file_s, "r");
    if(!file) return 2;

    fread(hdr, sizeof(uint32_t), 8, file);
    for(i=0; i<5; ++i) hdr[i] = be32toh(hdr[i]);
    hdr[4]+=hdr[3];
    root = hdr[1]+(hdr[2]*sizeof(uint32_t));
    fseek(file, 0x20+root, SEEK_SET);
    buf = malloc(hdr[4]*ddefs[DatRoot].size);
    fread(buf, ddefs[DatRoot].size, hdr[4], file);
    hdr[5] = hdr[0] - (root+(hdr[4]*ddefs[DatRoot].size));
    strt = malloc(hdr[5]);
    fread(strt, hdr[5], 1, file);

    for(i=0; i < hdr[3]; ++i) {
        printf("-- %08x %s --\n", root+i*ddefs[DatRoot].size, ddefs[DatRoot].name);
        ddefs[DatRoot].print(ddefs[DatRoot].vars, buf+(i*ddefs[DatRoot].size), strt);
    }
    printf("\n");
    for(; i < hdr[4]; ++i) {
        printf("-- %08x %s --\n", root+i*ddefs[DatRoot].size, ddefs[DatRoot].name);
        ddefs[DatRoot].print(ddefs[DatRoot].vars, buf+(i*ddefs[DatRoot].size), strt);
    }

    free(buf);
    fclose(file);
    return 0;
}

int pfstruct(char* file_s, uint32_t type, uint32_t off) {
    FILE* file;
    uint8_t* buf;
    if(type==DatNULL) return 3;
    file = fopen(file_s, "r");
    if(!file) return 2;
    buf = malloc(ddefs[type].size);
    fseek(file, off, SEEK_SET);
    fread(buf, ddefs[type].size, 1, file);
    ddefs[type].print(ddefs[type].vars, buf, 0);

    free(buf);
    fclose(file);
    return 0;
}

void usage() {
    printf(
    "Usage: %s -[ihrobtz] FILE\n"
    "  -i TYPE OFF  prints struct of TYPE at OFF in body\n"
    "  -h           prints header\n"
    "  -r           prints roots\n"
    "  -o           prints struct offsets discovered by relocation table\n"
    "  -b           prints struct offsets discovered by known node types\n"
    "  -t TYPE      prints all structs of TYPE (if TYPE is invalid or omitted, prints all structs)\n"
    "  -z           prints structure sizes\n",
    exename);
    exit(0);
}

int main(int argc, char** argv) {
    char* file_s = NULL, *type_s = NULL, *off_s = NULL;
    enum { ModeInsp, ModeRoot, ModeHdr, ModeBody, ModeType, ModeSize };
    uint32_t mode = ModeBody;
    int i;

    for(i = strlen(argv[0])-1; i>=0 && argv[0][i] != '/'; --i);
    exename = &(argv[0][i+1]);
    
    definit();

    if(argc<2) usage();
    for(i = 1; i < argc; ++i) {
        if(argv[i][0] == '-')
            switch(argv[i][1]) {
            case 'i': mode = ModeInsp; type_s = argv[++i]; off_s = argv[++i]; break;
            case 'h': mode = ModeHdr;  break;
            case 'r': mode = ModeRoot; break;
            case 'b': mode = ModeBody; break;
            case 't': mode = ModeType; type_s = (argc==++i)?NULL:argv[i]; break;
            case 'z': for(i=1; i<LEN(ddefs); ++i) printf("%s %x\n", ddefs[i].name, ddefs[i].size); return 0;
            default:  printf("Unrecognized option %s\n", argv[i]);
                      usage();
            }
        else
            file_s = argv[i];
    }
    
    if(!file_s) {
        printf("You need to specificy a file\n");
        usage();
    }
    
    switch(mode) {
    case ModeInsp: return pfbody(file_s, type_s, off_s);
    case ModeRoot: return pfroot(file_s);
    case ModeHdr:  return pfstruct(file_s, DatHdr, 0);
    case ModeBody: return pfoffs(file_s);
    case ModeType: return pfonly(file_s, type_s);
    }

    return 0;
}

