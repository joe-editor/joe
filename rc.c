/*
 *	*rc file parser
 *	Copyright
 *		(C) 1992 Joseph H. Allen; 
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include "utils.h"
#include "macro.h"
#include "cmd.h"
#include "bw.h"
#include "help.h"
#include "vs.h"
#include "va.h"
#include "menu.h"
#include "umath.h"
#include "uedit.h"
#include "pw.h"
#include "path.h"
#include "w.h"
#include "tw.h"
#include "termcap.h"
#include "rc.h"

#define OPT_BUF_SIZE 300

static struct context {
	struct context *next;
	char *name;
	KMAP *kmap;
} *contexts = 0;		/* List of named contexts */

/* Find a context of a given name- if not found, one with an empty kmap
 * is created.
 */

KMAP *getcontext(char *name)
{
	struct context *c;

	for (c = contexts; c; c = c->next)
		if (!strcmp(c->name, name))
			return c->kmap;
	c = (struct context *) malloc(sizeof(struct context));

	c->next = contexts;
	c->name = strdup(name);
	contexts = c;
	return c->kmap = mkkmap();
}

OPTIONS *options = 0;
extern int mid, dspasis, dspctrl, force, help, pgamnt, square, csmode, nobackups, lightoff, exask, skiptop, noxon, lines, staen, columns, Baud, dopadding, orphan, marking, beep, keepup, nonotice;
extern char *backpath;

#ifdef __MSDOS__
OPTIONS pdefault = { 0, 0, 0, 0, 76, 0, 0, 8, ' ', 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,

	0
};
OPTIONS fdefault = { 0, 0, 0, 0, 76, 0, 0, 8, ' ', 1, "main", "\\i%n %m %M",
	" %S Ctrl-K H for help", 0, 0, 0, 0, 1, 0, 0, 0, 0
};
#else
OPTIONS pdefault = { 0, 0, 0, 0, 76, 0, 0, 8, ' ', 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0
};
OPTIONS fdefault = { 0, 0, 0, 0, 76, 0, 0, 8, ' ', 1, "main", "\\i%n %m %M",
	" %S Ctrl-K H for help", 0, 0, 0, 0, 0, 0, 0, 0, 0
};
#endif

void setopt(OPTIONS * n, char *name)
{
	OPTIONS *o;

	for (o = options; o; o = o->next)
		if (rmatch(o->name, name)) {
			*n = *o;
			return;
		}
	*n = fdefault;
}

/* Set a global or local option 
 * returns 0 for no such option,
 *         1 for option accepted
 *         2 for option + argument accepted
 */

