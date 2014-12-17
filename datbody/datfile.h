#ifndef DATFILE_H
#define DATFILE_H

#define DECLARE(type) \
struct type##_t; \
struct type##_bt; \
typedef struct type##_t type; \
extern const int type##bufsize; \
void* serialize##type(void*, const type*); \
type* deserialize##type(type*, void*); \
void print##type(type*); \
int read##type(type*, FILE*); \
dword write##type(const type*, FILE*); 

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

#define perr(x, ...) fprintf(stderr, "Error: " x ": %s:%u\n", ##__VA_ARGS__, __FILE__, __LINE__)

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
#define DEF_offset(type, name)      type* name;

#define DEFB_field(type, name)      DEF_field(type, name)
#define DEFB_array(type, name, num) DEF_array(type, name, num)
#define DEFB_offset(type, name)     dword name;

#define SER_field(type, name)       buf->name = swap##type(src->name);
#define SER_array(type, name, num)  FORIDX(num) { buf->name[_i] = swap##type(src->name[_i]); }
#define SER_offset(type, name)

#define DES_field(type, name)       dst->name = swap##type(buf->name);
#define DES_array(type, name, num)  FORIDX(num) { dst->name[_i] = swap##type(buf->name[_i]); }
#define DES_offset(type, name)      dst->name = (type*)swapdword(buf->name);

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
    if(dst->name) { \
        dword offset = (dword)dst->name; \
        dst->name = malloc(sizeof(type)); \
        fseek(fp, offset + DatHeaderbufsize, SEEK_SET); \
        if(!read##type(dst->name, fp)) { \
            free(dst->name); \
            dst->name = NULL; \
        } \
    }

#define WRIT_field(type, name)
#define WRIT_array(type, name, num)
#define WRIT_offset(type, name) \
    if(src->name) { \
        buf->name = write##type(src->name, fp, header); \
        _storeoffset(header, bufoff+offsetof(buf, buf->name)); \
    }

#define DSTR_field(type, name)
#define DSTR_array(type, name, num)
#define DSTR_offset(type, name) \
    if(ptr->name) { \
        destroy##type(ptr->name); \
        free(ptr->name); \
    }
static inline void destroyvoid(void* ptr) {}

#define DEFINE(type) \
struct type##_t { \
    X##type(XDEF) \
}; \
struct type##_bt { \
    X##type(XDEFB) \
};\
const int type##bufsize = sizeof(type##_bt);

#define SERIALIZE(type) \
void* serialize##type(void* dst, const type* src) { \
    struct type##_bt* buf = dst; \
    X##type(XSER) \
    return dst; \
}

#define DESERIALIZE(type) \
type* deserialize##type(type* dst, void* src) { \
    struct type##_bt* = src; \
    X##type(XDES) \
    return dst; \
}

#define PRINT(type) \
void print##type(type* src) { \
    printf(#type "\n"); \
    X##type(XPRNT) \
}

#define READ(type) \
int read##type(type* dst, FILE* fp) { \
    struct type##_bt buf; \
    if(fread(&buf, type##bufsize, 1, fp) != 1) { \
        perr("reading " #type " into buffer"); \
        return 0; \
    } \
    deserialize##type(dst, &buf); \
    X##type(XREAD) \
    return 1; \
}

#define WRITE(type) \
dword write##type(const type* src, FILE* fp, const DatHeader* header) { \
    struct type##_bt buf; \
    serialize##type(&buf, src); \
    X##type(XWRIT) \
    dword bufoff = ftell(fp) - DatHeaderbufsize; \
    if(!fwrite(buf, type##bufsize, 1, fp)) { \
        perr("writing " #type "to file"); \
        return 0; \
    } \
    header->datasz += type##bufsize; \
    return swapdword(bufoff); \
}

#define DESTROY(type) \
void destroy##type(const type* ptr) { \
    X##type(XDSTR) \
}

#define XDEF(x) DEF_##x
#define XDEFB(x) DEFB_##x
#define XSER(x) SER_##x
#define XDES(x) DES_##x
#define XPRNT(x) PRNT_##x
#define XREAD(x) READ_##x
#define XWRIT(x) WRIT_##x
#define XDSTR(x) DSTR_##x

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

static inline void _storeoffset(const DatHeader* header, dword offset) {
    if(header->reltnum == header->reltable)
        header->reltable = realloc(header->reltable,  header->reltable[0]*=2);
    if(table)
        header->reltable[header->reltnum++] = offset;
}

#include<datxdef.h>
// -- DatHeader --

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
    dword*  reltable;
    char* stringtable;
    dword stringnum;
} DatHeader;
const int DatHeaderbufsize = 0x20;

