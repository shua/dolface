
/* -- Function Declarations -- */
static void _PDATM(DatStructVar*, uint8_t*);
static void PDGEN(DatStructVar*, uint8_t*, DatStructVar);
static void PDROOT(DatStructVar*, uint8_t*, DatStructVar);
static void PDVERTATTR(DatStructVar*, uint8_t*, DatStructVar);
static DatStructVar  ROOTTYPE(uint32_t*, char*);

static int TDGEN(DatStructVar*, uint8_t*, offtype*);
static int TDMAC(DatStructVar*, uint8_t*, offtype*);
static int TDMHE(DatStructVar*, uint8_t*, offtype*);
static int TDVERTATTR(DatStructVar*, uint8_t*, offtype*);

/* -- Type Declarations -- */
/* DatHdr and DatRoot are special, everything else can be changed */
enum {
 DatNULL = 0,
 DatHdr,
 DatRoot,
 DatJoint,
 DatJointData,
 DatTransform,
 DatMesh,
 DatMaterial,
 DatColor,
 DatTexture,
 DatTexUnk,
 DatImage,
 DatImageData,
 DatPalette,
 DatPaletteData,
 DatVertAttr,
 DatVertData,
 DatDisplayArr,
 DatWeightList,
 DatWeight,
 DatHitBoxData,
 DatCollision,
 
 DatMatAnimA,
 DatMatAnimB,
 DatMatAnimC,
 DatMatAnimCi,
 DatMatAnimCp,
 DatMatAnimD,
 DatMatAnimDd,
 DatMatAnimDdd,
 DatMatAnimAl,

 DatMapHeadA,
 DatMapHeadB,
 DatMapHeadC,
 DatMapHeadD,
 DatMapHeadE,
 DatMapHeadJD,
 DatMapHeadHalf,
 DatMapHeadAoN,
 DatMapHeadQnl,
 DatMapHeadQuin,
 DatMapHeadQna,
 DatMapHeadQnb,
 DatMapHeadQnc,
 DatMapHeadQnd,
 DatMapHeadQne,
 DatMapPlitB,
 DatMapPlitBl,
 DatMapPlitC,
 DatMapPlitCa,
 DatMapPlitCb,

 DatGrP,
 DatGrPA,
 DatLast
};

