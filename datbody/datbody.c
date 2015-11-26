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

typedef struct Templateo Templateo;
struct Template {
	char *name;
	char *format;
	char **mems;
	Tempinfo *tinfo;
	int *off;
	void (*print)(Templateo*,void*);
	int (*null)(Templateo*,void*);
	Templateo *(*gen)(Templateo*,void*,void*);
	unsigned int size;
	unsigned int cmem;
	Template *next;
};

struct Templateo {
	unsigned int off;
	Template* temp;
};

typedef struct genHdrInfo {
	Template *temps;
	Template *tempx;
	char *strs;
} genHdrInfo;

static int   r_tempoappend(Templateo **arrp, size_t arrn, Templateo *app, size_t appn);
static Template  *r_tempfromroot(Template *temps, char *name);
static Templateo *g_hdr(Templateo *tempo, void *data, void *infop);
static Templateo *g_generic(Templateo *tempo, void *data, void *info);
static void  r_gentree(Templateo **tempop, genHdrInfo *ghi, void *data);

static int axtoi(const char *str);
static void c_cleanuptemps(Template **tempp, Template *lst);
static void c_flattentempL(Template *temps, Template *lst);
static int c_getarr(char *str);
static void c_parsemems(Template *temp, char *str);
static void c_tempsfromtemplates(Template **lstp);
static void c_temps(Template **tempps);
static Template *c_newtemp(Template *next, char *line);
static void c_resolvtemps(Template *temp);
static void c_resolvmem(Template *temp, void *mem);
static void c_resolvnullnames(Template *temp);
static int c_resolvsize(Template *temps, Template *temp);
static void c_resolvtype(Template *temps, char **type);
static int  tempcmp(const void *a, const void *b);
static void d_temp(Template *temp);
static void d_temps(Template *temps);
static void eprintf(const char *, ...);
static Template *gettemp(Template *temps, char *type);
static void i_cmdloop(Templateo *tempos, genHdrInfo *ghi, void *data);
static int  i_maxtypelen(Template *temps);
static void i_printtemp(Template *temp);
static void i_printtemps(Template *temps);
static int  i_scanarr(unsigned int *arrp, char *line);
static int  i_scanoff(unsigned int *offp, char *line);
static int  i_scantype(Template *temps, Template **tempp, int max, char *line);
static int  n_generic(Templateo *tempo, void *data);
static void p_generic(Templateo *tempo, void *data);
static void p_hdr(Templateo *tempo, void *data);
static void p_char(Templateo *tempo, void *data);
static void r_dat(void **dst, char *file);
static void r_unpack(Template *temp, void *src, void *dst, int mem);
static void usage();

static int debug = 0;

#include "datbody.conf.h"

char *argv0;

void
r_gentree(Templateo **tempop, genHdrInfo *ghi, void *data) {
	Templateo *tempo = *tempop;
	Template *hdrtemp;

	printf("r_gentree %s\n", ghi->temps[0].name);
	hdrtemp = gettemp(ghi->temps, "hdr");
	tempo = calloc(1, sizeof(Templateo));
	tempo[0].off = 0;
	tempo[0].temp = hdrtemp;
	printf("  hdrtemp->name %s\n", hdrtemp->name);
	r_tempoappend(&tempo, 1, hdrtemp->gen(tempo, data, ghi), 0);
}

int
r_tempoappend(Templateo **arrp, size_t arrn, Templateo *app, size_t appn) {
	Templateo *arr = *arrp;

	if(!app) return arrn;
	if(arrn == 0 && arr)
		while(arr[arrn].temp) ++arrn;
	if(appn == 0 && app)
		while(app[appn].temp) ++appn;

	++appn; // array of 3 will be 3 data plus 1 null terminator

	if(!(arr = realloc(arr, (arrn + appn) * sizeof(Templateo)))) {
		eprintf("realloc Templateo array");
		return 0;
	}

	memcpy(&arr[arrn-1], app, appn * sizeof(Templateo));
	*arrp = arr;
	return arrn + appn;
}

