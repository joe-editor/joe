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
int set_attr PARAMS((SCRN *t, int c));

/* Encode character as utf8 */
void utf8_putc PARAMS((int c));

/* void outatr(SCRN *t,int *scrn,int *attr,int x,int y,int c,int a);
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

#define outatr(t,scrn,attr,x,y,c,a) do { \
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
#define BG_VALUE (7<<BG_SHIFT)
#define BG_NOT_DEFAULT (8<<BG_SHIFT)
#define BG_MASK (15<<BG_SHIFT)

#define BG_DEFAULT (0<<BG_SHIFT) /* default */
#define BG_BLACK (8<<BG_SHIFT)
#define BG_RED (9<<BG_SHIFT)
#define BG_GREEN (10<<BG_SHIFT)
#define BG_YELLOW (11<<BG_SHIFT)
#define BG_BLUE (12<<BG_SHIFT)
#define BG_MAGENTA (13<<BG_SHIFT)
#define BG_CYAN (14<<BG_SHIFT)
#define BG_WHITE (15<<BG_SHIFT)

#define FG_SHIFT 17
#define FG_VALUE (7<<FG_SHIFT)
#define FG_NOT_DEFAULT (8<<FG_SHIFT)
#define FG_MASK (15<<FG_SHIFT)

#define FG_DEFAULT (0<<FG_SHIFT)
#define FG_WHITE (8<<FG_SHIFT) /* default */
#define FG_CYAN (9<<FG_SHIFT)
#define FG_MAGENTA (10<<FG_SHIFT)
#define FG_BLUE (11<<FG_SHIFT)
#define FG_YELLOW (12<<FG_SHIFT)
#define FG_GREEN (13<<FG_SHIFT)
#define FG_RED (14<<FG_SHIFT)
#define FG_BLACK (15<<FG_SHIFT)

void outatr PARAMS((int wide,struct charmap *map,SCRN *t,int *scrn,int *attrf,int xx,int yy,int c,int a));

#endif

/*
 * translate character and its attribute into something printable
 */
void xlat PARAMS((int *attr, unsigned char *c));
void xlat_utf_ctrl PARAMS((int *attr, unsigned char *c));

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
void magic PARAMS((SCRN *t, int y, int *cs, int *ca, int *s, int *a,int placex));

int clrins PARAMS((SCRN *t));

int meta_color PARAMS((unsigned char *s));

/* Generate a field */
void genfield PARAMS((SCRN *t,int *scrn,int *attr,int x,int y,int ofst,unsigned char *s,int len,int atr,int width,int flg));

/* Column width of a string takes into account utf-8) */
int txtwidth PARAMS((unsigned char *s,int len));

/* Generate a field: formatted */
void genfmt PARAMS((SCRN *t, int x, int y, int ofst, unsigned char *s, int flg));

/* Column width of formatted string */
int fmtlen PARAMS((unsigned char *s));

/* Offset within formatted string of particular column */
int fmtpos PARAMS((unsigned char *s, int goal));

#endif
