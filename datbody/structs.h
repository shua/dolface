// DatHdr and DatRoot are special, everything else can be changed
enum {
 DatNULL = 0,
 DatHdr,
 DatRoot,
 DatJoint,
 DatJointData,
 DatMesh,
 DatMaterial,
 DatColor,
 DatTexture,
 DatImage,
 DatPalette,
 DatVertAr,
 DatDisplayList,
 DatHitBoxData,
 DatCollision,
 DatLast
};

DatStructVar hvars[] = {
    { AtWord,   "filesz",   { .ui = PHEX } },
    { AtWord,   "datasz",   { .ui = PHEX } },
    { AtWord,   "reltnum",  0 },
    { AtWord,   "rootnum",  0 },
    { AtWord,   "srootnum", 0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    0
};
DatStructVar rvars[] = {
    { AtOff,    "child",    0 },
    { AtWord,   "name",     0 },
    0
};
DatStructVar xyzsub[] = {
    { 12,       0,      1 },
    { AtFloat,  "x",    0 },
    { AtFloat,  "y",    0 },
    { AtFloat,  "z",    0 },
    0
};
DatStructVar jvars[] = {
    { AtWord,   0,          0 },
    { AtWord,   "flags",    { .ui = PHEX } },
    { AtOff,    "child",    { .ui = DatJoint } },
    { AtOff,    "next",     { .ui = DatJoint } },
    { AtOff,    "data",     { .ui = DatJointData } },
    { AtSub,    "rotation", { .sv = xyzsub } },
    { AtSub,    "scale",    { .sv = xyzsub } },
    { AtSub,    "translation", { .sv = xyzsub } },
    { AtOff,    "transform", 0 },
    { AtWord,   0,          0 },
    0
};
DatStructVar jdvars[] = {
    { AtWord,   0,          0 },
    { AtOff,    "next",     { .ui = DatJointData } },
    { AtOff,    "material", {.ui = DatMaterial } },
    { AtOff,    "mesh",     {.ui = DatMesh } },
    0
};
DatStructVar mvars[] = {
    { AtWord,   0,          0 },
    { AtOff,    "next",     { .ui = DatMesh } },
    { AtOff,    "verts",    { .ui = DatVertAr} },
    { AtHalf,   "flags",    0 },
    { AtHalf,   "displaynum", 0 },
    { AtOff,    "display",  0 },
    { AtOff,    "weight",   0 },
    0
};
DatStructVar mtvars[] = {
    { AtWord,   0,          0 },
    { AtWord,   "flags",    0 },
    { AtOff,    "texture",  { .ui = DatTexture } },
    { AtOff,    "color",    { .ui = DatColor } },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    0
};
DatStructVar cvars[] = {
    { AtWord,   "diffuse",  0 },
    { AtWord,   "ambient",  0 },
    { AtWord,   "specular", 0 },
    { AtFloat,  0,          0 },
    0
};
DatStructVar stwsub[] = {
    { 8,        0,          1 },
    { AtWord,   "s",        0 },
    { AtWord,   "t",        0 },
    0
};
DatStructVar stbsub[] = {
    { 2,        0,          1 },
    { AtByte,   "s",        0 },
    { AtByte,   "t",        0 },
    0
};
DatStructVar tvars[] = {
    { AtWord,   0,          0 },
    { AtOff,    "next",     { .ui = DatTexture } },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtSub,    "rotation", { .sv = xyzsub } },
    { AtSub,    "scale",    { .sv = xyzsub } },
    { AtSub,    "translation", { .sv = xyzsub } },
    { AtSub,    "wrap",     { .sv = stwsub } },
    { AtSub,    "scale",    { .sv = stbsub } },
    { AtHalf,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtOff,    "image",    { .ui = DatImage } },
    { AtOff,    "palette",  { .ui = DatPalette } },
    { AtWord,   0,          0 },
    { AtOff,    0,          0 },
    0
};
DatStructVar ivars[] = {
    { AtOff,    "data",     0 },
    { AtHalf,   "width",    0 },
    { AtHalf,   "height",   0 },
    { AtWord,   "format",   0 },
    0
};
DatStructVar pvars[] = {
    { AtOff,    "data",     0 },
    { AtWord,   "format",   0 },
    { AtWord,   0,          0 },
    { AtHalf,   "colornum", 0 },
    { AtHalf,   0,          0},
    0
};
DatStructVar vvars[] = {
    { AtWord,   "ident",    0 },
    { AtWord,   "usage",    0 },
    { AtWord,   "format",   0 },
    { AtWord,   "type",     0 },
    { AtByte,   "scale",    0 },
    { AtByte,   0,          0 },
    { AtHalf,   "stride",   0 },
    { AtOff,    "data",     0 },
    0
};
DatStructVar dvars[] = {
    { AtByte,   0,          0 },
    { AtByte,   "primitive", 0 },
    { AtHalf,   "indices",  0 },
    0
};
DatStructVar hbvars[] = {
    { AtWord,   "primary",  0 },
    { AtHalf,   "scale",    0 },
    { AtHalf,   "z_off",    0 },
    { AtHalf,   "y_off",    0 },
    { AtHalf,   "x_off",    0 },
    { AtWord,   "physics",  0 },
    { AtWord,   "effects",  0 },
    0
};
DatStructVar cnsub[] = {
    { 4,        0,          5 },
    { AtHalf,   "start",    0 },
    { AtHalf,   "num",      0 },
    0
};
DatStructVar cnvars[] = {
    { AtOff,    "vertices", { .ui = DatVertAr } },
    { AtWord,   "vertn",    0 },
    { AtOff,    "indices",  0 },
    { AtWord,   "indn",     0 },
    { AtSub,    "indextable", { .sv = cnsub } },
    { AtOff,    0,          0 },
    { AtWord,   0,          0 },
    0
};

DatStruct ddefs[] = {
    [DatNULL] =     { "Unknown", 0, 0 },
    [DatHdr] =      { "Header", 0, hvars},
    [DatRoot] =     { "Root", 0, rvars},
    [DatJoint] =    { "Joint", 0, jvars},
    [DatJointData] = { "JointData", 0, jdvars},
    [DatMesh] =     { "Mesh", 0, mvars},
    [DatMaterial] = { "Material", 0, mtvars},
    [DatColor] =    { "Color", 0, cvars},
    [DatTexture] =  { "Texture", 0, tvars},
    [DatImage] =    { "Image", 0, ivars},
    [DatPalette] =  { "Palette", 0, pvars},
    [DatVertAr] =   { "VertArray", 0, vvars},
    [DatDisplayList] = { "DisplayList", 0, dvars},
    [DatHitBoxData] = { "HitBoxData", 0, hbvars},
    [DatCollision] = { "Collision", 0, cnvars},
};
