/*
 *	Help system
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *		(C) 2001 Marek 'Marx' Grac
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

#include <stdio.h>
#include <string.h>

#include "blocks.h"
#include "scrn.h"
#include "charmap.h"
#include "utils.h"
#include "vs.h"
#include "utf8.h"
#include "w.h"

#define NOT_ENOUGH_MEMORY -11

struct help *help_actual = NULL;	/* actual help screen */
struct help *help_ptr = NULL;		/* build pointer */

/* 
 * Process help file
 * Returns new line number
 */

int help_init(FILE *fd,unsigned char *bf,int line)
{
	unsigned char buf[1024];			/* input buffer */

	struct help *tmp;
	unsigned int bfl;				/* buffer length */
	unsigned int hlpsiz, hlpbsz;			/* number of used/allocated bytes for tmp->text */
	unsigned char *tempbuf;

	if (bf[0] == '{') {			/* start of help screen */
		tmp = (struct help *) joe_malloc(sizeof(struct help));

		tmp->text = NULL;
		tmp->lines = 0;
		hlpsiz = 0;
		hlpbsz = 0;
		tmp->name = vsncpy(NULL, 0, sz(bf + 1) - 1);

		while ((fgets((char *)buf, sizeof(buf), fd)) && (buf[0] != '}')) {
			++line;
			bfl = strlen((char *)buf);
			if (hlpsiz + bfl > hlpbsz) {
				if (tmp->text) {
					tempbuf = (unsigned char *) joe_realloc(tmp->text, hlpbsz + bfl + 1024);
					tmp->text = tempbuf;
				} else {
					tmp->text = (unsigned char *) joe_malloc(bfl + 1024);
					tmp->text[0] = 0;
				}
				hlpbsz += bfl + 1024;
			}
			strcpy((char *)(tmp->text + hlpsiz), (char *)buf);
			hlpsiz += bfl;
			++tmp->lines;
		}
		tmp->prev = help_ptr;
		tmp->next = NULL;
		if (help_ptr) {
			help_ptr->next = tmp;
		} else {
			help_actual = tmp;
		}
		help_ptr = tmp;
		if (buf[0] == '}') {		/* set new help screen as actual one */
			++line;
		} else {
			fprintf(stderr, "\n%d: EOF before end of help text\n",line);
		}
	}
	return line;
}

/*
 * Find context help - find help entry with the same name
 */

struct help *find_context_help(unsigned char *name)
{
	struct help *tmp = help_actual;

	while (tmp->prev != NULL)	/* find the first help entry */
		tmp = tmp->prev;

	while (tmp != NULL && strcmp(tmp->name, name) != 0)
		tmp = tmp->next;

	return tmp;
}

/*
 * Display help text
 */
