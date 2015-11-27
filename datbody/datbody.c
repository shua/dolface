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
	int flags;
};

static int r_objsappend(Object **arrp, size_t arrn, Object *app, size_t appn);
static int r_objslen(Object *objs);
static Template *r_tempfromroot(Template *temps, char *name);
static Object *g_hdr(Object *obj, void *data, Geninfo *gi);
static Object *g_root(Object *obj, void *data, Geninfo *gi);
static Object *g_generic(Object *obj, void *data, Geninfo *gi);
static void p_root(Object *obj, void *data, Geninfo *gi);
static void r_gentree(Geninfo *gi, void *data);
static Object *i_findobj(Object *objs, unsigned int off);
static Object *i_treeorlocal(Object *local, void *data, Geninfo *gi);
static void d_objs(Object *obj);
static void d_obj(Object *obj);
static void d_objrmdup(Object *obj);
static void d_gi(Geninfo *gi);

static int axtoi(const char *str);
static int c_getarr(char *str);
static void c_parsemems(Template *temp, char *str);
static void c_tempsfromtemplates(Template **lstp);
static void c_temps(Template **tempps);
static Template *c_newtemp(Template *next, char *line);
static void c_resolvtemps(Template *temp);
static void c_resolvmem(Template *temp, int mem);
static void c_resolvnullnames(Template *temp);
static int c_resolvsize(Template *temps, Template *temp);
static void c_resolvtype(Template *temps, char **type);
static void d_temp(Template *temp);
static void d_temps(Template *temps);
static void eprintf(const char *, ...);
static Template *gettemp(Template *temps, char *type);
static void i_cmdloop(Object *objs, Geninfo *gi, void *data);
static void i_printtemp(Template *temp);
static void i_printtemps(Template *temps);
static int  i_scanarr(unsigned int *arrp, char *line);
static int  i_scanoff(unsigned int *offp, char *line);
static int  i_scantype(Template *temps, Template **tempp, char *line);
static int  n_generic(Object *obj, void *data);
static void p_generic(Object *obj, void *data, Geninfo *gi);
static void p_hdr(Object *obj, void *data, Geninfo *gi);
static void p_char(Object *obj, void *data, Geninfo *gi);
static void r_dat(void **dst, char *file);
static void r_unpack(Template *temp, void *src, void *dst, int mem);
static void usage();

static int debug = 0;

#include "datbody.conf.h"

char *argv0;

Object *
i_treeorlocal(Object *local, void *data, Geninfo *gi) {
	Object *found;

	found = i_findobj(gi->hdr, local->off);
	if(!found && !local->temp) {
		local->temp = gettemp(gi->temps, "x");
		return local;
	}
	if(!found || (local->temp && found->temp != local->temp)) {
		local->flags &= ~FTraverse;
		local->temp->gen(local, data, gi);
		return local;
	}
	return found;
}

void
d_gi(Geninfo *gi) {
	if(gi->strs) free(gi->strs);
	gi->strs = NULL;
}

