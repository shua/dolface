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

enum {
	FARR = 1,
	VARR = 2,
	NARR = 3,
	ARRM = (1 << 2) - 1,
	RSVT = 1 << 2,
	RSVA = 1 << 3,
	MAXOFF = 0xffffffff,
};

typedef struct Defo_t Defo;

typedef struct {
	unsigned int flags;
	union {
		char *type;
		struct Def_t *def;
	};
	union {
		unsigned int ui;
		char *s;
	};
} Defx;

typedef struct Def_t {
	char *type;
	char *format;
	char **mems;
	Defx *extra;
	int  *off;
	void (*print)(Defo*,void*);
	int  (*null)(Defo*,void*);
	Defo *(*gen)(Defo*,void*,void*);
	unsigned int size;
	unsigned int meml;
} Def;

typedef struct DefL_t {
	struct DefL_t *next;
	Def def;
} DefL;

struct Defo_t {
	unsigned int off;
	Def*         def;
};

typedef struct genHdrInfo_t {
	Def *defs;
	DefL *defx;
	char *strs;
} genHdrInfo;

static int   defoappend(Defo **arrp, size_t arrn, Defo *app, size_t appn);
static Def  *genrootgetdef(Def *defs, char *name);
static Defo *g_hdr(Defo *defo, void *data, void *infop);
static Defo *g_generic(Defo *defo, void *data, void *info);
static void  gentree(Defo **defop, genHdrInfo *ghi, void *data);

static int   axtoi(const char *str);
static void  cleanupdefs(Def **defp, DefL *lst);
static void  cmdloop(Defo *defos, genHdrInfo *ghi, void *data);
static int   defcmp(const void *a, const void *b);
static void  destroydef(Def *def);
static void  destroydefs(Def *defs);
static void  eprintf(const char *, ...);
static void  flattendefL(Def *defs, DefL *lst);
static int   getarr(char *str);
static Def  *getdef(Def *defs, char *type);
static void  initdef(Def *def, char *str);
static int   maxtypelen(Def *defs);
static int   n_generic(Defo *defo, void *data);
static DefL *newdefL(DefL *next, char *line);
static void  p_generic(Defo *defo, void *data);
static void  p_hdr(Defo *defo, void *data);
static void  p_char(Defo *defo, void *data);
static void  parsemems(Def *def, char *str);
static void  printdef(Def *def);
static void  printdefs(Def *defs);
static void  protos2lst(DefL **lstp);
static void  readdat(void **dst, char *file);
static void  readdefs(Def **defps);
static void  resolvdefs(Def *def);
static void  resolvmem(Def *def, void *mem);
static void  resolvnullnames(Def *def);
static int   resolvsize(Def *defs, Def *def);
static void  resolvtype(Def *defs, char **type);
static void  unpack(Def *def, void *src, void *dst, int mem);
static void  usage();
static int   scanarr(unsigned int *arrp, char *line);
static int   scanoff(unsigned int *offp, char *line);
static int   scantype(Def *defs, Def **defp, int max, char *line);

static int debug = 0;

#include "datbody.conf.h"

char *argv0;

void
gentree(Defo **defop, genHdrInfo *ghi, void *data) {
	Defo *defo = *defop;
	Def *hdrdef;

	printf("gentree %s\n", ghi->defs[0].type);
	hdrdef = getdef(ghi->defs, "hdr");
	defo = calloc(1, sizeof(Defo));
	defo[0].off = 0;
	defo[0].def = hdrdef;
	printf("  hdrdef->type %s\n", hdrdef->type);
	defoappend(&defo, 1, hdrdef->gen(defo, data, ghi), 0);
}

