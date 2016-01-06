#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "arg.h"

#define LEN(x) (sizeof(x) / sizeof(*(x)))
#define MAXOFF 0xffffffff

enum { SizeConst, SizeVar, SizeTerm }; /* array size */
enum { FWhiteBlack = 1, FInline = 1<<1, FTraverse = 1<<2 };

typedef struct Template Template;
typedef struct {
	unsigned int arrterm;
	union {
		char *type;
		Template *temp;
	};
	union {
		unsigned int tconst;
		char *tvar;
	};
} Tempinfo;

typedef struct Object Object;
typedef struct Geninfo Geninfo;
struct Template {
	char *name;
	char *format;
	char **mems;
	Tempinfo *tinfo;
	int *off;
	void (*print)(Object*,void*, Geninfo*);
	int (*null)(Object*,void*);
	Object *(*gen)(Object*,void*,Geninfo*);
	unsigned int size;
	unsigned int lmem;
	Template *next;
};

struct Object {
	unsigned int off;
	Template* temp;
	unsigned char *data;
	Object **child;
	int nchild;
	int flags;
};

struct Geninfo {
	Template *temps;
	char *strs;
	Object *hdr;
	Object *parent;
	Object **map;
	int flags;
};

typedef union {
	int i;
	unsigned int ui;
	float f;
	const void *v;
} Arg;

typedef struct {
	char *cmd;
	char *desc;
	int (*fun) (char *, Geninfo *, unsigned char *);
	Arg *arg;
} Cmd;

typedef struct {
	char *pre;
	char *suf;
	char *name;
} RootSpec;

typedef struct {
	char *str;
	void (*print)(Object*,void*,Geninfo*);
	int  (*null)(Object*,void*);
	Object *(*gen)(Object*,void*,Geninfo*);
} TempSpec;


static int cmd_map(char *args, Geninfo *gi, unsigned char *buf);
static int cmd_asset(char *args, Geninfo *gi, unsigned char *buf);
static int cmd_print(char *args, Geninfo *gi, unsigned char *buf);
static int cmd_help(char *args, Geninfo *gi, unsigned char *buf);
static void cmd_printarr(Geninfo *gi, Object *obj, int arr, unsigned char *buf);
static void cmd_printlist(Geninfo *gi, Object *obj, unsigned char *buf);

static int axtoi(const char *str);
static int c_getarr(char *str);
static Template *c_newtemp(Template *next, char *line);
static void c_parsearr(char **pb, char **pe, Template *temp, int mem);
static void c_parsememname(char **pb, char **pe, Template *temp, int mem);
static void c_parsemems(Template *temp, char *str);
static void c_parsetype(char **pb, char **pe, Template *temp, int mem, char *str);
static void c_temps(Template **tempps);
static void c_tempspecs(Template **lstp);
static void c_resolvtemps(Template *temp);
static void c_resolvmem(Template *temp, void *mem);
static void c_resolvnullnames(Template *temp);
static int c_resolvsize(Template *temps, Template *temp);
static void c_resolvtype(Template *temps, char **type);
static void d_gi(Geninfo *gi);
static void d_obj(Object *obj);
static void d_objs(Object *obj);
static void d_objrmdup(Object *obj);
static void d_temp(Template *temp);
static void d_temps(Template *temps);
static void eprintf(const char *, ...);
static Object *g_generic(Object *obj, void *data, Geninfo *gi);
static Object *g_hdr(Object *obj, void *data, Geninfo *gi);
static Object *g_root(Object *obj, void *data, Geninfo *gi);
static Template *gettemp(Template *temps, char *type);
static void i_cmdloop(Geninfo *gi, void *data);
static Object *i_findobj(Object **map, Object *obj);
static void i_printtemp(Template *temp);
static void i_printtemps(Template *temps);
static int n_generic(Object *obj, void *data);
static void p_atom(Object *obj, void *data, Geninfo *gi);
static void p_char(Object *obj, void *data, Geninfo *gi);
static void p_generic(Object *obj, void *data, Geninfo *gi);
static void p_hdr(Object *obj, void *data, Geninfo *gi);
static void p_root(Object *obj, void *data, Geninfo *gi);
static int r_array(Object *obj, Object **arr, int arrz, int arrn);
static int r_cmppobj(const void *a, const void *b);
static int r_count(Object *top);
static void r_dat(void **dst, char *file);
static Object **r_objmap(Object *top);
static void r_gentree(Geninfo *gi, void *data, int trav);
static void r_unpack(Template *temp, void *src, void *dst, int mem);
static Template *r_tempfromroot(Template *temps, char *name);
static void usage();

static int debug = 0;

#include "datbody.conf.h"

char *argv0;

int
cmd_help(char *args, Geninfo *gi, unsigned char *buf) {
	int i;
	for(i=0; i<LEN(cmds); ++i)
		printf("%s : %s\n", cmds[i].cmd, cmds[i].desc);
	return 0;
}

