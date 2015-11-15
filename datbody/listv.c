#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

typedef struct {
	char *title;
	char **opts;
	int sel, nopts;
} Opt;

typedef struct {
	WINDOW *win;
	int x,y,w,h;
	char bl, br, bt, bb;
	Opt *opt;
} Win;

typedef struct DL_t {
	struct DL_t *next;
	struct DL_t *prev;
	void *data;
} DL;

static int pin[2];
static int pout[2];
static int perr[2];
static Win *lwin = NULL;

static void blank(Win *win);
static int  checkkeys(Win *win, int nom);
static void destroyDL(DL *dl, void (*destdata)(void*));
static void destroyOpt(void *o);
static void draw(Win *win);
static void eprintf(const char *fmt, ...);
static int  initpipes();
static void initspace(Win* wins, DL **optdlp);
static void initwin(Win *win, int h, int w, int y, int x);
static void logwin(const char *log, ...);
static void nextwin(Win *wins, DL **optdlp);
static void parsechild(Opt *o);
static void prime(Win *wins);
static void run(Win *wins, DL **optdlp, char **result);
static void updatenext(Win *mwin, Win *nwin);

#include "listv.conf.h"

void
blank(Win *win) {
	wclear(win->win);
	box(win->win, 0, 0);
	if(win->opt && win->opt->title)
		mvwprintw(win->win, 0, 1, "%s", win->opt->title);
}

int
checkkeys(Win *win, int nom) {
	int c;
	c = wgetch(win->win);
	if(win->opt->sel == -1)
		return c;
	switch(c) {
	case KEY_UP:
		if(win->opt->sel != 0)
			--(win->opt->sel);
		break;
	case KEY_DOWN:
		if(win->opt->sel != win->opt->nopts-1)
			++(win->opt->sel);
		break;
	default:
		return c;
	}
	return (nom) ? 0 : c;
}

void
destroyDL(DL *dl, void (*destdata)(void*)) {
	DL *n;

	while(dl->prev) dl = dl->prev;
	while(dl) {
		n = dl->next;
		dl->prev = NULL;
		if(dl->data != NULL && destdata != NULL)
			destdata(dl->data);
		dl->data = NULL;
		dl->next = NULL;
		free(dl);
		dl = n;
	}
}

void
clearOpt(Opt *o) {
	int i;
	if(o->title)
		free(o->title);
	for(i=0; i < o->nopts; ++i) {
		if(o->opts[i]) {
			free(o->opts[i]);
			o->opts[i] = NULL;
		}
	}
	if(o->opts)
		free(o->opts);
	o->title = NULL;
	o->opts = NULL;
	o->nopts = 0;
	o->sel = -1;
}

void
destroyOpt(void *o) {
	clearOpt(o);
	free(o);
}