int
defoappend(Defo **arrp, size_t arrn, Defo *app, size_t appn) {
	Defo *arr = *arrp;

	if(!app) return arrn;
	if(arrn == 0 && arr)
		while(arr[arrn].def) ++arrn;
	if(appn == 0 && app)
		while(app[appn].def) ++appn;

	++appn; // array of 3 will be 3 data plus 1 null terminator

	if(!(arr = realloc(arr, (arrn + appn) * sizeof(Defo)))) {
		eprintf("realloc Defo array");
		return 0;
	}

	memcpy(&arr[arrn-1], app, appn * sizeof(Defo));
	*arrp = arr;
	return arrn + appn;
}

Defo *
g_root(Defo *defo, void *data, void *info) {
	Def *def = defo->def, *childdef;
	Defo *ret;
	unsigned char *buf = data;
	int i, reti=0;
	unsigned int offbuf, namebuf;
	genHdrInfo *ghi = info;

	unpack(defo->def, &buf[defo->off], &offbuf, 0);
	unpack(defo->def, &buf[defo->off], &namebuf, 1);

	ret = calloc(1, sizeof(Defo));
	ret->def = genrootgetdef(ghi->defs, &ghi->strs[namebuf]);
	if(!ret->def) ret->def = getdef(ghi->defs, "x");

	ret->off = offbuf;
	return ret;
}

Defo *
g_generic(Defo *defo, void *data, void *info) {
	Def *def = defo->def;
	Defo *ret, *tmp, tmp2;
	unsigned char *buf = data;
	int i, reti=0;
	unsigned int ibuf;

	printf("g_generic %s %s %x\n", def->type, def->format, defo->off);
	for(i=0; def->format[i]; ++i) {
		switch(def->format[i]) {
		case 'p':
			unpack(def, &buf[defo->off], &ibuf, i);
			tmp2.def = def->extra[i].def;
			tmp2.off = ibuf;
			tmp = tmp2.def->gen(&tmp2, &buf[defo->off], info);
			break;
		case '?':
			tmp2.def = def->extra[i].def;
			tmp2.off = def->off[i] + defo->off;
			tmp = tmp2.def->gen(&tmp2, &buf[defo->off], info);
			break;
		default:
			tmp = NULL;
			break;
		}
		if(!tmp) continue;

		if((reti = defoappend(&ret, reti, tmp, 0)) == 0) {
			eprintf("append Defo array");
			free(tmp);
			free(ret);
			return NULL;
		}
	}

	return ret;
}

DefL *
genroot(DefL *defx, Def *defs, char *strs, void *data, unsigned int off) {
	unsigned int ibuf;
	Def *rootdef, *childdef, *tmpdef;
	DefL *ret;
	unsigned char *buf = data;

	rootdef = getdef(defs, "root");
	if(!rootdef) {
		eprintf("root type not defined");
	}

	printf("genroot\n");
	unpack(rootdef, buf + off, &ibuf, 1);
	printf("  strs[%d]\n", ibuf);
	childdef = genrootgetdef(defs, &strs[ibuf]);
	if(!childdef) return NULL;

	ret = calloc(1, sizeof(DefL));
	ret->next = defx;
	
	tmpdef = &ret->def;
	tmpdef->type = calloc(1, strlen(rootdef->type)+1);
	tmpdef->format = calloc(1, strlen(rootdef->format)+1);
	tmpdef->mems = calloc(2, sizeof(char*));
	tmpdef->extra = calloc(2, sizeof(Defx));
	tmpdef->off = calloc(2, sizeof(int));
	
	strcpy(tmpdef->type, rootdef->type);
	strcpy(tmpdef->format, "px");
	tmpdef->mems[0] = calloc(1, strlen(rootdef->mems[0]) + 1);
	tmpdef->mems[1] = calloc(1, strlen(rootdef->mems[1]) + 1);
	strcpy(tmpdef->mems[0], rootdef->mems[0]);
	strcpy(tmpdef->mems[1], rootdef->mems[1]);
	tmpdef->extra[0].def = childdef;
	tmpdef->off[0] = rootdef->off[0];
	tmpdef->off[1] = rootdef->off[1];
	tmpdef->print = rootdef->print;
	tmpdef->null = rootdef->null;
	tmpdef->gen = rootdef->gen;
	tmpdef->size = rootdef->size;
	tmpdef->meml = rootdef->meml;


	return ret;
}

