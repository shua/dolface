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
 AtArr,
 AtLast
};
enum { PHEX, PDEC, PBIT, PSIZ, PSGN, PFLG };
enum { SFixed, SList, SCust };

typedef struct {
    uint32_t o;
    int      t;
    int      v;
    int      z;
    uint32_t p;
    int      pt;
} offtype;
const offtype INITIALIZE_OFFTYPE = { 0, 0, 0, 1, 0, 0 };

typedef  union {
        struct {
        uint32_t filesz;
        uint32_t datasz; 
        uint32_t reltnum;
        uint32_t rootnum;
        uint32_t srootnum;
        uint32_t _p[3];
        };
        uint32_t m[8];
} DHdr;
    
typedef struct {
    uint8_t* data;
    offtype* offtypes;
    int dz;
    int otz;
    int otm;
} DData;

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
typedef void (*DatPrintFun)(int, DatStructVar*, uint8_t*, DatStructVar);
typedef int (*DatTravFun)(DatStructVar*, uint8_t*, offtype*, offtype*);
typedef struct {
    char* name;
    unsigned int size;
    DatStructVar* vars;
    DatTravFun trav;
    DatPrintFun print;
} DatStruct;

typedef struct {
    char* pre;
    char* suf;
    int type;
} DatRootFmt;

#include "structs.h"

int  cmpot(const void*, const void*);
void definit();
int  datreadot(DData*, char*);
int  datreadbuf(uint8_t**, char*);
void documentation();
void documentrec(DatStructVar*, int);
int  insot(offtype*, size_t, offtype);

int  pfbody(char*, char*, char*);
int  pfonly(char*, char*);
int  pfoffs(char*);
int  pfroot(char*);
int  pfstruct(char*, uint32_t, uint32_t);
int  sortuot(offtype*, size_t);
void usage();

char* exename;

void 
definit() {
    for(DatStruct* dp = ddefs; dp < ddefs + LEN(ddefs); ++dp) {
        if(!dp->vars) continue;
        DatStructVar* dvp = dp->vars;
        while(dvp->type) {
            switch (dvp->type) {
            case AtByte: dp->size += 1; break;
            case AtHalf: dp->size += 2; break;
            case AtSub:  dp->size += (dvp->extra.sv->type*dvp->extra.sv->extra.ui); break;
            default: dp->size += 4; break;
            }
            ++dvp;
        }
    }
}

int
pfbody(char* file_s, char* type_s, char* off_s) {
    uint32_t i, type = 0, off;
    if(!type_s) {
        fprintf(stderr, "Missing: TYPE\n");
        usage();
    }
    if(!off_s) {
        fprintf(stderr, "Missing: OFF\n");
        usage();
    }

    if(sscanf(off_s, ":%8x:", &off) != 1 && sscanf(off_s, "%x", &off) != 1) return 3;
    for(i=1; i < DatLast; i++) {
        if(!ddefs[i].name) continue;
        if(!strcmp(type_s, ddefs[i].name)) {
            type = i;
            break;
        }
    }
    return pfstruct(file_s, type, off+0x20);
}

offtype* 
lfind(offtype* key, offtype* base, size_t _s, size_t size, int(*compar)(const void*, const void*)) {
    int i;
    for(i=0; i<size; ++i) {
        if(!compar(key, base+i)) return base+i;
    }
    return 0;
}

offtype* 
otbsearch(offtype* key, offtype* base, size_t _s, size_t size, int(*compar)(const void*, const void*)) {
    int i;
    while(size) {
        i=(size/2);
        switch(compar(key, base+i)) {
        case -1: size=i; break;
        case  0: return base+i;
        case  1: base+=(i+1);size-=(i+1); break;
        }
    }
    return 0;
}

int
datreadbuf(uint8_t **dst, char* file_s) {
    uint32_t i, *hdr;
    uint8_t *buf;
    FILE *file;

    file = fopen(file_s, "r");
    if(!file) return 2;

    hdr = calloc(sizeof(uint32_t), 8);
    fread(hdr, sizeof(uint32_t), 8, file);
    for(i=0; i<5; ++i) hdr[i] = be32toh(hdr[i]);
    hdr = realloc(hdr, hdr[0]+1);
    buf = (uint8_t*)hdr;
    fread(buf+0x20, hdr[0]-0x20, 1, file);
    fclose(file);

    *dst = buf;
    return 0;
}

