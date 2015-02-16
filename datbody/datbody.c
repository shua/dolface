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

enum { PHEX, PDEC, PBIT };

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
    uint32_t o;
    int      t;
    int      v;
} offtype;

#include "structs.h"

int  cmprot(const void*, const void*);
void definit();
int  getrtype(uint32_t*, char*, char*);
void pdatomic(char*, unsigned int, uint8_t*, DatExtra);
void pdstruct(DatStructVar*, uint8_t*);
int  pfbody(char*, char*, char*);
int  pfonly(char*, char*);
int  pfoffs(char*, int);
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
    uint32_t *body, *relt, *root, *strt;
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
    strt = (uint32_t*)(buf+hdr[1]+(hdr[2]*sizeof(uint32_t))+(hdr[3]*ddefs[DatRoot].size));
    for(;root!=strt; root+=2) {
        if(!root[0]) continue;
        ots[j].o = be32toh(root[0]);
        ots[j].t = getrtype(root, file_s, (char*)strt);
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
        pdstruct(ddefs[ots[i].t].vars, buf+ots[i].o);
    }

    free(ots);
    free(buf);
    return 0;
}

int pfoffs(char* file_s, int mode) {
    uint32_t i, j, hdr[8];
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
    if(mode==0) {
        j = hdr[2]*2;
        relt = (uint32_t*)(buf+hdr[1]);
        for(i=0; i<hdr[2]; ++i) {
            ots[i].o = be32toh(relt[i]);
            ots[i].t = -1;
            body = (uint32_t*)(buf+ots[i].o);
            ots[hdr[2]+i].o = be32toh(*body);
            ots[hdr[2]+i].t = DatNULL;
        }
        qsort(ots, hdr[2]*2, sizeof(offtype), cmprot);
    } else {
        i = j = 0;
        body = (uint32_t*)(buf);
        root = (uint32_t*)(buf+hdr[1]+(hdr[2]*sizeof(uint32_t)));
        strt = (uint32_t*)(buf+hdr[1]+(hdr[2]*sizeof(uint32_t))+(hdr[3]*ddefs[DatRoot].size));
        for(;root!=strt; root+=2) {
            if(!root[0]) continue;
            ots[j].o = be32toh(root[0]);
            ots[j].t = getrtype(root, file_s, (char*)strt);
            ++j;
        }
        for(;i<j && j<hdr[2]; ++i) {
            if(ots[i].v) continue;
            printf("%3d %08x %s", i, ots[i].o, ddefs[ots[i].t].name);
            j+=travdv(buf+ots[i].o, ots+j, ddefs[ots[i].t].vars);
            printf("\n");
            ots[i].v=1;
            if((j+1)==hdr[2]) {
                j = sortuot(ots, j);
                i=0;
            }
        }
        if(i<j) printf("INFO: more offsets than in relocation table?\n");
        else j = sortuot(ots, j);
    }

    for(i=0; i<j; ++i) {
        printf("%08x", ots[i].o);
        if(ots[i].t >= 0)
            printf(" %s", ddefs[ots[i].t].name);
        printf("\n");
    }

    free(ots);
    free(buf);
    return 0;
}

int getrtype(uint32_t* root, char* file_s, char* strt) {
    int strl;
    char* name;
    char JointSuf[] = "Share_joint";
    char MatAnimSuf[] = "Share_matanim_joint";
    name = strt+be32toh(root[1]);
    strl = strlen(name);
    if(strcmp(JointSuf, name+(strl-sizeof(JointSuf)+1)) == 0)
        return DatJoint;
    else if(strcmp(MatAnimSuf, name+(strl-sizeof(MatAnimSuf)+1)) == 0)
        return DatMatAnimA;
    else
        return DatNULL;
}

int cmprot(const void* a, const void* b) { 
    return  (((offtype*)a)->o > ((offtype*)b)->o) - 
            (((offtype*)a)->o < ((offtype*)b)->o);
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
    qsort(ots, nmemb, sizeof(offtype), cmprot);
    for(i=0; i<(nmemb-1); ++i) {
        if(cmprot(ots+i, ots+i+1) == 0) {
            ots[i].o = 0xFFFFFFFF;
            ots[i+1].v |= ots[i].v;
            ++dup;
        }
    }
    if(dup==0) return nmemb;
    qsort(ots, nmemb, sizeof(offtype), cmprot);
    return nmemb-dup;
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
    enum { ModeInsp, ModeRoot, ModeHdr, ModeRelt, ModeBody, ModeType, ModeSize };
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
            case 'o': mode = ModeRelt; break;
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
    case ModeRelt: return pfoffs(file_s, 0);
    case ModeBody: return pfoffs(file_s, 1);
    case ModeType: return pfonly(file_s, type_s);
    }

    return 0;
}

