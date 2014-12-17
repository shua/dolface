#ifndef DATCUT_H
#define DATCUT_H
#include <datxdef.h>
#include <stddef.h>

#define XTYPES(_) \
_(DatHeader) \
_(RootNode) \
_(Joint) \
_(JointData) \
_(Mesh) \
_(Collision) \
_(Material) \
_(Color) \
_(Texture) \
_(Image) \
_(Palette) \
_(VertexAttr) \
_(DisplayList)

#define BUF_field(type, name) type name;
#define BUF_array(type, name, num) type name[num];
#define BUF_offset(type, name) uint32 name;
#define XBUF(x) BUF_x

#define BUFFER(name) \
typedef struct #name_buf { \
    X#name(XBUF) \
} name_buf; \

typedef struct DatOffsetM_t {
    struct DatOffsetM_t* type;
    uint32 offset;
} DatOffsetM;

#define DUMP_field(type, name)
#define DUMP_array(type, name, num)
#define DUMP_offset(type, name) { type, offsetof(type_buf, name) }, 
#define XDUMP(x) DUMP_x

#define DUMPTYPE(name) \
struct DatOffsetM name[] = { \
    X#name(XDUMP) \
    { NULL, 0 } \
}

XTYPES(BUFFER)

int relocDump(char*, char*);
#endif // DATCUT_H
