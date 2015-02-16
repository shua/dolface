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
 DatTexUnk,
 DatImage,
 DatPalette,
 DatVertAr,
 DatDisplayList,
 DatHitBoxData,
 DatCollision,
 DatMatAnimA,
 DatMatAnimB,
 DatMatAnimC,
 DatMatAnimCdh,
 DatMatAnimCd,
 DatMatAnimD,
 DatMatAnimDd,
 DatMatAnimDdd,
 DatLast
};

DatStructVar hvars[] = {
    { AtWord,   "filesz",   0 },
    { AtWord,   "datasz",   0 },
    { AtWord,   "reltnum",  { .ui = PDEC } },
    { AtWord,   "rootnum",  { .ui = PDEC } },
    { AtWord,   "srootnum", { .ui = PDEC } },
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
    { AtWord,   "flags",    0 },
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
    { AtOff,    "texunk",  { .ui =  DatTexUnk } },
    0
};
DatStructVar tuvars[] = {
    { AtWord,   0,          0 },
    { AtWord,   "101",      0 },
    { AtWord,   "8580080f", 0 },
    { AtWord,   "07070707", 0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
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
    { AtWord,   "vertn",    { .ui = PDEC } },
    { AtOff,    "indices",  0 },
    { AtWord,   "indn",     { .ui = PDEC } },
    { AtSub,    "indextable", { .sv = cnsub } },
    { AtOff,    0,          0 },
    { AtWord,   0,          0 },
    0
};

DatStructVar maavars[] = {
    { AtOff,    "next",     { .ui = DatMatAnimA } },
    { AtOff,    "child",    0 },
    { AtOff,    "MAB",      { .ui = DatMatAnimB } },
    0
};
DatStructVar mabvars[] = {
    { AtOff,    "next",     { .ui = DatMatAnimB } },
    { AtOff,    0,          0 },
    { AtOff,    "MAC",      { .ui = DatMatAnimC } },
    { AtOff,    0,          0 },
    0
};
DatStructVar macvars[] = {
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtOff,    "MAD",      { .ui = DatMatAnimD } },
    { AtOff,    "datahdr",  { .ui = DatMatAnimCdh } },
    { AtWord,   0,          0 },
    { AtOff,    "animdata", 0 },
    0
};
DatStructVar macdhvars[] = {
    { AtOff,    "a",        { .ui = DatMatAnimCd } },
    { AtOff,    "b",        { .ui = DatMatAnimCd } },
    { AtOff,    "c",        { .ui = DatMatAnimCd } },
    { AtOff,    "d",        { .ui = DatMatAnimCd } },
    0
};
DatStructVar macdvars[] = {
    { AtOff,    "animdata", 0 },
    { AtWord,   "01000100", 0 },
    { AtWord,   "e",        0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    0
};
DatStructVar madvars[] = {
    { AtWord,   0,          0 },
    { AtWord,   "40a00000", 0 },
    { AtOff,    "data",     0 },
    0
};
DatStructVar maddvars[] = { 
    { AtWord,   0,          0 },
    { AtWord,   "b",        0 },
    { AtWord,   0,          0 },
    { AtWord,   "1860000",  0 },
    { AtOff,    "data",     0 },
    0
};
DatStructVar madddvars[] = {
    { AtWord,   "41000140", 0 },
    { AtWord,   "18001c0",  0 },
    { AtWord,   "2c00000",  0 },
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
    [DatTexUnk] =   { "TextureUnk", 0, tuvars},
    [DatImage] =    { "Image", 0, ivars},
    [DatPalette] =  { "Palette", 0, pvars},
    [DatVertAr] =   { "VertArray", 0, vvars},
    [DatDisplayList] = { "DisplayList", 0, dvars},
    [DatHitBoxData] = { "HitBoxData", 0, hbvars},
    [DatCollision] = { "Collision", 0, cnvars},

    [DatMatAnimA] = { "MatAnimA", 0, maavars},
    [DatMatAnimB] = { "MatAnimB", 0, mabvars},
    [DatMatAnimC] = { "MatAnimC", 0, macvars},
    [DatMatAnimCdh] = { "MatAnimCdh", 0, macdhvars},
    [DatMatAnimCd] = { "MatAnimCd", 0, macdvars},
    [DatMatAnimD] = { "MatAnimD", 0, madvars},
    [DatMatAnimDd] = { "MatAnimDd", 0, maddvars},
    [DatMatAnimDdd] = { "MatAnimDdd", 0, madddvars},

    [DatLast] = { 0, 0, 0 },
};