/* -- Original Known Vars -- */
DatStructVar hvars[] = {
    { AtWord,   "filesz",   0 },
    { AtWord,   "datasz",   0 },
    { AtWord,   "reltnum",  { .ui = PDEC } },
    { AtWord,   "rootnum",  { .ui = PDEC } },
    { AtWord,   "srootnum", { .ui = PDEC } },
    { AtWord,   "30303142", 0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    0
};
DatStructVar rvars[] = {
    { AtOff,   "child",     0 },
    { AtWord,   "name",     0 },
    0
};
DatStructVar xyzsub[] = {
    { 12,       0,          1 },
    { AtFloat,  "x",        0 },
    { AtFloat,  "y",        0 },
    { AtFloat,  "z",        0 },
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
    { AtSub,    "position", { .sv = xyzsub } },
    { AtOff,    "trnsform", { .ui = DatTransform } },
    { AtWord,   0,          0 },
    0
};
DatStructVar jdvars[] = {
    { AtWord,   0,          0 },
    { AtOff,    "next",     { .ui = DatJointData } },
    { AtOff,    "material", { .ui = DatMaterial } },
    { AtOff,    "mesh",     { .ui = DatMesh } },
    0
};
DatStructVar jtvars[] = {
    { AtFloat,  "x.x",      0 },
    { AtFloat,  "y.x",      0 },
    { AtFloat,  "z.x",      0 },
    { AtFloat,  "p.x",      0 },
    { AtFloat,  "x.y",      0 },
    { AtFloat,  "y.y",      0 },
    { AtFloat,  "z.y",      0 },
    { AtFloat,  "p.y",      0 },
    { AtFloat,  "x.z",      0 },
    { AtFloat,  "y.z",      0 },
    { AtFloat,  "z.z",      0 },
    { AtFloat,  "p.z",      0 },
    0
};
DatStructVar mvars[] = {
    { AtWord,   0,          0 },
    { AtOff,    "next",     { .ui = DatMesh } },
    { AtOff,    "verts",    { .ui = DatVertAttr} },
    { AtHalf,   "flags",    0 },
    { AtHalf,   "displayn", { .ui = PSIZ } },
    { AtArr,    "display",  { .ui = DatDisplayArr } },
    { AtList,   "weight",   { .ui = DatWeightList } },
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
    { AtSub,    "position", { .sv = xyzsub } },
    { AtSub,    "wrap",     { .sv = stwsub } },
    { AtSub,    "scale",    { .sv = stbsub } },
    { AtHalf,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtOff,    "image",    { .ui = DatImage } },
    { AtOff,    "palette",  { .ui = DatPalette } },
    { AtWord,   0,          0 },
    { AtOff,    "texunk",   { .ui =  DatTexUnk } },
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
    { AtOff,    "data",     { .ui = DatImageData } },
    { AtHalf,   "width",    0 },
    { AtHalf,   "height",   0 },
    { AtWord,   "format",   0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    0
};
DatStructVar pvars[] = {
    { AtOff,    "data",     { .ui = DatPaletteData } },
    { AtWord,   "format",   0 },
    { AtWord,   0,          0 },
    { AtHalf,   "colornum", 0 },
    { AtHalf,   0,          0 },
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
    { AtOff,    "data",     { .ui = DatVertData } },
    0
};
DatStructVar dvars[] = {
    { AtByte,   0,          0 },
    { AtByte,   "primitiv", 0 },
    { AtHalf,   "indices",  0 },
    0
};
DatStructVar wlvars[] = {
    { AtList,   "weight",   { .ui = DatWeight } },
    0
};
DatStructVar wvars[] = {
    { AtOff,    "joint",    { .ui = DatJoint } },
    { AtFloat,  "weight",   0 },
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
    { AtOff,    "vertices", { .ui = DatVertAttr } },
    { AtWord,   "vertn",    { .ui = PSIZ } },
    { AtOff,    "indices",  0 },
    { AtWord,   "indn",     { .ui = PSIZ } },
    { AtSub,    "indtable", { .sv = cnsub } },
    { AtOff,    0,          0 },
    { AtWord,   0,          0 },
    0
};

/* -- MatAnim Vars -- */
DatStructVar maavars[] = {
    { AtOff,    "next",     { .ui = DatMatAnimA } },
    { AtOff,    "child",    { .ui = DatMatAnimA } },
    { AtOff,    "MAB",      { .ui = DatMatAnimB } },
    0
};
DatStructVar malvars[] = {
    { AtOff,    0,          { .ui = DatMatAnimA } },
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
    { AtArr,    "MACi",     { .ui = DatMatAnimCi } },
    { AtArr,    "MACp",     { .ui = DatMatAnimCp } },
    { AtHalf,   "MACin",    { .ui = PSIZ } },
    { AtHalf,   "MACpn",    { .ui = PSIZ } },
    0
};
DatStructVar macivars[] = {
    { AtOff,    "image",    { .ui = DatImage } },
    0
};
DatStructVar macpvars[] = {
    { AtOff,    "palette",  { .ui = DatPalette } },
    0
};
DatStructVar madvars[] = {
    { AtWord,   0,          0 },
    { AtFloat,  "40a00000", 0 },
    { AtOff,    "data",     { .ui = DatMatAnimDd } },
    { AtWord,   0,          0 },
    0
};
DatStructVar maddvars[] = { 
    { AtOff,    "next",     { .ui = DatMatAnimDd } },
    { AtWord,   "b",        0 },
    { AtWord,   0,          0 },
    { AtWord,   "1860000",  0 },
    { AtOff,    "data",     { .ui = DatMatAnimDdd } },
    0
};
DatStructVar madddvars[] = {
    { AtWord,   "41000140", 0 },
    { AtWord,   "18001c0",  0 },
    { AtWord,   "2c00000",  0 },
    0
};

/* -- MapHead Vars -- */
DatStructVar mhavars[] = {
    { AtArr,    "jointdf",  { .ui = DatMapHeadJD } },
    { AtWord,   "jointdfn", { .ui = PSIZ } },
    { AtArr,    "b",        { .ui = DatMapHeadB } },
    { AtWord,   "bn",       { .ui = PSIZ } },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtArr,    "MHE",      { .ui = DatMapHeadE } },
    { AtWord,   "MHEn",     { .ui = PSIZ } },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    0
};
DatStructVar mhbvars[] = {
    { AtOff,    "joint1",   { .ui = DatJoint } },
    { AtList,   "MHQl",     { .ui = DatMapHeadQnl } },
    { AtList,   "MAl",      { .ui = DatMatAnimAl } },
    { AtOff,    "MHHalf",   { .ui = DatMapHeadHalf } },
    { AtOff,    "MHC",      { .ui = DatMapHeadC } },
    { AtWord,   0,          0 },
    { AtList,   "MPBl",     { .ui = DatMapPlitBl } },
    { AtOff,    "MHD",      { .ui = DatMapHeadD } },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtOff,    "allornot", { .ui = DatMapHeadAoN } },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    0
};
DatStructVar mhcvars[] = {
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtOff,    "plitCa1",  { .ui = DatMapPlitCa } },
    { AtOff,    "plitCa2",  { .ui = DatMapPlitCa } },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtOff,    "self?",    { .ui = DatMapHeadC } },
    { AtWord,   0,          0 },
    0
};
DatStructVar mhdvars[] = {
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtHalf,   0,          0 },
    { AtHalf,   0,          0 },
    { AtWord,   "flags?",   0 },
    { AtWord,   0,          0 },
    { AtOff,    "self?",    { .ui = DatMapHeadD } },
    { AtWord,   0,          0 },
    0
};
DatStructVar mhevars[] = {
    { AtOff,    0,          0 },
    0
};
DatStructVar mhjdvars[] = {
    { AtOff,    "joint",    { .ui = DatJoint } },
    { AtArr,    "halves",   { .ui = DatMapHeadHalf } },
    { AtWord,   "halvesn",  { .ui = PSIZ } },
    0
};
DatStructVar mhhalfvars[] = {
    { AtHalf,   "a",        0 },
    { AtHalf,   "b",        0 },
    0
};
DatStructVar mhaonvars[] = {
    { AtWord, "allornot",   0 },
    0
};
DatStructVar mhqnlvars[] = {
    { AtOff,    0,          { .ui = DatMapHeadQuin } },
    0
};
DatStructVar mhqnvars[] = {
    { AtOff,    "next",     { .ui = DatMapHeadQuin } },
    { AtOff,    "child",    { .ui = DatMapHeadQuin } },
    { AtOff,    "data",     { .ui = DatMapHeadQna } },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    0
};
DatStructVar mhqnavars[] = { 
    { AtWord,   0,          0 },
    { AtWord,   "flags?",   0 },
    { AtOff,    "MHQnb",    { .ui = DatMapHeadQnb } },
    { AtWord,   0,          0 },
    0
};
DatStructVar mhqnbvars[] = {
    { AtOff,    "next",     { .ui = DatMapHeadQnb } },
    { AtWord,   "datan",    { .ui = PSIZ } },
    { AtWord,   "MHQnc",    { .ui = DatMapHeadQnc } },
    { AtWord,   0,          0 },
    { AtOff,    "data",     0 },
    0
};
DatStructVar mhqncvars[] = {
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtOff,    "MHQnd",    { .ui = DatMapHeadQnd } },
    { AtWord,   0,          0 },
    0
};
DatStructVar mhqndvars[] = {
    { AtOff,    "next",     { .ui = DatMapHeadQnd } },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtOff,    "MHQne",    { .ui = DatMapHeadQne } },
    0
};
DatStructVar mhqnevars[] = {
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    0
};