Templateo *
g_root(Templateo *tempo, void *data, void *info) {
	Template *temp = tempo->temp, *childtemp;
	Templateo *ret;
	unsigned char *buf = data;
	int i, reti=0;
	unsigned int offbuf, namebuf;
	genHdrInfo *ghi = info;

	r_unpack(tempo->temp, &buf[tempo->off], &offbuf, 0);
	r_unpack(tempo->temp, &buf[tempo->off], &namebuf, 1);

	ret = calloc(1, sizeof(Templateo));
	ret->temp = r_tempfromroot(ghi->temps, &ghi->strs[namebuf]);
	if(!ret->temp) ret->temp = gettemp(ghi->temps, "x");

	ret->off = offbuf;
	return ret;
}

Templateo *
g_generic(Templateo *tempo, void *data, void *info) {
	Template *temp = tempo->temp;
	Templateo *ret, *tmp, tmp2;
	unsigned char *buf = data;
	int i, reti=0;
	unsigned int ibuf;

	printf("g_generic %s %s %x\n", temp->name, temp->format, tempo->off);
	for(i=0; temp->format[i]; ++i) {
		switch(temp->format[i]) {
		case 'p':
			r_unpack(temp, &buf[tempo->off], &ibuf, i);
			tmp2.temp = temp->tinfo[i].temp;
			tmp2.off = ibuf;
			tmp = tmp2.temp->gen(&tmp2, &buf[tempo->off], info);
			break;
		case '?':
			tmp2.temp = temp->tinfo[i].temp;
			tmp2.off = temp->off[i] + tempo->off;
			tmp = tmp2.temp->gen(&tmp2, &buf[tempo->off], info);
			break;
		default:
			tmp = NULL;
			break;
		}
		if(!tmp) continue;

		if((reti = r_tempoappend(&ret, reti, tmp, 0)) == 0) {
			eprintf("append Templateo array");
			free(tmp);
			free(ret);
			return NULL;
		}
	}

	return ret;
}

Template *
genroot(Template *tempx, Template *temps, char *strs, void *data, unsigned int off) {
	unsigned int ibuf;
	Template *roottemp, *childtemp, *ret;
	unsigned char *buf = data;

	roottemp = gettemp(temps, "root");
	if(!roottemp) {
		eprintf("root type not defined");
	}

	printf("genroot\n");
	r_unpack(roottemp, buf + off, &ibuf, 1);
	printf("  strs[%d]\n", ibuf);
	childtemp = r_tempfromroot(temps, &strs[ibuf]);
	if(!childtemp) return NULL;

	ret = calloc(1, sizeof(Template));
	ret->next = tempx;
	
	ret->name = calloc(1, strlen(roottemp->name)+1);
	ret->format = calloc(1, strlen(roottemp->format)+1);
	ret->mems = calloc(2, sizeof(char*));
	ret->tinfo = calloc(2, sizeof(Tempinfo));
	ret->off = calloc(2, sizeof(int));
	
	strcpy(ret->name, roottemp->name);
	strcpy(ret->format, "px");
	ret->mems[0] = calloc(1, strlen(roottemp->mems[0]) + 1);
	ret->mems[1] = calloc(1, strlen(roottemp->mems[1]) + 1);
	strcpy(ret->mems[0], roottemp->mems[0]);
	strcpy(ret->mems[1], roottemp->mems[1]);
	ret->tinfo[0].temp = childtemp;
	ret->off[0] = roottemp->off[0];
	ret->off[1] = roottemp->off[1];
	ret->print = roottemp->print;
	ret->null = roottemp->null;
	ret->gen = roottemp->gen;
	ret->size = roottemp->size;
	ret->cmem = roottemp->cmem;


	return ret;
}

Template *
r_tempfromroot(Template *temps, char *name) {
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

		return gettemp(temps, rootnames[i].name);
	}

	return NULL;
}

