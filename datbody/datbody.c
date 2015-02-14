#include<endian.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

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

enum { PDEC, PHEX, PBIT };

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
typedef struct {
    char* name;
    unsigned int size;
    DatStructVar* vars;
} DatStruct;

typedef struct {
    uint32_t  o;
    char*     n;
} offname;

#include "structs.h"

int  cmpron(const void*, const void*);
void definit();
void pdatomic(char*, unsigned int, uint8_t*, DatExtra);
void pdstruct(DatStructVar*, uint8_t*);
int  pfbody(char*, char*, char*);
int  pfoffs(char*, int);
int  pfrelt(char*);
int  pfroot(char*);
int  pfstrings(char*); 
int  pfstruct(char*, uint32_t, uint32_t);
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

void pdatomic(char* buf, unsigned int type, uint8_t* ptr, DatExtra extra) {
    char format[] = "%10d";
    if(extra.ui==PHEX) format[3]='x';
    switch(type) {
        case AtWord:  sprintf(buf, format, be32toh(*(uint32_t*)ptr)); break;
        case AtHalf:  sprintf(buf, format, be16toh(*(uint16_t*)ptr)); break;
        case AtByte:  sprintf(buf, format, *(uint8_t*)ptr); break;
        case AtFloat: sprintf(buf, "%10f", be32toh(*(uint32_t*)ptr)); break;
        case AtOff:   sprintf(buf, "[%08x]", be32toh(*(uint32_t*)ptr)); break;
        case AtSub:   sprintf(buf, "%10d", extra.sv->extra.ui); break;
    }
}

void pdstruct(DatStructVar* dvp, uint8_t* ptr) {
    char atnames[][7] = {
        [AtWord] =  "word ",
        [AtHalf] =  "half ",
        [AtByte] =  "byte ",
        [AtFloat] = "float",
        [AtOff] =   "off  ",
        [AtSub] =   "sub  ",
    };
    char atbuf[11];
    int i;
    while(dvp->type) {
        pdatomic(atbuf, dvp->type, ptr, dvp->extra);
        printf("%5s %10s %s", atnames[dvp->type], atbuf, dvp->name);
        switch(dvp->type) {
        case AtSub: 
            for(i=0; i<dvp->extra.sv->extra.ui; ++i) {
                printf("\n");
                pdstruct(dvp->extra.sv+1, ptr);
                ptr+=dvp->extra.sv->type;
            }
            break;
        case AtHalf: ptr+=2; break;
        case AtByte: ptr+=1; break;
        case AtOff: printf(" %s", ddefs[dvp->extra.ui].name);
        default: ptr+=4; break;
        }
        if(dvp->type != AtSub) printf("\n");
        ++dvp;
    }
}