Def *
genrootgetdef(Def *defs, char *name) {
	int i;
	int namen;
	int checkn;
	
	namen = strlen(name);
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

		return getdef(defs, rootnames[i].name);
	}

	return NULL;
}

Defo *
g_hdr(Defo *defo, void *data, void *infop) {
	Def *def = defo->def;
	Def *rootdef;
	Defo *ret, *tmp;
	DefL *custrootdef;
	genHdrInfo *ghi = infop;
	unsigned char *buf = data;
	int i, reti=0;
	unsigned int ibuf;
	unsigned int filesz, bodysz, reltnum, rootnum, xrefnum, strtabsz;
	unsigned int rootoff;

	printf("g_hdr\n");
	unpack(def, data, &filesz, 0);
	strtabsz = filesz - 0x20;
	unpack(def, data, &bodysz, 1);
	strtabsz -= bodysz;
	unpack(def, data, &reltnum, 2);
	strtabsz -= reltnum * 4;
	rootoff = filesz - strtabsz;
	unpack(def, data, &rootnum, 3);
	unpack(def, data, &xrefnum, 4);
	strtabsz -= (rootnum + xrefnum) * 8;
	ghi->strs = buf + filesz - strtabsz;

	ret = calloc(rootnum, sizeof(Defo));
	rootdef = getdef(ghi->defs, "root");
	for(i=0; i<rootnum; ++i) {
		printf("loopey\n");
		if((custrootdef = genroot(ghi->defx, ghi->defs, ghi->strs, data, rootoff)) != NULL) {
			printf("  custrootdef %s\n", custrootdef->def.type);
			ret[i].def = &custrootdef->def;
			ghi->defx = custrootdef;
		} else {
			ret[i].def = rootdef;
		}
		ret[i].off = rootoff + i * rootdef->size;
	}

	reti = rootnum;
	for(i=0; i<rootnum; ++i) {
		tmp = ret[i].def->gen(&ret[i], data, infop);
		if(!tmp) continue;
		if((reti = defoappend(&ret, reti, tmp, 0)) == 0) {
			eprintf("append Defo array");
			free(tmp);
			free(ret);
			return NULL;
		}
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

void
cleanupdefs(Def **defp, DefL *lst) {
	Def *defs;
	DefL *nlst, *olst = lst;
	int n;

	for(n = 0; lst; lst = nlst, ++n) nlst = lst->next;
	defs = calloc(sizeof(Def), n+1);
	flattendefL(defs, olst);
	qsort(defs, n, sizeof(Def), defcmp);
	resolvdefs(defs);
	*defp = defs;
}

void
cmdloop(Defo *defos, genHdrInfo *ghi, void *data) {
	char line[512];
	unsigned char *buf = data;
	int i, typelen, scanned, nread;
	unsigned int arr;
	Def *defs = ghi->defs, *xdef;
	Defo defo;
	int hfd;

	typelen = maxtypelen(defs);
	xdef = getdef(defs, "x");

	while(1) {
		if((nread = read(STDIN_FILENO, line, 512)) <= 0)
			return;
		for(i=0; line[i] !='\n'; ++i);
		line[i] = 0;
		if(line[0] == 0) continue;
		--nread;
		scanned = 0;

		defo.off = MAXOFF;
		if(scanned < nread && ((i = scanoff(&defo.off, line+scanned)) <= 0 || defo.off == MAXOFF)) {
			fprintf(stderr, "Failed to scan off %d %x\n", i, defo.off);
			continue;
		}
		scanned += i;

		defo.def = NULL;
		if(scanned < nread && ((i = scantype(defs, &(defo.def), typelen, line+scanned)) <= 0 || defo.def == NULL)) {
			fprintf(stderr, "Unrecognized type \"%s\"\n", line);
			continue;
		}
		scanned += i;

		arr = 1;
		if(scanned < nread && ((i = scanarr(&arr, line+scanned)) <= 0 || arr == 0)) {
			fprintf(stderr, "Weird array string \"%s\"\n", line);
			continue;
		}
		scanned += i;

		if(defo.def->print) {
			if(arr != -1) {
				while(arr > 0) {
					defo.def->print(&defo, buf+defo.off);
					defo.off += defo.def->size;
					--arr;
				}
			} else {
				if(!defo.def->null) {
					fprintf(stderr, "Type %s does not support storage as null terminated array\n", defo.def->type);
					continue;
				}
				while(!defo.def->null(&defo, buf+defo.off)) {
					defo.def->print(&defo, buf+defo.off);
					defo.off += defo.def->size;
				}
			}
			putchar('\0');
			if(fflush(NULL) != 0) eprintf("fflush:");
		}
		if(fflush(NULL) != 0) eprintf("fflush:");
	}
}

int 
defcmp(const void *a, const void *b) {
	return strcmp(((Def*)a)->type, ((Def*)b)->type);
}

void
destroydef(Def *def) {
	int mem;

	for(mem=0; def->format[mem]; ++mem) {
		if(def->mems[mem])
			free(def->mems[mem]);
	}

	free(def->off);
	def->off = NULL;
	free(def->extra);
	def->extra = NULL;
	free(def->mems);
	def->mems = NULL;
	free(def->format);
	def->format = NULL;
	free(def->type);
	def->type = NULL;
}

void
destroydefs(Def *defs) {
	while(defs->type) {
		destroydef(defs);
		++defs;
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

void
flattendefL(Def *defs, DefL *lst) {
	DefL *nlst = lst->next;
	int i;

	for(i = 0; lst; lst = nlst, ++i) {
		nlst = lst->next;
		memcpy(defs+i, &(lst->def), sizeof(Def));
		free(lst);
	}
}

int
getarr(char *str) {
	char *e;
	int ret = FARR;

	if(str[0] == '*')
		return (str[1] == 0) ? NARR : 0;
	e = str;
	while((*e) && *e != ']') {
		if(!isalnum(*e)) return 0;
		if((ret == FARR) && !isdigit(*e))
			ret = VARR;
		++e;
	}

	return ret;
}

Def *
getdef(Def *defs, char *type) {
	Def *def;

	for(def = defs; def->type; ++def) {
		if(strcmp(def->type, type)==0)
			return def;
	}
	return NULL;
}

void
initdef(Def *def, char *str) {
	char* c;
	int memn = 0;
	while((*str) && isspace(*str)) ++str;
	if(!(*str)) return;
	c = str;

	while((*c) && !isspace(*c)) ++c;
	if(!(*c)) eprintf("invalid type definition: there's only one word");
	def->type = calloc(1, c-str+1);
	strncpy(def->type, str, c-str);
	while((*c) && isspace(*c)) ++c;
	if(!(*c)) eprintf("invalid format string: expected FORMAT got NULL");
	str = c;

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
		default:  eprintf("Invalid FORMAT character %c", *c);
		}
	}
	memn = c - str;

	def->format = calloc(memn, sizeof(char*));
	strncpy(def->format, str, memn);
	def->mems = calloc(memn, sizeof(char*));
	def->extra = calloc(memn, sizeof(Defx));
	def->off = calloc(memn, sizeof(int));

	parsemems(def, c);
	return;
}

int
maxtypelen(Def *defs) {
	int i, l;

	for(i = 0; defs->type; ++defs) {
		if((l = strlen(defs->type)) > i)
			i = l;
	}
	return i;
}

int
n_generic(Defo *defo, void *data) {
	Def *def = defo->def;
	unsigned int off = defo->off;
	unsigned int ibuf;
	unsigned char *buf = data;

	unpack(def, buf, &ibuf, 0);
	return !ibuf;
}

DefL *
newdefL(DefL *next, char *line) {
	DefL tmpdef = { 0 };
	DefL *lst;

	initdef(&(tmpdef.def), line);
	if(!tmpdef.def.type) return NULL;
	lst = calloc(1, sizeof(DefL));
	memcpy(lst, &tmpdef, sizeof(DefL));
	lst->next = next;
	return lst;
}

void 
p_generic(Defo *defo, void *data) {
	Def *def = defo->def;
	unsigned int off = defo->off;
	char *f, *fmt = def->format;
	unsigned int ibuf, flags;
	float fbuf;
	int i;

	for(f = fmt, i = 0; *f; ++f, ++i) {
		printf("%08x %c %*s : ", 
		       off + def->off[i], 
		       *f, 
		       def->meml, 
		       def->mems[i]);

		switch(*f) {
		case 'b':
		case '.':
		case 'w':
		case 'x':
		case ':':
			unpack(def, data, &ibuf, i);
			printf("%x", ibuf);
			break;
		case 'f':
			unpack(def, data, &fbuf, i);
			printf("%f", fbuf);
			break;
		case '?':
			printf("%08x %s 1", 
					off + def->off[i], 
					def->extra[i].def->type);
			break;
		case 'p':
			unpack(def, data, &ibuf, i);
			printf("%08x %s ",
					ibuf + 0x20,
					def->extra[i].def->type);
			switch(def->extra[i].flags) {
			case FARR:
				printf("%d", def->extra[i].ui);
				break;
			case VARR:
				unpack(def, data, &ibuf, def->extra[i].ui);
				printf("%d", ibuf);
				break;
			case NARR:
				printf("*");
				break;
			}
			break;
		}
		printf("\n");
	}
}

void 
p_hdr(Defo *defo, void *data) {
	Def *def = defo->def;
	unsigned int off = defo->off;
	char *f;
	unsigned int ibuf;
	int hdrsz = 0x20, rootsz = 0x8, offsz = 0x4, filesz;
	int relt, roots, xrefs, strings;
	int i;

	for(f = def->format, i = 0; *f; ++f, ++i) {
		printf("%08x %c %*s : ", 
		       off + def->off[i], 
		       *f, 
		       def->meml, 
		       def->mems[i]);
		unpack(def, data, &ibuf, i);
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
			printf("         p %*s : %08x p %d\n", def->meml, "reltbl", relt, ibuf);
			roots = relt + ibuf*offsz;
			break;
		case 3:
			printf("         p %*s : %08x root %d\n", def->meml, "rootarr", roots, ibuf);
			xrefs = roots + ibuf*rootsz;
			break;
		case 4:
			printf("         p %*s : %08x root %d\n", def->meml, "xrefarr", xrefs, ibuf);
			strings = xrefs + ibuf*rootsz;
			break;
		case 5:
			printf("         p %*s : %08x c %d\n", def->meml, "strings", strings, filesz - strings);
			break;
		}
	}
	if(fflush(NULL) != 0) eprintf("fflush:");
}

void
p_char(Defo *defo, void *data) {
	Def *def = defo->def;
	unsigned int off = defo->off;
	if(*(char*)data == 0)
		printf("\n");
	else
		printf("%c", *(char*)data);
	fflush(NULL);
}

void
parsemems(Def *def, char *str) {
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

	for(b = names, f = def->format, mem = 0, last = 0;
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

				def->extra[mem].flags  = getarr(b);
				if(!def->extra[mem].flags) 
					eprintf("Invalid array size specifier: \"%s\"", b);
				else if(def->extra[mem].flags == FARR)
					def->extra[mem].ui = atoi(b);
				else if(def->extra[mem].flags == VARR) {
					def->extra[mem].s = calloc(1, e-b+1);
					strncpy(def->extra[mem].s, b, e-b);
				}
				b = e+1;
				while(isspace(*b)) ++b;
			}
			if(last) eprintf("Cannot leave %c name empty, needs TYPE specifier (at format[%d])", *f, mem);
			if(*b == '(') {
				++b; e = b;
				while((*e) && *e != ')') ++e;
			} else if(*f == '?')
				eprintf("Need to specify TYPE for inline (?): \"%s\"", b);
			if(!(*e)) eprintf("Reached EOL while parsing TYPE: %s", b);
			if(b == e) eprintf("Error parsing TYPE in line: %s", str);
			def->extra[mem].type = calloc(1, e-b+1);
			strncpy(def->extra[mem].type, b, e-b);
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
				def->mems[mem] = calloc(1, e-b+1);
				strncpy(def->mems[mem], b, e-b);
				if(def->meml < (e-b))
					def->meml = e-b;
			}
			if(!last) b = e + 1;
			break;
		case '.':
		case ':':
			def->mems[mem] = NULL;
			break;
		default: eprintf("Unrecognized char in format string: %s", def->format);
		}
	}
	free(names);
}

