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
#include <string.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "cmd.h"
#include "kbd.h"
#include "macro.h"
#include "menu.h"
#include "path.h"
#include "pw.h"
#include "tw.h"
#include "mouse.h"
#include "uedit.h"
#include "umath.h"
#include "utils.h"
#include "vs.h"
#include "b.h"
#include "syntax.h"
#include "va.h"
#include "utf8.h"
#include "charmap.h"
#include "ufile.h"
#include "usearch.h"
#include "ushell.h"
#include "undo.h"
#include "w.h"

#define OPT_BUF_SIZE 300

static struct context {
	struct context *next;
	unsigned char *name;
	KMAP *kmap;
} *contexts = NULL;		/* List of named contexts */

/* Find a context of a given name- if not found, one with an empty kmap
 * is created.
 */

KMAP *kmap_getcontext(unsigned char *name)
{
	struct context *c;

	for (c = contexts; c; c = c->next)
		if (!strcmp(c->name, name))
			return c->kmap;
	c = (struct context *) joe_malloc(sizeof(struct context));

	c->next = contexts;
	c->name = joe_strdup(name);
	contexts = c;
	return c->kmap = mkkmap();
}

/* JM - ngetcontext(name) - like getcontext, but return NULL if it
 * doesn't exist, instead of creating a new one.
 */

KMAP *ngetcontext(unsigned char *name)
{
	struct context *c;
	for(c=contexts;c;c=c->next)
		if(!strcmp(c->name,name))
			return c->kmap;
	return 0;
}

unsigned char **get_keymap_list()
{
	unsigned char **lst = 0;
	struct context *c;
	for (c=contexts; c; c=c->next)
		lst = vaadd(lst, vsncpy(NULL,0,sz(c->name)));

	return lst;
}

OPTIONS *options = NULL;

/* Global variable options */
extern int mid, dspasis, dspctrl, force, help, pgamnt, square, csmode, nobackups, lightoff, exask, skiptop;
extern int noxon, lines, staen, columns, Baud, dopadding, orphan, marking, beep, keepup, nonotice;
extern int notite, usetabs, assume_color, guesscrlf, guessindent, menu_explorer, icase, wrap, autoswap;
extern int menu_jump, break_links;
extern int xmouse;
extern unsigned char *backpath;
extern int nolocks, nomodcheck;

/* Set to use ~/.joe_state file */
int joe_state;

/* Default options for prompt windows */

OPTIONS pdefault = {
	NULL,		/* *next */
	NULL,		/* *name_regex */
	NULL,		/* *contents_regex */
	0,		/* overtype */
	0,		/* lmargin */
	76,		/* rmargin */
	0,		/* autoindent */
	0,		/* wordwrap */
	8,		/* tab */
	' ',		/* indent char */
	1,		/* indent step */
	NULL,		/* *context */
	NULL,		/* *lmsg */
	NULL,		/* *rmsg */
	0,		/* line numbers */
	0,		/* read only */
	0,		/* french spacing */
	0,		/* spaces */
#ifdef __MSDOS__
	1,		/* crlf */
#else
	0,		/* crlf */
#endif
	0,		/* Highlight */
	NULL,		/* Syntax name */
	NULL,		/* Syntax */
	NULL,		/* Name of character set */
	NULL,		/* Character set */
	0,		/* Smart home key */
	0,		/* Goto indent first */
	0,		/* Smart backspace key */
	0,		/* Purify indentation */
	0,		/* Picture mode */
	0,		/* single_quoted */
	0,		/* c_comment */
	0,		/* cpp_comment */
	0,		/* pound_comment */
	0,		/* vhdl_comment */
	0,		/* semi_comment */
	NULL,		/* text_delimiters */
	NULL,		/* macro to execute for new files */
	NULL,		/* macro to execute for existing files */
	NULL,		/* macro to execute before saving new files */
	NULL,		/* macro to execute before saving existing files */
	NULL		/* macro to execute on first change */
};

/* Default options for file windows */

OPTIONS fdefault = {
	NULL,		/* *next */
	NULL,		/* *name_regex */
	NULL,		/* *contents_regex */
	0,		/* overtype */
	0,		/* lmargin */
	76,		/* rmargin */
	0,		/* autoindent */
	0,		/* wordwrap */
	8,		/* tab */
	' ',		/* indent char */
	1,		/* indent step */
	US "main",		/* *context */
	US "\\i%n %m %M",	/* *lmsg */
	US " %S Ctrl-K H for help",	/* *rmsg */
	0,		/* line numbers */
	0,		/* read only */
	0,		/* french spacing */
	0,		/* spaces */
#ifdef __MSDOS__
	1,		/* crlf */
#else
	0,		/* crlf */
#endif
	0,		/* Highlight */
	NULL,		/* Syntax name */
	NULL,		/* Syntax */
	NULL,		/* Name of character set */
	NULL,		/* Character set */
	0,		/* Smart home key */
	0,		/* Goto indent first */
	0,		/* Smart backspace key */
	0,		/* Purity indentation */
	0,		/* Picture mode */
	0,		/* single_quoted */
	0,		/* c_comment */
	0,		/* cpp_comment */
	0,		/* pound_comment */
	0,		/* vhdl_comment */
	0,		/* semi_comment */
	NULL,		/* text_delimiters */
	NULL, NULL, NULL, NULL, NULL	/* macros (see above) */
};

