
/* -- Function Declarations -- */
static void _PDATM(DatStructVar*, uint8_t*);
static void PDGEN(int, DatStructVar*, uint8_t*, DatStructVar);
static void PDROOT(int, DatStructVar*, uint8_t*, DatStructVar);
static void PDVERTATTR(int, DatStructVar*, uint8_t*, DatStructVar);
static DatStructVar  ROOTTYPE(uint32_t*, char*);

static int TDGEN(DatStructVar*, uint8_t*, offtype*, offtype*);
static int TDLIST(DatStructVar*, uint8_t*, offtype*, offtype*);
static int TDMAC(DatStructVar*, uint8_t*, offtype*, offtype*);
static int TDMHE(DatStructVar*, uint8_t*, offtype*, offtype*);
static int TDVERTATTR(DatStructVar*, uint8_t*, offtype*, offtype*);
static int TDPREMDF(DatStructVar*, uint8_t*, offtype*, offtype*);
static int TDMDF(DatStructVar*, uint8_t*, offtype*, offtype*);
static int TDJOINT(DatStructVar*, uint8_t*, offtype*, offtype*);

/* -- Type Declarations -- */
/* DatHdr and DatRoot are special, everything else can be changed */
enum {
 DatNULL = 0,
 DatHdr,
 DatRoot,
 DatJoint,
 DatJointData,
 DatJointDataA,
 DatJointDataAd,
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
 DatIndData,
 DatDisplayArr,
 DatWeightl,
 DatWeight,
 DatHitBoxData,
 DatCollision,
 DatCollData,
 
 DatMatAnimA,
 DatMatAnimAl,
 DatMatAnimB,
 DatMatAnimC,
 DatMatAnimCi,
 DatMatAnimCp,
 DatMatAnimD,
 DatMatAnimE,
 DatMatAnimF,

 DatMapHeadA,
 DatMapHeadB,
 DatMapHeadC,
 DatMapHeadD,
 DatMapHeadE,
 DatMapHeadF,
 DatMapHeadFl,
 DatMapHeadG,
 DatMapHeadH,

 DatMapHeadJD,
 DatMapHalf,
 DatMapHalfA,
 DatMapHeadAoN,

 DatMHAJointo,
 DatMHAMPEo,
 DatMHAMato,

 DatMapQuinl,
 DatMapQuin,
 DatMapQuinA,
 DatMapQuinB,
 DatMapQuinC,
 DatMapQuinD,
 DatMapQuinE,
 DatMapQuinF,
 DatMapPlitB,
 DatMapPlitBl,
 DatMapPlitC,
 DatMapPlitCa,
 DatMapPlitCb,
 DatMapPlitD,
 DatMapPlitE,
 DatMapPtcl,
 DatMapTexg,

 DatMapDataFtr,

 DatGrP,
 DatGrPA,
 DatQMS,
 DatQMSl,
 DatQMSA,
 DatQMSB,
 DatQMSC,
 DatQMSCd,