int
cmd_map(char *args, Geninfo *gi, unsigned char *buf) {
	Object **map = gi->map;
	while(*map) {
		printf("%x %s\n", 
		       (*map)->off,
		       (*map)->temp->name);
		map++;
	}
	return 0;
}

int
cmd_asset(char *args, Geninfo *gi, unsigned char *buf) {
	struct {
		char *temp;
		int num;
	} assets[] = {
		{ "root" , 0 },
		{ "joint", 0 },
		{ "texture", 0 },
		{ "image", 0 },
		{ "mesh", 0 },
		{ "color", 0},
	};
	Object **pobj = gi->map;
	int i;

	while(*pobj) {
		for(i=0; i<LEN(assets); ++i) {
			if(strcmp(assets[i].temp, (**pobj).temp->name)==0) {
				assets[i].num++;
				break;
			}
		}
		pobj++;
	}

	for(i=0; i<LEN(assets); ++i)
		printf("%s %d\n", assets[i].temp, assets[i].num);

	return 0;
}

int
cmd_print(char *args, Geninfo *gi, unsigned char *buf) {
	Object obj = (Object){ 0 };
	int argc = 0, arr;
	char tempname[64];
	char arrstr[10];

	argc = sscanf(args, "%x %63s %9s", &obj.off, tempname, arrstr);
	if(argc == 0) {
		printf("p OFF [TYPE] [ARR]\n");
		return 1;
	} 
	if(argc >= 2) {
		obj.temp = gettemp(gi->temps, tempname);
		if(!obj.temp)
			printf("p: Unrecognized TYPE: %s\n", tempname);
	}
	if(argc == 3) {
		arr = atoi(arrstr);
		if(!arr && arrstr[0] != '0') {
			printf("p: ARR must be a number\n");
			arr = 1;
		}
	} else
		arr = 1;


	if(arrstr[0] == '*') {
		printf("p %x %s *\n", obj.off, obj.temp->name);
		cmd_printlist(gi, &obj, buf);
	} else {
		printf("p %x %s %d\n", obj.off, (obj.temp) ? obj.temp->name : NULL, arr);
		cmd_printarr(gi, &obj, arr, buf);
	}

	putchar('\0');
	if(fflush(NULL) != 0) eprintf("fflush:");
	return 0;
}

int
axtoi(const char *str) {
	unsigned int ret, pret, i;

	for(ret = 0, i = 0; str[i]; ++i) {
		ret *= 0x10;
		switch(str[i]) {
		case 'F':
		case 'f': ++ret;
		case 'E':
		case 'e': ++ret;
		case 'D':
		case 'd': ++ret;
		case 'C':
		case 'c': ++ret;
		case 'B':
		case 'b': ++ret;
		case 'A':
		case 'a': ++ret;
		case '9': ++ret;
		case '8': ++ret;
		case '7': ++ret;
		case '6': ++ret;
		case '5': ++ret;
		case '4': ++ret;
		case '3': ++ret;
		case '2': ++ret;
		case '1': ++ret;
		case '0':
				  break;
		default:
				  return pret;
		}
		pret = ret;
	}
	return ret;
}

int
c_getarr(char *str) {
	char *e;
	int ret = SizeConst;

	if(str[0] == '*')
		return (str[1] == 0) ? SizeTerm : 0;
	e = str;
	while((*e) && *e != ']') {
		if(!isalnum(*e)) return 0;
		if((ret == SizeConst) && !isdigit(*e))
			ret = SizeVar;
		++e;
	}

	return ret;
}

Template *
c_newtemp(Template *next, char *line) {
	char* c;
	int memn = 0;
	Template *temp;

	while((*line) && isspace(*line)) ++line;
	if(!(*line)) return NULL;
	c = line;

	temp = calloc(1, sizeof(Template));
	while((*c) && !isspace(*c)) ++c;
	if(!(*c)) eprintf("invalid type definition: there's only one word");
	temp->name = calloc(1, c-line+1);
	strncpy(temp->name, line, c-line);
	while((*c) && isspace(*c)) ++c;
	if(!(*c)) eprintf("invalid format string: expected FORMAT got NULL");
	line = c;

	while((*c) && !isspace(*c)) {
		switch(*c) {
		case 'b':
		case 'w':
		case 'x':
		case 'f':
		case 'p':
		case '.':
		case ',':
		case ':':
		case '?': ++c; break;
		default:  eprintf("Invalid FORMAT character %c", *c);
		}
	}
	memn = c - line;

	temp->format = calloc(memn, sizeof(char*));
	strncpy(temp->format, line, memn);
	temp->mems = calloc(memn, sizeof(char*));
	temp->tinfo = calloc(memn, sizeof(Tempinfo));
	temp->off = calloc(memn, sizeof(int));

	c_parsemems(temp, c);

	temp->next = next;
	return temp;
}