void
printdef(Def *def) {
	int mem;
	printf("TYPE: %s\n", def->type);
	for(mem=0; def->format[mem]; ++mem) {
		printf("%04x %c %*s",
				def->off[mem],
				def->format[mem],
				def->meml,
				(def->mems[mem]) ? def->mems[mem] : "");

		switch(def->format[mem]) {
		case 'p':
			if(def->extra[mem].flags == NARR)
				printf(" [*]");
			else if(def->extra[mem].flags == FARR)
				printf(" [%d]", def->extra[mem].ui);
			else if(def->extra[mem].flags == VARR)
				printf(" [%s]", def->mems[def->extra[mem].ui]);
		case '?':
			printf(" (%s)", def->extra[mem].def->type);
		}
		printf(";\n");
	}
	printf("size: %d\n", def->size);
}

void
printdefs(Def *defs) {
	while(defs->type) {
		printdef(defs);
		printf("\n");
		++defs;
	}
}

void
protos2lst(DefL **lstp) {
	DefL *lst, *nlst;
	int i;

	lst = *lstp;
	for(i = 0; i < LEN(protos); ++i) {
		if((nlst = newdefL(lst, protos[i].str)) == NULL)
			continue;
		nlst->def.print = protos[i].print;
		nlst->def.null = protos[i].null;
		nlst->def.gen = protos[i].gen;
		lst = nlst;
	}
	*lstp = lst;
}