struct glopts {
	char *name;		/* Option name */
	int type;		/*      0 for global option flag
				   1 for global option numeric
				   2 for global option string
				   4 for local option flag
				   5 for local option numeric
				   6 for local option string
				   7 for local option numeric+1
				 */
	int *set;		/* Address of global option */
	char *addr;		/* Local options structure member address */
	char *yes;		/* Message if option was turned on, or prompt string */
	char *no;		/* Message if option was turned off */
	char *menu;		/* Menu string */
	int ofst;		/* Local options structure member offset */
	int low;		/* Low limit for numeric options */
	int high;		/* High limit for numeric options */
} glopts[] = {

	{
	"overwrite", 4, 0, (char *) &fdefault.overtype, "Overtype mode", "Insert mode", "T Overtype "}
	, {
	"autoindent", 4, 0, (char *) &fdefault.autoindent, "Autoindent enabled", "Autindent disabled", "I Autoindent "}
	, {
	"wordwrap", 4, 0, (char *) &fdefault.wordwrap, "Wordwrap enabled", "Wordwrap disabled", "Word wrap "}
	, {
	"tab", 5, 0, (char *) &fdefault.tab, "Tab width (%d): ", 0, "D Tab width ", 0, 1, 64}
	, {
	"lmargin", 7, 0, (char *) &fdefault.lmargin, "Left margin (%d): ", 0, "Left margin ", 0, 0, 63}
	, {
	"rmargin", 7, 0, (char *) &fdefault.rmargin, "Right margin (%d): ", 0, "Right margin ", 0, 7, 255}
	, {
	"square", 0, &square, 0, "Rectangle mode", "Text-stream mode", "X Rectangle mode "}
	, {
	"indentc", 5, 0, (char *) &fdefault.indentc, "Indent char %d (SPACE=32, TAB=9, ^C to abort): ", 0, " Indent char ", 0, 0, 255}
	, {
	"istep", 5, 0, (char *) &fdefault.istep, "Indent step %d (^C to abort): ", 0, " Indent step ", 0, 1, 64}
	, {
	"french", 4, 0, (char *) &fdefault.french, "One space after periods for paragraph reformat", "Two spaces after periods for paragraph reformat", " french spacing "}
	, {
	"spaces", 4, 0, (char *) &fdefault.spaces, "Inserting spaces when tab key is hit", "Inserting tabs when tab key is hit", " no tabs "}
	, {
	"mid", 0, &mid, 0, "Cursor will be recentered on scrolls", "Cursor will not be recentered on scroll", "Center on scroll "}
	, {
	"crlf", 4, 0, (char *) &fdefault.crlf, "CR-LF is line terminator", "LF is line terminator", "Z CR-LF (MS-DOS) "}
	, {
	"linums", 4, 0, (char *) &fdefault.linums, "Line numbers enabled", "Line numbers disabled", "N Line numbers "}
	, {
	"marking", 0, &marking, 0, "Anchored block marking on", "Anchored block marking off", "Marking "}
	, {
	"asis", 0, &dspasis, 0, "Characters above 127 shown as-is", "Characters above 127 shown in inverse", "Meta chars as-is "}
	, {
	"force", 0, &force, 0, "Last line forced to have NL when file saved", "Last line not forces to have NL", "Force last NL "}
	, {
	"nobackups", 0, &nobackups, 0, "Backup files will not be made", "Backup files will be made", " Disable backups "}
	, {
	"lightoff", 0, &lightoff, 0, "Highlighting turned off after block operations", "Highlighting not turned off after block operations", "Auto unmark "}
	, {
	"exask", 0, &exask, 0, "Prompt for filename in save & exit command", "Don't prompt for filename in save & exit command", "Exit ask "}
	, {
	"beep", 0, &beep, 0, "Warning bell enabled", "Warning bell disabled", "Beeps "}
	, {
	"nosta", 0, &staen, 0, "Top-most status line disabled", "Top-most status line enabled", " Disable status line "}
	, {
	"keepup", 0, &keepup, 0, "Status line updated constantly", "Status line updated once/sec", " Fast status line "}
	, {
	"pg", 1, &pgamnt, 0, "Lines to keep for PgUp/PgDn or -1 for 1/2 window (%d): ", 0, " No. PgUp/PgDn lines ", 0, -1, 64}
	, {
	"csmode", 0, &csmode, 0, "Start search after a search repeats previous search", "Start search always starts a new search", "Continued search "}
	, {
	"rdonly", 4, 0, (char *) &fdefault.readonly, "Read only", "Full editing", "O Read only "}
	, {
	"backpath", 2, (int *) &backpath, 0, "Backup files stored in (%s): ", 0, "Path to backup files "}
	, {
	"nonotice", 0, &nonotice, 0, 0, 0, 0}
	, {
	"noxon", 0, &noxon, 0, 0, 0, 0}
	, {
	"orphan", 0, &orphan, 0, 0, 0, 0}
	, {
	"help", 0, &help, 0, 0, 0, 0}
	, {
	"dopadding", 0, &dopadding, 0, 0, 0, 0}
	, {
	"lines", 1, &lines, 0, 0, 0, 0, 0, 2, 1024}
	, {
	"baud", 1, &Baud, 0, 0, 0, 0, 0, 50, 32767}
	, {
	"columns", 1, &columns, 0, 0, 0, 0, 0, 2, 1024}
	, {
	"skiptop", 1, &skiptop, 0, 0, 0, 0, 0, 0, 64}
	, {
	0, 0, 0}
};

int isiz = 0;

static void izopts(void)
{
	int x;

	for (x = 0; glopts[x].name; ++x)
		switch (glopts[x].type) {
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			glopts[x].ofst = glopts[x].addr - (char *) &fdefault;
		}
	isiz = 1;
}

