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
 AtTri,
 AtOff,
 AtSub,
 AtLast
};

enum { PDEC, PHEX, PBIT };

enum {
 DatHdr,
 DatRoot,
 DatJoint,
 DatJointData,
 DatMesh,
 DatMaterial,
 DatColor,
 DatNULL,
 DatLast = DatNULL
};

typedef struct {
    char* type;
    uint32_t off;
} DatOffset;

typedef union {
    int i;
    unsigned int ui;
    float f;
    void* v;
} DatExtra;

typedef struct {
    unsigned int type;
    char*        name;
    DatExtra     extra;
} DatStructVar;
typedef struct {
    char* name;
    unsigned int size;
    DatStructVar* vars;
} DatStruct;

#include "structs.h"

void definit();
void pdstruct(unsigned int, uint8_t*);
int  pfbody(char*, char*, char*);
int  pfroot(char*);
int  pfstrings(char*); 
int  pfstruct(char*, uint32_t, uint32_t);
void usage();

void definit() {
    for(DatStruct* dp = ddefs; dp < ddefs + LEN(ddefs); ++dp) {
        if(!dp->vars) continue;
        DatStructVar* dvp = dp->vars;
        while(dvp->type) {
            switch (dvp->type) {
            case AtByte: dp->size += 1; break;
            case AtHalf: dp->size += 2; break;
            default: dp->size += 4; break;
            }
            ++dvp;
        }
    }
}

void pdstruct(unsigned int type, uint8_t* ptr) {
    if(type==DatNULL) return;
    DatStructVar* dvp = ddefs[type].vars;
    while(dvp->type) {
        switch(dvp->type) {
        case AtWord:  printf((dvp->extra.ui == PHEX)?"word  %10x ":"word  %10d ", be32toh(*(uint32_t*)ptr)); ptr+=4; break;
        case AtHalf:  printf((dvp->extra.ui == PHEX)?"half  %10x ":"half  %10d ", be16toh(*(uint16_t*)ptr)); ptr+=2; break;
        case AtByte:  printf((dvp->extra.ui == PHEX)?"byte  %10x ":"byte  %10d ", *(uint8_t*)ptr); ptr+=1; break;
        case AtFloat: printf("float %10f ", be32toh(*(float*)ptr)); ptr+=4; break;
        case AtTri:   printf("float { ");
                  printf("%f ", be32toh(*(float*)ptr)); ptr+=4;
                  printf("%f ", be32toh(*(float*)ptr)); ptr+=4;
                  printf("%f ", be32toh(*(float*)ptr)); ptr+=4;
                  printf("} "); break;
        case AtOff:   printf("off   [%08x] %s ", be32toh(*(uint32_t*)ptr), (dvp->extra.ui==DatNULL)?"Unknown":ddefs[dvp->extra.ui].name); ptr+=4; break;
        //TODO: this is not correct, inline structs have size
        case AtSub: printf("inline struct "); ptr+=4; break; 
        }
        printf("%s\n", dvp->name);
        ++dvp;
    }
}

int main(int argc, char** argv) {
    char* file_s = NULL, *type_s = NULL, *off_s = NULL;
    enum { ModeStrs, ModeBody, ModeRoot, ModeHdr };
    uint32_t mode = ModeBody;
    int i;
    FILE* fp;

    if(argc<2) usage();
    for(i = 1; i < argc; ++i) {
        if(argv[i][0] == '-')
            switch(argv[i][1]) {
            case 'b': mode = ModeBody; type_s = argv[++i]; off_s = argv[++i]; break;
            case 'r': mode = ModeRoot; break;
            case 'h': mode = ModeHdr;  break;
            case 'z': mode = ModeStrs; break;
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
    case ModeStrs: return pfstrings(file_s);
    case ModeBody: return pfbody(file_s, type_s, off_s);
    case ModeRoot: return pfroot(file_s);
    case ModeHdr:  return pfstruct(file_s, DatHdr, 0);
    }

    return 0;
}

int pfbody(char* file_s, char* type_s, char* off_s) {
    uint32_t i, type, off;
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
        pdstruct(DatRoot, buf+(i*ddefs[DatRoot].size));
    printf("\n");
    for(; i < (root+sroot); ++i)
        pdstruct(DatRoot, buf+(i*ddefs[DatRoot].size));

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
    pdstruct(type, buf);

    free(buf);
    fclose(file);
    return 0;
}

void usage() {
    printf(
            "Usage: datstructs -[bhrz] FILE\n"
            "  -b TYPE OFF  prints structure of TYPE at OFF in body\n"
            "  -z           prints string table\n"
            "  -h           prints header\n"
            "  -r           prints roots\n"
          );
    exit(0);
}