 DatItemData,
 DatItemDatal,
 DatItemDataA,
 DatItemDataB,
 DatItemDataC,
 DatItemDataD,
 DatItemDataE,
 DatItemDataF,
 DatItemDataG,
 DatItemDataH,
 DatYakumono,
 DatYakuAll,

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
    { AtWord,   "flags",    { .ui = PFLG } },
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
DatStructVar jdavars[] = {
    { AtWord,   "flags",    0 }, /* not sure */
    { AtWord,   0,          0 },
    { AtOff,    "data1",    { .ui = DatJointDataAd } },
    { AtWord,   0,          0 },
    { AtOff,    "data2",    { .ui = DatJointDataAd } },
    { AtOff,    "data3",    { .ui = DatJointDataAd } },
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
    { AtHalf,   "flags",    { .ui = PFLG } },
    { AtHalf,   "displayn", { .ui = PSIZ } },
    { AtArr,    "display",  { .ui = DatDisplayArr } },
    { AtOff,    "weight",   { .ui = DatWeightl } },
    0
};
DatStructVar mtvars[] = {
    { AtWord,   0,          0 },
    { AtWord,   "flags",    { .ui = PFLG } },
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
    { AtOff,    "weight",   { .ui = DatWeight } },
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
    { AtOff,    "vert",     { .ui = DatVertData } },
    { AtWord,   "vertn",    { .ui = PSIZ } },
    { AtOff,    "ind",      { .ui = DatIndData } },
    { AtWord,   "indn",     { .ui = PSIZ } },
    { AtSub,    "indtable", { .sv = cnsub } },
    { AtOff,    "colld",    { .ui = DatCollData } },
    { AtWord,   "colldn",   { .ui = PSIZ } },
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
    { AtOff,    "MAD",      { .ui = DatMatAnimD } },
    { AtOff,    "MAC",      { .ui = DatMatAnimC } },
    { AtOff,    0,          0 },
    0
};
DatStructVar macvars[] = {
    { AtOff,    "next",     { .ui = DatMatAnimC } },
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
    { AtOff,    "MAE",     { .ui = DatMatAnimE } },
    { AtWord,   0,          0 },
    0
};
DatStructVar maevars[] = { 
    { AtOff,    "next",     { .ui = DatMatAnimE } },
    { AtWord,   "MAFn",     { .ui= PSIZ } },
    { AtWord,   0,          0 },
    { AtWord,   "flags",    0 }, /* not sure */
    { AtOff,    "MAF",      { .ui = DatMatAnimF } },
    0
};
DatStructVar mafvars[] = {
    { AtByte,   0,          0 },
    0
};
/* -- ItemData Vars -- */
DatStructVar idvars[] = {
    { AtWord,   "type",     0 },
    { AtOff,    "IDA",      0 },
    0
};
DatStructVar idlvars[] = {
    { AtOff,    0,          { .ui = DatItemData } },
    0
};
DatStructVar idavars[] = {
    { AtOff,    "IDB",      { .ui = DatItemDataB } },
    { AtOff,    "IDC",      { .ui = DatItemDataC } },
    { AtOff,    "IDD",      { .ui = DatItemDataD } },
    { AtOff,    "IDE",      { .ui = DatItemDataE } },
    { AtOff,    "IDF",      { .ui = DatItemDataF } },
    { AtWord,   0,          0 },
    0
};
DatStructVar idcvars[] = {
    { AtOff,    "next",     { .ui = DatItemDataC } },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    0
};
DatStructVar iddvars[] = {
    { AtWord,   0,          0 },
    { AtOff,    "IDG",      { .ui = DatItemDataG } },
    0
};
DatStructVar idfvars[] = {
    { AtOff,    "joint",    { .ui = DatJoint } },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    0
};
DatStructVar idgvars[] = {
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
DatStructVar idhvars[] = {
    { AtWord,   0,          0 },
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

/* -- MapHead Vars -- */
DatStructVar mhavars[] = {
    { AtArr,    "jointdf",  { .ui = DatMapHeadJD } },
    { AtWord,   "jointdfn", { .ui = PSIZ } },
    { AtArr,    "MHB",      { .ui = DatMapHeadB } },
    { AtWord,   "MHBn",     { .ui = PSIZ } },
    { AtArr,    "jointdo",  { .ui = DatMHAJointo } },
    { AtWord,   "jointdon", { .ui = PSIZ } },
    { AtArr,    "MHE",      { .ui = DatMapHeadE } },
    { AtWord,   "MHEn",     { .ui = PSIZ } },
    { AtArr,    "MPEo",     { .ui = DatMHAMPEo } },
    { AtWord,   "MPEon",    { .ui = PSIZ } },
    { AtArr,    "mato",     { .ui = DatMHAMato } },
    { AtWord,   "maton",    { .ui = PSIZ } },
    0
};
DatStructVar mhbvars[] = {
    { AtOff,    "joint",    { .ui = DatJoint } },
    { AtOff,    "MHQl",     { .ui = DatMapQuinl } }, /* keep this as off instead of list, because it has a special traversal */
    { AtOff,    "MAl",      { .ui = DatMatAnimAl } },  /* same here */
    { AtOff,    "MHFl",     { .ui = DatMapHeadFl } },  /* same here */
    { AtOff,    "MHC",      { .ui = DatMapHeadC } },
    { AtOff,    "MHH",      { .ui = DatMapHeadH } },
    { AtOff,    "MPBl",     { .ui = DatMapPlitBl } },
    { AtOff,    "MHD",      { .ui = DatMapHeadD } },
    { AtArr,    "Halfl",    { .ui = DatMapHalfA } },
    { AtWord,   "Halfln",   { .ui = PSIZ } },
    { AtOff,    "allornot", { .ui = DatMapHeadAoN } },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    0
};
DatStructVar mdfvars[] = {
    { AtOff,    "joint",    { .ui = DatJoint } },
    { AtOff,    "MHQl",     { .ui = DatMapQuinl } },
    { AtOff,    "MAAl",     { .ui = DatMatAnimAl } },
    { AtOff,    "MHFl",     { .ui = DatMapHeadFl } },
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
    0
};
DatStructVar mhdvars[] = {
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtHalf,   0,          0 },
    { AtHalf,   0,          0 },
    { AtWord,   "flags",    0 }, /* not sure */
    { AtWord,   0,          0 },
    { AtOff,    "self",     { .ui = DatMapHeadD } }, /* not sure */
    { AtWord,   0,          0 },
    0
};
DatStructVar mhevars[] = {
    { AtOff,    0,          { .ui = DatMapPlitC } },
    0
};
DatStructVar mhfvars[] = {
    { AtOff,    "next",     { .ui = DatMapHeadF } },
    { AtOff,    "child",    { .ui = DatMapHeadF } },
    { AtOff,    "MHG",      { .ui = DatMapHeadG } },
    0
};
DatStructVar mhflvars[] = {
    { AtOff,    "MHF",      { .ui = DatMapHeadF } },
    0
};
DatStructVar mhgvars[] = {
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    0
};
DatStructVar mhhvars[] = {
    { AtWord,   0,          0 },
    { AtOff,    "triple04", { .ui = DatNULL } },
    { AtOff,    "triple08", { .ui = DatNULL } },
    { AtOff,    "triple0c", { .ui = DatNULL } },
    { AtWord,   0,          0 },
    { AtOff,    "MHC",      { .ui = DatMapHeadC } },
    { AtOff,    "self",     { .ui = DatMapHeadH } }, /* not sure */
    0
};
DatStructVar mhajdovars[] = {
    { AtOff,    "jointd",   { .ui = DatJointDataA } },
    0
};
DatStructVar mhampeovars[] = {
    { AtOff,    "MPE",      { .ui = DatMapPlitE } },
    { AtWord,   0,          0 },
    0
};
DatStructVar mhamovars[] = {
    { AtOff,    "material", { .ui = DatMaterial } },
    0
};
DatStructVar mhjdvars[] = {
    { AtOff,    "joint",    { .ui = DatJoint } },
    { AtArr,    "halves",   { .ui = DatMapHalf } },
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
DatStructVar mqlvars[] = {
    { AtOff,    0,          { .ui = DatMapQuin } },
    0
};
DatStructVar mqvars[] = {
    { AtOff,    "next",     { .ui = DatMapQuin } },
    { AtOff,    "child",    { .ui = DatMapQuin } },
    { AtOff,    "MAD",      { .ui = DatMatAnimD } },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    0
};

/* -- MapPlit Vars -- */
DatStructVar mpbvars[] = {
    { AtOff,    0,          { .ui = DatMapPlitC } },
    { AtOff,    0,          { .ui = DatMapPlitD } },
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
    { AtWord,   "flags",    0 }, /* not sure */
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
DatStructVar mpdvars[] = {
    { AtOff,    0,          { .ui = DatMapPlitE } },
    { AtWord,   0,          0 },
    0
};
DatStructVar mpevars[] = {
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
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

/* -- quake_mode_set -- */
DatStructVar qmsvars[] = {
    { AtOff,    "QMSA",     { .ui = DatQMSA } },
    { AtOff,    "QMSl",     { .ui = DatQMSl } },
    { AtOff,    "QMSB",     { .ui = DatQMSB } },
    { AtOff,    0,          0 },
    { AtOff,    "self",     0 }, /* not sure */
    0
};
DatStructVar qmslvars[] = {
    { AtOff,    "QMS",      { .ui = DatQMS } },
    0
};
DatStructVar qmsbvars[] = {
    { AtWord,   0,          0 },
    { AtWord,   "flags",    0 }, /* not sure */
    { AtOff,    "QMSC",     { .ui = DatQMSC } },
    { AtWord,   0,          0 },
    0
};
DatStructVar qmscvars[] = {
    { AtOff,    "next",     { .ui = DatQMSC } },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtWord,   0,          0 },
    { AtOff,    "data",     { .ui = DatQMSCd } },
    0
};

/* -- Type Definition -- */
DatStruct ddefs[] = {
    [DatNULL] =         { "Unknown",        0, 0,       TDGEN, PDGEN},
    [DatHdr] =          { "Header",         0, hvars,   TDGEN, PDGEN},
    [DatRoot] =         { "Root",           0, rvars,   TDGEN, PDROOT},
    [DatJoint] =        { "Joint",          0, jvars,   TDJOINT, PDGEN},
    [DatJointData] =    { "JointData",      0, jdvars,  TDGEN, PDGEN},
    [DatJointDataA] =   { "JointDataA",     0, jdavars,  TDGEN, PDGEN},
    [DatJointDataAd] =  { "JointDataAData", 0, 0,  TDGEN, PDGEN},
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
    [DatIndData] =      { "IndData",       0, 0,       TDGEN, PDGEN},
    [DatDisplayArr] =   { "DisplayArr",     0, dvars,   TDGEN, PDGEN},
    [DatWeightl] =      { "WeightList",     0, wlvars,  TDLIST, PDGEN},
    [DatWeight] =       { "Weight",         0, wvars,   TDLIST, PDGEN},
    [DatHitBoxData] =   { "HitBoxData",     0, hbvars,  TDGEN, PDGEN},
    [DatCollision] =    { "Collision",      0, cnvars,  TDGEN, PDGEN},
    [DatCollData] =     { "CollData",       0, 0,       TDGEN, PDGEN},

    [DatMatAnimA] =     { "MatAnimA",       0, maavars, TDGEN, PDGEN},
    [DatMatAnimAl] =    { "MatAnimAList",   0, malvars, TDPREMDF, PDGEN},
    [DatMatAnimB] =     { "MatAnimB",       0, mabvars, TDGEN, PDGEN},
    [DatMatAnimC] =     { "MatAnimC",       0, macvars, TDMAC, PDGEN},
    [DatMatAnimCi] =    { "MatAnimCi",      0, macivars, TDGEN, PDGEN},
    [DatMatAnimCp] =    { "MatAnimCp",      0, macpvars, TDGEN, PDGEN},
    [DatMatAnimD] =     { "MatAnimD",       0, madvars, TDGEN, PDGEN},
    [DatMatAnimE] =     { "MatAnimE",       0, maevars, TDGEN, PDGEN},
    [DatMatAnimF] =     { "MatAnimF",       0, mafvars, TDGEN, PDGEN},

    [DatMapHeadA] =     { "MapHeadA",       0, mhavars, TDGEN, PDGEN},
    [DatMapHeadB] =     { "MapHeadB",       0, mhbvars, TDGEN, PDGEN},
    [DatMapHeadC] =     { "MapHeadC",       0, mhcvars, TDGEN, PDGEN},
    [DatMapHeadD] =     { "MapHeadD",       0, mhdvars, TDGEN, PDGEN},
    [DatMapHeadE] =     { "MapHeadE",       0, mhevars, TDMHE, PDGEN},
    [DatMapHeadF] =     { "MapHeadF",       0, mhfvars, TDGEN, PDGEN},
    [DatMapHeadFl] =    { "MapHeadFList",   0, mhflvars, TDPREMDF, PDGEN},
    [DatMapHeadG] =     { "MapHeadG",       0, mhgvars, TDGEN, PDGEN},
    [DatMapHeadH] =     { "MapHeadH",       0, mhhvars, TDGEN, PDGEN},
    [DatMapHeadJD] =    { "MapHeadJD",      0, mhjdvars, TDGEN, PDGEN},
    [DatMapHeadAoN] =   { "AllorNot",       0, mhaonvars, TDGEN, PDGEN},
    [DatMapHalf] =      { "MapHalf",        0, mhhalfvars, TDGEN, PDGEN},
    [DatMapHalfA] =     { "MapHalfList",    0, 0,       TDGEN, PDGEN},

    [DatMHAJointo] =    { "MHAJointo",      0, mhajdovars, TDGEN, PDGEN},
    [DatMHAMPEo] =      { "MHAMPEo",        0, mhampeovars, TDGEN, PDGEN},
    [DatMHAMato] =      { "MHAMato",        0, mhamovars, TDGEN, PDGEN},

    [DatMapQuinl] =     { "MapQuinList",    0, mqlvars, TDPREMDF, PDGEN},
    [DatMapQuin] =      { "MapQuin",        0, mqvars, TDGEN, PDGEN},

    [DatMapDataFtr] =   { "MapDataFtr",     0, mdfvars, TDMDF, PDGEN},

    [DatMapPlitB] =     { "MapPlitB",       0, mpbvars, TDGEN, PDGEN},
    [DatMapPlitBl] =    { "MapPlitBl",      0, mpblvars, TDLIST, PDGEN},
    [DatMapPlitC] =     { "MapPlitC",       0, mpcvars, TDGEN, PDGEN},
    [DatMapPlitCa] =    { "MapPlitCa",      0, mpcavars, TDGEN, PDGEN},
    [DatMapPlitCb] =    { "MapPlitCb",      0, mpcbvars, TDGEN, PDGEN},
    [DatMapPlitD] =     { "MapPlitD",       0, mpdvars, TDGEN, PDGEN},
    [DatMapPlitE] =     { "MapPlitE",       0, mpevars, TDGEN, PDGEN},

    [DatMapPtcl] =      { "MapPtcl",        0, 0,       TDGEN, PDGEN},
    [DatMapTexg] =      { "MapTexg",        0, 0,       TDGEN, PDGEN},

    [DatGrP] =          { "GroundParam",    0, grvars,  TDGEN, PDGEN}, 
    [DatGrPA] =         { "GroundParamA",   0, gravars, TDGEN, PDGEN}, 

    [DatQMS] =          { "QuakeMS",        0, qmsvars, TDGEN, PDGEN},
    [DatQMSl] =         { "QuakeMSList",    0, qmslvars, TDGEN, PDGEN},
    [DatQMSA] =         { "QuakeMSA",       0, 0,       TDGEN, PDGEN},
    [DatQMSB] =         { "QuakeMSB",       0, qmsbvars, TDGEN, PDGEN},
    [DatQMSC] =         { "QuakeMSC",       0, qmscvars, TDGEN, PDGEN},
    [DatQMSCd] =        { "QuakeMSCd",      0, 0,       TDGEN, PDGEN},
    
    [DatItemData] =     { "ItemData",       0, idvars,  TDGEN, PDGEN},
    [DatItemDatal] =    { "ItemDataList",   0, idlvars, TDLIST, PDGEN},
    [DatItemDataA] =    { "ItemDataA",      0, idavars, TDGEN, PDGEN},
    [DatItemDataB] =    { "ItemDataB",      0, 0,       TDGEN, PDGEN},
    [DatItemDataC] =    { "ItemDataC",      0, idcvars, TDGEN, PDGEN},
    [DatItemDataD] =    { "ItemDataD",      0, iddvars, TDGEN, PDGEN},
    [DatItemDataE] =    { "ItemDataE",      0, 0,       TDGEN, PDGEN},
    [DatItemDataF] =    { "ItemDataF",      0, idfvars, TDGEN, PDGEN},
    [DatItemDataG] =    { "ItemDataG",      0, idgvars, TDGEN, PDGEN},
    [DatItemDataH] =    { "ItemDataH",      0, idhvars, TDGEN, PDGEN},
    [DatYakumono] =     { "Yakumono",       0, 0,       TDGEN, PDGEN},
    [DatYakuAll] =      { "ALDYakuAll",     0, 0,       TDGEN, PDGEN},
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
    { 0, "_image", DatImageData },
    { "coll_data", 0, DatCollision },
    { "grGroundParam", 0, DatGrP },
    { "map_head", 0, DatMapHeadA },
    { "map_plit", 0, DatMapPlitBl },
    { "map_ptcl", 0, DatMapPtcl },
    { "map_texg", 0, DatMapTexg },
    { "itemdata", 0, DatItemDatal },
    { "quake_model_set", 0, DatQMS },
    { "ALDYakuAll", 0,      DatYakuAll },
    { "yakumono_param", 0,  DatYakumono },
    { 0, 0, 0 },
};

/* -- Function Definitions -- */
void
_PDFLAG(uint32_t flag) {
    int i;
    for(i=0; flag; (flag >>= 1), ++i) {
        if(flag & 1) printf(" f_%02d", i);
    }
}

void 
_PDATM(DatStructVar* dvp, uint8_t* ptr) {
    char format[] = "%10x";
    uint32_t temp;
    if(dvp->extra.ui==PDEC) format[3]='d';
    switch(dvp->type) {
        case AtWord:  printf(format, be32toh(*(uint32_t*)ptr)); break;
        case AtHalf:  printf(format, be16toh(*(uint16_t*)ptr)); break;
        case AtByte:  printf(format, *(uint8_t*)ptr); break;
        case AtFloat: printf("%10f", *(float*)(temp=be32toh(*(uint32_t*)ptr), &temp)); break;
        case AtArr:
        case AtOff:   printf("*%08x", be32toh(*(uint32_t*)ptr)); break;
        case AtSub:   if(dvp->extra.sv->extra.ui>1) printf("[%d]", dvp->extra.sv->extra.ui); break;
    }
}

void
_indent(int n) {
    while(n--) { printf("    "); }
}

void
PDGEN(int pre, DatStructVar* dvp, uint8_t* ptr, DatStructVar ctx) {
    char atnames[AtLast] = {
        [AtWord] =  'uint32_t',
        [AtHalf] =  'uint16_t',
        [AtByte] =  'uint8_t ',
        [AtFloat] = 'float   ',
        [AtOff] =   'uint32_t',
        [AtSub] =   'struct  ',
        [AtArr] =   'uint32_t',
    };
    int i;
    if(!dvp) return;
    while(dvp->type) {
        _indent(pre);
        printf("%c %s: ", atnames[dvp->type], dvp->name);
        _PDATM(dvp, ptr);
        switch(dvp->type) {
        case AtSub:
            for(i=0; i<dvp->extra.sv->extra.ui; ++i) {
                printf("{\n");
                PDGEN(pre+1, dvp->extra.sv+1, ptr, (DatStructVar){ 0 });
                ptr+=dvp->extra.sv->type;
                printf("}, ");
            }
            break;
        case AtHalf: ptr+=2; break;
        case AtByte: ptr+=1; break;
        case AtWord: if(dvp->extra.ui == PFLG) _PDFLAG(be32toh(*(uint32_t*)ptr)); ptr+=4; break;
        case AtArr:
        case AtOff: if(dvp->extra.ui) printf(" %s", ddefs[dvp->extra.ui].name);
        default: ptr+=4; break;
        }
        printf("\n");
        ++dvp;
    }
}

void
PDROOT(int pre, DatStructVar* dvp, uint8_t* ptr, DatStructVar ctx) {
    char* strt;
    if(!dvp) return;
    if(ctx.type != DatHdr) return PDGEN(pre, dvp, ptr, ctx);
    strt = ctx.name;
    if(ctx.extra.ui==0) ctx.extra.ui=1;

    while(ctx.extra.ui--) {
        _indent(pre);
        _PDATM(dvp, ptr);
        printf(" %11s ", ddefs[ROOTTYPE((uint32_t*)ptr, strt).type].name);
        ptr+=4;
        printf("%s\n", strt+be32toh(*(uint32_t*)ptr));
        ptr+=4;
    }
}

void
PDVERTATTR(int pre, DatStructVar* dvp, uint8_t* ptr, DatStructVar ctx) {
    if(!dvp) return;
    while(be32toh(*(uint32_t*)ptr)!=0xff) {
        PDGEN(pre, dvp, ptr, ctx);
        ptr+=ddefs[DatVertAttr].size;
    }
    PDGEN(pre, dvp, ptr, ctx);
}

int
TDGEN(DatStructVar* dvp, uint8_t* buf, offtype* ots, offtype *self) {
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
                    ots[i] = INITIALIZE_OFFTYPE;
                    ots[i].o = be32toh(*(uint32_t*)buf);
                    ots[i].t = dvp->extra.ui;
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
                        ots[i] = INITIALIZE_OFFTYPE;
                        ots[i].o = be32toh(*(uint32_t*)buf);
                        ots[i].t = dvp->extra.ui;
                        ots[i].v = -(asize)+1;
                        ots[i].z = asize;
#ifdef DEBUG
                        printf("\n-- a %11s %8x %8x", ddefs[ots[i].t].name, ots[i].o, ots[i].z);
#endif
                        ++i;
                    }
                    asize = 1;
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
                    i += TDGEN(dvp->extra.sv+1, buf, ots+i, self);
                    buf += dvp->extra.sv->type;
                }
                break;
        }
    }
    return i;
}