int glopt(char *s, char *arg, OPTIONS * options, int set)
{
	int val;
	int ret = 0;
	int st = 1;
	int x;

	if (!isiz)
		izopts();
	if (s[0] == '-')
		st = 0, ++s;
	for (x = 0; glopts[x].name; ++x)
		if (!strcmp(glopts[x].name, s)) {
			switch (glopts[x].type) {
				case 0:
				if (set)
					*glopts[x].set = st;
				break;

				case 1:
				if (set && arg) {
					sscanf(arg, "%d", &val);
					if (val >= glopts[x].low && val <= glopts[x].high)
						*glopts[x].set = val;
				}
				break;

				case 2:
				if (set) {
					if (arg)
						*(char **) glopts[x].set = strdup(arg);
					else
						*(char **) glopts[x].set = 0;
				}
				break;

				case 4:
				if (options)
					*(int *) ((char *) options + glopts[x].ofst) = st;
				else if (set == 2)
					*(int *) ((char *) &fdefault + glopts[x].ofst) = st;
				break;

				case 5:
				if (arg) {
					if (options) {
						sscanf(arg, "%d", &val);
						if (val >= glopts[x].low && val <= glopts[x].high)
							*(int *) ((char *)
								  options + glopts[x].ofst) = val;
					} else if (set == 2) {
						sscanf(arg, "%d", &val);
						if (val >= glopts[x].low && val <= glopts[x].high)
							*(int *) ((char *)
								  &fdefault + glopts[x].ofst) = val;
					}
				}
				break;

				case 7:
				if (arg) {
					int zz = 0;

					sscanf(arg, "%d", &zz);
					if (zz >= glopts[x].low && zz <= glopts[x].high) {
						--zz;
						if (options)
							*(int *) ((char *)
								  options + glopts[x].ofst) = zz;
						else if (set == 2)
							*(int *) ((char *)
								  &fdefault + glopts[x].ofst) = zz;
					}
				}
				break;
			}
			if ((glopts[x].type & 3) == 0 || !arg)
				return 1;
			else
				return 2;
		}
	if (!strcmp(s, "lmsg")) {
		if (arg) {
			if (options)
				options->lmsg = strdup(arg);
			else if (set == 2)
				fdefault.lmsg = strdup(arg);
			ret = 2;
		} else
			ret = 1;
	} else if (!strcmp(s, "rmsg")) {
		if (arg) {
			if (options)
				options->rmsg = strdup(arg);
			else if (set == 2)
				fdefault.rmsg = strdup(arg);
			ret = 2;
		} else
			ret = 1;
	} else if (!strcmp(s, "keymap")) {
		if (arg) {
			int y;

			for (y = 0; !isspace(arg[y]); ++y) ;
			if (!arg[y])
				arg[y] = 0;
			if (options && y)
				options->context = strdup(arg);
			ret = 2;
		} else
			ret = 1;
	} else if (!strcmp(s, "mnew")) {
		if (arg) {
			int sta;

			if (options)
				options->mnew = mparse(NULL, arg, &sta);
			else if (set == 2)
				fdefault.mnew = mparse(NULL, arg, &sta);
			ret = 2;
		} else
			ret = 1;
	} else if (!strcmp(s, "mold")) {
		if (arg) {
			int sta;

			if (options)
				options->mold = mparse(NULL, arg, &sta);
			else if (set == 2)
				fdefault.mold = mparse(NULL, arg, &sta);
			ret = 2;
		} else
			ret = 1;
	} else if (!strcmp(s, "msnew")) {
		if (arg) {
			int sta;

			if (options)
				options->msnew = mparse(NULL, arg, &sta);
			else if (set == 2)
				fdefault.msnew = mparse(NULL, arg, &sta);
			ret = 2;
		} else
			ret = 1;
	} else if (!strcmp(s, "msold")) {
		if (arg) {
			int sta;

			if (options)
				options->msold = mparse(NULL, arg, &sta);
			else if (set == 2)
				fdefault.msold = mparse(NULL, arg, &sta);
			ret = 2;
		} else
			ret = 1;
	}

	return ret;
}

static int optx = 0;

static int doabrt1(BW * bw, int *xx)
{
	free(xx);
	return -1;
}