void help_display(SCREEN *t)
{
	unsigned char *str;
	int y, x, c, z;
	int atr = 0;

	if (help_actual) {
		str = help_actual->text;
	} else {
		str = NULL;
	}

	for (y = skiptop; y != t->wind; ++y) {
		if (t->t->updtab[y]) {
			unsigned char *start = str, *eol;
			int width=0;
			int nspans=0;
			int spanwidth;
			int spancount=0;
			int spanextra;
			int len;

			eol = (unsigned char *)strchr((char *)str, '\n');

			/* First pass: count no. springs \| and determine minimum width */
			while(*str && *str!='\n') {
				if (*str == '\\') {
					++str;
					switch(*str) {
						case 'i':
						case 'I':
						case 'u':
						case 'U':
						case 'd':
						case 'D':
						case 'b':
						case 'B':
						case 'f':
						case 'F':
							++str;
							break;
						case '|':
							++str;
							++nspans;
							break;
						case 0:
							break;
						default:
							++str;
							++width;
					}
				} else {
					len = eol - str;
					c = utf8_decode_fwrd(&str, &len);
					width += joe_wcwidth(!!locale_map->type,
							     c);
				}
			}
			str = start;
			/* Now calculate span width */
			if (width >= t->w - 1 || nspans==0) {
				spanwidth = 0;
				spanextra = nspans;
			} else {
				spanwidth = ((t->w - 1) - width)/nspans;
				spanextra = nspans - ((t->w - 1) - width - nspans*spanwidth);
			}
			/* Second pass: display text */
			for (x = 0; x != t->w - 1; ++x) {
				if (*str == '\n' || !*str) {
					if (eraeol(t->t, x, y)) {
						return;
					} else {
						break;
					}
				} else {
					if (*str == '\\') {
						switch (*++str) {
						case '|':
							++str;
							for (z=0;z!=spanwidth;++z)
								outatr(locale_map,t->t,t->t->scrn+x+y*t->w+z,t->t->attr+x+y*t->w+z,x+z,y,' ',atr);
							if (spancount++ >= spanextra) {
								outatr(locale_map,t->t,t->t->scrn+x+y*t->w+z,t->t->attr+x+y*t->w+z,x+z,y,' ',atr);
								++z;
							}
							x += z-1;
							continue;
						case 'i':
						case 'I':
							atr ^= INVERSE;
							++str;
							--x;
							continue;
						case 'u':
						case 'U':
							atr ^= UNDERLINE;
							++str;
							--x;
							continue;
						case 'd':
						case 'D':
							atr ^= DIM;
							++str;
							--x;
							continue;
						case 'b':
						case 'B':
							atr ^= BOLD;
							++str;
							--x;
							continue;
						case 'f':
						case 'F':
							atr ^= BLINK;
							++str;
							--x;
							continue;
						case 0:	
							--x;
							continue;
						}
					}
					len = eol - str;
					c = utf8_decode_fwrd(&str, &len);

					outatr(locale_map,
					       t->t, t->t->scrn + x + y * t->w, 
				       	       t->t->attr + x + y * t->w, x, y,
					       c, atr);
					x += (joe_wcwidth(!!locale_map->type, c) - 1);
				}
			}
			atr = 0;
			t->t->updtab[y] = 0;
		}

		while (*str && *str != '\n')
			++str;
		if (*str == '\n')
			++str;
	}
}

/*
 * Show help screen 
 */
int help_on(SCREEN *t)
{
	if (help_actual) {
		t->wind = help_actual->lines + skiptop;
		if ((t->h - t->wind) < FITHEIGHT) {
			t->wind = t->h - FITHEIGHT;
		}
		if (t->wind < 0) {
			t->wind = skiptop;
			return -1;
		}
		wfit(t);
		msetI(t->t->updtab + skiptop, 1, t->wind);
		return 0;
	} else {
		return -1;
	}
}

/*
 * Hide help screen
 */
static void help_off(SCREEN *t)
{
	t->wind = skiptop;
	wfit(t);
}

/*
 * Show/hide current help screen
 */
int u_help(BASE *base)
{
	W *w = base->parent;
	struct help *new_help;

	if (w->huh && (new_help = find_context_help(w->huh)) != NULL) {
		if (help_actual != new_help) {
			if (w->t->wind != skiptop)
				help_off(w->t);
			help_actual = new_help;		/* prepare context help */
		}
	}
	if (w->t->wind == skiptop) {
		return help_on(w->t);			/* help screen is hidden, so show the actual one */
	} else {
		help_off(w->t);				/* hide actual help screen */
		return 0;
	}
}

/*
 * Show next help screen (if it is possible)
 */
int u_help_next(BASE *base)
{
	W *w = base->parent;

	if (help_actual && help_actual->next) {		/* is there any next help screen? */
		if (w->t->wind != skiptop) {
			help_off(w->t);			/* if help screen was visible, then hide it */
		}
		help_actual = help_actual->next;	/* change to next help screen */
		return help_on(w->t);			/* show actual help screen */
	} else {
		return -1;
	}
}

/*
 * Show previous help screen (if it is possible)
 */
int u_help_prev(BASE *base)
{
	W *w = base->parent;

	if (help_actual && help_actual->prev) {		/* is there any previous help screen? */
		if (w->t->wind != skiptop)
			help_off(w->t);			/* if help screen was visible, then hide it */
		help_actual = help_actual->prev;	/* change to previous help screen */
		return help_on(w->t);			/* show actual help screen */
	} else {
		return -1;
	}
}