void
draw(Win *win) {
	int i;
	if(!win) 
		return;
	if(!win->opt || win->opt->nopts == 0) {
		mvwprintw(win->win, 1, 1, "NO OPTIONS");
		return;
	}
	for(i=0; i<win->opt->nopts; ++i) {
		if(win->opt->sel == i) {
			wattron(win->win, A_REVERSE);
			mvwprintw(win->win, i+1, 1, "%s", win->opt->opts[i]);
			wattroff(win->win, A_REVERSE);
		} else {
			mvwprintw(win->win, i+1, 1, "%s", win->opt->opts[i]);
		}
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

int
initpipes() {
	pid_t pid;
	pid = -1;
	if(pipe(pin) == -1 || pipe(pout) == -1 || pipe(perr) == -1) {
		eprintf("pipe:");
		return 1;
	}

	if((pid = fork()) == -1) {
		eprintf("fork:");
		return 2;
	} else if(pid == 0) {
		close(pin[1]);
		close(pout[0]);
		close(perr[0]);
		dup2(pin[0], STDIN_FILENO);
		dup2(pout[1], STDOUT_FILENO);
		dup2(pout[1], STDERR_FILENO);
		close(pin[0]);
		close(pout[1]);
		close(perr[1]);
		execl("/bin/sh", "sh", "-c", childcmd, NULL);
		fprintf(stderr, "initpipes: execl sh -c \"%s\"\n", childcmd);
		eprintf("exec:");
		_exit(EXIT_FAILURE);
	} else {
		close(pin[0]);
		close(pout[1]);
		close(perr[1]);
		fcntl(perr[0], F_SETFL, O_NONBLOCK);
	}
	return 0;
}

void
initspace(Win* wins, DL **optdlp) {
	Win *pwin = &(wins[0]), *mwin = &(wins[1]), *nwin = &(wins[2]);
	DL *optdl; 
	initscr();
	noecho();
	cbreak();
	curs_set(0);

	initwin(pwin, LINES, COLS/4, 0, 0);
	initwin(mwin, LINES, COLS/2, 0, pwin->x + pwin->w);
	initwin(nwin, LINES, COLS - pwin->w - mwin->w, 0, mwin->x + mwin->w);

	optdl = (*optdlp = calloc(sizeof(DL), 1));
	optdl->next = calloc(sizeof(DL), 1);
	optdl->prev = NULL;
	optdl->next->prev = optdl;
	optdl->next->data = calloc(sizeof(Opt), 1);
	optdl->data = calloc(sizeof(Opt), 1);
	mwin->opt = optdl->data;
	nwin->opt = optdl->next->data;
	nwin->opt->sel = -1;

	blank(mwin);
	blank(nwin);
	lwin = pwin;

	keypad(mwin->win, TRUE);
	refresh();
}

void
initwin(Win *win, int h, int w, int y, int x) {
	win->win = newwin(h, w, y, x);
	win->x = x;
	win->y = y;
	win->w = w;
	win->h = h;
}

void
logerr() {
	char c[1024];
	int i = 0;;
	if((i = read(perr[0], c, 1023)) > 0) {
		c[1023] = 0;
		logwin("child: %s", c);
	}
}

void
logwin(const char *log, ...) {
	if(lwin == NULL) return;
	va_list ap;

	va_start(ap, log);
	vwprintw(lwin->win, log, ap);
	va_end(ap);

	wprintw(lwin->win, "\n");
	wrefresh(lwin->win);
}

int
nbread(int fd, char *buf, int len, int usec) {
	struct timeval tv;
	fd_set fdI, fdE;

	FD_ZERO(&fdI);
	FD_SET(fd, &fdI);
	tv.tv_sec = 0;
	tv.tv_usec = usec;

	if(select(FD_SETSIZE, &fdI, NULL, NULL, &tv) > 0) {
		return read(fd, buf, len);
	}
	logerr();
	return 0;
}

void
nextwin(Win *wins, DL **optdlp) {
	DL *optdl = *optdlp;
	Win *mwin = &(wins[1]), *nwin = &(wins[2]);
	optdl = optdl->next;
	mwin->opt = optdl->data;
	mwin->opt->sel = 0;
	optdl->next = calloc(sizeof(DL), 1);
	optdl->next->prev = optdl;
	optdl->next->data = calloc(sizeof(Opt), 1);
	nwin->opt = optdl->next->data;
	nwin->opt->sel = -1;
	*optdlp = optdl;
}

void
prevwin(Win *wins, DL **optdlp) {
	DL *optdl = *optdlp;
	Win *mwin = &(wins[1]), *nwin = &(wins[2]);
	if(!optdl->prev) return;
	optdl = optdl->prev;
	mwin->opt = optdl->data;
	nwin->opt = optdl->next->data;
	nwin->opt->sel = -1;
	destroyOpt(optdl->next->next->data);
	free(optdl->next->next);
	optdl->next->next = NULL;
	*optdlp = optdl;
}

void
parsechild(Opt *o) {
	char **opts = NULL;
	char line[1024], last = ' ';
	int nread;
	int i, nopts = 0;

	while(1) {
		i = 0;
		while(i < 1024) {
			if((nread = nbread(pout[0], line+i, 1, 100000)) < 0) {
				eprintf("read:");
				break;
			} else if(nread == 0) {
				logwin("no more");
				logerr();
				break;
			}
			if(line[i] == '\n') {
				line[i] = 0;
				break;
			}
			++i;
		}
		if(i >= 1024) {
			fprintf(stderr, "line way too big\n");
			continue;
		}
		if(i == 0) break;

		opts = realloc(opts, (nopts+1)*sizeof(char*));
		if(opts == NULL)
			eprintf("realloc:");
		opts[nopts] = malloc(i+1);
		line[i] = 0;
		strncpy(opts[nopts], line, i+1);
		++nopts;
	}
	if(o->nopts) {
		while(o->nopts--)
			if(o->opts[o->nopts])
				free(o->opts[o->nopts]);
		free(o->opts);
	}

	o->opts = opts;
	o->nopts = nopts;
}

void
prime(Win *wins) {
	Win *mwin = &(wins[1]), *nwin = &(wins[2]);
	if(write(pin[1], primer, sizeof(primer)) <= 0)
		eprintf("write:");
	if(write(pin[1], "\n", 1) <= 0)
		eprintf("write2:");
	parsechild(mwin->opt);

	write(pin[1], 
	      mwin->opt->opts[mwin->opt->sel],
	      strlen(mwin->opt->opts[mwin->opt->sel]));
	write(pin[1], "\n", 1);
	parsechild(nwin->opt);
}

void
run(Win *wins, DL **optdlp, char **result) {
	DL *optdl = *optdlp;
	char *choice = NULL;
	Win *mwin = &(wins[1]), *nwin = &(wins[2]);

	do {
		draw(mwin);
		draw(nwin);
		wrefresh(mwin->win);

		switch(checkkeys(mwin, 0)) {
		case KEY_RIGHT:
			nextwin(wins, optdlp);
			blank(mwin);
			updatenext(mwin, nwin);
			break;
		case KEY_LEFT:
			prevwin(wins, optdlp);
			blank(mwin);
			updatenext(mwin, nwin);
		case KEY_UP:
		case KEY_DOWN:
			updatenext(mwin, nwin);
			break;
		case 10:
			choice = mwin->opt->opts[mwin->opt->sel];
			break;
		}
	} while (choice == NULL);
	*result = choice;
}

void
updatenext(Win *mwin, Win *nwin) {
	char *next;
	next = nextinput(mwin->opt->opts[mwin->opt->sel]);
	if(next == NULL) {
		blank(nwin);
		clearOpt(nwin->opt);
		nwin->opt->nopts = 0;
	} else {
		logwin("nextinput \"%s\" %d %c", next, strlen(next), next[strlen(next)-1]);
		write(pin[1], 
				next,
				strlen(next));
		write(pin[1], "\n", 1);
		free(next);
		parsechild(nwin->opt);
		blank(nwin);
	}
	draw(nwin);
	wrefresh(nwin->win);
}

void
simprun(Win *wins, void *optdl, char **choicep) {
	Win *mwin = &(wins[1]), *nwin = &(wins[2]);
	int i;
	char *next;
	logwin("begin");
	for(i=0; i<mwin->opt->nopts; ++i) logwin(mwin->opt->opts[i]);
	clearOpt(nwin->opt);

	do {
		draw(mwin);
		draw(nwin);
		wrefresh(mwin->win);
		wrefresh(nwin->win);

		switch(checkkeys(mwin, 0)) {
		case KEY_UP:
		case KEY_DOWN:
			next = nextinput(mwin->opt->opts[mwin->opt->sel]);
			logwin("nextinput \"%s\"", next);
			free(next);
			break;
		case 10:
			*choicep = mwin->opt->opts[mwin->opt->sel];
			return;
		}
		logerr();
	} while(1);       
}

int
main(int argc, char **argv) {
	char *choice = NULL;
	Win wins[3] = { 0 };
	DL *optdl;

	initpipes();
	initspace(wins, &optdl);
	prime(wins);
	/* run(wins, &optdl, &choice); */
	run(wins, &optdl, &choice);
	endwin();
	if(choice)
		printf("%s\n", choice);
	destroyDL(optdl, destroyOpt);
	return 0;
}