static int doopt1(BW * bw, char *s, int *xx, int *notify)
{
	int ret = 0;
	int x = *xx;
	int v;

	free(xx);
	switch (glopts[x].type) {
		case 1:
		v = calc(bw, s);
		if (merr) {
			msgnw(bw->parent, merr);
			ret = -1;
		} else if (v >= glopts[x].low && v <= glopts[x].high)
			*glopts[x].set = v;
		else
			msgnw(bw->parent, "Value out of range"), ret = -1;
		break;
		case 2:
		if (s[0])
			*(char **) glopts[x].set = strdup(s);
		break;
		case 5:
		v = calc(bw, s);
		if (merr) {
			msgnw(bw->parent, merr);
			ret = -1;
		} else if (v >= glopts[x].low && v <= glopts[x].high)
			*(int *) ((char *) &bw->o + glopts[x].ofst) = v;
		else
			msgnw(bw->parent, "Value out of range"), ret = -1;
		break;
		case 7:
		v = calc(bw, s) - 1.0;
		if (merr) {
			msgnw(bw->parent, merr);
			ret = -1;
		} else if (v >= glopts[x].low && v <= glopts[x].high)
			*(int *) ((char *) &bw->o + glopts[x].ofst) = v;
		else
			msgnw(bw->parent, "Value out of range"), ret = -1;
		break;
	}
	vsrm(s);
	bw->b->o = bw->o;
	wfit(bw->parent->t);
	updall();
	if (notify)
		*notify = 1;
	return ret;
}

static int doopt(MENU * m, int x, void *object, int flg)
{
	BW *bw = m->parent->win->object;
	int *xx;
	char buf[OPT_BUF_SIZE];
	int *notify = m->parent->notify;

	switch (glopts[x].type) {
		case 0:
		if (!flg)
			*glopts[x].set = !*glopts[x].set;
		else if (flg == 1)
			*glopts[x].set = 1;
		else
			*glopts[x].set = 0;
		uabort(m, MAXINT);
		msgnw(bw->parent, *glopts[x].set ? glopts[x].yes : glopts[x].no);
		break;

		case 4:
		if (!flg)
			*(int *) ((char *) &bw->o + glopts[x].ofst) = !*(int *) ((char *) &bw->o + glopts[x].ofst);
		else if (flg == 1)
			*(int *) ((char *) &bw->o + glopts[x].ofst) = 1;
		else
			*(int *) ((char *) &bw->o + glopts[x].ofst) = 0;
		uabort(m, MAXINT);
		msgnw(bw->parent, *(int *) ((char *) &bw->o + glopts[x].ofst) ? glopts[x].yes : glopts[x].no);
		if (glopts[x].ofst == (char *) &fdefault.readonly - (char *) &fdefault)
			bw->b->rdonly = bw->o.readonly;
		break;

		case 1:
		snprintf(buf, OPT_BUF_SIZE, glopts[x].yes, *glopts[x].set);
		xx = (int *) malloc(sizeof(int));

		*xx = x;
		m->parent->notify = 0;
		uabort(m, MAXINT);
		if (wmkpw(bw->parent, buf, NULL, doopt1, NULL, doabrt1, utypebw, xx, notify))
			return 0;
		else
			return -1;

		case 2:
		if (*(char **) glopts[x].set)
			snprintf(buf, OPT_BUF_SIZE, glopts[x].yes, *(char **) glopts[x].set);
		else
			snprintf(buf, OPT_BUF_SIZE, glopts[x].yes, "");
		xx = (int *) malloc(sizeof(int));

		*xx = x;
		m->parent->notify = 0;
		uabort(m, MAXINT);
		if (wmkpw(bw->parent, buf, NULL, doopt1, NULL, doabrt1, utypebw, xx, notify))
			return 0;
		else
			return -1;

		case 5:
		snprintf(buf, OPT_BUF_SIZE, glopts[x].yes, *(int *) ((char *) &bw->o + glopts[x].ofst));
		goto in;

		case 7:
		snprintf(buf, OPT_BUF_SIZE, glopts[x].yes, *(int *) ((char *) &bw->o + glopts[x].ofst) + 1);
	      in:xx = (int *) malloc(sizeof(int));

		*xx = x;
		m->parent->notify = 0;
		uabort(m, MAXINT);
		if (wmkpw(bw->parent, buf, NULL, doopt1, NULL, doabrt1, utypebw, xx, notify))
			return 0;
		else
			return -1;
	}
	if (notify)
		*notify = 1;
	bw->b->o = bw->o;
	wfit(bw->parent->t);
	updall();
	return 0;
}

