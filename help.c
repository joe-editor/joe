/*
	Help system
	Copyright
		(C) 1992 Joseph H. Allen
		(C) 2001 Marek 'Marx' Grac

	This file is part of JOE (Joe's Own Editor)
*/

#include "help.h"
#include <string.h>

#include "config.h"
#include "tty.h"
#include "b.h"
#include "termcap.h"
#include "kbd.h"
#include "scrn.h"
#include "w.h"
#include "vs.h"
#include "menu.h"
#include "tw.h"
#include "blocks.h"

/* The loaded help screen */

char *hlptxt = 0;		/* ???  */
int hlpsiz = 0;			/* ??? */
int hlpbsz = 0;			/* ??? */
int hlplns = 0;			/* ??? */

int help_index = 0;		/* index of last shown help screen */

char **help_names;		/* ??? */

struct help *help_first;	/* first screen of help list */
struct help **help_structs;	/* array of help screens */

/*
 *	???
 */
int get_help (char *name) {
	int x;

	for (x = 0; help_structs[x]; ++x) {
		if (!strcmp (help_structs[x]->name, name)) {
			break;
		}
	}

	if (help_structs[x]) {
		return x;
	} else {
		return -1;
	}
}

/*
 * Display help text
 */
void help_display (SCREEN *t) {
	char *str = hlptxt;
	int y, x, c;
	int atr = 0;

	for (y = skiptop; y != t->wind; ++y) {
		if (t->t->updtab[y]) {
			for (x = 0; x != t->w - 1; ++x) {
				if (*str == '\n' || !*str) {
					if (eraeol (t->t, x, y)) {
						return;
					} else {
						break;
					}
				} else {
					if (*str == '\\') {
						switch (*++str) {
							case 'i':
							case 'I':
								atr ^= INVERSE;
								++str;
								--x;
								goto cont;
							case 'u':
							case 'U':
								atr ^= UNDERLINE;
								++str;
								--x;
								goto cont;
							case 'd':
							case 'D':
								atr ^= DIM;
								++str;
								--x;
								goto cont;
							case 'b':
							case 'B':
								atr ^= BOLD;
								++str;
								--x;
								goto cont;
							case 'f':
							case 'F':
								atr ^= BLINK;
								++str;
								--x;
								goto cont;
							case 0:
								--x;
								goto cont;
							default:
								c =	(unsigned char) *str++;
						}
					} else {
						c = (unsigned char) *str++;
					}
  					outatr (t->t, t->t->scrn + x + y * t->w, x, y, c, atr);
					cont:;
				}
			}
			atr = 0;
			t->t->updtab[y] = 0;
		}
	
		while (*str && *str != '\n') {
			++str;
		}
		if (*str == '\n') {
			++str;
		}
	}
}

/*
 * Create/Eliminate help window 
 */
int help_on (SCREEN *t) {
	struct help *h = help_structs[help_index];

	hlptxt = h->hlptxt;
	hlpsiz = h->hlpsiz;
	hlpbsz = h->hlpbsz;
	hlplns = h->hlplns;

	if (!hlptxt) {
		return -1;
	}
	t->wind = hlplns + skiptop;
	if (t->h - t->wind < FITHEIGHT) {
		t->wind = t->h - FITHEIGHT;
	}
	if (t->wind < 0) {
		t->wind = skiptop;
		return -1;
	}
	wfit (t);
	msetI (t->t->updtab + skiptop, 1, t->wind);
	return 0;
}

void help_off (SCREEN *t) {
	t->wind = skiptop;
	wfit (t);
}

/*
 * Toggle help on/off
 */
int u_help (BASE *base) {
	int h;
	W *w = base->parent;

	if (w->huh && (h = get_help (w->huh)) > -1) {
		if (w->t->wind != skiptop) {
			help_off (w->t);
		}
		help_index = h;
		return help_on (w->t);
	} else if (w->t->wind == skiptop) {
		return help_on (w->t);
	} else {
		help_off (w->t);
		return 0;
	}
}

/*
 * Goto next/prev help screen
 */
int u_help_next (BASE *base) {
	W *w = base->parent;

	if (help_structs[help_index + 1]) {
		if (w->t->wind != skiptop) {
			help_off (w->t);
		}
		++help_index;
		return help_on (w->t);
	} else {
		return -1;
	}
}

int u_help_prev (BASE *base) {
	W *w = base->parent;

	if (help_index) {
		if (w->t->wind != skiptop) {
			  help_off (w->t);
		}
		--help_index;
		return help_on (w->t);
	} else {
		return -1;
	}
}

/*
 * Convert list of help screens into an array 
 */
void help_to_array (void) {
	struct help *tmp;
	int nhelp = 0;		/* number of help screens */

	for (tmp = help_first; (tmp = tmp->next); ++nhelp);
	++nhelp;

	if (nhelp) {
		  help_structs = (struct help **) malloc (sizeof (struct help *) * (nhelp + 1));
		  help_structs[nhelp] = NULL;
		  tmp = help_first;

		  while (nhelp--) {
			    help_structs[nhelp] = tmp;
			    tmp = tmp->next;
		    }
	}
}