int
TDLIST(DatStructVar *dvp, uint8_t *buf, offtype *ots, offtype *self) {
    int t,s;
    for(t=0, s=0; dvp[t].type; ++t) {
        switch(dvp[t].type) {
        case AtByte: s+=1; break;
        case AtHalf: s+=2; break;
        case AtSub:  s+=dvp[t].extra.sv->extra.ui; break;
        default:     s+=4; break;
        }
    }

    for(t=0;*(uint32_t*)buf;  buf+=s)
        t+=TDGEN(dvp, buf, ots+t, self);

    return t;
}

int
TDMHE(DatStructVar* dvp, uint8_t* buf, offtype* ots, offtype *self) {
    if(!dvp) return 0;
    *ots = INITIALIZE_OFFTYPE;
    ots->t=dvp->extra.ui;
    ots->o=be32toh(*(uint32_t*)buf);
    ots->v=0;
#ifdef DEBUG
    if(ots->o && ots->o !=0xc0000000)
        printf("\n-- o %11s %8x", ddefs[ots->t].name, ots->o);
#endif
    return (ots->o && ots->o != 0xc0000000);
}

int
TDVERTATTR(DatStructVar* dvp, uint8_t* buf, offtype* ots, offtype *self) {
    int t=0;
    int i=0;
    if(!dvp) return 0;
    while(be32toh(*(uint32_t*)buf)!=0xff && (i++) < 4) {
        t += TDGEN(dvp, buf, ots+t, self);
        buf += ddefs[DatVertAttr].size;
    }
    return t;
}

