
static struct {
	unsigned int filesz;
	unsigned int bodysz;
	char *strings;
} file_data;

static const struct {
	char *str;
	void (*print)(Object*,void*,Geninfo*);
	int  (*null)(Object*,void*);
	Object *(*gen)(Object*,void*,Geninfo*);
} templates[] = {
	{ "hdr xxxxx::: filesz bodysz reltnum rootnum xrefnum", p_hdr, 0, g_hdr },
	{ "root xx child name", p_root, 0, g_root },
	{ "xyz fff x y z", p_generic, 0, g_generic },
	{ "joint :xppp???xx flags (joint)child (joint)next (jointdata)data (xyz)rotation (xyz)scale (xyz)position transform", p_generic, 0, g_generic },
	{ "jointdata :pxx (jointdata)next material mesh", p_generic, 0, g_generic },
	{ "jointdataA x:p:pp flags (jointdataAd)data1 (jointdataAd)data2 (jointdataAd)data3", p_generic, 0, g_generic },
	{ "jointdataAd :", p_generic, 0, g_generic },
	{ "transform ???? [4](x)x [4](x)y [4](x)z [4](x)w", p_generic, n_generic, g_generic },
	{ "mesh :ppwwpp (mesh)next (vertattr)verts flags ndisplay [ndisplay](display)display [*](x)weight", p_generic, n_generic, g_generic },
	{ "material :xpp:: flags (texture)texture (color)color", p_generic, n_generic, g_generic },
	{ "color xxxf diffuse ambient specular something", p_generic, n_generic, g_generic },
	{ "texture :p::?????wxxxppxp (texture)next (xyz)rotation (xyz)scale (xyz)position (st_x)wrap (st_b)scale (image)image (palette)palette (texunk)texunk", p_generic, n_generic, g_generic },
	{ "st_x xx s t", p_generic, n_generic, g_generic }, 
	{ "st_b bb s t", p_generic, n_generic, g_generic }, 
	{ "texunk :xxx:::: n101 n8580080f n07070707", p_generic, n_generic, g_generic },
	{ "image pwwx::: (b)data width height format", p_generic, n_generic, g_generic }, /* special */
	{ "palette px:ww (b)data format colornum", p_generic, n_generic, g_generic }, /*special*/
	{ "vertattr xxxxb.wp ident usage format type scale stride (b)data", p_generic, n_generic, g_generic }, /*special*/
	{ "display .bw primitive indices", p_generic, n_generic, g_generic },
	{ "weight pf (joint)joint weight", p_generic, n_generic, g_generic },
	{ "hitbox xwwwwxx primary scale z y x physics effects", p_generic, n_generic, g_generic },
	{ "collision pxpx?px [nvert](vertex)vertices nvert [nind](index)indices [5](indtab)indextable [ncoll](colldata)colldata", p_generic, n_generic, g_generic },
	{ "indtab ww start num", p_generic, n_generic, g_generic },

	{ "matanimA ppp (matanimA)next (matanimA)child (matanimB)maB", p_generic, n_generic, g_generic},
	{ "matanimB pppp (matanimB)next (matanimD)maD (matanimC)maC (x)something", p_generic, n_generic, g_generic },
	{ "matanimC p:ppww (matanimC)next (matanimD)maD [nimage](pimage)images [npalette](ppalette)palettes nimage npalette", p_generic, n_generic, g_generic },
	{ "matanimD :fp: c40a00000 (matanimE)maE", p_generic, n_generic, g_generic },
	{ "matanimE pxxxp (matanimE)next ndata flags [ndata](b)data", p_generic, n_generic, g_generic },

	{ "mapheadA pxpxpxpxpxpx" 
	  " [nmjoint](mjoint)mjoint nmjoint"
	  " [nmhB](mapheadB)mhB nmhB"
	  " [njointdA](jointdataA)jointdata njointdA"
	  " [nmhE](mapheadE)mhE nmhE"
	  " [npmhE](pmapheadE)pmhE pnmhE"
	  " [npmat](pmat)pmat npmat", 
	  p_generic, n_generic, g_generic },
	{ "mapheadB pppppppppxp::"
	  " (joint)joint"
	  " (mapheadQ)mhQ"
	  " (matanimA)maA"
	  " (mapheadF)mhF"
	  " (mapheadC)mhC"
	  " (mapheadH)mhH"
	  " (mapplitB)mpB"
	  " (mapheadD)mhD"
	  " [nhalf](mapheadHalf)half nhalf"
	  "" , p_generic, n_generic, g_generic },

	{ "", p_generic, n_generic, g_generic },

	{ "pimage p (image)image", p_generic, n_generic, g_generic },
	{ "ppalette p (palette)palette", p_generic, n_generic, g_generic },
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