/* Update options */

void lazy_opts(OPTIONS *o)
{
	o->syntax = load_dfa(o->syntax_name);
	o->charmap = find_charmap(o->map_name);
	if (!o->charmap)
		o->charmap = locale_map;
}

/* Set local options depending on file name and contents */

void setopt(B *b, unsigned char *parsed_name)
{
	OPTIONS *o;
	int x;
	unsigned char *pieces[26];
	for (x = 0; x!=26; ++x)
		pieces[x] = NULL;

	for (o = options; o; o = o->next)
		if (rmatch(o->name_regex, parsed_name)) {
			if(o->contents_regex) {
				P *p = pdup(b->bof);
				if (pmatch(pieces,o->contents_regex,strlen((char *)o->contents_regex),p,0,0)) {
					prm(p);
					b->o = *o;
					lazy_opts(&b->o);
					goto done;
				} else {
					prm(p);
				}
			} else {
				b->o = *o;
				lazy_opts(&b->o);
				goto done;
			}
		}

	b->o = fdefault;
	lazy_opts(&b->o);

	done:
	for (x = 0; x!=26; ++x)
		vsrm(pieces[x]);
}

/* Table of options and how to set them */

/* local means it's in an OPTION structure, global means it's in a global
 * variable */

struct glopts {
	unsigned char *name;		/* Option name */
	int type;		/*      0 for global option flag
				   1 for global option numeric
				   2 for global option string
				   4 for local option flag
				   5 for local option numeric
				   6 for local option string
				   7 for local option numeric+1, with range checking
				 */
	int *set;		/* Address of global option */
	unsigned char *addr;		/* Local options structure member address */
	unsigned char *yes;		/* Message if option was turned on, or prompt string */
	unsigned char *no;		/* Message if option was turned off */
	unsigned char *menu;		/* Menu string */
	int ofst;		/* Local options structure member offset */
	int low;		/* Low limit for numeric options */
	int high;		/* High limit for numeric options */
} glopts[] = {
	{US "overwrite",4, NULL, (unsigned char *) &fdefault.overtype, US "Overtype mode", US "Insert mode", US "T Overtype " },
	{US "autoindent",	4, NULL, (unsigned char *) &fdefault.autoindent, US "Autoindent enabled", US "Autoindent disabled", US "I Autoindent " },
	{US "wordwrap",	4, NULL, (unsigned char *) &fdefault.wordwrap, US "Wordwrap enabled", US "Wordwrap disabled", US "Word wrap " },
	{US "tab",	5, NULL, (unsigned char *) &fdefault.tab, US "Tab width (%d): ", 0, US "D Tab width ", 0, 1, 64 },
	{US "lmargin",	7, NULL, (unsigned char *) &fdefault.lmargin, US "Left margin (%d): ", 0, US "Left margin ", 0, 1, 63 },
	{US "rmargin",	7, NULL, (unsigned char *) &fdefault.rmargin, US "Right margin (%d): ", 0, US "Right margin ", 0, 7, 255 },
	{US "square",	0, &square, NULL, US "Rectangle mode", US "Text-stream mode", US "X Rectangle mode " },
	{US "icase",	0, &icase, NULL, US "Ignore case by default", US "Case sensitive by default", US "  Case insensitivity " },
	{US "wrap",	0, &wrap, NULL, US "Search wraps", US "Search doesn't wrap", US "  Search wraps " },
	{US "menu_explorer",	0, &menu_explorer, NULL, US "Menu explorer mode", US "Simple completion", US "  Menu explorer " },
	{US "menu_jump",	0, &menu_jump, NULL, US "Jump is on", US "Jump is off", US "  Jump into menu " },
	{US "autoswap",	0, &autoswap, NULL, US "Autoswap ^KB and ^KK", US "  Autoswap off ", US "  Autoswap mode " },
	{US "indentc",	5, NULL, (unsigned char *) &fdefault.indentc, US "Indent char %d (SPACE=32, TAB=9, ^C to abort): ", 0, US "  Indent char ", 0, 0, 255 },
	{US "istep",	5, NULL, (unsigned char *) &fdefault.istep, US "Indent step %d (^C to abort): ", 0, US "  Indent step ", 0, 1, 64 },
	{US "french",	4, NULL, (unsigned char *) &fdefault.french, US "One space after periods for paragraph reformat", US "Two spaces after periods for paragraph reformat", US "  french spacing " },
	{US "highlight",	4, NULL, (unsigned char *) &fdefault.highlight, US "Highlighting enabled", US "Highlighting disabled", US "Highlighting " },
	{US "spaces",	4, NULL, (unsigned char *) &fdefault.spaces, US "Inserting spaces when tab key is hit", US "Inserting tabs when tab key is hit", US "  no tabs " },
	{US "mid",	0, &mid, NULL, US "Cursor will be recentered on scrolls", US "Cursor will not be recentered on scroll", US "Center on scroll " },
	{US "guess_crlf",0, &guesscrlf, NULL, US "Automatically detect MS-DOS files", US "Do not automatically detect MS-DOS files", US "  Auto detect CR-LF " },
	{US "guess_indent",0, &guessindent, NULL, US "Automatically detect indentation", US "Do not automatically detect indentation", US "  Guess indent " },
	{US "crlf",	4, NULL, (unsigned char *) &fdefault.crlf, US "CR-LF is line terminator", US "LF is line terminator", US "Z CR-LF (MS-DOS) " },
	{US "linums",	4, NULL, (unsigned char *) &fdefault.linums, US "Line numbers enabled", US "Line numbers disabled", US "N Line numbers " },
	{US "marking",	0, &marking, NULL, US "Anchored block marking on", US "Anchored block marking off", US "Marking " },
	{US "asis",	0, &dspasis, NULL, US "Characters above 127 shown as-is", US "Characters above 127 shown in inverse", US "  Meta chars as-is " },
	{US "force",	0, &force, NULL, US "Last line forced to have NL when file saved", US "Last line not forced to have NL", US "Force last NL " },
	{US "joe_state",0, &joe_state, NULL, US "~/.joe_state file will be updated", US "~/.joe_state file will not be updated", US "  Joe_state file " },
	{US "nobackups",	0, &nobackups, NULL, US "Backup files will not be made", US "Backup files will be made", US "  Disable backups " },
	{US "nolocks",	0, &nolocks, NULL, US "Files will not be locked", US "Files will be locked", US "  Disable locks " },
	{US "nomodcheck",	0, &nomodcheck, NULL, US "No mtime check", US "Mtime checking enabled", US "  Disable mtime check " },
	{US "break_links",	0, &break_links, NULL, US "Hardlinks will be broken", US "Hardlinks not broken", US "  Break hard links " },
	{US "lightoff",	0, &lightoff, NULL, US "Highlighting turned off after block operations", US "Highlighting not turned off after block operations", US "Auto unmark " },
	{US "exask",	0, &exask, NULL, US "Prompt for filename in save & exit command", US "Don't prompt for filename in save & exit command", US "  Exit ask " },
	{US "beep",	0, &beep, NULL, US "Warning bell enabled", US "Warning bell disabled", US "Beeps " },
	{US "nosta",	0, &staen, NULL, US "Top-most status line disabled", US "Top-most status line enabled", US "  Disable status line " },
	{US "keepup",	0, &keepup, NULL, US "Status line updated constantly", US "Status line updated once/sec", US "  Fast status line " },
	{US "pg",		1, &pgamnt, NULL, US "Lines to keep for PgUp/PgDn or -1 for 1/2 window (%d): ", 0, US "  No. PgUp/PgDn lines ", 0, -1, 64 },
	{US "csmode",	0, &csmode, NULL, US "Start search after a search repeats previous search", US "Start search always starts a new search", US "Continued search " },
	{US "rdonly",	4, NULL, (unsigned char *) &fdefault.readonly, US "Read only", US "Full editing", US "O Read only " },
	{US "smarthome",	4, NULL, (unsigned char *) &fdefault.smarthome, US "Smart home key enabled", US "Smart home key disabled", US "  Smart home key " },
	{US "indentfirst",	4, NULL, (unsigned char *) &fdefault.indentfirst, US "Smart home goes to indent first", US "Smart home goes home first", US "  To indent first " },
	{US "smartbacks",	4, NULL, (unsigned char *) &fdefault.smartbacks, US "Smart backspace key enabled", US "Smart backspace key disabled", US "  Smart backspace " },
	{US "purify",	4, NULL, (unsigned char *) &fdefault.purify, US "Indentation clean up enabled", US "Indentation clean up disabled", US "  Clean up indents " },
	{US "picture",	4, NULL, (unsigned char *) &fdefault.picture, US "Picture drawing mode enabled", US "Picture drawing mode disabled", US "Picture mode " },
	{US "backpath",	2, (int *) &backpath, NULL, US "Backup files stored in (%s): ", 0, US "  Path to backup files " },
	{US "syntax",	9, NULL, NULL, US "Select syntax (^C to abort): ", 0, US "Y Syntax" },
	{US "encoding",13, NULL, NULL, US "Select file character set (^C to abort): ", 0, US "Encoding " },
	{US "single_quoted",	4, NULL, (unsigned char *) &fdefault.single_quoted, US "Single quoting enabled", US "Single quoting disabled", US "  ^G ignores ' ' " },
	{US "c_comment",	4, NULL, (unsigned char *) &fdefault.c_comment, US "/* comments enabled", US "/* comments disabled", US "  ^G ignores /**/ " },
	{US "cpp_comment",	4, NULL, (unsigned char *) &fdefault.cpp_comment, US "// comments enabled", US "// comments disabled", US "  ^G ignores // " },
	{US "pound_comment",	4, NULL, (unsigned char *) &fdefault.pound_comment, US "# comments enabled", US "# comments disabled", US "  ^G ignores # " },
	{US "vhdl_comment",	4, NULL, (unsigned char *) &fdefault.vhdl_comment, US "-- comments enabled", US "-- comments disabled", US "  ^G ignores -- " },
	{US "semi_comment",	4, NULL, (unsigned char *) &fdefault.vhdl_comment, US "; comments enabled", US "; comments disabled", US "  ^G ignores ; " },
	{US "text_delimiters",	6, NULL, (unsigned char *) &fdefault.text_delimiters, US "Text delimiters (%s): ", 0, US "  Text delimiters " },
	{US "floatmouse",	0, &floatmouse, 0, "Clicking can move the cursor past end of line", "Clicking past end of line moves cursor to the end", "  Click past end " },
	{US "rtbutton",	0, &rtbutton, 0, "Mouse action is done with the right button", "Mouse action is done with the left button", "  Right button " },
	{US "nonotice",	0, &nonotice, NULL, 0, 0, 0 },
	{US "noxon",	0, &noxon, NULL, 0, 0, 0 },
	{US "orphan",	0, &orphan, NULL, 0, 0, 0 },
	{US "help",	0, &help, NULL, 0, 0, 0 },
	{US "dopadding",	0, &dopadding, NULL, 0, 0, 0 },
	{US "lines",	1, &lines, NULL, 0, 0, 0, 0, 2, 1024 },
	{US "baud",	1, &Baud, NULL, 0, 0, 0, 0, 50, 32767 },
	{US "columns",	1, &columns, NULL, 0, 0, 0, 0, 2, 1024 },
	{US "skiptop",	1, &skiptop, NULL, 0, 0, 0, 0, 0, 64 },
	{US "notite",	0, &notite, NULL, 0, 0, 0 },
	{US "mouse",	0, &xmouse, NULL, 0, 0, 0 },
	{US "usetabs",	0, &usetabs, NULL, 0, 0, 0 },
	{US "assume_color", 0, &assume_color, NULL, 0, 0, 0 },
	{ NULL,		0, NULL, NULL, NULL, NULL, NULL, 0, 0, 0 }
};