void
c_parsearr(char **pb, char **pe, Template *temp, int mem) {
	char *b = *pb, *e = *pe;
	if(*b == '[') {
		++b; e = b;
		while((*e) && *e != ']') ++e;
		if(!(*e)) eprintf("Reached EOL while parsing array: %s (%s)", b, temp->name);
		*e = 0;

		temp->tinfo[mem].arrterm = c_getarr(b);
		switch(temp->tinfo[mem].arrterm) {
		case SizeConst: 
			temp->tinfo[mem].tconst = atoi(b);
			break;
		case SizeVar:
			temp->tinfo[mem].tvar = calloc(1, e-b+1);
			strncpy(temp->tinfo[mem].tvar, b, e-b);
			break;
		case SizeTerm:
			break;
		default:
			eprintf("Invalid array size specifier: \"%s\" (%s)", b, temp->name);
		}
		b = e+1;
		while(isspace(*b)) ++b;
	}
	*pb = b; *pe = e;
}

void
c_parsememname(char **pb, char **pe, Template *temp, int mem) {
	char *b = *pb, *e = *pe;
	if(b != e) {
		*e = 0;
		temp->mems[mem] = calloc(1, e-b+1);
		strncpy(temp->mems[mem], b, e-b);
		if(temp->lmem < (e-b))
			temp->lmem = e-b;
	}
	*pb = b; *pe = e;
}

void
c_parsemems(Template *temp, char *str) {
	char *b, *e, *names;
	int mem, last;

	/* remove leading whitespace */
	while((*str) && isspace(*str)) ++str;
	if(!(*str)) return;
	e = str;
	while(*e) ++e;
	/* remove trailing whitespace */
	do { --e; } while (isspace(*e));
	++e;

	/* allocate new string */
	names = calloc(1, e - str + 1);
	strncpy(names, str, e - str);

	for(b = names, mem = 0, last = 0;
			temp->format[mem] && *b; 
			++mem) {
		while(isspace(*b)) ++b;
		switch(temp->format[mem]) {
		case 'p': 
		case '?':
			if(last) eprintf("Cannot leave %c name empty, needs TYPE specifier (%s[%d])", temp->format[mem], temp->name, mem);
			c_parsearr(&b, &e, temp, mem);
			c_parsetype(&b, &e, temp, mem, str);
		case 'b':
		case 'w':
		case 'x':
		case 'f': 
			if(last) continue;
			e = b;
			while((*e) && !isspace(*e)) ++e;
			last = !(*e);
			c_parsememname(&b, &e, temp, mem);
			if(!last) b = e + 1;
			break;
		case '.':
		case ',':
		case ':':
			temp->mems[mem] = NULL;
			break;
		default: eprintf("Unrecognized char '%c' in format string: %s", temp->format[mem], temp->format);
		}
	}
	free(names);
}

void
c_parsetype(char **pb, char **pe, Template *temp, int mem, char *str) {
	char *b = *pb, *e = *pe;
	if(*b == '(') {
		++b; e = b;
		while((*e) && *e != ')') ++e;
	} else if(temp->format[mem] == '?')
		eprintf("Need to specify TYPE for inline (?): \"%s\" (%s)", b, temp->name);
	if(!(*e)) eprintf("Reached EOL while parsing TYPE: %s (%s)", b, temp->name);
	if(b == e) eprintf("Error parsing TYPE in line: %s (%s)", str, temp->name);
	temp->tinfo[mem].type = calloc(1, e-b+1);
	strncpy(temp->tinfo[mem].type, b, e-b);
	b = e+1;
	while(isspace(*b)) ++b;
	*pb = b; *pe = e;
}

void
c_temps(Template **tempp) {
	Template *temps = NULL;

	c_tempspecs(&temps);
	c_resolvtemps(temps);
	if(debug > 0)
		i_printtemps(temps);

	*tempp = temps;
}

void
c_tempspecs(Template **lstp) {
	Template *lst, *nlst;
	int i;

	lst = *lstp;
	for(i = 0; i < LEN(tempspecs); ++i) {
		if((nlst = c_newtemp(lst, tempspecs[i].str)) == NULL)
			continue;
		nlst->print = tempspecs[i].print;
		nlst->null = tempspecs[i].null;
		nlst->gen = tempspecs[i].gen;
		lst = nlst;
	}
	*lstp = lst;
}