/* -- MapPlit Vars -- */
DatStructVar mpbvars[] = {
    { AtOff,    0,          { .ui = DatMapPlitC } },
    { AtOff,    0,          { .ui = DatMapPlitC } },
    0
};
DatStructVar mpblvars[] = {
    { AtOff,    0,          { .ui = DatMapPlitB } },
    0
};
DatStructVar mpcvars[] = {
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtHalf,   0,          0 },
    { AtHalf,   0,          0 },
    { AtWord,   "flags?",   0 },
    { AtOff,    0,          { .ui = DatMapPlitCa } },
    { AtWord,   0,          0 },
    { AtOff,    0,          { .ui = DatMapPlitCb } },
    0
};
DatStructVar mpcavars[] = {
    { AtFloat,  0,          0 },
    { AtFloat,  0,          0 },
    { AtFloat,  0,          0 },
    { AtFloat,  0,          0 },
    { AtFloat,  0,          0 },
    0
};
DatStructVar mpcbvars[] = {
    { AtFloat,  0,          0 },
    0
};

/* grGroundParam */
DatStructVar grsub[] = {
    { 4,        0,          19 },
    { AtHalf,   0,          0 },
    { AtHalf,   0,          0 },
    0
};
DatStructVar grvars[] = {
    { AtFloat,  0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtFloat,  0,          0 },
    { AtFloat,  0,          0 },
    { AtFloat,  0,          0 },
    { AtFloat,  0,          0 },
    { AtFloat,  0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtFloat,  0,          0 },
    { AtFloat,  0,          0 },
    { AtFloat,  0,          0 },
    { AtFloat,  0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtFloat,  0,          0 },
    { AtFloat,  0,          0 },
    { AtFloat,  0,          0 },
    { AtFloat,  0,          0 },
    { AtSub,    0,          { .sv = grsub } },
    { AtArr,    "GRA",      { .ui = DatGrPA } },
    { AtWord,   "GRAn",     { .ui = PSIZ } },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    0
};
DatStructVar grasub[] = {
    { 4,        0,          20 },
    { AtHalf,   0,          0 },
    { AtHalf,   0,          0 },
    0
};
DatStructVar gravars[] = {
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtSub,    0,          { .sv = grasub } },
    0
};