int main(int argc, char** argv) {
    char* file_s = NULL, *type_s = NULL, *off_s = NULL;
    enum { ModeRelt, ModeBody, ModeRoot, ModeHdr, ModeOffRelt, ModeOffBody, ModeSize };
    uint32_t mode = ModeBody;
    int i;

    for(i = strlen(argv[0])-1; i>=0 && argv[0][i] != '/'; --i);
    exename = &(argv[0][i+1]);

    if(argc<2) usage();
    for(i = 1; i < argc; ++i) {
        if(argv[i][0] == '-')
            switch(argv[i][1]) {
            case 'b': mode = ModeBody; type_s = argv[++i]; off_s = argv[++i]; break;
            case 'h': mode = ModeHdr;  break;
            case 'r': mode = ModeRoot; break;
            case 'o': if(argv[i][2] == 't')      mode = ModeOffRelt;
                      else if(argv[i][2] == 'b') mode = ModeOffBody;
                      break;
            case 't': mode = ModeRelt; break;
            case 'z': mode = ModeSize; file_s = ""; break;
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

    definit();
    
    switch(mode) {
    case ModeRelt: return pfrelt(file_s);
    case ModeBody: return pfbody(file_s, type_s, off_s);
    case ModeRoot: return pfroot(file_s);
    case ModeHdr:  return pfstruct(file_s, DatHdr, 0);
    case ModeOffRelt: return pfoffs(file_s, 0);
    case ModeOffBody: return pfoffs(file_s, 1);
    case ModeSize: for(i=1; i<LEN(ddefs); ++i) printf("%s %x\n", ddefs[i].name, ddefs[i].size);
    }

    return 0;
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

int pfoffs(char* file_s, int mode) {
    uint32_t i, j, hdr[8];
    uint32_t *body, *relt, *root, *strt;
    uint8_t* buf;
    FILE* file;
    offname* ons;
    char* unk = ddefs[DatNULL].name;

    file = fopen(file_s, "r");
    if(!file) return 2;

    fread(hdr, sizeof(uint32_t), 8, file);
    for(i=0; i<5; ++i) hdr[i] = be32toh(hdr[i]);
    buf = malloc(hdr[0]);
    fread(buf, sizeof(uint32_t), hdr[0]-0x20, file);
    ons = calloc(hdr[2]*2, sizeof(offname));
    if(mode==0) {
        relt = (uint32_t*)(buf+hdr[1]);
        for(i=0; i<hdr[2]; ++i) {
            ons[i].o = be32toh(relt[i]);
            ons[i].n = 0;
            body = (uint32_t*)(buf+ons[i].o);
            ons[hdr[2]+i].o = be32toh(*body);
            ons[hdr[2]+i].n = unk;
        }
    } else {
        body = (uint32_t*)(buf);
        root = (uint32_t*)(buf+hdr[1]+(hdr[2]*sizeof(uint32_t)));
        strt = (uint32_t*)(buf+hdr[1]+(hdr[2]*sizeof(uint32_t))+(hdr[3]*ddefs[DatRoot].size));
        for(;root!=strt; root+=2) {
        }
    }

    qsort(ons, hdr[2]*2, sizeof(offname), cmpron);
    for(i=0; i<hdr[2]*2; ++i) {
        printf("%08x", ons[i].o);
        if(ons[i].n)
            printf(" %s", ons[i].n);
        printf("\n");
    }

    return 0;
}

int getrtype(uint32_t* root, char* file_s, char* strt) {

}

int cmpron(const void* a, const void* b) { 
    return ((offname*)b)->o - ((offname*)a)->o; 
}

int travdv(uint8_t* buf, offname* ons, DatStructVar* dvp) {
    int i;
    for(i=0; dvp->type; ++dvp) {
        switch(dvp->type) {
            case AtByte: buf+=2; break;
            case AtHalf: buf+=2; break;
            case AtOff: 
                ons[i].o = be32toh(*(uint32_t*)buf);
                ons[i].n = ddefs[dvp->type].name;
                if(ons[i].o) ++i;
            case AtFloat:
            case AtWord: buf+=4; break;
            case AtSub: 
                i += travdv(buf, ons+i, dvp->extra.sv); 
                buf += dvp->extra.sv->type;
                break;
        }
    }
    return i;
}

int pfrelt(char* file_s) {
    uint32_t i, hdr[3];
    uint32_t* buf;
    FILE* file;

    file = fopen(file_s, "r");
    if(!file) return 2;

    fread(hdr, sizeof(uint32_t), 3, file);
    for(i=0; i<3; ++i) hdr[i] = be32toh(hdr[i]);
    buf = malloc(hdr[2] * sizeof(uint32_t));
    fseek(file, 0x20 + hdr[1], SEEK_SET);
    fread(buf, sizeof(uint32_t), hdr[2], file);
    for(i=0; i<hdr[2]; ++i)
        printf("%08x\n", be32toh(buf[i]));
    
    free(buf);
    fclose(file);
    return 0;
}

int pfroot(char* file_s) {
    uint32_t i, body, root, sroot, reln;
    uint8_t* buf;
    FILE* file;

    file = fopen(file_s, "r");
    if(!file) return 2;

    fseek(file, 4, SEEK_SET);
    fread(&body, sizeof(uint32_t), 1, file);
    body = be32toh(body);
    fread(&reln, sizeof(uint32_t), 1, file);
    fread(&root, sizeof(uint32_t), 1, file);
    fread(&sroot, sizeof(uint32_t), 1, file);
    reln = be32toh(reln);
    root = be32toh(root);
    sroot = be32toh(sroot);
    fseek(file, 0x20+body+(reln*4), SEEK_SET);
    buf = malloc((root+sroot)*ddefs[DatRoot].size);
    fread(buf, ddefs[DatRoot].size, root+sroot, file);
    for(i=0; i < root; ++i)
        pdstruct(ddefs[DatRoot].vars, buf+(i*ddefs[DatRoot].size));
    printf("\n");
    for(; i < (root+sroot); ++i)
        pdstruct(ddefs[DatRoot].vars, buf+(i*ddefs[DatRoot].size));

    free(buf);
    fclose(file);
    return 0;
}

int pfstrings(char* file_s) {
    uint32_t i, hdr[5], size;
    char* buf;
    FILE* file;

    file = fopen(file_s, "r");
    if(!file) return 2;

    fread(hdr, sizeof(uint32_t), 5, file);
    for(i=0; i<5; ++i) hdr[i] = be32toh(hdr[i]);
    size = hdr[0] - (0x20+hdr[1]+(hdr[2]*4)+((hdr[3]+hdr[4])*ddefs[DatRoot].size));
    buf = malloc(size + 1);
    fseek(file, hdr[0] - size, SEEK_SET);
    fread(buf, size, 1, file);
    for(i=0; i < size; ++i) {
        if(buf[i])  putchar(buf[i]);
        else        putchar('\n');
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
    pdstruct(ddefs[type].vars, buf);

    free(buf);
    fclose(file);
    return 0;
}

void usage() {
    printf(
    "Usage: %s -[bhrstz] FILE\n"
    "  -b TYPE OFF  prints structure of TYPE at OFF in body\n"
    "  -h           prints header\n"
    "  -r           prints roots\n"
    "  -t           prints relocation table\n"
    "    -tt        prints offsets pointed to by relocation table\n"
    "  -z           prints structure sizes\n",
    exename);
    exit(0);
}