static int doabrt(MENU * m, int x, char **s)
{
	optx = x;
	for (x = 0; s[x]; ++x)
		free(s[x]);
	free(s);
	return -1;
}

int umode(BW * bw)
{
	int size;
	char **s;
	int x;

	bw->b->o.readonly = bw->o.readonly = bw->b->rdonly;
	for (size = 0; glopts[size].menu; ++size) ;
	s = (char **) malloc(sizeof(char *) * (size + 1));

	for (x = 0; x != size; ++x) {
		s[x] = (char *) malloc(40);
		switch (glopts[x].type) {
			case 0:
			snprintf(s[x], OPT_BUF_SIZE, "%s%s", glopts[x].menu, *glopts[x].set ? "ON" : "OFF");
			break;

			case 1:
			snprintf(s[x], OPT_BUF_SIZE, "%s%d", glopts[x].menu, *glopts[x].set);
			break;

			case 2:
			strcpy(s[x], glopts[x].menu);
			break;

			case 4:
			snprintf(s[x], OPT_BUF_SIZE, "%s%s", glopts[x].menu, *(int *) ((char *) &bw->o + glopts[x].ofst) ? "ON" : "OFF");
			break;

			case 5:
			snprintf(s[x], OPT_BUF_SIZE, "%s%d", glopts[x].menu, *(int *) ((char *) &bw->o + glopts[x].ofst));
			break;

			case 7:
			snprintf(s[x], OPT_BUF_SIZE, "%s%d", glopts[x].menu, *(int *) ((char *) &bw->o + glopts[x].ofst) + 1);
			break;
		}
	}
	s[x] = 0;
	if (mkmenu(bw->parent, s, doopt, doabrt, NULL, optx, s, NULL))
		return 0;
	else
		return -1;
}


/* Process rc file
 * Returns 0 if the rc file was succefully processed
 *        -1 if the rc file couldn't be opened
 *         1 if there was a syntax error in the file
 */

