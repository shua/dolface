
enum {
    GX_TF_I4,
    GX_TF_I8,
    GX_TF_IA4,
    GX_TF_IA8,
    GX_TF_RGB565,
    GX_TF_RGB5A3,
    GX_TF_RGBA8,
    GX_TF_CI4,
    GX_TF_CI8,
    GX_TF_CI14 = 0xa,
    GX_TF_CMPR = 0xe,
};
enum {
    GX_CLAMP = 0,
    GX_REPEAT = 1,
    GX_MIRROR = 2,
};
enum {
    GX_VA_PNMTXIDX,
    GX_VA_TEX0MTXIDX,
    GX_VA_TEX1MTXIDX,
    GX_VA_TEX2MTXIDX,
    GX_VA_TEX3MTXIDX,
    GX_VA_TEX4MTXIDX,
    GX_VA_TEX5MTXIDX,
    GX_VA_TEX6MTXIDX,
    GX_VA_TEX7MTXIDX,
    GX_VA_POS,
    GX_VA_NRM,
    GX_VA_CLR0,
    GX_VA_CLR1,
    GX_VA_TEX0,
    GX_VA_TEX1,
    GX_VA_TEX2,
    GX_VA_TEX3,
    GX_VA_TEX4,
    GX_VA_TEX5,
    GX_VA_TEX6,
    GX_VA_TEX7,
    GX_POS_MTX_ARRAY,
    GX_NRM_MTX_ARRAY,
    GX_TEX_MTX_ARRAY,
    GX_LIGHT_ARRAY,
    GX_VA_NBT,
    GX_VA_MAX_ATTR,
    GX_VA_NULL = 0xff,
};
enum {
    GX_NONE,
    GX_DIRECT,
    GX_INDEX8,
    GX_INDEX16,
};
enum {
    GX_POS_XY,
    GX_POS_XYZ,
};
enum {
    GX_NRM_XYZ,
    GX_NRM_NBT,
    GX_NRM_NBT3,
};
enum {
    GX_CLR_RGB,
    GX_CLR_RGBA,
};
enum {
    GX_TEX_S,
    GX_TEX_ST,
};
// Vertex size specifier
enum {
    GX_U8,
    GX_S8,
    GX_U16,
    GX_S16,
    GX_F32,
};
// Color size specifier
enum {
    GX_RGB565,
    GX_RGB8,
    GX_RGBX8,
    GX_RGBA4,
    GX_RGBA6,
    GX_RGBA8,
};
// Primitive type name
enum {
    GX_POINTS = 0xb8,
    GX_LINES = 0xa8,
    GX_LINESTRIP = 0xb0,
    GX_TRIANGLES = 0x90,
    GX_TRAINGLESTRIP = 0x98,
    GX_TRIANGLEFAN = 0xa0,
    GX_QUADS = 0x80,
};