/* -- Type Definition -- */
DatStruct ddefs[] = {
    [DatNULL] =         { "Unknown",        0, 0,       TDGEN, PDGEN},
    [DatHdr] =          { "Header",         0, hvars,   TDGEN, PDGEN},
    [DatRoot] =         { "Root",           0, rvars,   TDGEN, PDROOT},
    [DatJoint] =        { "Joint",          0, jvars,   TDGEN, PDGEN},
    [DatJointData] =    { "JointData",      0, jdvars,  TDGEN, PDGEN},
    [DatTransform] =    { "Transform",      0, jtvars,  TDGEN, PDGEN},
    [DatMesh] =         { "Mesh",           0, mvars,   TDGEN, PDGEN},
    [DatMaterial] =     { "Material",       0, mtvars,  TDGEN, PDGEN},
    [DatColor] =        { "Color",          0, cvars,   TDGEN, PDGEN},
    [DatTexture] =      { "Texture",        0, tvars,   TDGEN, PDGEN},
    [DatTexUnk] =       { "TextureUnk",     0, tuvars,  TDGEN, PDGEN},
    [DatImage] =        { "Image",          0, ivars,   TDGEN, PDGEN},
    [DatImageData] =    { "ImageData",      0, 0,       TDGEN, PDGEN},
    [DatPalette] =      { "Palette",        0, pvars,   TDGEN, PDGEN},
    [DatPaletteData] =  { "PaletteData",    0, 0,       TDGEN, PDGEN},
    [DatVertAttr] =     { "VertAttr",       0, vvars,   TDVERTATTR, PDVERTATTR},
    [DatVertData] =     { "VertData",       0, 0,       TDGEN, PDGEN},
    [DatDisplayArr] =   { "DisplayArr",     0, dvars,   TDGEN, PDGEN},
    [DatWeightList] =   { "WeightList",     0, wlvars,  TDGEN, PDGEN},
    [DatWeight] =       { "Weight",         0, wvars,   TDGEN, PDGEN},
    [DatHitBoxData] =   { "HitBoxData",     0, hbvars,  TDGEN, PDGEN},
    [DatCollision] =    { "Collision",      0, cnvars,  TDGEN, PDGEN},

    [DatMatAnimA] =     { "MatAnimA",       0, maavars, TDGEN, PDGEN},
    [DatMatAnimAl] =    { "MatAnimAList",   0, malvars, TDGEN, PDGEN},
    [DatMatAnimB] =     { "MatAnimB",       0, mabvars, TDGEN, PDGEN},
    [DatMatAnimC] =     { "MatAnimC",       0, macvars, TDMAC, PDGEN},
    [DatMatAnimCi] =    { "MatAnimCi",      0, macivars, TDGEN, PDGEN},
    [DatMatAnimCp] =    { "MatAnimCp",      0, macpvars, TDGEN, PDGEN},
    [DatMatAnimD] =     { "MatAnimD",       0, madvars, TDGEN, PDGEN},
    [DatMatAnimDd] =    { "MatAnimDd",      0, maddvars, TDGEN, PDGEN},
    [DatMatAnimDdd] =   { "MatAnimDdd",     0, madddvars, TDGEN, PDGEN},

    [DatMapHeadA] =     { "MapHeadA",       0, mhavars, TDGEN, PDGEN},
    [DatMapHeadB] =     { "MapHeadB",       0, mhbvars, TDGEN, PDGEN},
    [DatMapHeadC] =     { "MapHeadC",       0, mhcvars, TDGEN, PDGEN},
    [DatMapHeadD] =     { "MapHeadD",       0, mhdvars, TDGEN, PDGEN},
    [DatMapHeadE] =     { "MapHeadE",       0, mhevars, TDMHE, PDGEN},
    [DatMapHeadJD] =    { "MapHeadJD",      0, mhjdvars, TDGEN, PDGEN},
    [DatMapHeadHalf] =  { "MapHeadHalf",    0, mhhalfvars, TDGEN, PDGEN},
    [DatMapHeadAoN] =   { "AllorNot",       0, mhaonvars, TDGEN, PDGEN},
    [DatMapHeadQnl] =   { "MapHeadQnList",  0, mhqnlvars, TDGEN, PDGEN},
    [DatMapHeadQuin] =  { "MapHeadQuin",    0, mhqnvars, TDGEN, PDGEN},
    [DatMapHeadQna] =   { "MapHeadQna",     0, mhqnavars, TDGEN, PDGEN},
    [DatMapHeadQnb] =   { "MapHeadQnb",     0, mhqnbvars, TDGEN, PDGEN},
    [DatMapHeadQnc] =   { "MapHeadQnc",     0, mhqncvars, TDGEN, PDGEN},
    [DatMapHeadQnd] =   { "MapHeadQnd",     0, mhqndvars, TDGEN, PDGEN},
    [DatMapHeadQne] =   { "MapHeadQne",     0, mhqnevars, TDGEN, PDGEN},

    [DatMapPlitB] =     { "MapPlitB",       0, mpbvars, TDGEN, PDGEN},
    [DatMapPlitBl] =    { "MapPlitBl",      0, mpblvars, TDGEN, PDGEN},
    [DatMapPlitC] =     { "MapPlitC",       0, mpcvars, TDGEN, PDGEN},
    [DatMapPlitCa] =    { "MapPlitCa",      0, mpcavars, TDGEN, PDGEN},
    [DatMapPlitCb] =    { "MapPlitCb",      0, mpcbvars, TDGEN, PDGEN},

    [DatGrP] =          { "GroundParam",    0, grvars,  TDGEN, PDGEN}, 
    [DatGrPA] =         { "GroundParamA",   0, gravars, TDGEN, PDGEN}, 

    [DatLast] = { 0 },
};