int 
datreadot(DData *ddata, char* file_s) {
    uint32_t i, j, z, u, t;
    uint32_t *relt, *root, *strt;
    uint8_t *buf, *body;
    offtype *ots, *match;
    DHdr *hdr;

    if(ddata->dz==0 && datreadbuf(&(ddata->data), file_s)) return 1;
    hdr = (DHdr*)(buf = ddata->data);
    z = 2*hdr->reltnum;
    ots = calloc(z, sizeof(offtype));
    body = buf+0x20;

    /* construct base array from reloc table */
    relt = (uint32_t*)(body+hdr->datasz);
    for(i=0; i<hdr->reltnum; ++i) {
        ots[i] = INITIALIZE_OFFTYPE;
        j = *(uint32_t*)(body+be32toh(relt[i]));
        /* reposition all offsets to offset+0x20, so it can be accessed with buf+offset, 
         * and checking offset==0 will return false for real offsets 
         * (including those pointing to the position 0x00000000) */
        /* it's a workaround for all offsets pointing to whatever data struct 
         * starts the file (at body[0]). */
        ots[i].o = be32toh(j)+0x20;
        *(uint32_t*)(body+be32toh(relt[i])) = be32toh(ots[i].o);
        ots[i].t = DatNULL;
    }

    root = (uint32_t*)(body+hdr->datasz+(hdr->reltnum*sizeof(uint32_t)));
    strt = (uint32_t*)(body+hdr->datasz+(hdr->reltnum*sizeof(uint32_t))+((hdr->rootnum+hdr->srootnum)*ddefs[DatRoot].size));
    for(j=i+hdr->rootnum+hdr->srootnum; root!=strt; root+=2) {
        if(!root[0]) continue;
        ots[i] = INITIALIZE_OFFTYPE;
        ots[i].o = be32toh(*root)+0x20;
        ots[i].t = ROOTTYPE(root, (char*)strt).type;
        ots[i].v = 0;
        /* we have to add the root offsets twice, because the structs they point to 
         * might not be pointed to by anything else in the body, and thus wouldn't 
         * show up in the relocation table scan */
        /* this way, after the list is sorted and duplicates removed, the second list 
         * of roots can be copied to the end, and we can start the main loop there */
        ots[j++] = ots[i++];
    }
    /* u becomes the end of the unknowns, and the sorted array */
    u = sortuot(ots, i);
    j -= hdr->rootnum + hdr->srootnum;
    if((otbsearch(ots+2, ots, sizeof(offtype), u, cmpot)) != ots+2) printf("not sorted correctly\n");
    for(i=u; i < (u + hdr->rootnum + hdr->srootnum);)
        ots[i++] = ots[j++];
    j = i;

    /* start i at the beginning of the unsorted/unvisited offtypes */
    for(i=u;i<j && j<z; ++i) {
        if(ots[i].t == DatNULL) {
            ots[i--] = ots[--j];
            continue;
        }

        match = otbsearch(ots+i, ots, sizeof(offtype), u, cmpot);
        if(match != NULL) {
            if(match->t != DatNULL && match->t != ots[i].t) {
                fprintf(stderr, "%08x %-15s is double-defined: %s (%8x %s, %8x %s)\n", 
                        match->o-0x20, ddefs[match->t].name, ddefs[ots[i].t].name, 
                        match->p-0x20, ddefs[match->pt].name, ots[i].p-0x20, ddefs[ots[i].pt].name);
                continue;
            } else {
                t = match->v;
                *match = ots[i];
                match->v = (t==1)?1:match->v;
                ots[i--] = ots[--j];
                if(match->v==1) continue;
            }
        } else {
            fprintf(stderr, "%08x %-15s weird offset not found in relocation table or root nodes: (%8x %s)\n", 
                    ots[i].o-0x20, ddefs[ots[i].t].name, ots[i].p-0x20, ddefs[ots[i].pt].name);
            match = ots+i;
        }

        if(match->o-0x20 > hdr->datasz) {
            fprintf(stderr, "%08x %-15s offset exceeds datasz %8x (%8x %s)\n", 
                    match->o-0x20, ddefs[match->t].name, hdr->datasz, match->p-0x20, ddefs[match->pt].name);
            match->v=1;
            continue;
        }
#ifdef DEBUG
        printf("%-11s %8x %8x %d", ddefs[match->t].name, match->o-0x20, match->p-0x20, match->z);
#endif
        while(match->v!=1) {
            /* use offtype.v as negative counter for array */
            t=j+ddefs[match->t].trav(
                    ddefs[match->t].vars,
                    buf+match->o + (-match->v)*ddefs[match->t].size, 
                    ots+j,
                    match);
#ifdef DEBUG
            while(j<t) { 
                if(match->o % 4) 
                    fprintf(stderr, "%08x %-15s offset isn't aligned: (%8x %s)\n", 
                            match->o-0x20, ddefs[match->t].name, match->p-0x20, ddefs[match->pt].name);
                ots[j].p  = match->o;
                ots[j].pt = match->t;
                ++j;
            }
#else
            j=t;
#endif
            ++(match->v);
        
            if(j+16>=z) {
                printf("warning: too many cooks!\n");
                match=realloc(ots, z*2);
                if(match) {
                    ots = match;
                    z*=2;
                } else {
                    fprintf(stderr, "Realloc fail\n");
                    z=j;
                    match->v=1;
                }
            }
        }
#ifdef DEBUG
        printf("\n");
#endif

    }
    /* last unique sort, not really necessary because you can "| sort -u", but what reason is there to leave the output unsorted? */
    u = sortuot(ots, j);
    
    for(i=0; i<hdr->reltnum; ++i) {
        j = *(uint32_t*)(body+be32toh(relt[i]));
        j = be32toh(j)-0x20;
        *(uint32_t*)(body+be32toh(relt[i])) = be32toh(j);
    }
    for(i=0; i<u; ++i) {
        ots[i].o -= 0x20;
    }

    *ddata = (DData){ buf, ots, hdr->filesz+1, z, u };
    return 0;
}

