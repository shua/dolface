
static TempSpec tempspecs[] = {
	{ "hdr xxxxx::: filesz bodysz reltnum rootnum xrefnum", p_hdr, 0, g_hdr },
	{ "root xx child name", p_root, 0, g_root },
	{ "joint :xppp???px flags (joint)child (joint)next (jointdata)data (xyz)rotation (xyz)scale (xyz)position (transform)transform", p_generic, 0, g_generic },
	{ "jointdata :ppp (jointdata)next (material)material (mesh)mesh", p_generic, 0, g_generic },
	{ "jointdataA x:p:pp flags (jointdataAd)data1 (jointdataAd)data2 (jointdataAd)data3", p_generic, 0, g_generic },
	{ "jointdataAd :", p_generic, 0, g_generic },
	{ "transform ???? [4](x)x [4](x)y [4](x)z [4](x)w", p_generic, n_generic, g_generic },
	{ "mesh :ppwwpp (mesh)next (vertattr)verts flags ndisplay [ndisplay](display)display [*](x)weight", p_generic, n_generic, g_generic },
	{ "material :xpp:: flags (texture)texture (color)color", p_generic, n_generic, g_generic },
	{ "color xxxf diffuse ambient specular something", p_generic, n_generic, g_generic },
	{ "texture :p::?????,:::pp:p" 
	  " (texture)next"
	  " (xyz)rotation (xyz)scale (xyz)position"
	  " (st_x)wrap (st_b)scale"
	  " (image)image (palette)palette (texunk)texunk", 
	  p_generic, n_generic, g_generic },
	{ "texunk :xxx:::: n101 n8580080f n07070707", p_generic, n_generic, g_generic },
	{ "image pwwx::: (b)data width height format", p_generic, n_generic, g_generic }, /* special */
	{ "palette px:w, (b)data format colornum", p_generic, n_generic, g_generic }, /*special*/
	{ "vertattr xxxxb.wp ident usage format type scale stride (b)data", p_generic, n_generic, g_generic }, /*special*/
	{ "display .bw primitive indices", p_generic, n_generic, g_generic },
	{ "weight pf (joint)joint weight", p_generic, n_generic, g_generic },
	{ "hitbox xwwwwxx primary scale z y x physics effects", p_generic, n_generic, g_generic },
	{ "collision pxpx?px [nvert](vertex)vertices nvert [nind](index)indices nind [5](indtab)indextable [ncoll](b)colldata ncoll", p_generic, n_generic, g_generic },
	{ "vertex :", p_generic, n_generic, g_generic },
	{ "index :", p_generic, n_generic, g_generic },
	{ "indtab ww start num", p_generic, n_generic, g_generic },

	{ "matanimA ppp (matanimA)next (matanimA)child (matanimB)maB", p_generic, n_generic, g_generic},
	{ "matanimB pppp (matanimB)next (matanimD)maD (matanimC)maC (x)something", p_generic, n_generic, g_generic },
	{ "matanimC p:ppww (matanimC)next (matanimD)maD [nimage](pimage)images [npalette](ppalette)palettes nimage npalette", p_generic, n_generic, g_generic },
	{ "matanimD :fp: c40a00000 (matanimE)maE", p_generic, n_generic, g_generic },
	{ "matanimE px:xp (matanimE)next ndata flags [ndata](b)data", p_generic, n_generic, g_generic },

	{ "mapheadA pxpxpxpxpxpx" 
	  " [nmapjoint](mapjoint)mapjoint nmapjoint"
	  " [nmhB](mapheadB)mhB nmhB"
	  " [njointdA](jointdataA)jointdata njointdA"
	  " [nmhE](mapheadE)mhE nmhE"
	  " [npmhE](pmapheadE)pmhE npmhE"
	  " [nmaterials](pmaterial)materials nmaterials", 
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
	  " [nhalf](st_w)half nhalf"
	  " (x)allornot",
	  p_generic, n_generic, g_generic },
	{ "mapheadC ::::::pp::::::"
	  " [5](f)mpCa1"
	  " [5](f)mpCa2",
	  p_generic, n_generic, g_generic },
	{ "mapheadD ::,,x:p: flags (mapheadD)self", p_generic, n_generic, g_generic },
	{ "mapheadE p (mapplitC)mpC", p_generic, n_generic, g_generic },
	{ "mapheadF ppp (mapheadF)next (mapheadF)child (mapheadG)mhG",
	  p_generic, n_generic, g_generic },
	{ "mapheadG ::", p_generic, n_generic, g_generic },
	{ "mapheadH :ppp:pp"
	  " (x)triplet1 (x)triplet2 (x)triplet3"
	  " (mapheadC)mhC (mapheadH)self",
	  p_generic, n_generic, g_generic },
	{ "mapheadQ ppp:: (mapheadQ)next (mapheadQ)child (matanimD)maD",
	  p_generic, n_generic, g_generic },
	{ "mapjoint ppx (joint)joint [nhalf](st_w)half nhalf",
	  p_generic, n_generic, g_generic },

	{ "mapplitB pp (mapplitC)mpC (mapplitD)mpD",
	  p_generic, n_generic, g_generic },
	{ "mapplitC :::,,xp:p flags [5](f)mpCa (f)mpCb",
	  p_generic, n_generic, g_generic },
	{ "mapplitD p: (mapplitE)mpE",
	  p_generic, n_generic, g_generic },
	{ "mapplitE ::::",
	  p_generic, n_generic, g_generic },

/*
	{ "", p_generic, n_generic, g_generic },
 */

	{ "pimage p (image)image", p_generic, n_generic, g_generic },
	{ "pmaterial p (material)material", p_generic, n_generic, g_generic },
	{ "ppalette p (palette)palette", p_generic, n_generic, g_generic },
	{ "pmapheadE p (mapheadE)mapheadE", p_generic, n_generic, g_generic },
	{ "pmapplitB p (mapplitB)mapplitB", p_generic, n_generic, g_generic },
	{ "xyz fff x y z", p_generic, 0, g_generic },
	{ "st_x xx s t", p_generic, n_generic, g_generic }, 
	{ "st_w ww s t", p_generic, n_generic, g_generic }, 
	{ "st_b bb s t", p_generic, n_generic, g_generic }, 
	{ "b b", p_atom, n_generic, g_generic },
	{ "w w", p_atom, n_generic, g_generic },
	{ "x x", p_atom, n_generic, g_generic },
	{ "p x", p_atom, n_generic, g_generic },
	{ "f f", p_atom, n_generic, g_generic },
	{ "c b", p_char, n_generic, g_generic },
};

static RootSpec rootspecs[] = {
    { 0, "_animjoint", 0 },
    { 0, "_matanim_joint", "matanimA" },
    { 0, "_shapeanim_joint", 0 },
    { 0, "_joint", "joint" },
    { "ftData", 0, 0 },
    { 0, "_tlut_desc", "palette" },
    { 0, "_tlut", 0 },
    { 0, "_image", 0 },
    { "coll_data", 0, "collision" },
    { "grGroundParam", 0, 0 },
    { "map_head", 0, "mapheadA" },
    { "map_plit", 0, "pmapplitB" }, /* should be a list */
    { "map_ptcl", 0, 0 },
    { "map_texg", 0, 0 },
    { "itemdata", 0, 0 },
    { "quake_model_set", 0, 0 },
    { "ALDYakuAll", 0,      0 },
    { "yakumono_param", 0,  0 },
    { 0, 0, 0 },
};

static Cmd cmds[] = {
	{ "m", "prints file map",   cmd_map , 0 },
	{ "a", "prints out assets", cmd_asset, 0 },
	{ "p", "prints dat obj",    cmd_print, 0 },
	{ "?", "prints this help",   cmd_help, 0 },
};