/* -- Root Identifying Table -- */
DatRootFmt dnametype[] = {
    { 0, "_animjoint", 0 },
    { 0, "_matanim_joint", DatMatAnimA },
    { 0, "_shapeanim_joint", 0 },
    { 0, "_joint", DatJoint },
    { "ftData", 0, 0 },
    { 0, "_tlut_desc", DatPalette },
    { 0, "_tlut", DatPaletteData },
    { "coll_data", 0, DatCollision },
    { "grGroundParam", 0, DatGrP },
    { "map_head", 0, DatMapHeadA },
    { "map_plit", 0, DatMapPlitBl },
    { 0, 0, 0 },
};

/* -- Function Definitions -- */
void 
_PDATM(DatStructVar* dvp, uint8_t* ptr) {
    char format[] = "%10d";
    uint32_t temp;
    if(dvp->extra.ui==PHEX) format[3]='x';
    switch(dvp->type) {
        case AtWord:  printf(format, be32toh(*(uint32_t*)ptr)); break;
        case AtHalf:  printf(format, be16toh(*(uint16_t*)ptr)); break;
        case AtByte:  printf(format, *(uint8_t*)ptr); break;
        case AtFloat: printf("%10f", *(float*)(temp=be32toh(*(uint32_t*)ptr), &temp)); break;
        case AtList:
        case AtArr:
        case AtOff:   printf(":%08x:", be32toh(*(uint32_t*)ptr)); break;
        case AtSub:   if(dvp->extra.sv->extra.ui>1) printf("[%d]", dvp->extra.sv->extra.ui); break;
    }
}

