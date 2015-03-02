
static void _PDATM(DatStructVar*, uint8_t*);
static void PDGEN(DatStructVar*, uint8_t*, void*);
static void PDROOT(DatStructVar*, uint8_t*, void*);
static int  ROOTTYPE(uint32_t*, char*);

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
 DatVertAttr,
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
    { AtSub,    "position", { .sv = xyzsub } },
    { AtOff,    "trnsform", 0 },
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
    { AtOff,    "verts",    { .ui = DatVertAttr} },
    { AtHalf,   "flags",    0 },
    { AtHalf,   "displayn", 0 },
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
    { AtOff,    "data",     0 },
    0
};
DatStructVar dvars[] = {
    { AtByte,   0,          0 },
    { AtByte,   "primitiv", 0 },
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
    { AtOff,    "vertices", { .ui = DatVertAttr } },
    { AtWord,   "vertn",    { .ui = PDEC } },
    { AtOff,    "indices",  0 },
    { AtWord,   "indn",     { .ui = PDEC } },
    { AtSub,    "indtable", { .sv = cnsub } },
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
    { AtOff,    "data",     { .ui = DatMatAnimDd } },
    0
};
DatStructVar maddvars[] = { 
    { AtWord,   0,          0 },
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


DatStruct ddefs[] = {
    [DatNULL] =     { "Unknown",    0,  0,      PDGEN},
    [DatHdr] =      { "Header",     0,  hvars,  PDGEN},
    [DatRoot] =     { "Root",       0,  rvars,  PDROOT},
    [DatJoint] =    { "Joint",      0,  jvars,  PDGEN},
    [DatJointData] = { "JointData", 0,  jdvars, PDGEN},
    [DatMesh] =     { "Mesh",       0,  mvars,  PDGEN},
    [DatMaterial] = { "Material",   0,  mtvars, PDGEN},
    [DatColor] =    { "Color",      0,  cvars,  PDGEN},
    [DatTexture] =  { "Texture",    0,  tvars,  PDGEN},
    [DatTexUnk] =   { "TextureUnk", 0,  tuvars, PDGEN},
    [DatImage] =    { "Image",      0,  ivars,  PDGEN},
    [DatPalette] =  { "Palette",    0,  pvars,  PDGEN},
    [DatVertAttr] = { "VertAttr",   0,  vvars,  PDGEN},
    [DatDisplayList] = { "DisplayList", 0, dvars, PDGEN},
    [DatHitBoxData] = { "HitBoxData", 0, hbvars, PDGEN},
    [DatCollision] = { "Collision", 0,  cnvars, PDGEN},

    [DatMatAnimA] = { "MatAnimA",   0,  maavars, PDGEN},
    [DatMatAnimB] = { "MatAnimB",   0,  mabvars, PDGEN},
    [DatMatAnimC] = { "MatAnimC",   0,  macvars, PDGEN},
    [DatMatAnimCdh] = { "MatAnimCdh", 0, macdhvars, PDGEN},
    [DatMatAnimCd] = { "MatAnimCd", 0,  macdvars, PDGEN},
    [DatMatAnimD] = { "MatAnimD",   0,  madvars, PDGEN},
    [DatMatAnimDd] = { "MatAnimDd", 0,  maddvars, PDGEN},
    [DatMatAnimDdd] = { "MatAnimDdd", 0, madddvars, PDGEN},

    [DatLast] = { 0, 0, 0, 0 },
};

DatRootFmt dnametype[] = {
    { 0, "_animjoint", 0 },
    { 0, "_matanim_joint", DatMatAnimA },
    { 0, "_shapeanim_joint", 0 },
    { 0, "_joint", DatJoint },
    { "ftData", 0, 0 },
    { "coll_data", 0, DatCollision },
    { 0, 0, 0 },
};

static void _PDATM(DatStructVar* dvp, uint8_t* ptr) {
    char format[] = "%10d";
    if(dvp->extra.ui==PHEX) format[3]='x';
    switch(dvp->type) {
        case AtWord:  printf(format, be32toh(*(uint32_t*)ptr)); break;
        case AtHalf:  printf(format, be16toh(*(uint16_t*)ptr)); break;
        case AtByte:  printf(format, *(uint8_t*)ptr); break;
        case AtFloat: printf("%10f", be32toh(*(uint32_t*)ptr)); break;
        case AtOff:   printf("[%08x]", be32toh(*(uint32_t*)ptr)); break;
        case AtSub:   printf("%10d", dvp->extra.sv->extra.ui); break;
    }
}

static void PDGEN(DatStructVar* dvp, uint8_t* ptr, void* parent) {
    char atnames[7] = {
        [AtWord] =  'w',
        [AtHalf] =  'h',
        [AtByte] =  'b',
        [AtFloat] = 'f',
        [AtOff] =   'o',
        [AtSub] =   's',
    };
    int i;
    while(dvp->type) {
        printf("%c %8s ", atnames[dvp->type], dvp->name);
        _PDATM(dvp, ptr);
        switch(dvp->type) {
        case AtSub: 
            for(i=0; i<dvp->extra.sv->extra.ui; ++i) {
                printf("\n");
                PDGEN(dvp->extra.sv+1, ptr, 0);
                ptr+=dvp->extra.sv->type;
            }
            break;
        case AtHalf: ptr+=2; break;
        case AtByte: ptr+=1; break;
        case AtOff: if(dvp->extra.ui) printf(" %s", ddefs[dvp->extra.ui].name);
        default: ptr+=4; break;
        }
        if(dvp->type != AtSub) printf("\n");
        ++dvp;
    }
}

static void PDROOT(DatStructVar* dvp, uint8_t* ptr, void* parent) {
    char* strt;
    if(!parent) return PDGEN(dvp, ptr, 0);
    strt = parent;
    printf("off   %8s : ", dvp->name);
    _PDATM(dvp, ptr);
    printf(" %s\n", ddefs[ROOTTYPE((uint32_t*)ptr, strt)].name);
    ptr+=4; ++dvp;
    printf("word  %8s : %s\n", dvp->name, strt+be32toh(*(uint32_t*)ptr));
}

static int ROOTTYPE(uint32_t* root, char* strt) {
    int i, strl;
    char* name;
    name = strt+be32toh(root[1]);
    strl = strlen(name);
    for(i=0; dnametype[i].suf || dnametype[i].pre; ++i) {
        if(dnametype[i].pre && strcmp(dnametype[i].pre, name) != 0) continue;
        if(dnametype[i].suf && strcmp(dnametype[i].suf, name+(strl-strlen(dnametype[i].suf))) != 0) continue;
        return dnametype[i].type;
    }
    return DatNULL;
}