Templateo *
g_hdr(Templateo *tempo, void *data, void *infop) {
	Template *temp = tempo->temp;
	Template *roottemp;
	Templateo *ret, *tmp;
	Template *custroottemp;
	genHdrInfo *ghi = infop;
	unsigned char *buf = data;
	int i, reti=0;
	unsigned int ibuf;
	unsigned int filesz, bodysz, reltnum, rootnum, xrefnum, strtabsz;
	unsigned int rootoff;

	printf("g_hdr\n");
	r_unpack(temp, data, &filesz, 0);
	strtabsz = filesz - 0x20;
	r_unpack(temp, data, &bodysz, 1);
	strtabsz -= bodysz;
	r_unpack(temp, data, &reltnum, 2);
	strtabsz -= reltnum * 4;
	rootoff = filesz - strtabsz;
	r_unpack(temp, data, &rootnum, 3);
	r_unpack(temp, data, &xrefnum, 4);
	strtabsz -= (rootnum + xrefnum) * 8;
	ghi->strs = buf + filesz - strtabsz;

	ret = calloc(rootnum, sizeof(Templateo));
	roottemp = gettemp(ghi->temps, "root");
	for(i=0; i<rootnum; ++i) {
		printf("loopey\n");
		if((custroottemp = genroot(ghi->tempx, ghi->temps, ghi->strs, data, rootoff)) != NULL) {
			printf("  custroottemp %s\n", custroottemp->name);
			ret[i].temp = custroottemp;
			ghi->tempx = custroottemp;
		} else {
			ret[i].temp = roottemp;
		}
		ret[i].off = rootoff + i * roottemp->size;
	}

	reti = rootnum;
	for(i=0; i<rootnum; ++i) {
		tmp = ret[i].temp->gen(&ret[i], data, infop);
		if(!tmp) continue;
		if((reti = r_tempoappend(&ret, reti, tmp, 0)) == 0) {
			eprintf("append Templateo array");
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
c_cleanuptemps(Template **tempp, Template *lst) {
	Template *temps;
	Template *nlst, *olst = lst;
	int n;

	for(n = 0; lst; lst = nlst, ++n) nlst = lst->next;
	temps = calloc(sizeof(Template), n+1);
	c_flattentempL(temps, olst);
	qsort(temps, n, sizeof(Template), tempcmp);
	c_resolvtemps(temps);
	*tempp = temps;
}

void
c_flattentempL(Template *temps, Template *lst) {
	Template *nlst = lst->next;
	int i;

	for(i = 0; lst; lst = nlst, ++i) {
		nlst = lst->next;
		memcpy(temps+i, lst, sizeof(Template));
		free(lst);
	}
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
			if(last) eprintf("Cannot leave %c name empty, needs TYPE specifier (at format[%d])", *f, mem);
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
				if(temp->cmem < (e-b))
					temp->cmem = e-b;
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
	Template *lst = NULL;

	c_tempsfromtemplates(&lst);
	c_cleanuptemps(&temps, lst);
	if(debug > 0)
		i_printtemps(temps);

	*tempp = temps;
}

Template *
c_newtemp(Template *next, char *line) {
	char* c;
	int memn = 0;
	Template *temp;

	temp = calloc(1, sizeof(Template));
	while((*line) && isspace(*line)) ++line;
	if(!(*line)) return NULL;
	c = line;

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
c_resolvtemps(Template *temps) {
	Template *temp;
	Tempinfo *info;
	char *f = NULL;
	int unresolved, i;

	for(temp = temps; temp->name; ++temp) {
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
		for(temp = temps; temp->name; ++temp)
			i += c_resolvsize(temps, temp);
		if(unresolved && i == unresolved) {
			fprintf(stderr, "Possible circular dependancy in inline structs");
			for(temp = temps; temp; ++temp) {
				if(c_resolvsize(temps, temp))
					fprintf(stderr, "%s\n", temp->name);
			}
			exit(EXIT_FAILURE);
		}
	}

	for(temp = temps; temp->name; ++temp)
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
		case ':':
			if(strcmp(temp->mems[i], *(char**)mem)==0)
				goto found;
		default:
			continue;
		}
	}
	eprintf("Couldn't resolve array length \"%s\" to member name in %s",
			*(char*)mem, temp->name);

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

	if(temp->cmem < 3+d)
		temp->cmem = 3+d;
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

	for(temp = temps; temp->name; ++temp) {
		if(strcmp(temp->name, *type)==0)
			goto found;
	}
	eprintf("Couldn't resolve TYPE \"%s\"", *type);

found:
	free(*type);
	(*(Template**)type) = temp;
}

int 
tempcmp(const void *a, const void *b) {
	return strcmp(((Template*)a)->name, ((Template*)b)->name);
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
	while(temps->name) {
		d_temp(temps);
		++temps;
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

	for(temp = temps; temp->name; ++temp) {
		if(strcmp(temp->name, type)==0)
			return temp;
	}
	return NULL;
}

void
i_cmdloop(Templateo *tempos, genHdrInfo *ghi, void *data) {
	char line[512];
	unsigned char *buf = data;
	int i, typelen, scanned, nread;
	unsigned int arr;
	Template *temps = ghi->temps, *xtemp;
	Templateo tempo;
	int hfd;

	typelen = i_maxtypelen(temps);
	xtemp = gettemp(temps, "x");

	while(1) {
		if((nread = read(STDIN_FILENO, line, 512)) <= 0)
			return;
		for(i=0; line[i] !='\n'; ++i);
		line[i] = 0;
		if(line[0] == 0) continue;
		--nread;
		scanned = 0;

		tempo.off = MAXOFF;
		if(scanned < nread && ((i = i_scanoff(&tempo.off, line+scanned)) <= 0 || tempo.off == MAXOFF)) {
			fprintf(stderr, "Failed to scan off %d %x\n", i, tempo.off);
			continue;
		}
		scanned += i;

		tempo.temp = NULL;
		if(scanned < nread && ((i = i_scantype(temps, &(tempo.temp), typelen, line+scanned)) <= 0 || tempo.temp == NULL)) {
			fprintf(stderr, "Unrecognized type \"%s\"\n", line);
			continue;
		}
		scanned += i;

		arr = 1;
		if(scanned < nread && ((i = i_scanarr(&arr, line+scanned)) <= 0 || arr == 0)) {
			fprintf(stderr, "Weird array string \"%s\"\n", line);
			continue;
		}
		scanned += i;

		if(tempo.temp->print) {
			if(arr != -1) {
				while(arr > 0) {
					tempo.temp->print(&tempo, buf+tempo.off);
					tempo.off += tempo.temp->size;
					--arr;
				}
			} else {
				if(!tempo.temp->null) {
					fprintf(stderr, "Type %s does not support storage as null terminated array\n", tempo.temp->name);
					continue;
				}
				while(!tempo.temp->null(&tempo, buf+tempo.off)) {
					tempo.temp->print(&tempo, buf+tempo.off);
					tempo.off += tempo.temp->size;
				}
			}
			putchar('\0');
			if(fflush(NULL) != 0) eprintf("fflush:");
		}
		if(fflush(NULL) != 0) eprintf("fflush:");
	}
}

int
i_maxtypelen(Template *temps) {
	int i, l;

	for(i = 0; temps->name; ++temps) {
		if((l = strlen(temps->name)) > i)
			i = l;
	}
	return i;
}

void
i_printtemp(Template *temp) {
	int mem;
	printf("TYPE: %s\n", temp->name);
	for(mem=0; temp->format[mem]; ++mem) {
		printf("%04x %c %*s",
				temp->off[mem],
				temp->format[mem],
				temp->cmem,
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
	while(temps->name) {
		i_printtemp(temps);
		printf("\n");
		++temps;
	}
}

int
i_scanarr(unsigned int *arrp, char *line) {
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
i_scanoff(unsigned int *offp, char *line) {
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
i_scantype(Template *temps, Template **tempp, int max, char *line) {
	Template *temp = temps;
	char *c;
	int i = 0, n;

	c = line;
	while((*c) && isspace(*c)) ++c;
	if(!(*c)) return 0;

	for(i = 0; i <= max; ++i) {
		if((!(c[i]) || isspace(c[i])) && temp->name[i] == '\0') {
			*tempp = temp;
			return c - line + i;
		}

		while(temp->name && (n = strncmp(temp->name, c, i+1)) < 0) ++temp;
		if(!(temp->name) || n != 0) return 0;
	}
	return 0;
}

int
n_generic(Templateo *tempo, void *data) {
	Template *temp = tempo->temp;
	unsigned int off = tempo->off;
	unsigned int ibuf;
	unsigned char *buf = data;

	r_unpack(temp, buf, &ibuf, 0);
	return !ibuf;
}

void 
p_generic(Templateo *tempo, void *data) {
	Template *temp = tempo->temp;
	unsigned int off = tempo->off;
	char *f, *fmt = temp->format;
	unsigned int ibuf, flags;
	float fbuf;
	int i;

	for(f = fmt, i = 0; *f; ++f, ++i) {
		printf("%08x %c %*s : ", 
		       off + temp->off[i], 
		       *f, 
		       temp->cmem, 
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
			printf("%08x %s 1", 
					off + temp->off[i], 
					temp->tinfo[i].temp->name);
			break;
		case 'p':
			r_unpack(temp, data, &ibuf, i);
			printf("%08x %s ",
					ibuf + 0x20,
					temp->tinfo[i].temp->name);
			switch(temp->tinfo[i].arrterm) {
			case SizeConst:
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
p_hdr(Templateo *tempo, void *data) {
	Template *temp = tempo->temp;
	unsigned int off = tempo->off;
	char *f;
	unsigned int ibuf;
	int hdrsz = 0x20, rootsz = 0x8, offsz = 0x4, filesz;
	int relt, roots, xrefs, strings;
	int i;

	for(f = temp->format, i = 0; *f; ++f, ++i) {
		printf("%08x %c %*s : ", 
		       off + temp->off[i], 
		       *f, 
		       temp->cmem, 
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
			printf("         p %*s : %08x p %d\n", temp->cmem, "reltbl", relt, ibuf);
			roots = relt + ibuf*offsz;
			break;
		case 3:
			printf("         p %*s : %08x root %d\n", temp->cmem, "rootarr", roots, ibuf);
			xrefs = roots + ibuf*rootsz;
			break;
		case 4:
			printf("         p %*s : %08x root %d\n", temp->cmem, "xrefarr", xrefs, ibuf);
			strings = xrefs + ibuf*rootsz;
			break;
		case 5:
			printf("         p %*s : %08x c %d\n", temp->cmem, "strings", strings, filesz - strings);
			break;
		}
	}
	if(fflush(NULL) != 0) eprintf("fflush:");
}

void
p_char(Templateo *tempo, void *data) {
	Template *temp = tempo->temp;
	unsigned int off = tempo->off;
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
	float fbuf;

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
	genHdrInfo ghi;
	Templateo *tempos;

	ARGBEGIN {
	case 'v': ++debug;
	} ARGEND;

	if(argc < 1) {
		usage();
		exit(EXIT_SUCCESS);
	}

	c_temps(&temps);
	r_dat((void**)&buf, argv[0]);
	ghi = (genHdrInfo){ temps, NULL, NULL };
	//r_gentree(&tempos, &ghi, buf);

	i_cmdloop(tempos, &ghi, buf);
	printf("Goodbye\n");

	if(buf)
		free(buf);
	d_temps(temps);
	free(temps);
	return 0;
}