/* Initialize .ofsts above.  Is this really necessary? */

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
			glopts[x].ofst = glopts[x].addr - (unsigned char *) &fdefault;
		}
	isiz = 1;
}

/* Set a global or local option:
 * 's' is option name
 * 'arg' is a possible argument string (taken only if option has an arg)
 * 'options' points to options structure to modify (can be NULL).
 * 'set'==0: set only in 'options' if it's given.
 * 'set'!=0: set global variable option.
 * return value: no. of fields taken (1 or 2), or 0 if option not found.
 *
 * So this function is used both to set options, and to parse over options
 * without setting them.
 *
 * These combinations are used:
 *
 * glopt(name,arg,NULL,1): set global variable option
 * glopt(name,arg,NULL,0): parse over option
 * glopt(name,arg,options,0): set file local option
 * glopt(name,arg,&fdefault,1): set default file options
 * glopt(name,arg,options,1): set file local option
 */

int glopt(unsigned char *s, unsigned char *arg, OPTIONS *options, int set)
{
	int val;
	int ret = 0;
	int st = 1;	/* 1 to set option, 0 to clear it */
	int x;

	/* Initialize offsets */
	if (!isiz)
		izopts();

	/* Clear instead of set? */
	if (s[0] == '-') {
		st = 0;
		++s;
	}

	for (x = 0; glopts[x].name; ++x)
		if (!strcmp(glopts[x].name, s)) {
			switch (glopts[x].type) {
			case 0: /* Global variable flag option */
				if (set)
					*glopts[x].set = st;
				break;
			case 1: /* Global variable integer option */
				if (set && arg) {
					sscanf((char *)arg, "%d", &val);
					if (val >= glopts[x].low && val <= glopts[x].high)
						*glopts[x].set = val;
				}
				break;
			case 2: /* Global variable string option */
				if (set) {
					if (arg)
						*(unsigned char **) glopts[x].set = joe_strdup(arg);
					else
						*(unsigned char **) glopts[x].set = 0;
				}
				break;
			case 4: /* Local option flag */
				if (options)
					*(int *) ((unsigned char *) options + glopts[x].ofst) = st;
				break;
			case 5: /* Local option integer */
				if (arg) {
					if (options) {
						sscanf((char *)arg, "%d", &val);
						if (val >= glopts[x].low && val <= glopts[x].high)
							*(int *) ((unsigned char *)
								  options + glopts[x].ofst) = val;
					} 
				}
				break;
			case 6: /* Local string option */
				if (options) {
					if (arg) {
						*(unsigned char **) ((unsigned char *)
								  options + glopts[x].ofst) = joe_strdup(arg);
					} else {
						*(unsigned char **) ((unsigned char *)
								  options + glopts[x].ofst) = 0;
					}
				}
				break;
			case 7: /* Local option numeric + 1, with range checking */
				if (arg) {
					int zz = 0;

					sscanf((char *)arg, "%d", &zz);
					if (zz >= glopts[x].low && zz <= glopts[x].high) {
						--zz;
						if (options)
							*(int *) ((unsigned char *)
								  options + glopts[x].ofst) = zz;
					}
				}
				break;

			case 9: /* Set syntax */
				if (arg && options)
					options->syntax_name = joe_strdup(arg);
				/* this was causing all syntax files to be loaded...
				if (arg && options)
					options->syntax = load_dfa(arg); */
				break;

			case 13: /* Set byte mode encoding */
				if (arg && options)
					options->map_name = joe_strdup(arg);
				break;
			}
			/* This is a stupid hack... */
			if ((glopts[x].type & 3) == 0 || !arg)
				return 1;
			else
				return 2;
		}
	/* Why no case 6, string option? */
	/* Keymap, mold, mnew, etc. are not strings */
	/* These options do not show up in ^T */
	if (!strcmp(s, "lmsg")) {
		if (arg) {
			if (options)
				options->lmsg = joe_strdup(arg);
			ret = 2;
		} else
			ret = 1;
	} else if (!strcmp(s, "rmsg")) {
		if (arg) {
			if (options)
				options->rmsg = joe_strdup(arg);
			ret = 2;
		} else
			ret = 1;
	} else if (!strcmp(s, "keymap")) {
		if (arg) {
			int y;

			for (y = 0; !joe_isspace(locale_map,arg[y]); ++y) ;
			if (!arg[y])
				arg[y] = 0;
			if (options && y)
				options->context = joe_strdup(arg);
			ret = 2;
		} else
			ret = 1;
	} else if (!strcmp(s, "mnew")) {
		if (arg) {
			int sta;

			if (options)
				options->mnew = mparse(NULL, arg, &sta);
			ret = 2;
		} else
			ret = 1;
	} else if (!strcmp(s, "mfirst")) {
		if (arg) {
			int sta;

			if (options)
				options->mfirst = mparse(NULL, arg, &sta);
			ret = 2;
		} else
			ret = 1;
	} else if (!strcmp(s, "mold")) {
		if (arg) {
			int sta;

			if (options)
				options->mold = mparse(NULL, arg, &sta);
			ret = 2;
		} else
			ret = 1;
	} else if (!strcmp(s, "msnew")) {
		if (arg) {
			int sta;

			if (options)
				options->msnew = mparse(NULL, arg, &sta);
			ret = 2;
		} else
			ret = 1;
	} else if (!strcmp(s, "msold")) {
		if (arg) {
			int sta;

			if (options)
				options->msold = mparse(NULL, arg, &sta);
			ret = 2;
		} else
			ret = 1;
	}

	return ret;
}