SERIALIZE(DatHeader)

DESERIALIZE(DatHeader)

PRINT(DatHeader)

int readDatHeader(DatHeader* dst, FILE* fp) {
    rewind(fp);
    if(fread(&dst, DatHeaderbufsize, 1, fp) != 1) {
        perr("reading DatHeader into buffer");
        return 0;
    }
    deserializeDatHeader(dst, dst);
    
    dst->rootnodes = malloc(sizeof(RootNode) * dst->rootnum);
    readRootNode(dst->rootnodes, fp, dst);
    dst->srootnodes = malloc(sizeof(RootNode) * dst->srootnum);
    readRootNode(dst->srootnodes, fp, dst);
    
    dst->reltable = malloc(sizeof(dword) * dst->reltnum);
    if(fread(dst->reltable, sizeof(dword) * dst->reltnum, 1, fp) != 1) {
        perr("reading relocation table");
    }

    int strtablesz =(dst->filesz - (
            dst->datasz + (dst->rootnum + dst->srootnum) * RootNodebufsize +
            dst->reltnum * sizeof(dword)));
    dst->stringtable = malloc(strtablesz);
    if(fread(dst->stringtable, strtablesz, 1, fp) != 1) {
        perr("reading string table");
    }
}

int writeDatHeader(DatHeader* src, FILE* fp) {
    fseek(fp, DatHeaderbufsize, SEEK_SET);
    if(src->reltable) free(src->reltable);
    src->reltnum = 1;
    src->reltable = malloc(1024);
    src->reltable[0] = 1024;

    struct RootNode_bt rootbuf[src->rootnum+src->srootnum];
    for(int ri=0; ri<src->rootnum; ++ri) {
        writeRootNode(src->rootnodes[ri], fp, src, rootbuf);
    }
    for(int ri=0; ri<src->srootnum; ++ri) {
        writeRootNode(src->srootnodes[ri], fp, src, rootbuf);
    }
    
    src->filesz = DatHeaderbufsize + src->datasz + 
        (src->rootnum + src->srootnum)*RootNodebufsize +
        (src->reltnum)*sizeof(dword) + src->stringnum;
    fseek(src->datasz+DatHeaderbufsize, SEEK_SET);
    if(!fwrite(src->reltable+1, sizeof(dword), src->reltnum, fp)) {
    fwrite(src->stringtable, src->stringnum, 1, fp);
    src->filesz = ftell(fp);
    serializeDatHeader(src, src);
    rewind(fp);
    fwrite(src, DatHeaderbufsize, 1, fp);
}

// -- RootNode --

struct RootNode_bt {
    dword child;
    dword name;
};
const int RootNodebufsize = sizeof(struct RootNode_bt);

enum RootChildType {
    RCT_UNKNOWN,
    RCT_JOINT,
    RCT_MESH,
    RCT_COL
};

typedef struct RootNode_t {
    union {
        void*       child;
        Joint*      joint;
        Mesh*       mesh;
        Collision*  collision;
    };
    dword name;
    RootChildType rct;
} RootNode;

SERIALIZE(RootNode)

DESERIALIZE(RootNode)

void printRootNode(RootNode* src) {
    printf("RootNode \n");
    switch(src->rct) {
        case RCT_JOINT: printf("%10s %10s", "Joint",     "joint");     break;
        case RCT_MESH:  printf("%10s %10s", "Mesh",      "mesh");      break;
        case RCT_COL:   printf("%10s %10s", "Collision", "collision"); break;
        default:        printf("%10s %10s", "UNKNOWN",   "child");     break;
    }
    printf("%8x\n", (unsigned int)src->child);
}

int readRootNode(RootNode* dst, FILE* fp, const DatHeader* header) {
    // being real cute
    // read the table of RootNode_bt's into the end of a table of RootNode_t's
    // since RootNode_t's are bigger, there shouldn't be an overlap, and if I deserialize
    // one at a time, it should be alright
    //   RRRR --BB bbbb    RRRR was read from BB, next will be
    //   rrrr RRRR BBbb    last four 'bytes' will be read from last two 'bytes'
    struct RootNode_bt *end;
    struct RootNode_bt *buf;
    struct RootNode_bt singlenode = {0};
    if(dst == header->rootnodes) {
        end = ((struct RootNode_bt*)(header->rootnodes + header->rootnum));
        buf = end - header->rootnum;
    } else if(dst == header->srootnodes) {
        end = ((struct RootNode_bt*)(header->srootnodes + header->srootnum));
        buf = end - header->srootnum;
    } else {
        printf("Info: readRootNode not called on rootnodes or srootnodes\n");
        buf = &singlenode;
        end = buf + 1;
    }
    if(fread(buf, RootNodebufsize, end-buf, fp) != end-buf) {
        perr("reading RootNode into buffer");
        return 0;
    }

    while(buf != end) {
        deserializeRootNode(dst, buf);

        // TODO determine child type
        dst->rct = RCT_JOINT;

        switch(dst->rct) {
            case RCT_JOINT: READ_offset(Joint,      joint)     break;
            case RCT_MESH:  READ_offset(Mesh,       mesh)      break;
            case RCT_COL:   READ_offset(Collision,  collision) break;
            default: perr("unknown root child type %d", dst->rct);
        }

        ++dst;
        ++buf;
    }
    return 1;
}

void writeRootNode(const RootNode* src, FILE* fp, const DatHeader* header, struct RootNode_bt* buf) {
    switch(src->rct) {
        case RCT_JOINT: src->child = (void*)writeJoint(src->joint, fp, header); break;
        case RCT_MESH:  src->child = (void*)writeMesh(src->mesh, fp, header); break;
        case RCT_COL:   src->child = (void*)writeCollision(src->collision, fp, header); break;
        default: perr("unknown root child type %d", dst->rct);
    }
    // have to defer storing the offsets until we know where buf will be written
    serializeRootNode(buf, src);
}

void destroyRootNode(const RootNode* ptr) {
    if(
}

// -- Joint --

DEFINE(Joint)

SERIALIZE(Joint)

DESERIALIZE(Joint)

PRINT(Joint)

READ(Joint)

WRITE(Joint)

DESTROY(Joint)

// -- JointData --

DEFINE(JointData)

SERIALIZE(JointData)

DESERIALIZE(JointData)

PRINT(JointData)

READ(JointData)

WRITE(JointData)

DESTROY(JointData)

// -- Mesh --

DEFINE(Mesh)

SERIALIZE(Mesh)

DESERIALIZE(Mesh)

PRINT(Mesh)

int readMesh(Mesh* dst, FILE* fp) {
    printf("Info: reading image data not yet implemented\n");
    return 0;
}

dword writeMesh(const Mesh* src, FILE* fp, const DatHeader* header) {
    printf("Info: writing mesh data not yet implemented\n");
    return 0;
}

DESTROY(Mesh)

// -- Material --

DEFINE(Material)

SERIALIZE(Material)

DESERIALIZE(Material)

PRINT(Material)

READ(Material)

WRITE(Material)

DESTROY(Mesh)

// -- Texture --

DEFINE(Texture)

SERIALIZE(Texture)

DESERIALIZE(Texture)

PRINT(Texture)

READ(Texture)

WRITE(Texture)

DESTROY(Texture)

// -- Color --

DEFINE(Color)

SERIALIZE(Color)

DESERIALIZE(Color)

PRINT(Color)

READ(Color)

WRITE(Color)

DESTROY(Color)

// -- Image --

DEFINE(Image)

SERIALIZE(Image)

DESERIALIZE(Image)

PRINT(Image)

int readImage(Image* dst, FILE* fp) {
    printf("Info: reading image data not yet implemented\n");
    return 0;
}

dword writeImage(const Image* src, FILE* fp, const DatHeader* header) {
    printf("Info: writing image data not yet implemented\n");
    return 0;
}

DESTROY(Image)

// -- Palette --

DEFINE(Palette)

SERIALIZE(Palette)

DESERIALIZE(Palette)

PRINT(Palette)

int readPalette(Palette* dst, FILE* fp) {
    printf("Info: reading palette data not yet implemented\n");
    return 0;
}

dword writePalette(const Palette* src, FILE* fp, const DatHeader* header) {
    printf("Info: writing palette data not yet implemented\n");
    return 0;
}

DESTROY(Palette)

#define UNDEF
#include<datxdef.h>
#undef UNDEF
READ(Texture)

WRITE(Texture)

DESTROY(Texture)

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

READ(Color)

WRITE(Color)

DESTROY(Color)

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

int readImage(Image* dst, FILE* fp) {
    printf("Info: reading image data not yet implemented\n");
    return 0;
}

dword writeImage(const Image* src, FILE* fp, const DatHeader* header) {
    printf("Info: writing image data not yet implemented\n");
    return 0;
}

DESTROY(Image)

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

int readPalette(Palette* dst, FILE* fp) {
    printf("Info: reading palette data not yet implemented\n");
    return 0;
}

dword writePalette(const Palette* src, FILE* fp, const DatHeader* header) {
    printf("Info: writing palette data not yet implemented\n");
    return 0;
}

DESTROY(Palette)

#endif // DATFILE_IMPLEMENT
#endif // DATFILE_H