int 
cmpot(const void* a, const void* b) { 
    return  (((offtype*)a)->o > ((offtype*)b)->o) - 
        (((offtype*)a)->o < ((offtype*)b)->o);
}

int
sortuot(offtype* ots, size_t nmemb) {
    int dup = 0, i;
    qsort(ots, nmemb, sizeof(offtype), cmpot);
    for(i=0; i<(nmemb-1); ++i) {
        if(cmpot(ots+i, ots+i+1) == 0) {
            ots[i].o = 0xFFFFFFFF;
            ots[i+1].v |= ots[i].v;
            if(ots[i+1].t==DatNULL) 
                ots[i+1].t = ots[i].t;
            ++dup;
        }
    }
    if(dup==0) return nmemb;
    qsort(ots, nmemb, sizeof(offtype), cmpot);
    return nmemb-dup;
}

int 
pfonly(char* file_s, char* type_s) {
    uint32_t i, type;
    DData ddata = { 0 };

    if(type_s) {
        for(i=1; i < LEN(ddefs); i++) {
            if(!strcmp(type_s, ddefs[i].name)) {
                type = i;
                break;
            }
        }
    } else {
        type = DatLast;
    }

    if(datreadot(&ddata, file_s)) return 1;

    for(i=0; i<ddata.otm; ++i) {
        if(type != DatLast && type != ddata.offtypes[i].t) continue;
        printf("-- %08x %s",
            ddata.offtypes[i].o,
            ddefs[ddata.offtypes[i].t].name
        );
        if(ddata.offtypes[i].z > 1) {
            if(ddefs[ddata.offtypes[i].t].size) {
                printf("[0:%d]", ddata.offtypes[i].z);
            } else {
                printf("[%d]", ddata.offtypes[i].z);
                ddata.offtypes[i].z = 1;
            }
        }
        printf(" --\n");
        ddefs[ddata.offtypes[i].t].print(
            0,
            ddefs[ddata.offtypes[i].t].vars, 
            ddata.data + 0x20 + ddata.offtypes[i].o, 
            (DatStructVar){ 0 }
        );
        for(ddata.offtypes[i].v = 1; ddata.offtypes[i].v < ddata.offtypes[i].z; ++(ddata.offtypes[i].v)) {
            printf("-- %08x %s[%d:%d] --\n", 
                   ddata.offtypes[i].o,
                   ddefs[ddata.offtypes[i].t].name,
                   ddata.offtypes[i].v,
                   ddata.offtypes[i].z
            );
            ddefs[ddata.offtypes[i].t].print(
                0,
                ddefs[ddata.offtypes[i].t].vars, 
                ddata.data + 0x20 + ddata.offtypes[i].o + (ddata.offtypes[i].v*ddefs[ddata.offtypes[i].t].size), 
                (DatStructVar){ 0 }
            );
        }
    }

    free(ddata.offtypes);
    free(ddata.data);
    return 0;
}