void
readdat(void **dst, char *file) {
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
readdefs(Def **defp) {
	Def *defs = NULL;
	DefL *lst = NULL;

	protos2lst(&lst);
	cleanupdefs(&defs, lst);
	if(debug > 0)
		printdefs(defs);

	*defp = defs;
}

void
resolvdefs(Def *defs) {
	Def *def;
	Defx *xp;
	char *f = NULL;
	int unresolved, i;

	for(def = defs; def->type; ++def) {
		for(f = def->format, i = 0; *f; ++f, ++i) {
			xp = &(def->extra[i]);
			if(*f == 'p' || *f == '?') {
				resolvtype(defs, &(xp->type));
				xp->flags |= RSVT;
			}
			if(*f == 'p' && xp->flags == VARR) {
				resolvmem(def, &(xp->s));
				xp->flags |= RSVA;
			}
		}
	}

	for(unresolved = -1, i = 0; 
			unresolved != 0; 
			unresolved = i, i = 0) {
		for(def = defs; def->type; ++def)
			i += resolvsize(defs, def);
		if(unresolved && i == unresolved) {
			fprintf(stderr, "Possible circular dependancy in inline structs");
			for(def = defs; def; ++def) {
				if(resolvsize(defs, def))
					fprintf(stderr, "%s\n", def->type);
			}
			exit(EXIT_FAILURE);
		}
	}

	for(def = defs; def->type; ++def)
		resolvnullnames(def);
}

void
resolvmem(Def *def, void *mem) {
	char *f;
	int i;
	for(f = def->format, i = 0; *f; ++f, ++i) {
		switch(*f) {
		case 'b':
		case 'w':
		case 'x':
		case '.':
		case ':':
			if(strcmp(def->mems[i], *(char**)mem)==0)
				goto found;
		default:
			continue;
		}
	}
	eprintf("Couldn't resolve array length \"%s\" to member name in %s",
			*(char*)mem, def->type);

found:
	free(*(char**)mem);
	*(unsigned int*)mem = i;
}

void
resolvnullnames(Def *def) {
	int i,d;
	for(i = def->size, d = 1; i > 0; i >>= 4, ++d);
	for(i = 0; def->format[i]; ++i) {
		if(def->mems[i]) continue;
		def->mems[i] = calloc(1, 4 + d);
		sprintf(def->mems[i], "unk%0*x", d, def->off[i]);
	}

	if(def->meml < 3+d)
		def->meml = 3+d;
}

int
resolvsize(Def *defs, Def *def) {
	char *f;
	int size = 0, i;
	if(def->size) return 0;

	for(f = def->format, i = 0; *f; ++f, ++i) {
		def->off[i] = size;
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
			if(def->extra[i].def->size == 0) return 1;
			size += def->extra[i].def->size;
			break;
		default: eprintf("format character %c not recognized", *f);
		}
	}
	def->size = size;
	return 0;
}

