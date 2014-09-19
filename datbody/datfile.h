#ifndef DATFILE_H
#define DATFILE_H

#define DECLARE(type) \
struct type##_t; \
typedef struct type##_t type; \
extern const int type##bufsize; \
void* serialize##type(void*, const type*); \
type* deserialize##type(type*, void*); \
void print##type(type*); \
int fread##type(type*, FILE*); \
int fwrite##type(const type*, FILE*); 

DECLARE(DatHeader)
DECLARE(RootNode)
DECLARE(Joint)
DECLARE(JointData)
DECLARE(Mesh)
DECLARE(Collision)
DECLARE(Material)
DECLARE(Color)
DECLARE(Texture)
DECLARE(Image)
DECLARE(Palette)
DECLARE(VertexAttr)
DECLARE(DisplayList)

#ifdef DATFILE_IMPLEMENT
#include<stdio.h>
#include<stdint.h>

typedef uint32_t dword;
typedef uint16_t dhalf;
typedef uint8_t  dbyte;
typedef union {
    struct {
        float x;
        float y;
        float z;
    };
    float m[3];
} floatxyz;

#define FORIDX(num) for(int _i=0; _i<(num); ++_i)

#define DEF_field(type, name)       type name;
#define DEF_array(type, name, num)  type name[num];
#define DEF_offset(type, name)      dword _##name##off;

#define PTR_field(type, name)
#define PTR_array(type, name, num)
#define PTR_offset(type, name)      type* name;

#define SER_field(type, name)       type* name##ptr = (type*)cbuf; *name##ptr = swap##type(src->name); cbuf+=sizeof(type);
#define SER_array(type, name, num)  \
    type* name##ptr; \
    FORIDX(num) { \
        name##ptr = (type*)cbuf; \
        *name##ptr = swap##type(src->name[_i]); \
        cbuf+=sizeof(type); \
    }
#define SER_offset(type, name) \
    dword* name##ptr = (dword*)cbuf; \
    *name##ptr = swapdword(*(dword*)&(src->_##name##off)); \
    cbuf+=sizeof(dword);

#define DES_field(type, name)       dst->name = swap##type(*((type*)cbuf)); cbuf+=sizeof(type);
#define DES_array(type, name, num)  FORIDX(num) { dst->name[_i] = swap##type(*((type*)cbuf)); cbuf += sizeof(type); }
#define DES_offset(type, name) \
    dword name##tmp = swapdword(*((dword*)cbuf)); \
    dst->_##name##off = (void*)name##tmp; \
    cbuf+=sizeof(dword);

