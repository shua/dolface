
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
	{ "hdr xxxxx::: filesz bodysz reltnum rootnum xrefnum", 
	  p_hdr, 0, g_hdr },
	{ "root xx child name", 
	  p_root, 0, g_root },
	{ "joint :xppp???xx"
	  " flags (joint)child (joint)next (jointdata)data"
	  " (xyz)rotation (xyz)scale (xyz)position transform", 
	  p_generic, n_generic, g_generic },
	{ "jointdata :ppp (jointdata)next (material)material (mesh)mesh", 
	  p_generic, n_generic, g_generic },
	{ "jointdataA x:p:pp flags (jointdataAd)data1 (jointdataAd)data2 (jointdataAd)data3", 
	  p_generic, n_generic, g_generic },
	{ "jointdataAd :", 
	  p_generic, n_generic, g_generic },
	{ "transform ???? [4](x)x [4](x)y [4](x)z [4](x)w", 
	  p_generic, n_generic, g_generic },
	{ "mesh :ppwwpp"
	  " (mesh)next (vertattr)verts flags"
	  " ndisplay [ndisplay](display)display [*](x)weight", 
	  p_generic, n_generic, g_generic },
	{ "material :xpp:: flags (texture)texture (color)color", 
	  p_generic, n_generic, g_generic },
	{ "color xxxf diffuse ambient specular something", 
	  p_generic, n_generic, g_generic },
	{ "texture :p::?????..:::pp:p"
	  " (texture)next (xyz)rotation (xyz)scale (xyz)position"
	  " (xst)wrap (bst)scale (image)image (palette)palette (texunk)texunk", 
	  p_generic, n_generic, g_generic },
	{ "texunk :xxx:::: c101 c8580080f c07070707", 
	  p_generic, n_generic, g_generic },
	{ "image pwwx::: (b)data width height format", 
	  p_generic, n_generic, g_generic }, /*special data*/
	{ "palette px:ww (b)data format colornum", 
	  p_generic, n_generic, g_generic }, /*special data*/
	{ "vertattr xxxxb.wp ident usage format type scale stride (b)data", 
	  p_generic, n_generic, g_generic }, /*special data*/
	{ "display .bw primitive indices", 
	  p_generic, n_generic, g_generic },
	{ "weight pf (joint)joint weight", 
	  p_generic, n_generic, g_generic },
	{ "hitbox xwwwwxx primary scale z y x physics effects", 
	  p_generic, n_generic, g_generic },
	{ "collision pxpx?px"
	  " [nvert](vertex)vertices nvert [nind](index)indices nind"
	  " [5](indtab)indextable [ncoll](b)colldata ncoll", 
	  p_generic, n_generic, g_generic },
	{ "indtab ww start num", 
	  p_generic, n_generic, g_generic },
	{ "vertex :",
	  p_generic, n_generic, g_generic },
	{ "index :",
	  p_generic, n_generic, g_generic },

	{ "matanimA ppp (matanimA)next (matanimA)child (matanimB)maB", 
	  p_generic, n_generic, g_generic},
	{ "matanimB pppp (matanimB)next (matanimD)maD (matanimC)maC (x)something", 
	  p_generic, n_generic, g_generic },
	{ "matanimC p:ppww"
	  " (matanimC)next (matanimD)maD"
	  " [nimage](pimage)images [npalette](ppalette)palettes"
	  " nimage npalette", 
	  p_generic, n_generic, g_generic },
	{ "matanimD :fp: c40a00000 (matanimE)maE", 
	  p_generic, n_generic, g_generic },
	{ "matanimE px:xp (matanimE)next ndata flags [ndata](b)data", 
	  p_generic, n_generic, g_generic },

	{ "mapheadA pxpxpxpxpxpx" 
	  " [nmjoint](mapheadjoint)mjoint    nmjoint"
	  " [nmhB](mapheadB)mhB              nmhB"
	  " [njointdA](pjointdataA)jointdata njointdA"
	  " [nmhE](mapheadE)mhE              nmhE"
	  " [nmhE](pmapplitE)pmhE            nmhE"
	  " [nmat](pmaterial)material        nmat", 
	  p_generic, n_generic, g_generic },
	{ "mapheadB pppppppppxp::"
	  " (joint)joint"
	  /* these three should be [*] but mapheadQ's are weird */
	  " (pmapheadQ)mhQ (pmatanimA)maA (pmapheadF)mhF"
	  " (mapheadC)mhC  (mapheadH)mhH [*](pmapplitB)mpB (mapheadD)mhD"
	  " [nhalf](wst)half nhalf"
	  " (x)allornot" , 
	  p_generic, n_generic, g_generic },
	{ "mapheadC ::::::pp:::::: (mapplitCa)plitCa1 (mapplitCa)plitCa2", 
	  p_generic, n_generic, g_generic },
	{ "mapheadD ::....x:p: flags (mapheadD)self", 
	  p_generic, n_generic, g_generic },
	{ "mapheadE p (mapplitC)plitC", 
	  p_generic, n_generic, g_generic },
	{ "mapheadF ppp (mapheadF)next (mapheadF)child (mapheadG)mhG", 
	  p_generic, n_generic, g_generic },
	{ "mapheadG ::", 
	  p_generic, n_generic, g_generic },
	{ "mapheadH :ppp:pp"
	  " (x)triple1 (x)triple2 (x)triple3"
	  " (mapheadC)mhC (mapheadH)self", 
	  p_generic, n_generic, g_generic },
	{ "mapheadQ ppp:: (mapheadQ)next (mapheadQ)child (matanimD)maD",
	  p_generic, n_generic, g_generic },
	{ "mapheadjoint ppx (joint)joint [nhalf](wst)half nhalf", 
	  p_generic, n_generic, g_generic },
	{ "mapdataftr pppp (joint)joint (pmapheadQ)mhQl (pmatanimA)maA (pmapheadF)mhF", 
	  p_generic, n_generic, g_generic },

	{ "mapplitB pp (mapplitC)mpC (mapplitD)mpD", 
	  p_generic, n_generic, g_generic },
	{ "mapplitC :::xp:p flags (mapplitCa)mpCa (f)mpCb", 
	  p_generic, n_generic, g_generic },
	{ "mapplitCa ? [5](f)f",
	  p_generic, n_generic, g_generic },
	{ "mapplitD p: (mapplitE)mpE", 
	  p_generic, n_generic, g_generic },
	{ "mapplitE ::::", 
	  p_generic, n_generic, g_generic },

	{ "quakems ppppp (quakemsA)qmsA (pquakems)qms (quakemsB)qmsB (x)qmsD (x)qmsE", 
	  p_generic, n_generic, g_generic },
	{ "quakemsA :", 
	  p_generic, n_generic, g_generic },
	{ "quakemsB :xp: flags (quakemsC)qmsC", 
	  p_generic, n_generic, g_generic },
	{ "quakemsC p:::p (quakemsC)next (b)data", 
	  p_generic, n_generic, g_generic },

	{ "grparam f:::::fffff::::ffff::ffff?px:::::::::"
	  " a  b c d e f  g h i j  k l m n  [19](wst)sub [ngpA](grparamA)gpA ngpA", 
	  p_generic, n_generic, g_generic },
	{ "grparamA ::::? [20](wst)sub", p_generic, n_generic, g_generic },

	{ "itemdata xp type (x)idA", p_generic, n_generic, g_generic },

	{ "pimage p (image)p", p_generic, n_generic, g_generic },
	{ "ppalette p (palette)p", p_generic, n_generic, g_generic },
	{ "pjointdataA p (jointdataA)p", p_generic, n_generic, g_generic },
	{ "pmapplitE p: (mapplitE)p", p_generic, n_generic, g_generic },
	{ "pmaterial p (material)p", p_generic, n_generic, g_generic },
	{ "pmapheadQ p (mapheadQ)p", p_generic, n_generic, g_generic },
	{ "pmatanimA p (matanimA)p", p_generic, n_generic, g_generic },
	{ "pmapheadF p (mapheadF)p", p_generic, n_generic, g_generic },
	{ "pmapplitB p (mapplitB)p", p_generic, n_generic, g_generic },
	{ "pquakems p (quakems)p", p_generic, n_generic, g_generic },

	{ "xst xx s t", p_generic, n_generic, g_generic }, 
	{ "wst ww s t", p_generic, n_generic, g_generic }, 
	{ "bst bb s t", p_generic, n_generic, g_generic }, 
	{ "xyz fff x y z", p_generic, 0, g_generic },
	{ "b b", p_generic, n_generic, g_generic },
	{ "w w", p_generic, n_generic, g_generic },
	{ "x x", p_generic, n_generic, g_generic },
	{ "p x", p_generic, n_generic, g_generic },
	{ "f f", p_generic, n_generic, g_generic },
	{ "c b", p_char, n_generic, g_generic },
/*
	{ "", p_generic, n_generic, g_generic },
*/
};

struct {
	char *pre;
	char *suf;
	char *name;
} rootnames[] = {
    { 0, "_animjoint",       0 },
    { 0, "_matanim_joint",   "matanimA" },
    { 0, "_shapeanim_joint", 0 },
    { 0, "_joint",           "joint" },
    { "ftData", 0,           0 },
    { 0, "_tlut_desc",       "palette" },
    { 0, "_tlut",            0 },
    { 0, "_image",           0 },
    { "coll_data", 0,        "collision" },
    { "grGroundParam", 0,    "grparam" },
    { "map_head", 0,         "mapheadA" },
    { "map_plit", 0,         "pmapplitB" },
    { "map_ptcl", 0,         0 },
    { "map_texg", 0,         0 },
    { "itemdata", 0,         "pitemdata" },
    { "quake_model_set", 0,  "quakems" },
    { "ALDYakuAll", 0,       0 },
    { "yakumono_param", 0,   0 },
    { 0, 0, 0 },
};
