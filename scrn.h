/*
 *	Device independant tty interface for JOE
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_SCRN_H
#define _JOE_SCRN_H 1

#include "config.h"
#include "types.h"

#include "tty.h"		/* ttputc() */

extern int skiptop;

/* SCRN *nopen(void);
 *
 * Open the screen (sets TTY mode so that screen may be used immediatly after
 * the 'nopen').
 */
SCRN *nopen PARAMS((CAP *cap));

/* void nresize(SCRN *t,int w,int h);
 *
 * Change size of screen.  For example, call this when you find out that
 * the Xterm changed size.
 */
void nresize PARAMS((SCRN *t, int w, int h));

/* void nredraw(SCRN *t);
 *
 * Invalidate all state variables for the terminal.  This way, everything gets
 * redrawn.
 */
void nredraw PARAMS((SCRN *t));

void npartial PARAMS((SCRN *t));
void nescape PARAMS((SCRN *t));
void nreturn PARAMS((SCRN *t));

/* void nclose(SCRN *t);
 *
 * Close the screen and restore TTY to initial state.
 *
 * if 'flg' is set, tclose doesn't mess with the signals.
 */
void nclose PARAMS((SCRN *t));

/* int cpos(SCRN *t,int x,int y);
 *
 * Set cursor position
 */
int cpos PARAMS((register SCRN *t, register int x, register int y));

/* int attr(SCRN *t,int a);
 *
 * Set attributes
 */
int attr PARAMS((SCRN *t, int c));

/* void outatr(SCRN *t,int *scrn,int x,int y,int c,int a);
 *
 * Output a character at the given screen cooridinate.  The cursor position
 * after this function is executed is indeterminate.
 */

/* Character attribute bits */

#ifdef __MSDOS__

#define INVERSE 1
#define UNDERLINE 2
#define BOLD 4
#define BLINK 8
#define DIM 16
extern unsigned atab[];

#define outatr(t,scrn,x,y,c,a) do { \
	(t); \
	(x); \
	(y); \
	*(scrn) = ((unsigned)(c) | atab[a]); \
} while(0)

#else

#define INVERSE		 256
#define UNDERLINE	 512
#define BOLD		1024
#define BLINK		2048
#define DIM		4096
#define AT_MASK		(INVERSE+UNDERLINE+BOLD+BLINK+DIM)

#define BG_SHIFT 13
#define BG_MASK (7<<BG_SHIFT)
#define BG_BLACK (0<<BG_SHIFT) /* default */
#define BG_RED (1<<BG_SHIFT)
#define BG_GREEN (2<<BG_SHIFT)
#define BG_YELLOW (3<<BG_SHIFT)
#define BG_BLUE (4<<BG_SHIFT)
#define BG_MAGENTA (5<<BG_SHIFT)
#define BG_CYAN (6<<BG_SHIFT)
#define BG_WHITE (7<<BG_SHIFT)

#define FG_SHIFT 16
#define FG_MASK (7<<FG_SHIFT)
#define FG_WHITE (0<<FG_SHIFT) /* default */
#define FG_CYAN (1<<FG_SHIFT)
#define FG_MAGENTA (2<<FG_SHIFT)
#define FG_BLUE (3<<FG_SHIFT)
#define FG_YELLOW (4<<FG_SHIFT)
#define FG_GREEN (5<<FG_SHIFT)
#define FG_RED (6<<FG_SHIFT)
#define FG_BLACK (7<<FG_SHIFT)

#define outatr(t, scrn, xx, yy, c, a) do {		\
	if(*(scrn) != ((c) | (a))) {			\
		*(scrn) = ((c) | (a));			\
		if((t)->ins)				\
			clrins(t);			\
		if((t)->x != (xx) || (t)->y != (yy))	\
			cpos((t), (xx), (yy));		\
		if((t)->attrib != (a))			\
			attr((t), (a));			\
		ttputc(c);				\
		++(t)->x;				\
	}						\
} while(0)

#endif

/*
 * translate character and its attribute into something printable
 */
void xlat PARAMS((int *attr, unsigned char *c));

/* int eraeol(SCRN *t,int x,int y);
 *
 * Erase from screen coordinate to end of line.
 */
int eraeol PARAMS((SCRN *t, int x, int y));

/* void nscrlup(SCRN *t,int top,int bot,int amnt);
 *
 * Buffered scroll request.  Request that some lines up.  'top' and 'bot'
 * indicate which lines to scroll.  'bot' is the last line to scroll + 1.
 * 'amnt' is distance in lines to scroll.
 */
void nscrlup PARAMS((SCRN *t, int top, int bot, int amnt));

/* void nscrldn(SCRN *t,int top,int bot,int amnt);
 *
 * Buffered scroll request.  Scroll some lines down.  'top' and 'bot'
 * indicate which lines to scroll.  'bot' is the last line to scroll + 1.
 * 'amnt' is distance in lines to scroll.
 */
void nscrldn PARAMS((SCRN *t, int top, int bot, int amnt));

/* void nscroll(SCRN *t);
 *
 * Execute buffered scroll requests
 */
void nscroll PARAMS((SCRN *t));

/* void magic(SCRN *t,int y,int *cur,int *new);
 *
 * Figure out and execute line shifting
 */
void magic PARAMS((SCRN *t, int y, int *cs, int *s, int placex));

int clrins PARAMS((SCRN *t));

#endif