int
TDMAC(DatStructVar* dvp, uint8_t* buf, offtype* ots, offtype *self) {
    int t,i;
    uint32_t img;
    uint32_t pal;
    if(!dvp) return 0;
    if(dvp != macvars) printf("TDMAC called on wrong type\n");
    t=i=0;
    img=pal=0;
    for(i=0; dvp[i].type; ++i) {
        switch(i) {
            case 0:
                ots->o=be32toh(*(uint32_t*)buf);
                if(ots->o) {
                    ots->t = dvp[i].extra.ui;
                    ots->v = 0;
                    ++t;
                    ++ots;
                    *ots=INITIALIZE_OFFTYPE;
                }
                break;
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
                *ots=INITIALIZE_OFFTYPE;
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
                *ots=INITIALIZE_OFFTYPE;
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
                *ots=INITIALIZE_OFFTYPE;
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

static int 
TDPREMDF(DatStructVar *dvp, uint8_t *buf, offtype *ots, offtype *self) {
    int y,t,i,j;
    uint32_t *b = (uint32_t*)buf;
    if(!dvp) return 0;
    /* null value indicates either null list (in which case we return 0) or null Joint point (in which case we return 0) */
    if(!be32toh(*(uint32_t*)buf)) return 0; 
    
    /* MapDataFtr structs exist in the file without any other parent structs pointing to them
     * the only offsets to MapDataFtr's is in the list at the end, where they often include their own offsets
     * However, it seems as if they follow this general order
     * 1    MapQuinList
     * 2    MatAnimAList (optional)
     * 3    MapHeadFList (optional)
     * 4    MapDataFtr
     *          Joint
     *          MapQuinList   :1:
     *          MatAnimAList  :2: or 0
     *          MapHeadFLIst  :3: or 0
     * 5    MapDataFtrList (optional, often includes :4:)
     */
    for(t=0, i=0; (ots[t].o = be32toh(b[i])); ++i) {
        ots[t++].t = dvp->extra.ui;
#ifdef DEBUG
        printf("\n-- o %11s %8x", ddefs[ots[t].t].name, ots[t].o);
#endif
    }
        
    switch(dvp->extra.ui) {
    case DatMapQuin: y=2; break;
    case DatMatAnimA:    y=3; break;
    case DatMapHeadF:    y=4; break;
    default: 
        printf("TDPREMDF called on non PREMDF struct %d /c { %d %d %d }\n", dvp->type, DatMapQuin, DatMatAnimA, DatMapHeadF);
        return 0;
    }

    for(i=0, j=y; j<5; ++j) {
        if(be32toh(b[i+y]) == self->o)
            break;
        for(; be32toh(b[i]); ++i);
    }

    if(j>=5) {
#ifdef DEBUG
        printf("%d couldn't find MapDataFtr\n", self->o);
#endif
        return t;
    }
    ots[t].t = DatMapDataFtr;
    ots[t].o = self->o+(i+1)*4;
#ifdef DEBUG
    printf("\n-- o %11s %8x", ddefs[ots[t].t].name, ots[t].o);
#endif
    ++t;
    return t;
}

static int 
TDMDF(DatStructVar *dvp, uint8_t *buf, offtype *ots, offtype *self) {
    int t,i;
    /* read normal struct */
    t=TDGEN(dvp,buf,ots,self);
    
    /* read the list at the end */
    for(i=4; (ots[t].o = be32toh(*(((uint32_t*)buf)+i))); ++i)
        ots[t++].t = DatMapDataFtr;

    return t;
}

static int
TDJOINT(DatStructVar *dvp, uint8_t *buf, offtype *ots, offtype *self) {
    int i, t, d, f;
    t=TDGEN(dvp,buf,ots,self);
    if(!(d=be32toh(((uint32_t*)buf)[4]))) return t;
    f=be32toh(*(uint32_t*)(buf+4));
    if(!(f & 1<<14)) return t;

    for(i=0; i<t && ots[i].o != d; ++i);
    if(i==t) return t;
    
    ots[i].t = DatJointDataA;
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