#define PRNT_field(type, name)      printf("%10s %10s %8x " FMT_##type "\n", #type, #name, src->name, src->name);
#define PRNT_array(type, name, num) \
    printf("%10s %10s %d {", #type, #name, num); \
    FORIDX(num-1) { \
        printf(FMT_##type ", ", src->name[_i]); \
    } \
    printf(FMT_##type " }\n", src->name[num-1]);
#define PRNT_offset(type, name)     printf("%10s %10s %8x\n", #type, #name, (unsigned int)src->_##name##off);

#define FMT_dword "%9d"
#define FMT_dhalf "%5d"
#define FMT_dbyte "%3d"
#define FMT_float "%f"
#define FMT_floatxyz

#define READ_field(type, name)
#define READ_array(type, name, num)
#define READ_offset(type, name)     \
    if(dst->_##name##off) { \
        dst->name = malloc(sizeof(type)); \
        fseek(dst->_##name##off + 0x20, "SEEK_SET"); \
        dst->name->_offset = dst->_##name##off; \
        fread##type(dst->name, fp); \
    }

#define RECF_field(type, name)
#define RECF_array(type, name, num)
#define RECF_offset(type, name)     _recfun##type(src->name, recf); recf(src->name);

#define DEFINE(type) \
struct type##_t { \
    X##type(XDEF) \
    dword _offset; \
    X##type(XPTR) \
}; \
const int type##bufsize = offsetof(struct type##_t, _offset);

#define SERIALIZE(type) \
void* serialize##type(void* dst, const type* src) { \
    void* cbuf = dst; \
    X##type(XSER) \
    return dst; \
}

#define DESERIALIZE(type) \
type* deserialize##type(type* dst, void* src) { \
    void* cbuf = src; \
    X##type(XDES) \
    return dst; \
}

#define PRINT(type) \
void print##type(type* src) { \
    printf(#type "\n"); \
    X##type(XPRNT) \
}

#define READ(type) \
int fread##type(type* dst, FILE* fp) { \
    char cbuf[type##bufsize]; \
    if(fread(cbuf, type##bufsize, 1, fp) != 1) { \
        fprintf(stderr, "Error reading " #type " into buffer\n"); \
        return 0; \
    } \
    deserialize##type(dst, cbuf); \
    X##type(XREAD) \
    return 1; \
}

#define WRITE(type) \
int fwrite##type(const type* src, FILE* fp) { \
    X##type(XWRIT) \
}
// Probably just going to serialize and write buffer

#define PTR_UTIL(type) \
void _recfun##type(const type* src, void (*fun)(void*)) { X##type(XRECF) } \
dword countoffsets##type(const type* src) { return 0+ X##type(XCOFF) 0; } \
dword layoutoffsets##type(const type* src, dword initial, ) { \
    \
}

#define XDEF(x) DEF_##x
#define XPTR(x) PTR_##x
#define XSER(x) SER_##x
#define XDES(x) DES_##x
#define XPRNT(x) PRNT_##x
#define XREAD(x) READ_##x
#define XWRIT(x)
#define XRECF(x) RECF_##x

#define swap32(x) ( \
    ((x) & 0xff000000) >> 24 | ((x) & 0xff0000) >> 8 | \
    ((x) & 0xff00) << 8      | ((x) & 0xff) << 24 \
)
#define swap16(x) (((x) & 0xff) << 8 | ((x) & 0xff00) >> 8)

#define swaparray(type, num, dst, src) FORIDX(num) { ((type*)dst)[_i] = swap##type(((type*)src)[_i]); }
static inline dword swapdword(dword src) { return swap32(src); }
static inline dhalf swapdhalf(dhalf src) { return swap16(src); }
static inline dbyte swapdbyte(dbyte src) { return src; }
static inline float swapfloat(float src) { uint32_t tmp = *((uint32_t*)&src); tmp = swap32(tmp); return *((float*)&tmp); }
static inline floatxyz swapfloatxyz(floatxyz src) { floatxyz tmp; swaparray(float, 3, tmp.m, src.m) return tmp; }

// -- DatHeader --

#define XDatHeader(_) \
_(field)    (dword, filesz) \
_(field)    (dword, datasz) \
_(field)    (dword, reltnum) \
_(field)    (dword, rootnum) \
_(field)    (dword, srootnum) \
_(field)    (dword, unknown14) \
_(field)    (dword, unknown18) \
_(field)    (dword, unknown1c)

typedef struct DatHeader_t {
    dword filesz;
    dword datasz;
    dword reltnum;
    dword rootnum;
    dword srootnum;
    dword unknown14;
    dword unknown18;
    dword unknown1c;
    RootNode* rootnodes;
    RootNode* srootnodes;
    dword*  relocationtable;
    char* stringtable;
} DatHeader;
const int DatHeaderbufsize = 0x20;

SERIALIZE(DatHeader)

DESERIALIZE(DatHeader)

PRINT(DatHeader)

int fread(DatHeader* dst, FILE* fp) {
    fseek(0, "SEEK_SET");
    if(fread(dst, DatHeaderbufsize, 1, fp) != 1) {
        fprintf(stderr, "Error reading DatHeader into buffer\n");
        return 0;
    }
    deserializeDatHeader(dst, dst);
    dst->rootnodes = malloc(sizeof(RootNode) * dst->rootnum);
    dst->srootnodes = malloc(sizeof(RootNode) * dst->srootnum);
    dst->relocationtable = malloc(sizeof(dword) * dst->reltnum);
    dst->stringtable = malloc(dst->filesz - (
            dst->datasz + (dst->rootnum + dst->srootnum) * RootNodebufsize +
            dst->reltnum * sizeof(dword)));
    fseek(dst->datasz, "SEEK_CUR");


    error:
}

