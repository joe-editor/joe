/*
	Help system
	Copyright
		(C) 1992 Joseph H. Allen
		(C) 2001 Marek 'Marx' Grac

	This file is part of JOE (Joe's Own Editor)
*/

#include "config.h"

#include <stdio.h>
#include <string.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "blocks.h"
#include "help.h"
#include "w.h"

#define NOT_ENOUGH_MEMORY -11

struct help *help_actual = NULL;			/* actual help screen */

/* 
 * Process help file
 * Returns 0 if the help file was succefully processed
 *        -1 if the help file couldn't be opened 
 *        NOT_ENOUGH_MEMORY if there is not enough memory
 */

int help_init(char *filename)
{
	FILE *fd;					/* help file */
	unsigned char buf[1024];			/* input buffer */

	struct help *tmp;
	unsigned int bfl;				/* buffer length */
	unsigned int hlpsiz, hlpbsz;			/* number of used/allocated bytes for tmp->text */
	char *tempbuf;

	if (!(fd = fopen(filename, "r")))		/* open the help file */
		return -1;				/* return if we couldn't open the file */

	fprintf(stderr, "Processing '%s'...", filename);
	fflush(stderr);

	while (fgets(buf, sizeof(buf), fd)) {
		if (buf[0] == '{') {			/* start of help screen */
			if (!(tmp = (struct help *) malloc(sizeof(struct help)))) {
				return NOT_ENOUGH_MEMORY;
			}

			tmp->text = NULL;
			tmp->lines = 0;
			hlpsiz = 0;
			hlpbsz = 0;

			while ((fgets(buf, sizeof(buf), fd)) && (buf[0] != '}')) {
				bfl = strlen(buf);
				if (hlpsiz + bfl > hlpbsz) {
					if (tmp->text) {
						tempbuf = (char *) realloc(tmp->text, hlpbsz + bfl + 1024);
						if (!tempbuf) {
							free (tmp->text);
							free (tmp);
							return NOT_ENOUGH_MEMORY;
						} else {
							tmp->text = tempbuf;
						}
					} else {
						tmp->text = (char *) malloc(bfl + 1024);
						if (!tmp->text) {
							free (tmp);
							return NOT_ENOUGH_MEMORY;
						} else {
							tmp->text[0] = 0;
						}
					}
					hlpbsz += bfl + 1024;
				}
				strcpy(tmp->text + hlpsiz, buf);
				hlpsiz += bfl;
				++tmp->lines;
			}
			if (buf[0] == '}') {		/* set new help screen as actual one */
				tmp->prev = help_actual;
				tmp->next = NULL;
				if (help_actual) {
					help_actual->next = tmp;
				}
				help_actual = tmp;
			} else {
				fprintf(stderr, "\nHelp file '%s' is not properly ended with } on new line.\n", filename);
				fprintf(stderr, "Do you want to accept incomplete help screen (y/n)?");
				fflush(stderr);
				fgets(buf, 8, stdin);
				if (!((buf[0] == 'y') || (buf[0] == 'Y'))) {
					free (tmp->text);
					free (tmp);
					return 0;
				} else {
					tmp->prev = help_actual;
					tmp->next = NULL;
					if (help_actual) {
						help_actual->next = tmp;
					}
					help_actual = tmp;
				}
			}
		}
	}
	fclose(fd);					/* close help file */

	fprintf(stderr, "done\n");
	
	while (help_actual && help_actual->prev) {	/* move to first help screen */
		help_actual = help_actual->prev;
	}

	return 0;
}

/*
 * Display help text
 */
void help_display(SCREEN *t)
{
	char *str;
	int y, x, c;
	int atr = 0;

	if (help_actual) {
		str = help_actual->text;
	} else {
		str = NULL;
	}

	for (y = skiptop; y != t->wind; ++y) {
		if (t->t->updtab[y]) {
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
							default:
								c = (unsigned char) *str++;
						}
					} else {
						c = (unsigned char) *str++;
					}
					outatr(t->t, t->t->scrn + x + y * t->w, x, y, c, atr);
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

	if (help_actual && help_actual->next) {		/* is there any previous help screen? */
		if (w->t->wind != skiptop) {
			help_off(w->t);			/* if help screen was visible, then hide it */
		}
		help_actual = help_actual->next;	/* change to previous help screen */
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