void
c_resolvtemps(Template *temps) {
	Template *temp;
	Tempinfo *info;
	char *f = NULL;
	int unresolved, i;

	for(temp = temps; temp; temp = temp->next) {
		for(f = temp->format, i = 0; *f; ++f, ++i) {
			info = &(temp->tinfo[i]);
			if(*f == 'p' || *f == '?') {
				c_resolvtype(temps, &(info->type));
			}
			if(*f == 'p' && info->arrterm == SizeVar) {
				c_resolvmem(temp, &(info->tvar));
			}
		}
	}

	for(unresolved = -1, i = 0; 
			unresolved != 0; 
			unresolved = i, i = 0) {
		for(temp = temps; temp; temp = temp->next)
			i += c_resolvsize(temps, temp);
		if(unresolved && i == unresolved) {
			fprintf(stderr, "Possible circular dependancy in inline structs");
			for(temp = temps; temp; temp = temp->next) {
				if(c_resolvsize(temps, temp))
					fprintf(stderr, "%s\n", temp->name);
			}
			exit(EXIT_FAILURE);
		}
	}

	for(temp = temps; temp; temp = temp->next)
		c_resolvnullnames(temp);
}

void
c_resolvmem(Template *temp, void *mem) {
	char *f;
	int i;
	for(f = temp->format, i = 0; *f; ++f, ++i) {
		switch(*f) {
		case 'b':
		case 'w':
		case 'x':
		case '.':
		case ',':
		case ':':
			if(temp->mems[i] && strcmp(temp->mems[i], *(char**)mem)==0)
				goto found;
		default:
			continue;
		}
	}
	eprintf("Couldn't resolve array length \"%s\" to member name in %s",
			*(char**)mem, temp->name);

found:
	free(*(char**)mem);
	*(unsigned int*)mem = i;
}

void
c_resolvnullnames(Template *temp) {
	int i,d;
	for(i = temp->size, d = 1; i > 0; i >>= 4, ++d);
	for(i = 0; temp->format[i]; ++i) {
		if(temp->mems[i]) continue;
		temp->mems[i] = calloc(1, 4 + d);
		sprintf(temp->mems[i], "unk%0*x", d, temp->off[i]);
	}

	if(temp->lmem < 3+d)
		temp->lmem = 3+d;
}

int
c_resolvsize(Template *temps, Template *temp) {
	char *f;
	int size = 0, i;
	if(temp->size) return 0;

	for(f = temp->format, i = 0; *f; ++f, ++i) {
		temp->off[i] = size;
		switch(*f) {
		case 'x':
		case 'f':
		case 'p':
		case ':':
			size += 4; break;
		case 'w':
		case ',':
			size += 2; break;
		case 'b':
		case '.':
			size += 1; break;
		case '?':
			if(temp->tinfo[i].temp->size == 0) return 1;
			size += temp->tinfo[i].temp->size;
			break;
		default: eprintf("format character %c not recognized", *f);
		}
	}
	temp->size = size;
	return 0;
}

void
c_resolvtype(Template *temps, char **type) {
	Template *temp;

	for(temp = temps; temp; temp = temp->next) {
		if(strcmp(temp->name, *type)==0)
			goto found;
	}
	eprintf("Couldn't resolve TYPE \"%s\"", *type);

found:
	free(*type);
	(*(Template**)type) = temp;
}

void
d_gi(Geninfo *gi) {
	if(gi->strs) free(gi->strs);
	gi->strs = NULL;
	if(gi->temps) d_temps(gi->temps);
	gi->temps = NULL;
	if(gi->hdr) d_objs(gi->hdr);
}

void
d_obj(Object *obj) {
	if(!obj) return;
	while(obj->nchild--) {
		if(obj->child[obj->nchild]) {
			d_obj(obj->child[obj->nchild]);
			free(obj->child[obj->nchild]);
		}
	}
	free(obj->child);
	obj->child = NULL;
	if(obj->data)
		free(obj->data);
	obj->data = NULL;
}

void
d_objrmdup(Object *obj) {
	int i;
	if(!obj) return;

	obj->flags ^= FWhiteBlack;
	for(i=0; i<obj->nchild; ++i) {
		if(!obj->child[i]) continue;
		/* if flag is equal, then child has already been visited and is pointed at by something else */
		if((obj->flags & FWhiteBlack) == (obj->child[i]->flags & FWhiteBlack)) {
			obj->child[i] = NULL;
			continue;
		}
		/* otherwise visit child */
		d_objrmdup(obj->child[i]);
	}
}

void
d_objs(Object *obj) {
	d_objrmdup(obj);
	d_obj(obj);
}

void
d_temp(Template *temp) {
	int mem;

	for(mem=0; temp->format[mem]; ++mem) {
		if(temp->mems[mem])
			free(temp->mems[mem]);
	}

	free(temp->off);
	temp->off = NULL;
	free(temp->tinfo);
	temp->tinfo = NULL;
	free(temp->mems);
	temp->mems = NULL;
	free(temp->format);
	temp->format = NULL;
	free(temp->name);
	temp->name = NULL;
}

void
d_temps(Template *temps) {
	Template *next;
	while(temps) {
		next = temps->next;
		d_temp(temps);
		temps->next = NULL;
		free(temps);
		temps = next;
	}
}

