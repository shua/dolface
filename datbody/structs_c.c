#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <endian.h>

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

char* type_s[] = {
    [AtWord]  = "uint32_t ",
    [AtHalf]  = "uint16_t ",
    [AtByte]  = "uint8_t  ",
    [AtFloat] = "float    ",
    [AtArr]   = "datarr   ",
    [AtOff]   = "datoff   ",
    [AtSub]   = ""
};

uint32_t type_z[] = {
    [AtWord]  = 4,
    [AtHalf]  = 2,
    [AtByte]  = 1,
    [AtFloat] = 4,
    [AtArr]   = 4,
    [AtOff]   = 4,
    [AtSub]   = 0
};

char* sub_s[] = {
    "datxyz",
    "datindtab",
    "dathnh",
    "datst_w",
    "datst_b"
};

void
c_fmt() {
    int i,o,so;
    DatStructVar* dvp,* sdvp;

    xyzsub[0].name = sub_s[0];
    cnsub[0].name = sub_s[1];
    grsub[0].name = sub_s[2];
    grasub[0].name = sub_s[2];
    stwsub[0].name = sub_s[3];
    stbsub[0].name = sub_s[4];

    printf("typedef uint32_t datoff;\n");
    printf("typedef uint32_t datarr;\n");
    printf("\n");
    printf( "typedef struct {\n"
            "    float x;\n"
            "    float y:\n"
            "    float z;\n"
            "} %s\n\n", sub_s[0]);
    printf( "typedef struct {\n"
            "    %s start;\n"
            "    %s nuw;\n"
            "} %s;\n\n", type_s[AtHalf], type_s[AtHalf], sub_s[1]);
    printf( "typedef struct {\n"
            "    %s unknown_00;\n"
            "    %s unknown_02;\n"
            ") %s;\n\n", type_s[AtHalf], type_s[AtHalf], sub_s[2]);
    printf( "typedef struct {\n"
            "    %s s;\n"
            "    %s t;\n"
            "} %s;\n\n", type_s[AtWord], type_s[AtWord], sub_s[3]);
    printf( "typedef struct {\n"
            "    %s s;\n"
            "    %s t;\n"
            "} %s;\n\n", type_s[AtByte], type_s[AtByte], sub_s[4]);

    for(i=0; i<LEN(ddefs); ++i) {
        dvp = ddefs[i].vars;
        if(dvp == 0) continue;
        printf("typedef struct {\n");
        for(o=0; dvp->type; ++dvp) {
            if((dvp->type == AtOff || dvp->type == AtArr) && dvp->extra.ui != DatNULL) {
                printf("    dat%s* ", ddefs[dvp->extra.ui].name);
            } else
                printf("    %s", type_s[dvp->type]);

            if(dvp->type != AtSub) goto notsub;
            sdvp = dvp->extra.sv;
            if(sdvp->name) {
                printf("%s ", sdvp->name);
                goto notsub;
            }
            printf("struct {\n");
            if(sdvp == 0) { printf("    };"); continue; }
            ++sdvp;
            for(so=0; sdvp->type; so+=type_z[sdvp->type], ++sdvp) {
                printf("        %s ", type_s[sdvp->type]);
                if(sdvp->name) {
                    if(sdvp->name[0] >= '0' && sdvp->name[0] <= '9')
                        putc('N', stdout);
                    printf("%s;\n", sdvp->name);
                } else
                    printf("unknown_%02x;\n", so);
            }
            printf("    }");
notsub:
            if(dvp->name) {
                if(dvp->name[0] >= '0' && dvp->name[0] <= '9')
                    putc('N', stdout);
                printf("%s", dvp->name);
            } else
                printf("unknown_%02x", o);
            o += type_z[dvp->type];
            
            if(dvp->type != AtSub) goto newline;
            sdvp = dvp->extra.sv;
            if(sdvp->extra.ui > 1)
                printf("[%d]", sdvp->extra.ui);
            o += sdvp->type * sdvp->extra.ui;
newline:
            printf(";\n");
        }
        printf("} dat%s;\n\n", ddefs[i].name);
    }
}

char* r2_sub_s [] = {
    "dat_xyz",
    "dat_indtab",
    "dat_hnh",
    "dat_st_w",
    "dat_st_b"
};

char r2_atfmt [] = {
    [AtWord]  = 'x',
    [AtHalf]  = 'w',
    [AtByte]  = 'b',
    [AtFloat] = 'f',
    [AtOff]   = 'p',
    [AtArr]   = 'p',
    [AtSub]   = '?'
};

void
r2_fmt() {
    int i,o,so;
    DatStructVar* dvp,* sdvp;

    xyzsub[0].name = r2_sub_s[0];
    cnsub[0].name = r2_sub_s[1];
    grsub[0].name = r2_sub_s[2];
    grasub[0].name = r2_sub_s[2];
    stwsub[0].name = r2_sub_s[3];
    stbsub[0].name = r2_sub_s[3];

    printf("e asm.arch=ppc\n");
    printf("e cfg.bigendian=true\n");
    printf("pf.dat_xyz fff x y z\n");
    printf("pf.dat_indtab ww start num\n");
    printf("pf.dat_hnh ww\n");
    printf("pf.dat_st_w xx s t\n");
    printf("pf.dat_st_b bb s t\n");

    for(i=0; i<LEN(ddefs); ++i) {
        if(ddefs[i].vars == 0) continue;
        printf("pf.dat%s ", ddefs[i].name);
        for(dvp=ddefs[i].vars; dvp->type; ++dvp) {
            if(dvp->type == AtSub && dvp->extra.sv && dvp->extra.sv->extra.ui > 1)
                printf("[%d]", dvp->extra.sv->extra.ui);
            putc(r2_atfmt[dvp->type], stdout);
        }

        o=0;
        for(dvp=ddefs[i].vars; dvp->type; ++dvp) {
            putc(' ', stdout);
            if(dvp->type == AtSub && dvp->extra.sv)
                printf("(%s)", dvp->extra.sv->name);

            if(dvp->name)
                printf("%s", dvp->name);
            else
                printf("unk%02x", o);

            if(dvp->type == AtSub && dvp->extra.sv)
                o += dvp->extra.sv->type * dvp->extra.sv->extra.ui;
            else
                o += type_z[dvp->type];
        }
        putc('\n', stdout);
    }
}

int
main(int argc, char** argv) {
    if(argc>1) c_fmt();
    else r2_fmt();
    return 0;
}