int procrc(CAP * cap, char *name)
{
	OPTIONS *o = 0;		/* Current options */
	KMAP *context = 0;	/* Current context */
	unsigned char buf[1024];	/* Input buffer */
	FILE *fd;		/* rc file */
	int line = 0;		/* Line number */
	int err = 0;		/* Set to 1 if there was a syntax error */

	strcpy(buf, name);
#ifdef __MSDOS__
	fd = fopen(buf, "rt");
#else
	fd = fopen(buf, "r");
#endif

	if (!fd)
		return -1;	/* Return if we couldn't open the rc file */

	fprintf(stderr, "Processing '%s'...", name);
	fflush(stderr);

	while (++line, fgets(buf, 1024, fd))
		switch (buf[0]) {
			case ' ':
			case '\t':
			case '\n':
			case '\f':
			case 0:
			break;	/* Skip comment lines */

			case '*':	/* Select file types for file-type dependant options */
			{
				int x;

				o = (OPTIONS *) malloc(sizeof(OPTIONS));
				*o = fdefault;
				for (x = 0; buf[x] && buf[x] != '\n' && buf[x] != ' ' && buf[x] != '\t'; ++x) ;
				buf[x] = 0;
				o->next = options;
				options = o;
				o->name = strdup(buf);
			}
			break;

			case '-':	/* Set an option */
			{
				unsigned char *opt = buf + 1;
				int x;
				unsigned char *arg = 0;

				for (x = 0; buf[x] && buf[x] != '\n' && buf[x] != ' ' && buf[x] != '\t'; ++x) ;
				if (buf[x] && buf[x] != '\n') {
					buf[x] = 0;
					for (arg = buf + ++x; buf[x] && buf[x] != '\n'; ++x) ;
				}
				buf[x] = 0;
				if (!glopt(opt, arg, o, 2)) {
					err = 1;
					fprintf(stderr, "\n%s %d: Unknown option %s", name, line, opt);
				}
			}
			break;

			case '{':	/* Ignore help text */
			{
				while ((fgets(buf, 256, fd)) && (buf[0] != '}'));
				if (buf[0] != '}') {
					err = 1;
					fprintf(stderr, "\n%s %d: End of joerc file occured before end of help text\n", name, line);
					break;
				}
			}
			break;

			case ':':	/* Select context */
			{
				int x, c;

				for (x = 1; !isspace_eof(buf[x]); ++x) ;
				c = buf[x];
				buf[x] = 0;
				if (x != 1)
					if (!strcmp(buf + 1, "def")) {
						int y;

						for (buf[x] = c; isblank(buf[x]); ++x) ;
						for (y = x; !isspace_eof(buf[y]); ++y) ;
						c = buf[y];
						buf[y] = 0;
						if (y != x) {
							int sta;
							MACRO *m;

							if (isblank(c)
							    && (m = mparse(NULL, buf + y + 1, &sta)))
								addcmd(buf + x, m);
							else {
								err = 1;
								fprintf(stderr, "\n%s %d: macro missing from :def", name, line);
							}
						} else {
							err = 1;
							fprintf(stderr, "\n%s %d: command name missing from :def", name, line);
						}
					} else if (!strcmp(buf + 1, "inherit"))
						if (context) {
							for (buf[x] = c; isblank(buf[x]); ++x) ;
							for (c = x; !isspace_eof(buf[c]); ++c) ;
							buf[c] = 0;
							if (c != x)
								kcpy(context, getcontext(buf + x));
							else {
								err = 1;
								fprintf(stderr, "\n%s %d: context name missing from :inherit", name, line);
							}
						} else {
							err = 1;
							fprintf(stderr, "\n%s %d: No context selected for :inherit", name, line);
					} else if (!strcmp(buf + 1, "include")) {
						for (buf[x] = c; isblank(buf[x]); ++x) ;
						for (c = x; !isspace_eof(buf[c]); ++c) ;
						buf[c] = 0;
						if (c != x) {
							switch (procrc(cap, buf + x)) {
								case 1:
								err = 1;
								break;
								case -1:
								fprintf(stderr, "\n%s %d: Couldn't open %s", name, line, buf + x);
								err = 1;
								break;
							}
							context = 0;
							o = 0;
						} else {
							err = 1;
							fprintf(stderr, "\n%s %d: :include missing file name", name, line);
						}
					} else if (!strcmp(buf + 1, "delete"))
						if (context) {
							int y;

							for (buf[x] = c; isblank(buf[x]); ++x) ;
							for (y = x; buf[y] != 0 && buf[y] != '\t' && buf[y] != '\n' && (buf[y] != ' ' || buf[y + 1]
															!= ' '); ++y) ;
							buf[y] = 0;
							kdel(context, buf + x);
						} else {
							err = 1;
							fprintf(stderr, "\n%s %d: No context selected for :delete", name, line);
					} else
						context = getcontext(buf + 1);
				else {
					err = 1;
					fprintf(stderr, "\n%s %d: Invalid context name", name, line);
				}
			}
			break;

			default:	/* Get key-sequence to macro binding */
			{
				int x, y;
				MACRO *m;

				if (!context) {
					err = 1;
					fprintf(stderr, "\n%s %d: No context selected for macro to key-sequence binding", name, line);
					break;
				}

				m = 0;
			      macroloop:
				m = mparse(m, buf, &x);
				if (x == -1) {
					err = 1;
					fprintf(stderr, "\n%s %d: Unknown command in macro", name, line);
					break;
				} else if (x == -2) {
					fgets(buf, 1024, fd);
					goto macroloop;
				}
				if (!m)
					break;

				/* Skip to end of key sequence */
				for (y = x; buf[y] != 0 && buf[y] != '\t' && buf[y] != '\n' && (buf[y] != ' ' || buf[y + 1] != ' '); ++y) ;
				buf[y] = 0;

				/* Add binding to context */
				if (kadd(cap, context, buf + x, m) == -1) {
					fprintf(stderr, "\n%s %d: Bad key sequence '%s'", name, line, buf + x);
					err = 1;
				}
			}
			break;
		}
	fclose(fd);		/* Close rc file */

	/* Print proper ending string */
	if (err)
		fprintf(stderr, "\ndone\n");
	else
		fprintf(stderr, "done\n");

	return err;		/* 0 for success, 1 for syntax error */
}
