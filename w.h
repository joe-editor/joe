/*
	Window management
	Copyright (C) 1992 Joseph H. Allen

	This file is part of JOE (Joe's Own Editor)
*/

#ifndef _Iw
#define _Iw 1

#include "config.h"
#include "queue.h"
#include "scrn.h"
#include "kbd.h"

typedef struct watom WATOM;
typedef struct screen SCREEN;
typedef struct window W;
typedef struct base BASE;

struct watom {
	char *context;		/* Context name */
	void (*disp) ();	/* Display window */
	void (*follow) ();	/* Called to have window follow cursor */
	int (*abort) ();	/* Common user functions */
	int (*rtn) ();
	int (*type) ();
	void (*resize) ();	/* Called when window changed size */
	void (*move) ();	/* Called when window moved */
	void (*ins) ();		/* Called on line insertions */
	void (*del) ();		/* Called on line deletions */
	int what;		/* Type of this thing */
};

struct screen {
	SCRN *t;		/* Screen data on this screen is output to */

	int wind;		/* Number of help lines on this screen */

	W *topwin;		/* Top-most window showing on screen */
	W *curwin;		/* Window cursor is in */

	int w, h;		/* Width and height of this screen */
};

struct window {
	LINK(W) link;		/* Linked list of windows in order they
				   appear on the screen */

	SCREEN *t;		/* Screen this thing is on */

	int x, y, w, h;		/* Position and size of window */
	/* Currently, x=0, w=width of screen. */
	/* y== -1 if window is not on screen */

	int ny, nh;		/* Temporary values for wfit */

	int reqh;		/* Requested new height or 0 for same */
	/* This is an argument for wfit */

	int fixed;		/* If this is zero, use 'hh'.  If not, this
				   is a fixed size window and this variable
				   gives its height */

	int hh;			/* Height window would be on a screen with
				   1000 lines.  When the screen size changes
				   this is used to calculate the window's
				   real height */

	W *win;			/* Window this one operates on */
	W *main;		/* Main window of this family */
	W *orgwin;		/* Window where space from this window came */
	int curx, cury;		/* Cursor position within window */
	KBD *kbd;		/* Keyboard handler for this window */
	WATOM *watom;		/* The type of this window */
	void *object;		/* Object which inherits this */

	char *msgt;		/* Message at top of window */

	char *msgb;		/* Message at bottom of window */

	char *huh;		/* Name of window for context sensitive hlp */

	int *notify;		/* Address of kill notification flag */
};

/* Anything which goes in window.object must start like this: */

struct base {
	W *parent;
};

/* Minimum text window height */
#define FITHEIGHT 4

/***************/
/* Subroutines */
/***************/

/* int getgrouph(W *);
 * Get height of a family of windows
 */
int getgrouph();

/* W *findtopw(W *);
 * Find first (top-most) window of a family
 */
W *findtopw();

/* W *findbotw(W *);
 * Find last (bottom-most) window a family
 */
W *findbotw();

int demotegroup();

/* W *lastw(SCREEN *t);
 * Find last window on screen
 */
W *lastw();

/* Determine number of main windows
 */
int countmain();

/* void wfit(SCREEN *t);
 *
 * Fit all of the windows onto the screen
 */
void wfit();

/*****************/
/* Main routines */
/*****************/

/* SCREEN *screate(SCRN *);
 *
 * Create a screen
 */
SCREEN *screate();

/* void sresize(SCREEN *t);
 * Screen size changed
 */
void sresize();

/* void chsize(SCREEN *t,int mul,int div)
 * Resize windows: each window is multiplied by the fraction mul/div
 */
void chsize();

/* W *wcreate(SCREEN *t,WATOM *watom,W *where,W *target,W *original,int height);
 *
 * Try to create a window
 *
 * 't'		Is the screen the window is placed on
 * 'watom'	Type of new window
 * 'where'	The window is placed after this window, or if 'where'==0, the
 *		window is placed on the end of the screen
 * 'target'	The window operates on this window.  The window becomes a
 *		member of 'target's family or starts a new family if
 *		'target'==0.
 * 'original'	Attempt to get 'height' from this window.  When the window is
 *              aborted, the space gets returned to 'original' if it still
 *		exists.  If 'original'==0, the window will force other
 *		windows to go off of the screen.
 * 'height'	The height of the window
 *
 * Returns the new window or returns 0 if there was not enough space to
 * create the window and maintain family integrity.
 */
W *wcreate();

/* int wabort(W *w);
 *
 * Kill a window and it's children
 */
int wabort();

/* int wnext(SCREEN *);
 *
 * Switch to next window
 */
int wnext();

/* int wprev(SCREEN *);
 *
 * Switch to previous window
 */
int wprev();

/* int wgrow(W *);
 *
 * increase size of window.  Return 0 for success, -1 for fail.
 */
int wgrow();

/* int wshrink(W *);
 *
 * Decrease size of window.  Returns 0 for success, -1 for fail.
 */
int wshrink();

/* void wshowone(W *);
 *
 * Show only one window on the screen
 */
void wshowone();

/* void wshowall(SCREEN *);
 *
 * Show all windows on the screen, including the given one
 */
void wshowall();

/* void wredraw(W *);
 *
 * Force complete redraw of window
 */
void wredraw();

/* void updall()
 *
 * Redraw all windows
 */
void updall();

void genfmt();
void gentxt();
int fmtlen();
int fmtpos();

/* void msgnw[t](W *w,char *text);
 * Display a message which will be eliminated on the next keypress.
 * msgnw displays message on bottom line of window
 * msgnwt displays message on top line of window
 */
void msgnw();
void msgnwt();

#define MSGBUFSIZE 300
extern char msgbuf[MSGBUFSIZE];	/* Message composition buffer for msgnw/msgnwt */

void msgout();			/* Output msgnw/msgnwt messages */

/* Common user functions */

int urtn();			/* User hit return */
int utype();			/* User types a character */
int uretyp();			/* Refresh the screen */
int ugroww();			/* Grow current window */
int uexpld();			/* Explode current window or show all windows */
int ushrnk();			/* Shrink current window */
int unextw();			/* Goto next window */
int uprevw();			/* Goto previous window */

void scrdel();
void scrins();

#endif