void
PDGEN(DatStructVar* dvp, uint8_t* ptr, DatStructVar ctx) {
    char atnames[AtLast] = {
        [AtWord] =  'w',
        [AtHalf] =  'h',
        [AtByte] =  'b',
        [AtFloat] = 'f',
        [AtOff] =   'o',
        [AtSub] =   's',
        [AtArr] =   'a',
        [AtList] =  'l',
    };
    int i;
    if(!dvp) return;
    while(dvp->type) {
        printf("%c %8s ", atnames[dvp->type], dvp->name);
        _PDATM(dvp, ptr);
        switch(dvp->type) {
        case AtSub:
            for(i=0; i<dvp->extra.sv->extra.ui; ++i) {
                printf("{\n");
                PDGEN(dvp->extra.sv+1, ptr, (DatStructVar){ 0 });
                ptr+=dvp->extra.sv->type;
                printf("}, ");
            }
            break;
        case AtHalf: ptr+=2; break;
        case AtByte: ptr+=1; break;
        case AtList:
        case AtArr:
        case AtOff: if(dvp->extra.ui) printf(" %s", ddefs[dvp->extra.ui].name);
        default: ptr+=4; break;
        }
        printf("\n");
        ++dvp;
    }
}

void
PDROOT(DatStructVar* dvp, uint8_t* ptr, DatStructVar ctx) {
    char* strt;
    if(!dvp) return;
    if(ctx.type != DatHdr) return PDGEN(dvp, ptr, ctx);
    strt = ctx.name;
    if(ctx.extra.ui==0) ctx.extra.ui=1;

    while(ctx.extra.ui--) {
        _PDATM(dvp, ptr);
        printf(" %11s ", ddefs[ROOTTYPE((uint32_t*)ptr, strt).type].name);
        ptr+=4;
        printf("%s\n", strt+be32toh(*(uint32_t*)ptr));
        ptr+=4;
    }
}

void
PDLIST(DatStructVar* dvp, uint8_t* ptr, DatStructVar ctx) {
}

int
TDGEN(DatStructVar* dvp, uint8_t* buf, offtype* ots) {
    int i, j, asize;
    char* asize_s;
    if(!dvp) return 0;
    asize = 1;
    asize_s = 0;
    for(i=0; dvp->type; ++dvp) {
        switch(dvp->type) {
            case AtByte: 
                if(dvp->extra.ui==PSIZ) {
                    asize=*buf; 
                    asize_s = dvp->name;
                }
                buf+=1; break;
            case AtHalf: 
                if(dvp->extra.ui==PSIZ) {
                    asize=be16toh(*(uint16_t*)buf); 
                    asize_s = dvp->name;
                }
                buf+=2; break;
            case AtOff:  
                if(*(uint32_t*)buf) {
                    ots[i].o = be32toh(*(uint32_t*)buf);
                    ots[i].t = dvp->extra.ui;
                    ots[i].v = 0;
#ifdef DEBUG
                    printf("\n-- o %11s %8x", ddefs[ots[i].t].name, ots[i].o);
#endif
                    ++i;
                }
                buf+=4; break;
            case AtArr:  
                if(*(uint32_t*)buf) {
                    if(!(asize_s) || !(dvp->name) || strncmp(asize_s, dvp->name, strlen(dvp->name))) {
                        if(dvp[1].type==AtWord && dvp[1].extra.ui == PSIZ && dvp[1].name
                                && !strncmp(dvp[1].name, dvp->name, strlen(dvp->name))) {
                            asize=be32toh(*(uint32_t*)(buf+4));
                        } else {
                            fprintf(stderr, "Couldn't find size for array %s: %s\n", dvp->name, dvp[1].name);
                            asize_s=0;
                            asize=1;
                        }
                    }
                    if(asize) {
                        ots[i].o = be32toh(*(uint32_t*)buf);
                        ots[i].t = dvp->extra.ui;
                        ots[i].v = -(asize-1);
#ifdef DEBUG
                        printf("\n-- a %11s %8x %8x", ddefs[ots[i].t].name, ots[i].o, -ots[i].v);
#endif
                        ++i;
                    }
                    asize = 1;
                }
                buf+=4; break;
            case AtList: 
                if(*(uint32_t*)buf) {
                    ots[i].o = be32toh(*(uint32_t*)buf);
                    ots[i].t = dvp->extra.ui;
                    ots[i].v = 2;
#ifdef DEBUG
                    printf("\n-- l %11s %8x", ddefs[ots[i].t].name, ots[i].o);
#endif
                    ++i;
                }
                buf+=4; break;
            case AtFloat:buf+=4; break;
            case AtWord: 
                if(dvp->extra.ui==PSIZ) {
                    asize=be32toh(*(uint32_t*)buf); 
                    asize_s=dvp->name;
                }
                buf+=4; break;
            case AtSub:
                for(j=0; j<dvp->extra.sv->extra.ui; ++j) {
                    i += TDGEN(dvp->extra.sv+1, buf, ots+i);
                    buf += dvp->extra.sv->type;
                }
                break;
        }
    }
    return i;
}