/* Option setting user interface (^T command) */

static int optx = 0; /* Menu cursor position: remember it for next time */

static int doabrt1(BW *bw, int *xx)
{
	joe_free(xx);
	return -1;
}

static int doopt1(BW *bw, unsigned char *s, int *xx, int *notify)
{
	int ret = 0;
	int x = *xx;
	int v;

	joe_free(xx);
	switch (glopts[x].type) {
	case 1:
		v = calc(bw, s);
		if (merr) {
			msgnw(bw->parent, merr);
			ret = -1;
		} else if (v >= glopts[x].low && v <= glopts[x].high)
			*glopts[x].set = v;
		else {
			msgnw(bw->parent, US "Value out of range");
			ret = -1;
		}
		break;
	case 2:
		if (s[0])
			*(unsigned char **) glopts[x].set = joe_strdup(s);
		break;
	case 6:
		*(unsigned char **)((unsigned char *)&bw->o+glopts[x].ofst) = joe_strdup(s);
		break;
	case 5:
		v = calc(bw, s);
		if (merr) {
			msgnw(bw->parent, merr);
			ret = -1;
		} else if (v >= glopts[x].low && v <= glopts[x].high)
			*(int *) ((unsigned char *) &bw->o + glopts[x].ofst) = v;
		else {
			msgnw(bw->parent, US "Value out of range");
			ret = -1;
		}
		break;
	case 7:
		v = calc(bw, s) - 1.0;
		if (merr) {
			msgnw(bw->parent, merr);
			ret = -1;
		} else if (v >= glopts[x].low && v <= glopts[x].high)
			*(int *) ((unsigned char *) &bw->o + glopts[x].ofst) = v;
		else {
			msgnw(bw->parent, US "Value out of range");
			ret = -1;
		}
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

static int dosyntax(BW *bw, unsigned char *s, int *xx, int *notify)
{
	int ret = 0;
	struct high_syntax *syn;

	syn = load_dfa(s);

	if (syn)
		bw->o.syntax = syn;
	else
		msgnw(bw->parent, US "Syntax definition file not found");

	vsrm(s);
	bw->b->o = bw->o;
	updall();
	if (notify)
		*notify = 1;
	return ret;
}

unsigned char **syntaxes = NULL; /* Array of available syntaxes */

static int syntaxcmplt(BW *bw)
{
	if (!syntaxes) {
		unsigned char *oldpwd = pwd();
		unsigned char **t;
		unsigned char *p;
		int x, y;

		if (chpwd(US (JOERC "syntax")))
			return -1;
		t = rexpnd(US "*.jsf");
		if (!t) {
			chpwd(oldpwd);
			return -1;
		}
		if (!aLEN(t)) {
			varm(t);
			chpwd(oldpwd);
			return -1;
		}

		for (x = 0; x != aLEN(t); ++x) {
			unsigned char *r = vsncpy(NULL,0,t[x],(unsigned char *)strrchr((char *)(t[x]),'.')-t[x]);
			syntaxes = vaadd(syntaxes,r);
		}
		varm(t);

		p = (unsigned char *)getenv("HOME");
		if (p) {
			unsigned char buf[1024];
			joe_snprintf_1((char *)buf,sizeof(buf),"%s/.joe/syntax",p);
			if (!chpwd(buf) && (t = rexpnd(US "*.jsf"))) {
				for (x = 0; x != aLEN(t); ++x)
					*strrchr((char *)t[x],'.') = 0;
				for (x = 0; x != aLEN(t); ++x) {
					for (y = 0; y != aLEN(syntaxes); ++y)
						if (!strcmp(t[x],syntaxes[y]))
							break;
					if (y == aLEN(syntaxes)) {
						unsigned char *r = vsncpy(NULL,0,sv(t[x]));
						syntaxes = vaadd(syntaxes,r);
					}
				}
				varm(t);
			}
		}

		vasort(av(syntaxes));
		chpwd(oldpwd);
	}
	return simple_cmplt(bw,syntaxes);
}

static int doencoding(BW *bw, unsigned char *s, int *xx, int *notify)
{
	int ret = 0;
	struct charmap *map;


	map = find_charmap(s);

	if (map) {
		bw->o.charmap = map;
		joe_snprintf_1((char *)msgbuf, JOE_MSGBUFSIZE, "%s encoding assumed for this file", map->name);
		msgnw(bw->parent, msgbuf);
	} else
		msgnw(bw->parent, US "Character set not found");

	vsrm(s);
	bw->b->o = bw->o;
	updall();
	if (notify)
		*notify = 1;
	return ret;
}

unsigned char **encodings = NULL; /* Array of available encodinges */

static int encodingcmplt(BW *bw)
{
	if (!encodings) {
		encodings = get_encodings();
		vasort(av(encodings));
	}
	return simple_cmplt(bw,encodings);
}

static int doopt(MENU *m, int x, void *object, int flg)
{
	BW *bw = m->parent->win->object;
	int *xx;
	unsigned char buf[OPT_BUF_SIZE];
	int *notify = m->parent->notify;

	switch (glopts[x].type) {
	case 0:
		if (!flg)
			*glopts[x].set = !*glopts[x].set;
		else if (flg == 1)
			*glopts[x].set = 1;
		else
			*glopts[x].set = 0;
		wabort(m->parent);
		msgnw(bw->parent, *glopts[x].set ? glopts[x].yes : glopts[x].no);
		break;
	case 4:
		if (!flg)
			*(int *) ((unsigned char *) &bw->o + glopts[x].ofst) = !*(int *) ((unsigned char *) &bw->o + glopts[x].ofst);
		else if (flg == 1)
			*(int *) ((unsigned char *) &bw->o + glopts[x].ofst) = 1;
		else
			*(int *) ((unsigned char *) &bw->o + glopts[x].ofst) = 0;
		wabort(m->parent);
		msgnw(bw->parent, *(int *) ((unsigned char *) &bw->o + glopts[x].ofst) ? glopts[x].yes : glopts[x].no);
		if (glopts[x].ofst == (unsigned char *) &fdefault.readonly - (unsigned char *) &fdefault)
			bw->b->rdonly = bw->o.readonly;
		break;
	case 6:
		wabort(m->parent);
		xx = (int *) joe_malloc(sizeof(int));
		*xx = x;
		if(*(unsigned char **)((unsigned char *)&bw->o+glopts[x].ofst))
			joe_snprintf_1((char *)buf, OPT_BUF_SIZE, "Delimiters (%s): ",*(unsigned char **)((unsigned char *)&bw->o+glopts[x].ofst));
		else
			joe_snprintf_0((char *)buf, OPT_BUF_SIZE, "Delimiters: ");
		if(wmkpw(bw->parent, buf, NULL, doopt1, NULL, doabrt1, utypebw, xx, notify, locale_map))
			return 0;
		else
			return -1;
		break;
	case 1:
		joe_snprintf_1((char *)buf, OPT_BUF_SIZE, (char *)glopts[x].yes, *glopts[x].set);
		xx = (int *) joe_malloc(sizeof(int));

		*xx = x;
		m->parent->notify = 0;
		wabort(m->parent);
		if (wmkpw(bw->parent, buf, NULL, doopt1, NULL, doabrt1, utypebw, xx, notify, locale_map))
			return 0;
		else
			return -1;
	case 2:
		if (*(unsigned char **) glopts[x].set)
			joe_snprintf_1((char *)buf, OPT_BUF_SIZE, (char *)glopts[x].yes, *(unsigned char **) glopts[x].set);
		else
			joe_snprintf_1((char *)buf, OPT_BUF_SIZE, (char *)glopts[x].yes, "");
		xx = (int *) joe_malloc(sizeof(int));

		*xx = x;
		m->parent->notify = 0;
		wabort(m->parent);
		if (wmkpw(bw->parent, buf, NULL, doopt1, NULL, doabrt1, utypebw, xx, notify, locale_map))
			return 0;
		else
			return -1;
	case 5:
		joe_snprintf_1((char *)buf, OPT_BUF_SIZE, (char *)glopts[x].yes, *(int *) ((unsigned char *) &bw->o + glopts[x].ofst));
		goto in;
	case 7:
		joe_snprintf_1((char *)buf, OPT_BUF_SIZE, (char *)glopts[x].yes, *(int *) ((unsigned char *) &bw->o + glopts[x].ofst) + 1);
	      in:xx = (int *) joe_malloc(sizeof(int));

		*xx = x;
		m->parent->notify = 0;
		wabort(m->parent);
		if (wmkpw(bw->parent, buf, NULL, doopt1, NULL, doabrt1, utypebw, xx, notify, locale_map))
			return 0;
		else
			return -1;

	case 9:
		joe_snprintf_1((char *)buf, OPT_BUF_SIZE, (char *)glopts[x].yes, "");
		m->parent->notify = 0;
		wabort(m->parent);
		if (wmkpw(bw->parent, buf, NULL, dosyntax, NULL, NULL, syntaxcmplt, NULL, notify, locale_map))
			return 0;
		else
			return -1;

	case 13:
		joe_snprintf_1((char *)buf, OPT_BUF_SIZE, (char *)glopts[x].yes, "");
		m->parent->notify = 0;
		wabort(m->parent);
		if (wmkpw(bw->parent, buf, NULL, doencoding, NULL, NULL, encodingcmplt, NULL, notify, locale_map))
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

static int doabrt(MENU *m, int x, unsigned char **s)
{
	optx = x;
	for (x = 0; s[x]; ++x)
		joe_free(s[x]);
	joe_free(s);
	return -1;
}

int umode(BW *bw)
{
	int size;
	unsigned char **s;
	int x;

	bw->b->o.readonly = bw->o.readonly = bw->b->rdonly;
	for (size = 0; glopts[size].menu; ++size) ;
	s = (unsigned char **) joe_malloc(sizeof(unsigned char *) * (size + 1));

	for (x = 0; x != size; ++x) {
		s[x] = (unsigned char *) joe_malloc(80);		/* FIXME: why 40 ??? */
		switch (glopts[x].type) {
		case 0:
			joe_snprintf_2((char *)(s[x]), OPT_BUF_SIZE, "%s%s", glopts[x].menu, *glopts[x].set ? "ON" : "OFF");
			break;
		case 1:
			joe_snprintf_2((char *)(s[x]), OPT_BUF_SIZE, "%s%d", glopts[x].menu, *glopts[x].set);
			break;
		case 2:
		case 9:
		case 13:
		case 6:
			strcpy((char *)(s[x]), (char *)glopts[x].menu);
			break;
		case 4:
			joe_snprintf_2((char *)(s[x]), OPT_BUF_SIZE, "%s%s", glopts[x].menu, *(int *) ((unsigned char *) &bw->o + glopts[x].ofst) ? "ON" : "OFF");
			break;
		case 5:
			joe_snprintf_2((char *)(s[x]), OPT_BUF_SIZE, "%s%d", glopts[x].menu, *(int *) ((unsigned char *) &bw->o + glopts[x].ofst));
			break;
		case 7:
			joe_snprintf_2((char *)(s[x]), OPT_BUF_SIZE, "%s%d", glopts[x].menu, *(int *) ((unsigned char *) &bw->o + glopts[x].ofst) + 1);
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

int procrc(CAP *cap, unsigned char *name)
{
	OPTIONS *o = &fdefault;	/* Current options */
	KMAP *context = NULL;	/* Current context */
	unsigned char buf[1024];	/* Input buffer */
	FILE *fd;		/* rc file */
	int line = 0;		/* Line number */
	int err = 0;		/* Set to 1 if there was a syntax error */

	strncpy((char *)buf, (char *)name, sizeof(buf) - 1);
	buf[sizeof(buf)-1] = '\0';
#ifdef __MSDOS__
	fd = fopen((char *)buf, "rt");
#else
	fd = fopen((char *)buf, "r");
#endif

	if (!fd)
		return -1;	/* Return if we couldn't open the rc file */

	fprintf(stderr, "Processing '%s'...", name);
	fflush(stderr);

	while (fgets((char *)buf, sizeof(buf), fd)) {
		line++;
		switch (buf[0]) {
		case ' ':
		case '\t':
		case '\n':
		case '\f':
		case 0:
			break;	/* Skip comment lines */

		case '=':	/* Define a global color */
			{
			parse_color_def(&global_colors,buf+1,name,line);
			}
			break;

		case '*':	/* Select file types for file-type dependant options */
			{
				int x;

				o = (OPTIONS *) joe_malloc(sizeof(OPTIONS));
				*o = fdefault;
				for (x = 0; buf[x] && buf[x] != '\n' && buf[x] != ' ' && buf[x] != '\t'; ++x) ;
				buf[x] = 0;
				o->next = options;
				options = o;
				o->name_regex = joe_strdup(buf);
			}
			break;
		case '+':	/* Set file contents match regex */
			{
				int x;

				for (x = 0; buf[x] && buf[x] != '\n' && buf[x] != '\r'; ++x) ;
				buf[x] = 0;
				if (o)
					o->contents_regex = joe_strdup(buf+1);
			}
			break;
		case '-':	/* Set an option */
			{
				unsigned char *opt = buf + 1;
				int x;
				unsigned char *arg = NULL;

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
				while ((fgets((char *)buf, 256, fd)) && (buf[0] != '}'))
					++line;
				++line;
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

				for (x = 1; !joe_isspace_eof(locale_map,buf[x]); ++x) ;
				c = buf[x];
				buf[x] = 0;
				if (x != 1)
					if (!strcmp(buf + 1, "def")) {
						int y;

						for (buf[x] = c; joe_isblank(locale_map,buf[x]); ++x) ;
						for (y = x; !joe_isspace_eof(locale_map,buf[y]); ++y) ;
						c = buf[y];
						buf[y] = 0;
						if (y != x) {
							int sta;
							MACRO *m;

							if (joe_isblank(locale_map,c)
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
							for (buf[x] = c; joe_isblank(locale_map,buf[x]); ++x) ;
							for (c = x; !joe_isspace_eof(locale_map,buf[c]); ++c) ;
							buf[c] = 0;
							if (c != x)
								kcpy(context, kmap_getcontext(buf + x));
							else {
								err = 1;
								fprintf(stderr, "\n%s %d: context name missing from :inherit", name, line);
							}
						} else {
							err = 1;
							fprintf(stderr, "\n%s %d: No context selected for :inherit", name, line);
					} else if (!strcmp(buf + 1, "include")) {
						for (buf[x] = c; joe_isblank(locale_map,buf[x]); ++x) ;
						for (c = x; !joe_isspace_eof(locale_map,buf[c]); ++c) ;
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
							o = &fdefault;
						} else {
							err = 1;
							fprintf(stderr, "\n%s %d: :include missing file name", name, line);
						}
					} else if (!strcmp(buf + 1, "delete"))
						if (context) {
							int y;

							for (buf[x] = c; joe_isblank(locale_map,buf[x]); ++x) ;
							for (y = x; buf[y] != 0 && buf[y] != '\t' && buf[y] != '\n' && (buf[y] != ' ' || buf[y + 1]
															!= ' '); ++y) ;
							buf[y] = 0;
							kdel(context, buf + x);
						} else {
							err = 1;
							fprintf(stderr, "\n%s %d: No context selected for :delete", name, line);
					} else
						context = kmap_getcontext(buf + 1);
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
					fgets((char *)buf, 1024, fd);
					++line;
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
	}
	fclose(fd);		/* Close rc file */

	/* Print proper ending string */
	if (err)
		fprintf(stderr, "\ndone\n");
	else
		fprintf(stderr, "done\n");

	return err;		/* 0 for success, 1 for syntax error */
}

/* Save a history buffer */

void save_hist(FILE *f,B *b)
{
	unsigned char buf[512];
	int len;
	if (b) {
		P *p = pdup(b->bof);
		P *q = pdup(b->bof);
		if (b->eof->line>10)
			pline(p,b->eof->line-10);
		pset(q,p);
		while (!piseof(p)) {
			pnextl(q);
			if (q->byte-p->byte<512) {
				len = q->byte - p->byte;
				brmem(p,buf,len);
			} else {
				brmem(p,buf,512);
				len = 512;
			}
			fprintf(f,"\t");
			emit_hdlc(f,buf,len);
			fprintf(f,"\n");
			pset(p,q);
		}
		prm(p);
		prm(q);
	}
	fprintf(f,"done\n");
}

/* Load a history buffer */

void load_hist(FILE *f,B **bp)
{
	B *b;
	unsigned char buf[1024];
	unsigned char bf[1024];
	P *q;

	b = *bp;
	if (!b)
		*bp = b = bmk(NULL);

	q = pdup(b->eof);

	while(fgets(buf,1023,f) && strcmp(buf,"done\n")) {
		unsigned char *p = buf;
		int len;
		parse_ws(&p,'#');
		len = parse_hdlc(&p,bf,1023);
		if (len>0) {
			binsm(q,bf,len);
			pset(q,b->eof);
		}
	}

	prm(q);
}

/* Save state */

void save_state()
{
	unsigned char *home = (unsigned char *)getenv("HOME");
	int old_mask;
	FILE *f;
	if (!joe_state)
		return;
	if (!home)
		return;
	joe_snprintf_1(stdbuf,stdsiz,"%s/.joe_state",home);
	old_mask = umask(0066);
	f = fopen(stdbuf,"w");
	umask(old_mask);
	if(!f)
		return;

	/* Write state information */
	fprintf(f,"search\n"); save_srch(f);
	fprintf(f,"macros\n"); save_macros(f);
	fprintf(f,"files\n"); save_hist(f,filehist);
	fprintf(f,"find\n"); save_hist(f,findhist);
	fprintf(f,"replace\n"); save_hist(f,replhist);
	fprintf(f,"run\n"); save_hist(f,runhist);
	fprintf(f,"build\n"); save_hist(f,buildhist);
	fprintf(f,"cmd\n"); save_hist(f,cmdhist);
	fprintf(f,"math\n"); save_hist(f,mathhist);
	fprintf(f,"yank\n"); save_yank(f);
	fclose(f);
}

/* Load state */

void load_state()
{
	unsigned char *home = (unsigned char *)getenv("HOME");
	unsigned char buf[1024];
	FILE *f;
	if (!joe_state)
		return;
	if (!home)
		return;
	joe_snprintf_1(stdbuf,stdsiz,"%s/.joe_state",home);
	f = fopen(stdbuf,"r");
	if(!f)
		return;

	/* Read state information */
	while(fgets(buf,1023,f)) {
		if(!strcmp(buf,"search\n"))
			load_srch(f);
		else if(!strcmp(buf,"macros\n"))
			load_macros(f);
		else if(!strcmp(buf,"files\n"))
			load_hist(f,&filehist);
		else if(!strcmp(buf,"find\n"))
			load_hist(f,&findhist);
		else if(!strcmp(buf,"replace\n"))
			load_hist(f,&replhist);
		else if(!strcmp(buf,"run\n"))
			load_hist(f,&runhist);
		else if(!strcmp(buf,"build\n"))
			load_hist(f,&buildhist);
		else if(!strcmp(buf,"cmd\n"))
			load_hist(f,&cmdhist);
		else if(!strcmp(buf,"math\n"))
			load_hist(f,&mathhist);
		else if(!strcmp(buf,"yank\n"))
			load_yank(f);
		else { /* Unknown... skip until next done */
			while(fgets(buf,1023,f) && strcmp(buf,"done\n"));
		}
	}

	fclose(f);
}