void
eprintf(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if(fmt[0] != '\0' && fmt[strlen(fmt)-1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	}

	fputc('\n', stderr);
	exit(EXIT_FAILURE);
}

Object *
g_generic(Object *obj, void *data, Geninfo *gi) {
	Object *tmp;
	unsigned char *buf = data;
	int i, j;
	unsigned int ibuf;

	if(debug > 1)
		printf("g_generic %s %s %x\n", obj->temp->name, obj->temp->format, obj->off);
	for(i=0,obj->nchild=0; obj->temp->format[i]; ++i) 
		if(obj->temp->format[i] == 'p' || obj->temp->format[i] == '?') 
			++obj->nchild;
	/* need to take arrays into account */
	obj->child = calloc(obj->nchild, sizeof(Object*));

	obj->data = malloc(obj->temp->size);
	memcpy(obj->data, &buf[obj->off], obj->temp->size);

	for(i=0,j=0; obj->temp->format[i]; ++i) {
		switch(obj->temp->format[i]) {
		case 'p':
			r_unpack(obj->temp, obj->data, &ibuf, i);
			if(ibuf) {
				tmp = obj->child[j] = calloc(1, sizeof(Object));
				tmp->temp = obj->temp->tinfo[i].temp;
				tmp->off = ibuf+gi->hdr->temp->size;
				tmp->flags = obj->flags;
				if(tmp->temp->gen && obj->flags & FTraverse)
					tmp->temp->gen(tmp, data, gi);
			}
			++j;
			/* need to loop for arrays */
			break;
		case '?':
			tmp = obj->child[j] = calloc(1, sizeof(Object));
			tmp->temp = obj->temp->tinfo[i].temp;
			tmp->off = obj->temp->off[i] + obj->off;
			tmp->flags = obj->flags | FInline;
			/* don't check FTraverse, we always traverse inlines */
			if(tmp->temp->gen)
				tmp->temp->gen(tmp, data, gi);
			++j;
			/* need to loop for arrays */
			break;
		default:
			break;
		}
	}

	return obj;
}

Object *
g_hdr(Object *obj, void *data, Geninfo *gi) {
	Template *roottemp;
	Object *ret;
	unsigned char *buf = data;
	int i, reti=0;
	unsigned int filesz, bodysz, reltnum, rootnum, xrefnum, strtabsz;
	unsigned int rootoff;

	roottemp = gettemp(gi->temps, "root");

	obj->data = malloc(obj->temp->size);
	memcpy(obj->data, data, obj->temp->size);

	r_unpack(obj->temp, obj->data, &filesz, 0);
	strtabsz = filesz - obj->temp->size;
	r_unpack(obj->temp, obj->data, &bodysz, 1);
	strtabsz -= bodysz;
	r_unpack(obj->temp, obj->data, &reltnum, 2);
	strtabsz -= reltnum * 4;
	rootoff = filesz - strtabsz;
	r_unpack(obj->temp, obj->data, &rootnum, 3);
	r_unpack(obj->temp, obj->data, &xrefnum, 4);
	strtabsz -= (rootnum + xrefnum) * roottemp->size;
	gi->strs = malloc(strtabsz);
	memcpy(gi->strs, buf + filesz - strtabsz, strtabsz);

	obj->child = calloc(rootnum, sizeof(Object*));
	obj->nchild = rootnum;
	reti = rootnum;
	for(i=0; i<rootnum; ++i) {
		ret = obj->child[i] = calloc(1, sizeof(Object));
		ret->off = rootoff + i * roottemp->size;
		ret->flags = obj->flags;
		ret->temp = roottemp;
		if(ret->temp->gen && (obj->flags & FTraverse))
			ret->temp->gen(ret, data, gi);
	}

	return ret;
}

Object *
g_root(Object *obj, void *data, Geninfo *gi) {
	Object *child;
	unsigned char *buf = data;
	unsigned int offbuf, namebuf;

	if(debug > 1)
		printf("g_root %s %s %x\n", obj->temp->name, obj->temp->format, obj->off);

	obj->data = malloc(obj->temp->size);
	memcpy(obj->data, &buf[obj->off], obj->temp->size);

	r_unpack(obj->temp, obj->data, &offbuf, 0);
	r_unpack(obj->temp, obj->data, &namebuf, 1);

	obj->child = calloc(1, sizeof(Object*));
	obj->nchild = 1;

	child = obj->child[0] = calloc(1, sizeof(Object));
	child->temp = r_tempfromroot(gi->temps, &gi->strs[namebuf]);
	child->off = offbuf + gi->hdr->temp->size;
	child->flags = obj->flags;
	if(!child->temp) child->temp = gettemp(gi->temps, "x");
	
	if(child->temp->gen && (obj->flags & FTraverse))
		child->temp->gen(child, data, gi);

	return obj;
}

Template *
gettemp(Template *temps, char *type) {
	Template *temp;

	if(!type || !temps) return NULL;

	for(temp = temps; temp; temp = temp->next) {
		if(strcmp(temp->name, type)==0)
			return temp;
	}
	return NULL;
}

Object *
i_findobj(Object **map, Object *obj) {
	int num=0;
	Object **ret = 0;
	Object **beg,**end,**mid;
	while(map[num++]);
	//ret = bsearch(&obj, map, num, sizeof(Object*), r_cmppobj);

	beg = map; end = map+num;
	while(beg<end) {
		mid = beg + ((end-beg)/2);
		if((**mid).off > obj->off)
			end = mid-1;
		else
			beg = mid;
	}
	if((obj->off - (**beg).off) < (**beg).temp->size)
		ret = beg;

	return (ret) ? *ret : NULL;
}

void
i_cmdloop(Geninfo *gi, void *data) {
	char line[512];
	int i,cmdn;

	while(1) {
		printf("> ");
		if(fgets(line, 512, stdin)==0)
			return;
		if(line[0] == '\n')
			continue;

		for(i=0; i<LEN(cmds); ++i) {
			cmdn = strlen(cmds[i].cmd);
			if(strncmp(cmds[i].cmd, line, cmdn) == 0) {
				while(line[cmdn] && isspace(line[cmdn])) cmdn++;
				cmds[i].fun(line+cmdn, gi, data);
				break;
			}
		}
		if(i==LEN(cmds))
			cmd_help(NULL, gi, data);
	}
}

void
cmd_printarr(Geninfo *gi, Object *obj, int arr, unsigned char *buf) {
	Object *found;
	while(arr > 0) {
		found = i_findobj(gi->map, obj);
		if(!found || (obj->temp && found->temp != obj->temp)) {
			found = obj;
			obj->flags &= ~FTraverse;
			if(!obj->temp)
				obj->temp = gettemp(gi->temps, "x");
			obj->temp->gen(obj, buf, gi);
		}
		found->temp->print(found, buf+found->off, gi);
		if(found == obj)
			d_objs(obj);
		obj->off += found->temp->size;
		--arr;
	}
}

void
cmd_printlist(Geninfo *gi, Object *obj, unsigned char *buf) {
	Object *found;
	while(1) {
		found = i_findobj(gi->map, obj);
		if(!found || (obj->temp && found->temp != obj->temp)) {
			found = obj;
			obj->flags &= ~FTraverse;
			if(!obj->temp)
				obj->temp = gettemp(gi->temps, "x");
			obj->temp->gen(obj, buf, gi);
		}

		if(found->temp->null 
		   && found->temp->null(found, buf+found->off))
			break;

		found->temp->print(found, buf+found->off, gi);
		if(found == obj)
			d_objs(obj);
		obj->off += found->temp->size;
		if(!found->temp->null)
			break;
	}
}

void
i_printtemp(Template *temp) {
	int mem;
	printf("TYPE: %s\n", temp->name);
	for(mem=0; temp->format[mem]; ++mem) {
		printf("%04x %c %*s",
				temp->off[mem],
				temp->format[mem],
				temp->lmem,
				(temp->mems[mem]) ? temp->mems[mem] : "");

		switch(temp->format[mem]) {
		case 'p':
			if(temp->tinfo[mem].arrterm == SizeTerm)
				printf(" [*]");
			else if(temp->tinfo[mem].arrterm == SizeConst)
				printf(" [%d]", temp->tinfo[mem].tconst);
			else if(temp->tinfo[mem].arrterm == SizeVar)
				printf(" [%s]", temp->mems[temp->tinfo[mem].tconst]);
		case '?':
			printf(" (%s)", temp->tinfo[mem].temp->name);
		}
		printf(";\n");
	}
	printf("size: %d\n", temp->size);
}

void
i_printtemps(Template *temps) {
	while(temps) {
		i_printtemp(temps);
		printf("\n");
		temps = temps->next;
	}
}

int
n_generic(Object *obj, void *data) {
	Template *temp = obj->temp;
	unsigned int ibuf;
	unsigned char *buf = data;

	r_unpack(temp, buf, &ibuf, 0);
	return !ibuf;
}

void
p_atom(Object *obj, void *data, Geninfo *gi) {
	int ibuf;
	float fbuf;
	switch(obj->temp->format[0]) {
	case 'b':
	case '.':
	case 'w':
	case ',':
	case 'x':
	case ':':
		r_unpack(obj->temp, data, &ibuf, 0);
		printf("%x", ibuf);
		break;
	case 'f':
		r_unpack(obj->temp, data, &fbuf, 0);
		printf("%f", fbuf);
		break;
	case '?':
		printf("%08x %s 1", 
				obj->off + obj->temp->off[0], 
				obj->temp->tinfo[0].temp->name);
		break;
	case 'p':
		r_unpack(obj->temp, data, &ibuf, 0);
		printf("%08x %s ",
				(ibuf) ? ibuf + 0x20 : 0,
				obj->temp->tinfo[0].temp->name);
		switch(obj->temp->tinfo[0].arrterm) {
		case SizeConst:
			if(obj->temp->tinfo[0].tconst)
				printf("%d", obj->temp->tinfo[0].tconst);
			break;
		case SizeVar:
			r_unpack(obj->temp, data, &ibuf, obj->temp->tinfo[0].tconst);
			printf("%d", ibuf);
			break;
		case SizeTerm:
			printf("*");
			break;
		}
		break;
	}
	printf("\n");
}

void
p_char(Object *obj, void *data, Geninfo *gi) {
	if(*(char*)data == 0)
		printf("\n");
	else
		printf("%c", *(char*)data);
	fflush(NULL);
}

void 
p_generic(Object *obj, void *data, Geninfo *gi) {
	Template *temp = obj->temp;
	unsigned int off = obj->off;
	char *f, *fmt = temp->format;
	unsigned int ibuf;
	float fbuf;
	int i;

	for(f = fmt, i = 0; *f; ++f, ++i) {
		printf("%08x %c %*s : ", 
		       off + temp->off[i], 
		       *f, 
		       temp->lmem, 
		       temp->mems[i]);

		switch(*f) {
		case 'b':
		case '.':
		case 'w':
		case ',':
		case 'x':
		case ':':
			r_unpack(temp, data, &ibuf, i);
			printf("%x", ibuf);
			break;
		case 'f':
			r_unpack(temp, data, &fbuf, i);
			printf("%f", fbuf);
			break;
		case '?':
			printf("%08x %s 1", 
					off + temp->off[i], 
					temp->tinfo[i].temp->name);
			break;
		case 'p':
			r_unpack(temp, data, &ibuf, i);
			printf("%08x %s ",
					(ibuf) ? ibuf + 0x20 : 0,
					temp->tinfo[i].temp->name);
			switch(temp->tinfo[i].arrterm) {
			case SizeConst:
				if(temp->tinfo[i].tconst)
					printf("%d", temp->tinfo[i].tconst);
				break;
			case SizeVar:
				r_unpack(temp, data, &ibuf, temp->tinfo[i].tconst);
				printf("%d", ibuf);
				break;
			case SizeTerm:
				printf("*");
				break;
			}
			break;
		}
		printf("\n");
	}
}

void 
p_hdr(Object *obj, void *data, Geninfo *gi) {
	Template *temp = obj->temp;
	unsigned int off = obj->off;
	char *f;
	unsigned int ibuf;
	int hdrsz = 0x20, rootsz = 0x8, offsz = 0x4, filesz;
	int relt, roots, xrefs, strings;
	int i;

	for(f = temp->format, i = 0; *f; ++f, ++i) {
		printf("%08x %c %*s : ", 
		       off + temp->off[i], 
		       *f, 
		       temp->lmem, 
		       temp->mems[i]);
		r_unpack(temp, data, &ibuf, i);
		printf("%x", ibuf);
		printf("\n");

		switch(i) {
		case 0:
			filesz = ibuf;
			break;
		case 1:
			relt = hdrsz + ibuf;
			break;
		case 2: 
			printf("         p %*s : %08x p %d\n", temp->lmem, "reltbl", relt, ibuf);
			roots = relt + ibuf*offsz;
			break;
		case 3:
			printf("         p %*s : %08x root %d\n", temp->lmem, "rootarr", roots, ibuf);
			xrefs = roots + ibuf*rootsz;
			break;
		case 4:
			printf("         p %*s : %08x root %d\n", temp->lmem, "xrefarr", xrefs, ibuf);
			strings = xrefs + ibuf*rootsz;
			break;
		}
	}
	printf("         p %*s : %08x c %d\n", temp->lmem, "strings", strings, filesz - strings);
	if(fflush(NULL) != 0) eprintf("fflush:");
}

void
p_root(Object *obj, void *data, Geninfo *gi) {
	unsigned int ibuf;
	
	printf("%08x p %*s : ", 
	       obj->off,
	       obj->temp->lmem,
	       obj->temp->mems[0]);
	r_unpack(obj->temp, obj->data, &ibuf, 0);

	printf("%x %s\n", ibuf + gi->hdr->temp->size, obj->child[0]->temp->name);
	printf("%08x x %*s : ", 
	       obj->off,
	       obj->temp->lmem,
	       obj->temp->mems[1]);
	r_unpack(obj->temp, obj->data, &ibuf, 1);
	printf("%d\n", ibuf);
	printf("           %*s : %s\n",
	       obj->temp->lmem,
	       "str",
	       &gi->strs[ibuf]);
}

int
r_array(Object *obj, Object **arr, int arrz, int arrn) {
	int i;
	obj->flags ^= FWhiteBlack;
	arr[arrn] = obj;
	++arrn;
	for (i = 0; i<obj->nchild; ++i) {
		if(obj->child[i] && (obj->child[i]->flags ^ obj->flags) & FWhiteBlack)
			arrn = r_array(obj->child[i], arr, arrz, arrn);
	}
	return arrn;
}

int
r_cmppobj(const void *a, const void *b) {
	return (*(Object**)a)->off - (*(Object**)b)->off;
}

int
r_count(Object *top) {
	int num = 0;
	int i;
	top->flags ^= FWhiteBlack;
	++num;
	for(i = 0; i<top->nchild; ++i) {
		if(top->child[i] && (top->child[i]->flags ^ top->flags & FWhiteBlack) 
				& FWhiteBlack)
			num += r_count(top->child[i]);
	}
	return num;
}

void
r_dat(void **dst, char *file) {
	int fd, filesz, hdrsz = 0x20;
	unsigned char *buf;

	if((fd = open(file, O_RDONLY)) == -1) {
		fprintf(stderr, "Can't open file %s\n", file);
		return;
	}

	buf = calloc(1, hdrsz);
	if(read(fd, buf, hdrsz) <= 0) {
		fprintf(stderr, "Read error\n");
		free(buf);
		return;
	}

	filesz = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
	buf = realloc(buf, filesz);
	if(!buf) eprintf("memory");

	if(read(fd, buf+hdrsz, filesz-hdrsz) <= 0) {
		fprintf(stderr, "Read error\n");
		free(buf);
		return;
	}

	close(fd);
	*dst = buf;
}

Object **
r_objmap(Object *top) {
	int num;
	Object **ret = NULL;
	num = r_count(top);
	ret = calloc(sizeof(Object*),num+1);
	r_array(top, ret, num, 0);
	qsort(ret, num, sizeof(Object*), r_cmppobj);
	return ret;
}

void
r_gentree(Geninfo *gi, void *data, int trav) {
	Object *obj;
	Template *hdrtemp;

	if(debug > 1)
		printf("r_gentree %s\n", gi->temps[0].name);
	hdrtemp = gettemp(gi->temps, "hdr");
	gi->hdr = obj = calloc(1, sizeof(Object));
	obj->off = 0;
	obj->temp = hdrtemp;
	obj->flags |= (!!trav) * FTraverse;
	hdrtemp->gen(obj, data, gi);
	
}

void
r_unpack(Template *temp, void *src, void *dst, int mem) {
	unsigned char *data = src;
	unsigned int ibuf;

	data += temp->off[mem];

	switch(temp->format[mem]) {
	case '.':
	case 'b':
		ibuf = data[0];
		*(unsigned int*)dst = ibuf;
		break;
	case ',':
	case 'w':
		ibuf = data[0] << 8 | data[1];
		*(unsigned int*)dst = ibuf;
		break;
	case ':':
	case 'x':
	case 'f':
	case 'p':
		ibuf = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
		*(unsigned int*)dst = ibuf;
		break;
	case '?':
		memcpy(dst, data, 
				((temp->format[mem+1]) ? temp->off[mem+1] : temp->size) - temp->off[mem]);
	}
}

Template *
r_tempfromroot(Template *temps, char *name) {
	int i;
	int namen;
	int checkn;
	
	namen = strlen(name);

	if(debug > 0)
		printf("trying to match %s\n", name);
	for(i=0; i<LEN(rootspecs); ++i) {
		if(rootspecs[i].pre) {
			checkn = strlen(rootspecs[i].pre);
			if(strncmp(rootspecs[i].pre, name, checkn) != 0)
				continue;
		}

		if(rootspecs[i].suf) {
			checkn = strlen(rootspecs[i].suf);
			if(strncmp(rootspecs[i].suf, name + (namen - checkn), checkn) != 0)
				continue;
		}

		return gettemp(temps, rootspecs[i].name);
	}

	return NULL;
}

void
usage() {
	printf("usage: datbody DATFILE\n");
}

int
main(int argc, char *argv[]) {
	unsigned char *buf = NULL;
	Geninfo gi = { 0 };
	int gen = 1;

	ARGBEGIN {
	case 'v': ++debug;
	case 'g': gen = 0;
	} ARGEND;

	if(argc < 1) {
		usage();
		exit(EXIT_SUCCESS);
	}

	c_temps(&gi.temps);
	r_dat((void**)&buf, argv[0]);
	
	r_gentree(&gi, buf, gen);
	gi.map = r_objmap(gi.hdr);

	i_cmdloop(&gi, buf);
	printf("Goodbye\n");

	if(buf) free(buf);
	d_gi(&gi);
	free(gi.hdr);
	return 0;
}