// -- RootNode --

#define XRootNode(_) \
_(offset)   (void, child) \
_(field)    (dword, name)

struct RootNode_t {
    union {
        void*       child;
        Joint*      joint;
        Mesh*       mesh;
        Collision*  collision;
    };
    dword name;
};

SERIALIZE(RootNode)

DESERIALIZE(RootNode)

// TODO custom print, that prints correct child type and name
PRINT(RootNode)

// -- Joint --

#define XJoint(_) \
_(field)    (dword, unknown) \
_(field)    (dword, flags) \
_(offset)   (Joint, child) \
_(offset)   (Joint, next) \
_(offset)   (JointData, data) \
_(field)    (floatxyz, rotation) \
_(field)    (floatxyz, scale) \
_(field)    (floatxyz, translation) \
_(offset)   (void, transform) \
_(field)    (dword, padding)

DEFINE(Joint)

SERIALIZE(Joint)

DESERIALIZE(Joint)

PRINT(Joint)

// -- JointData --

#define XJointData(_) \
_(field)    (dword, unknown) \
_(offset)   (JointData, next) \
_(offset)   (Material, material) \
_(offset)   (Mesh, mesh)

DEFINE(JointData)

SERIALIZE(JointData)

DESERIALIZE(JointData)

PRINT(JointData)

// -- Mesh --

#define XMesh(_) \
_(field)    (dword, unknown) \
_(offset)   (Mesh, next) \
_(offset)   (void, verts) \
_(field)    (dhalf, flags) \
_(field)    (dhalf, displaynum) \
_(offset)   (DisplayList, display) \
_(offset)   (void, weight)

DEFINE(Mesh)

SERIALIZE(Mesh)

DESERIALIZE(Mesh)

PRINT(Mesh)

// -- Material --

#define XMaterial(_) \
_(field)    (dword, unknown) \
_(field)    (dword, flags) \
_(offset)   (Texture, texture) \
_(offset)   (Color, color) \
_(field)    (dword, unknown10) \
_(field)    (dword, unknown14)

DEFINE(Material)

SERIALIZE(Material)

DESERIALIZE(Material)

PRINT(Material)

// -- Texture --

#define XTexture(_) \
_(field)    (dword, unknown) \
_(offset)   (Texture, next) \
_(field)    (dword, unknown08) \
_(field)    (dword, unknown0c) \
_(field)    (floatxyz, rotation) \
_(field)    (floatxyz, scale) \
_(field)    (floatxyz, translation) \
_(field)    (dword, wraps) \
_(field)    (dword, wrapt) \
_(field)    (dbyte, scales) \
_(field)    (dbyte, scalet) \
_(field)    (dhalf, unknown3e) \
_(field)    (dword, unknown40) \
_(field)    (dword, unknown44) \
_(field)    (dword, unknown48) \
_(offset)   (Image, image) \
_(offset)   (Palette, palette) \
_(field)    (dword, unknown54) \
_(field)    (dword, unknown58) \

DEFINE(Texture)

SERIALIZE(Texture)

DESERIALIZE(Texture)

PRINT(Texture)

// -- Color --

#define XColor(_) \
_(field)    (dword, diffuse) \
_(field)    (dword, ambient) \
_(field)    (dword, specular) \
_(field)    (float, unknown0c) \
_(field)    (float, unknown10)

DEFINE(Color)

SERIALIZE(Color)

DESERIALIZE(Color)

PRINT(Color)

// -- Image --

#define XImage(_) \
_(offset)   (void, data) \
_(field)    (dhalf, width) \
_(field)    (dhalf, height) \
_(field)    (dword, format)

DEFINE(Image)

SERIALIZE(Image)

DESERIALIZE(Image)

PRINT(Image)

// -- Palette --

#define XPalette(_) \
_(offset)   (void, data) \
_(field)    (dword, format) \
_(field)    (dword, unknown08) \
_(field)    (dhalf, colornum) \
_(field)    (dhalf, unknown0e)

DEFINE(Palette)

SERIALIZE(Palette)

DESERIALIZE(Palette)

PRINT(Palette)

#endif // DATFILE_IMPLEMENT
#endif // DATFILE_H