int
TDMHE(DatStructVar* dvp, uint8_t* buf, offtype* ots) {
    if(!dvp) return 0;
    ots->t=DatNULL;
    ots->o=be32toh(*(uint32_t*)buf);
    ots->v=0;
    return (ots->o && ots->o != 0xc0000000);
}

void
PDVERTATTR(DatStructVar* dvp, uint8_t* ptr, DatStructVar ctx) {
    if(!dvp) return;
    while(be32toh(*(uint32_t*)ptr)!=0xff) {
        PDGEN(dvp, ptr, ctx);
        ptr+=ddefs[DatVertAttr].size;
    }
    PDGEN(dvp, ptr, ctx);
}

int
TDVERTATTR(DatStructVar* dvp, uint8_t* buf, offtype* ots) {
    int t=0;
    int i=0;
    if(!dvp) return 0;
    while(be32toh(*(uint32_t*)buf)!=0xff && (i++) < 4) {
        t += TDGEN(dvp, buf, ots+t);
        buf += ddefs[DatVertAttr].size;
    }
    return t;
}

int
TDMAC(DatStructVar* dvp, uint8_t* buf, offtype* ots) {
    int t,i;
    uint32_t img;
    uint32_t pal;
    if(!dvp) return 0;
    if(dvp != macvars) printf("What is happening?\n");
    t=i=0;
    img=pal=0;
    for(i=0; dvp[i].type; ++i) {
        switch(i) {
        case 2:
            ots->o=be32toh(*(uint32_t*)buf);
            if(ots->o) {
                ots->t = dvp[i].extra.ui;
                ots->v = 0;
#ifdef DEBUG
                printf("\n-- o %11s %8x", ddefs[ots->t].name, ots->o);
#endif
                ++t;
                ++ots;
            }
            break;
        case 3: img=be32toh(*(uint32_t*)buf); break;
        case 4: pal=be32toh(*(uint32_t*)buf); break;
        case 5: if(img) {
                ots->o=img;
                ots->t=dvp[3].extra.ui;
                ots->v= -((be16toh(*(uint16_t*)buf))-1);
#ifdef DEBUG
                printf("\n-- a %11s %8x %8x", ddefs[ots->t].name, ots->o, -ots->v);
#endif
                ++t;
                ++ots;
            } break;
        case 6: if(pal) {
                ots->o=pal;
                ots->t=dvp[4].extra.ui;
                ots->v= -((be16toh(*(uint16_t*)buf))-1);
#ifdef DEBUG
                printf("\n-- a %11s %8x %8x", ddefs[ots->t].name, ots->o, -ots->v);
#endif
                ++t;
                ++ots;
            } break;
        default: break;
        }

        switch(dvp[i].type) {
        case AtByte: buf+=1; break;
        case AtHalf: buf+=2; break;
        default: buf+=4; break;
        }
    }
    return t;
}

DatStructVar
ROOTTYPE(uint32_t* root, char* strt) {
    int i, strl;
    char* name;
    name = strt+be32toh(root[1]);
    strl = strlen(name);
    for(i=0; dnametype[i].suf || dnametype[i].pre; ++i) {
        if(dnametype[i].pre && strncmp(dnametype[i].pre, name, strlen(dnametype[i].pre)) != 0) continue;
        if(dnametype[i].suf && strcmp(dnametype[i].suf, name+(strl-strlen(dnametype[i].suf))) != 0) continue;
        return (DatStructVar){ dnametype[i].type, 0, 0 };
    }
    return (DatStructVar){ 0 };
}