int
pfoffs(char* file_s) {
    int i;
    DData ddata = { 0 };

    if(datreadot(&ddata, file_s)) return 1;

    for(i=0; i<ddata.otm; ++i) {
        if(ddata.offtypes[i].t < 0) continue;
        if(ddata.offtypes[i].z > 1 && ddefs[ddata.offtypes[i].t].size > 0x8) 
            /* this is arbitrary, but if the struct is only 2 words big, 
             * I didn't need it to tell me where the beginning of each struct was in the array */
            for(ddata.offtypes[i].v=0; ddata.offtypes[i].v < ddata.offtypes[i].z; ++(ddata.offtypes[i].v))
                printf("%08x %s[%d:%d]\n", 
                    ddata.offtypes[i].o + ddata.offtypes[i].v*ddefs[ddata.offtypes[i].t].size,
                    ddefs[ddata.offtypes[i].t].name,
                    ddata.offtypes[i].v,
                    ddata.offtypes[i].z);
        else {
            printf("%08x %s", ddata.offtypes[i].o, ddefs[ddata.offtypes[i].t].name);
            if(ddata.offtypes[i].z > 1)
                printf("[%d]", ddata.offtypes[i].z);
            printf("\n");
        }
    }

    free(ddata.offtypes);
    free(ddata.data);
    return 0;
}

int 
pfroot(char* file_s) {
    uint32_t i, hdr[8], root;
    char* strt;
    uint8_t* buf;
    FILE* file;

    file = fopen(file_s, "r");
    if(!file) return 2;

    fread(hdr, sizeof(uint32_t), 8, file);
    for(i=0; i<5; ++i) hdr[i] = be32toh(hdr[i]);
    root = hdr[1]+(hdr[2]*sizeof(uint32_t));
    fseek(file, 0x20+root, SEEK_SET);
    buf = malloc((hdr[3]+hdr[4])*ddefs[DatRoot].size);
    fread(buf, ddefs[DatRoot].size, hdr[3]+hdr[4], file);
    hdr[5] = hdr[0] - (root+((hdr[3]+hdr[4])*ddefs[DatRoot].size));
    strt = malloc(hdr[5]);
    fread(strt, hdr[5], 1, file);

    if(hdr[3]) {
        printf("-- %08x %s[%d] --\n", root+i*ddefs[DatRoot].size, ddefs[DatRoot].name, hdr[3]);
        ddefs[DatRoot].print(0, ddefs[DatRoot].vars, buf, (DatStructVar){ DatHdr, strt, hdr[3] } );
    }
    if(hdr[4]) {
        printf("-- %08x %s[%d] --\n", root+i*ddefs[DatRoot].size, ddefs[DatRoot].name, hdr[4]);
        ddefs[DatRoot].print(0, ddefs[DatRoot].vars, buf+(hdr[3]*ddefs[DatRoot].size), (DatStructVar){ DatHdr, strt, hdr[4] } );
    }

    free(buf);
    fclose(file);
    return 0;
}