void
resolvtype(Def *defs, char **type) {
	Def *def = defs;

	for(def = defs; def->type; ++def) {
		if(strcmp(def->type, *type)==0)
			goto found;
	}
	eprintf("Couldn't resolve TYPE \"%s\"", *type);

found:
	free(*type);
	(*(Def**)type) = def;
}

void
unpack(Def *def, void *src, void *dst, int mem) {
	unsigned char *data = src;
	unsigned int ibuf;
	float fbuf;

	data += def->off[mem];

	switch(def->format[mem]) {
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
				((def->format[mem+1]) ? def->off[mem+1] : def->size) - def->off[mem]);
	}
}

void
usage() {
	printf("usage: datbody DATFILE\n");
}

int
scanarr(unsigned int *arrp, char *line) {
	unsigned int i;
	char *c = line;

	while(*c && isspace(*c)) ++c;
	if(!(*c)) {
		*arrp = 1;
		return c - line;
	}

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
scanoff(unsigned int *offp, char *line) {
	unsigned int i;
	char *c = line;

	while(*c && isspace(*c)) ++c;
	if(!(*c)) return 0;
	if(c[0] == '0' && c[1] == 'x')
		i = axtoi(c+2);
	else
		i = axtoi(c);

	*offp = i;
	while(isxdigit(*c)) ++c;
	return c - line;
}

int
scantype(Def *defs, Def **defp, int max, char *line) {
	Def *def = defs;
	char *c;
	int i = 0, n;

	c = line;
	while((*c) && isspace(*c)) ++c;
	if(!(*c)) return 0;

	for(i = 0; i <= max; ++i) {
		if((!(c[i]) || isspace(c[i])) && def->type[i] == '\0') {
			*defp = def;
			return c - line + i;
		}

		while(def->type && (n = strncmp(def->type, c, i+1)) < 0) ++def;
		if(!(def->type) || n != 0) return 0;
	}
	return 0;
}

int
main(int argc, char *argv[]) {
	unsigned char *buf = NULL;
	Def *defs;
	genHdrInfo ghi;
	Defo *defos;

	ARGBEGIN {
	case 'v': ++debug;
	} ARGEND;

	if(argc < 1) {
		usage();
		exit(EXIT_SUCCESS);
	}

	readdefs(&defs);
	readdat((void**)&buf, argv[0]);
	ghi = (genHdrInfo){ defs, NULL, NULL };
	gentree(&defos, &ghi, buf);

	cmdloop(defos, &ghi, buf);
	printf("Goodbye\n");

	if(buf)
		free(buf);
	destroydefs(defs);
	free(defs);
	return 0;
}