void
d_objs(Object *obj) {
	d_objrmdup(obj);
	d_obj(obj);
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
d_obj(Object *obj) {
	if(!obj) return;
	while(obj->child && obj->nchild--) {
		if(obj->child[obj->nchild]) {
			d_obj(obj->child[obj->nchild]);
			free(obj->child[obj->nchild]);
		}
	}
	if(obj->child)
		free(obj->child);
	obj->child = NULL;
	if(obj->data)
		free(obj->data);
	obj->data = NULL;
}

void
r_gentree(Geninfo *gi, void *data) {
	Object *obj;
	Template *hdrtemp;

	hdrtemp = gettemp(gi->temps, "hdr");
	gi->hdr = obj = calloc(1, sizeof(Object));
	obj->off = 0;
	obj->temp = hdrtemp;
	obj->flags |= FTraverse;
	hdrtemp->gen(obj, data, gi);
	
}

int
r_objslen(Object *objs) {
	int ret = 0;
	if(!objs) return 0;
	while(objs[ret].temp) ++ret;
	return ret;
}

int
r_objsappend(Object **arrp, size_t arrn, Object *app, size_t appn) {
	Object *arr = *arrp;

	if(!app || !app || !appn) return arrn;

	if(!(arr = realloc(arr, (arrn + appn + 1) * sizeof(Object)))) {
		eprintf("realloc Object array");
		return 0;
	}

	memcpy(&arr[arrn], app, appn * sizeof(Object));
	arr[arrn + appn].temp = NULL;
	*arrp = arr;
	return arrn + appn;
}

Object *
g_root(Object *obj, void *data, Geninfo *gi) {
	Object *child;
	unsigned char *buf = data;
	unsigned int offbuf, namebuf;

	if(debug) printf("g_root %s %s %x\n", obj->temp->name, obj->temp->format, obj->off);
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

int
getsize(Object *obj, int mem) {
	int a;

	switch(obj->temp->tinfo[mem].arrterm) {
	case SizeConst: 
		return (obj->temp->tinfo[mem].tconst) ? obj->temp->tinfo[mem].tconst : 1;
	case SizeVar:
		r_unpack(obj->temp, obj->data, &a, obj->temp->tinfo[mem].tconst);
		return a;
		break;
	case SizeTerm:
/*		printf("TODO null terminated lists as children\n"); */
	default:
		return 1;
	}
}

Object *
g_generic(Object *obj, void *data, Geninfo *gi) {
	Object *tmp;
	unsigned char *buf = data;
	int i, j, a;
	unsigned int ibuf;

	if(debug) printf("g_generic %s %s %x\n", obj->temp->name, obj->temp->format, obj->off);
	obj->data = malloc(obj->temp->size);
	memcpy(obj->data, &buf[obj->off], obj->temp->size);

	for(i=0,obj->nchild=0; obj->temp->format[i]; ++i)
		if(obj->temp->format[i] == 'p' || obj->temp->format[i] == '?')
			obj->nchild += getsize(obj, i);
	obj->child = calloc(obj->nchild, sizeof(Object*));

	for(i=0,j=0; obj->temp->format[i]; ++i) {
		switch(obj->temp->format[i]) {
		case 'p':
			r_unpack(obj->temp, obj->data, &ibuf, i);
			if(ibuf) {
				ibuf += gi->hdr->temp->size;
				a = getsize(obj, i);
				while(a-- > 0) {
					tmp = obj->child[j] = calloc(1, sizeof(Object));
					tmp->temp = obj->temp->tinfo[i].temp;
					tmp->off = ibuf;
					tmp->temp->gen(tmp, data, gi);
					ibuf += tmp->temp->size;
					++j;
				}
			}
			break;
		case '?':
			ibuf = obj->temp->off[i] + obj->off;
			a = getsize(obj, i);
			while(a-- > 0) {
				tmp = obj->child[j] = calloc(1, sizeof(Object));
				tmp->temp = obj->temp->tinfo[i].temp;
				tmp->off = ibuf;
				tmp->flags = obj->flags | FInline;
				/* don't check FTraverse, we always traverse inlines */
				if(tmp->temp->gen)
					tmp->temp->gen(tmp, data, gi);
				ibuf += tmp->temp->size;
				++j;
			}
			break;
		default:
			break;
		}
	}

	return obj;
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

Template *
r_tempfromroot(Template *temps, char *name) {
	int i;
	int namen;
	int checkn;
	
	namen = strlen(name);
/*	printf("trying to match %s\n", name); */
	for(i=0; i<LEN(rootnames); ++i) {
		if(!rootnames[i].name) continue;

		if(rootnames[i].pre) {
			checkn = strlen(rootnames[i].pre);
			if(strncmp(rootnames[i].pre, name, checkn) != 0)
				continue;
		}

		if(rootnames[i].suf) {
			checkn = strlen(rootnames[i].suf);
			if(strncmp(rootnames[i].suf, name + (namen - checkn), checkn) != 0)
				continue;
		}

		return gettemp(temps, rootnames[i].name);
	}

	return NULL;
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

void
c_parsemems(Template *temp, char *str) {
	char *b, *e, *names, *f;
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

	for(b = names, f = temp->format, mem = 0, last = 0;
			*f && *b; 
			++mem, ++f) {
		while(isspace(*b)) ++b;
		switch(*f) {
		case 'p': 
		case '?':
			if(*b == '[') {
				++b; e = b;
				while((*e) && *e != ']') ++e;
				if(!(*e)) eprintf("Reached EOL while parsing array: %s", b);
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
					eprintf("Invalid array size specifier: \"%s\"", b);
				}
				b = e+1;
				while(isspace(*b)) ++b;
			}
			if(last) eprintf("Cannot leave %c name empty, needs TYPE specifier (at %s[%d])", *f, temp->name, mem);
			if(*b == '(') {
				++b; e = b;
				while((*e) && *e != ')') ++e;
			} else if(*f == '?')
				eprintf("Need to specify TYPE for inline (?): \"%s\"", b);
			if(!(*e)) eprintf("Reached EOL while parsing TYPE: %s", b);
			if(b == e) eprintf("Error parsing TYPE in line: %s", str);
			temp->tinfo[mem].type = calloc(1, e-b+1);
			strncpy(temp->tinfo[mem].type, b, e-b);
			b = e+1;
			while(isspace(*b)) ++b;
		case 'b':
		case 'w':
		case 'x':
		case 'f': 
			if(last) continue;
			e = b;
			while((*e) && !isspace(*e)) ++e;
			last = !(*e);
			if(b != e) {
				*e = 0;
				temp->mems[mem] = calloc(1, e-b+1);
				strncpy(temp->mems[mem], b, e-b);
				if(temp->lmem < (e-b))
					temp->lmem = e-b;
			}
			if(!last) b = e + 1;
			break;
		case '.':
		case ':':
			temp->mems[mem] = NULL;
			break;
		default: eprintf("Unrecognized char in format string: %s", temp->format);
		}
	}
	free(names);
}

void
c_tempsfromtemplates(Template **lstp) {
	Template *lst, *nlst;
	int i;

	lst = *lstp;
	for(i = 0; i < LEN(templates); ++i) {
		if((nlst = c_newtemp(lst, templates[i].str)) == NULL)
			continue;
		nlst->print = templates[i].print;
		nlst->null = templates[i].null;
		nlst->gen = templates[i].gen;
		lst = nlst;
	}
	*lstp = lst;
}

void
c_temps(Template **tempp) {
	Template *temps = NULL;

	c_tempsfromtemplates(&temps);
	c_resolvtemps(temps);
	if(debug > 0)
		i_printtemps(temps);

	*tempp = temps;
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
		case ':':
		case '?': ++c; break;
		default:  eprintf("Invalid FORMAT character %c in %s", *c, temp->name);
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
				c_resolvmem(temp, i);
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
c_resolvmem(Template *temp, int mem) {
	int i;

	for(i=0; temp->format[i]; ++i) {
		switch(temp->format[i]) {
		case 'b':
		case 'w':
		case 'x':
			if(strcmp(temp->mems[i], temp->tinfo[mem].tvar)==0)
				goto found;
		default:
			continue;
		}
	}
	eprintf("Couldn't resolve array length \"%s\" to member name in %s",
			temp->tinfo[mem].tvar, temp->name);

found:
	free(temp->tinfo[mem].tvar);
	temp->tinfo[mem].tconst = i;
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

Template *
gettemp(Template *temps, char *type) {
	Template *temp;

	for(temp = temps; temp; temp = temp->next) {
		if(strcmp(temp->name, type)==0)
			return temp;
	}
	return NULL;
}

int
i_skipws(char *dst) {
	int r;
	char c = 0;
	while((r = read(STDIN_FILENO, &c, 1)) && isspace(c) && c != '\n');
	if(dst) *dst = c;
	return r;
}

int
i_readstr(char *dst, int max) {
	char c;
	int r;
	while((r = read(STDIN_FILENO, &c, 1)) && !isspace(c) && max--)
		if(dst) {
			*dst = c;
			++dst;
		}
	if(c == '\n') r = 0;
	*dst = 0;
	return r;
}

Object *
i_findobj(Object *objs, unsigned int off) {
	int i;
	Object *found;
	if(!objs) return NULL;
	if(objs->off == off)
		return objs;
	for(i=0; i<objs->nchild; ++i)
		if((found = i_findobj(objs->child[i], off)))
			return found;
	return NULL;
}

void
i_cmdloop(Object *objs, Geninfo *gi, void *data) {
	char line[512];
	unsigned char *buf = data;
	int rread;
	unsigned int arr;
	Object obj, *found;
	Template *xtemp;

	xtemp = gettemp(gi->temps, "x");
	while(1) {
		/* read off */
		obj = (Object){ 0 };
		line[0] = 0;
		if(!i_skipws(line)) return; /* ^D */
		if(isspace(line[0])) continue;
		rread = i_readstr(line+1, 511);
		i_scanoff(&obj.off, line);

		/* read type */
		obj.temp = NULL;
		line[0] = 0;
		if(rread && i_skipws(line) && !isspace(line[0])) {
			rread = i_readstr(line+1, 511);
			i_scantype(gi->temps, &(obj.temp), line);
			if(!obj.temp) {
				printf("Unrecognized type %s\n", line);
				if(rread) fflush(NULL);
				continue;
			}
		}

		/* read optional size */
		arr = 1;
		line[0] = 0;
		if(rread && i_skipws(line) && !isspace(line[0])) {
			if(i_readstr(line+1, 511) && i_skipws(NULL)) {
				printf("Unrecognized command\n");
				fflush(NULL);
				continue;
			}
			i_scanarr(&arr, line);
		}

		if(arr != -1) {
			while(arr > 0) {
				if(!(found = i_treeorlocal(&obj, data, gi))) break;
				found->temp->print(found, buf+found->off, gi);
				d_objs(&obj);
				obj.off += found->temp->size;
				--arr;
			}
		} else {
			while(1) {
				if(!(found = i_treeorlocal(&obj, data, gi))) break;

				if(!found->temp->null) {
					fprintf(stderr, "Type %s does not support storage as null terminated array\n", found->temp->name);
					break;
				}
				if(found->temp->null(found, buf+found->off))
					break;

				found->temp->print(found, buf+found->off, gi);
				d_objs(&obj);
				obj.off += found->temp->size;
			}
		}
		putchar('\0');
		if(fflush(NULL) != 0) eprintf("fflush:");
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
i_scanarr(unsigned int *arrp, char *line) {
	unsigned int i;
	char *c = line;

	if(*c == '*') {
		*arrp = -1;
		return c - line + 1;
	}

	i = atoi(c);
	*arrp = i;
	while(isdigit(*c)) ++c;
	return c - line;
}

int
i_scanoff(unsigned int *offp, char *line) {
	unsigned int i;
	char *c = line;

	if(c[0] == '0' && c[1] == 'x')
		i = axtoi(c+2);
	else
		i = axtoi(c);

	*offp = i;
	while(isxdigit(*c)) ++c;
	return c - line;
}

int
i_scantype(Template *temps, Template **tempp, char *line) {
	*tempp = gettemp(temps, line);
	return strlen(line);
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
			printf("%08x %s", 
					off + temp->off[i], 
					temp->tinfo[i].temp->name);
			ibuf = getsize(obj, i);
			if(ibuf > 1)
				printf(" %d", getsize(obj, i));
			break;
		case 'p':
			r_unpack(temp, data, &ibuf, i);
			printf("%08x %s",
					(ibuf) ? ibuf + 0x20 : 0,
					temp->tinfo[i].temp->name);
			ibuf = getsize(obj, i);
			if(ibuf > 1)
				printf(" %d", getsize(obj, i));
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
p_char(Object *obj, void *data, Geninfo *gi) {
	if(*(char*)data == 0)
		printf("\n");
	else
		printf("%c", *(char*)data);
	fflush(NULL);
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

void
usage() {
	printf("usage: datbody DATFILE\n");
}

int
main(int argc, char *argv[]) {
	unsigned char *buf = NULL;
	Template *temps;
	Geninfo gi = { 0 };
	Object *objs = NULL;

	ARGBEGIN {
	case 'v': ++debug;
	} ARGEND;

	if(argc < 1) {
		usage();
		exit(EXIT_SUCCESS);
	}

	c_temps(&temps);
	r_dat((void**)&buf, argv[0]);
	gi.temps = temps;
	r_gentree(&gi, buf);

	i_cmdloop(objs, &gi, buf);
	printf("Goodbye\n");

	if(buf) free(buf);
	d_objs(gi.hdr);
	free(gi.hdr);
	d_gi(&gi);
	d_temps(temps);
	return 0;
}