int 
pfstruct(char* file_s, uint32_t type, uint32_t off) {
    FILE* file;
    uint8_t* buf;
    if(type==DatNULL) return 3;
    file = fopen(file_s, "r");
    if(!file) return 2;
    buf = malloc(ddefs[type].size);
    fseek(file, off, SEEK_SET);
    fread(buf, ddefs[type].size, 1, file);
    ddefs[type].print(0, ddefs[type].vars, buf, (DatStructVar){ 0 });

    free(buf);
    fclose(file);
    return 0;
}

void
documentation() {
    int i;
    for(i=1; i<DatLast; ++i) { 
        if(!ddefs[i].vars) continue;
        printf("%s {\n", ddefs[i].name);
        documentrec(ddefs[i].vars, 1);
        printf("}\n");
    }
}

void
documentrec(DatStructVar* dvp, int lvl) {
    int i, j;
    char* atname[] = {
        [AtByte]  = "byte ",
        [AtHalf]  = "half ",
        [AtWord]  = "word ",
        [AtFloat] = "float",
        [AtOff]   = "off  ",
        [AtArr]   = "array",
        [AtSub]   = "sub  "
    };
    for(i=0; dvp->type; ++dvp) {
        for(j=0; j<lvl; ++j) printf("    ");
        printf("%s ", atname[dvp->type]);
        
        if(dvp->name) 
            printf("\"%s\" ", dvp->name);
        else
            printf("\"unknown%02x\" ", i);
        
        switch(dvp->type) {
        case AtOff:
        case AtArr:
            printf("(%s) ", ddefs[dvp->extra.ui].name); break;
        case AtSub:
            if(dvp->extra.sv->extra.ui > 1)
                printf("[%d]", dvp->extra.sv->extra.ui);
            printf(" {\n");
            documentrec(dvp->extra.sv+1, lvl+1);
            for(j=0; j<lvl; ++j) printf("    ");
            printf("}");
        default: break;
        }

        printf("\n");

        switch(dvp->type) {
        case AtByte: i+=1; break;
        case AtHalf: i+=2; break;
        case AtSub:  i+=dvp->extra.sv->type*dvp->extra.sv->extra.ui; break;
        default:     i+=4; break;
        }
    }
}

void 
usage() {
    printf(
    "Usage: %s -[ihrobtz] FILE\n"
    "  -i OFF TYPE  prints struct of TYPE at OFF in body\n"
    "  -h           prints header\n"
    "  -r           prints roots\n"
    "  -o           prints struct offsets discovered by relocation table\n"
    "  -b           prints struct offsets discovered by known node types\n"
    "  -t TYPE      prints all structs of TYPE (if TYPE is invalid or omitted, prints all structs)\n"
    "  -z           prints structure sizes\n",
    exename);
    exit(0);
}

int 
main(int argc, char** argv) {
    char* file_s = NULL, *type_s = NULL, *off_s = NULL;
    enum { ModeInsp, ModeRoot, ModeHdr, ModeBody, ModeType, ModeSize, ModeDoc };
    uint32_t mode = ModeBody;
    int i;

    for(i = strlen(argv[0])-1; i>=0 && argv[0][i] != '/'; --i);
    exename = &(argv[0][i+1]);
    
    definit();

    if(argc<2) usage();
    for(i = 1; i < argc; ++i) {
        if(argv[i][0] == '-')
            switch(argv[i][1]) {
            case 'i': mode = ModeInsp; off_s = argv[++i]; type_s = argv[++i]; break;
            case 'h': mode = ModeHdr;  break;
            case 'r': mode = ModeRoot; break;
            case 'b': mode = ModeBody; break;
            case 't': mode = ModeType; type_s = (argc==++i)?NULL:argv[i]; break;
            case 'z': for(i=1; i<LEN(ddefs); ++i) printf("%s %x\n", ddefs[i].name, ddefs[i].size); return 0;
            case 'd': documentation(); return 0;
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

