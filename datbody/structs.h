
DatStructVar hvars[] = {
    { AtWord, "filesz", { .ui = PHEX } },
    { AtWord, "datasz", { .ui = PHEX } },
    { AtWord, "reltnum", 0 },
    { AtWord, "rootnum", 0 },
    { AtWord, "srootnum", 0 },
    { AtWord, 0, 0 },
    { AtWord, 0, 0 },
    { AtWord, 0, 0 },
    0
};
DatStructVar rvars[] = {
    { AtOff,  "child", { .ui = DatNULL } },
    { AtWord, "name", 0 },
    0
};
DatStructVar jvars[] = {
    { AtWord, 0, 0 },
    { AtWord, "flags", { .ui = PHEX } },
    { AtOff, "child", { .ui = DatJoint } },
    { AtOff, "next", { .ui = DatJoint } },
    { AtOff, "data", { .ui = DatJointData } },
    { AtTri, "rotation", 0 },
    { AtTri, "scale", 0 },
    { AtTri, "translation", 0 },
    { AtOff, "transform", { .ui = DatNULL } },
    { AtWord, 0, 0 },
    0
};
DatStructVar jdvars[] = {
    { AtWord, 0, 0 },
    { AtOff, "next", { .ui = DatJointData } },
    { AtOff, "material", {.ui = DatMaterial } },
    { AtOff, "mesh", {.ui = DatMesh } },
    0
};
DatStructVar mvars[] = {
    { AtWord, 0, 0 },
    { AtOff, "next", 0 },
    { AtOff, "verts", 0 },
    { AtHalf, "flags", 0 },
    { AtHalf, "displaynum", 0 },
    { AtOff, "display", { .ui = DatNULL } },
    { AtOff, "weight", { .ui = DatNULL } },
    0
};
DatStructVar mtvars[] = {
    { AtWord, 0, 0 },
    { AtWord, "flags", 0 },
    { AtOff, "texture", { .ui = DatNULL } },
    { AtOff, "color", { .ui = DatColor } },
    { AtWord, 0, 0 },
    { AtWord, 0, 0 },
    0
};
DatStructVar cvars[] = {
    { AtWord, "diffuse", 0 },
    { AtWord, "ambient", 0 },
    { AtWord, "specular", 0 },
    { AtFloat, 0, 0 },
    0
};

DatStruct ddefs[] = {
    [DatHdr] = { "Header", 0, hvars},
    [DatRoot] = { "Root", 0, rvars},
    [DatJoint] = { "Joint", 0, jvars},
    [DatJointData] = { "JointData", 0, jdvars},
    [DatMesh] = { "Mesh", 0, mvars},
    [DatMaterial] = { "Material", 0, mtvars},
    [DatColor] = { "Color", 0, cvars},
};
