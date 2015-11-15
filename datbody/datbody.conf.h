
static struct {
	unsigned int filesz;
	unsigned int bodysz;
	char *strings;
} file_data;

static const struct {
	char *str;
	void (*print)(Defo*,void*);
	int  (*null)(Defo*,void*);
	Defo *(*gen)(Defo*,void*,void*);
} protos[] = {
	{ "hdr xxxxxxxx filesz bodysz reltnum rootnum xrefnum", p_hdr, 0, g_hdr },
	{ "root xx child name", p_generic, 0, g_generic },
	{ "xyz fff x y z", p_generic, 0, g_generic },
	{ "joint :xppp???xx flags (joint)child (joint)next (jointdata)data (xyz)rotation (xyz)scale (xyz)position transform", p_generic, 0, g_generic },
	{ "jointdata :pxx (jointdata)next material mesh", p_generic, 0, g_generic },
	{ "b b", p_generic, n_generic, g_generic },
	{ "w w", p_generic, n_generic, g_generic },
	{ "x x", p_generic, n_generic, g_generic },
	{ "p x", p_generic, n_generic, g_generic },
	{ "f f", p_generic, n_generic, g_generic },
	{ "c b", p_char, n_generic, g_generic },
};

struct {
	char *pre;
	char *suf;
	char *name;
} rootnames[] = {
    { 0, "_animjoint", 0 },
    { 0, "_matanim_joint", 0 },
    { 0, "_shapeanim_joint", 0 },
    { 0, "_joint", "joint" },
    { "ftData", 0, 0 },
    { 0, "_tlut_desc", 0 },
    { 0, "_tlut", 0 },
    { 0, "_image", 0 },
    { "coll_data", 0, 0 },
    { "grGroundParam", 0, 0 },
    { "map_head", 0, 0 },
    { "map_plit", 0, 0 },
    { "map_ptcl", 0, 0 },
    { "map_texg", 0, 0 },
    { "itemdata", 0, 0 },
    { "quake_model_set", 0, 0 },
    { "ALDYakuAll", 0,      0 },
    { "yakumono_param", 0,  0 },
    { 0, 0, 0 },
};
