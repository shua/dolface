#ifndef UNDEF

#ifndef DWORD
#define DWORD uint32_t
#endif
#ifndef DHALF
#define DHALF uint16_t
#endif
#ifndef DBYTE
#define DBYTE uint8_t
#endif
#ifndef DFLOAT
#define DFLOAT float
#endif
#ifndef DFL_XYZ
typedef union {
    struct {
        float x;
        float y;
        float z;
    };
    float  m[3];
} floatxyz;
#define DFL_XYZ floatxyz
#endif
#ifndef DOFF
#define DOFF void
#endif

#define XDatHeader(_) \
_(field)    (DWORD, filesz) \
_(field)    (DWORD, datasz) \
_(field)    (DWORD, reltnum) \
_(field)    (DWORD, rootnum) \
_(field)    (DWORD, srootnum) \
_(field)    (DWORD, unknown14) \
_(field)    (DWORD, unknown18) \
_(field)    (DWORD, unknown1c)

#define XRootNode(_) \
_(offset)   (DOFF, child) \
_(field)    (DWORD, name)

#define XJoint(_) \
_(field)    (DWORD, unknown) \
_(field)    (DWORD, flags) \
_(offset)   (Joint, child) \
_(offset)   (Joint, next) \
_(offset)   (JointData, data) \
_(field)    (DFL_XYZ, rotation) \
_(field)    (DFL_XYZ, scale) \
_(field)    (DFL_XYZ, translation) \
_(offset)   (DOFF, transform) \
_(field)    (DWORD, padding)

#define XJointData(_) \
_(field)    (DWORD, unknown) \
_(offset)   (JointData, next) \
_(offset)   (Material, material) \
_(offset)   (Mesh, mesh)

#define XMesh(_) \
_(field)    (DWORD, unknown) \
_(offset)   (Mesh, next) \
_(offset)   (DOFF, verts) \
_(field)    (DHALF, flags) \
_(field)    (DHALF, displaynum) \
_(offset)   (DisplayList, display) \
_(offset)   (DOFF, weight)

#define XMaterial(_) \
_(field)    (DWORD, unknown) \
_(field)    (DWORD, flags) \
_(offset)   (Texture, texture) \
_(offset)   (Color, color) \
_(field)    (DWORD, unknown10) \
_(field)    (DWORD, unknown14)

#define XTexture(_) \
_(field)    (DWORD, unknown) \
_(offset)   (Texture, next) \
_(field)    (DWORD, unknown08) \
_(field)    (DWORD, unknown0c) \
_(field)    (DFL_XYZ, rotation) \
_(field)    (DFL_XYZ, scale) \
_(field)    (DFL_XYZ, translation) \
_(field)    (DWORD, wraps) \
_(field)    (DWORD, wrapt) \
_(field)    (DBYTE, scales) \
_(field)    (DBYTE, scalet) \
_(field)    (DHALF, unknown3e) \
_(field)    (DWORD, unknown40) \
_(field)    (DWORD, unknown44) \
_(field)    (DWORD, unknown48) \
_(offset)   (Image, image) \
_(offset)   (Palette, palette) \
_(field)    (DWORD, unknown54) \
_(field)    (DWORD, unknown58) \

#define XColor(_) \
_(field)    (DWORD, diffuse) \
_(field)    (DWORD, ambient) \
_(field)    (DWORD, specular) \
_(field)    (DFLOAT, unknown0c) \
_(field)    (DFLOAT, unknown10)

#define XImage(_) \
_(offset)   (DOFF, data) \
_(field)    (DHALF, width) \
_(field)    (DHALF, height) \
_(field)    (DWORD, format)

#define XPalette(_) \
_(offset)   (DOFF, data) \
_(field)    (DWORD, format) \
_(field)    (DWORD, unknown08) \
_(field)    (DHALF, colornum) \
_(field)    (DHALF, unknown0e)

#else // UNDEF

#undef XDatHeader
#undef XRootNode
#undef XJoint
#undef XJointData
#undef XMesh
#undef XMaterial
#undef XTexture
#undef XColor
#undef XPalette

#endif // UNDEF
