/*
 *	Window management
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_W_H
#define _JOE_W_H 1

#include "config.h"
#include "types.h"

/***************/
/* Subroutines */
/***************/

/* int getgrouph(W *);
 * Get height of a family of windows
 */
int getgrouph PARAMS((W *w));

/* W *findtopw(W *);
 * Find first (top-most) window of a family
 */
W *findtopw PARAMS((W *w));

/* W *findbotw(W *);
 * Find last (bottom-most) window a family
 */
W *findbotw PARAMS((W *w));

int demotegroup PARAMS((W *w));

/* W *lastw(SCREEN *t);
 * Find last window on screen
 */
W *lastw PARAMS((SCREEN *t));

/* Determine number of main windows
 */
int countmain PARAMS((SCREEN *t));

/* void wfit(SCREEN *t);
 *
 * Fit all of the windows onto the screen
 */
void wfit PARAMS((SCREEN *t));

/*****************/
/* Main routines */
/*****************/

/* SCREEN *screate(SCRN *);
 *
 * Create a screen
 */
SCREEN *screate PARAMS((SCRN *scrn));

/* void sresize(SCREEN *t);
 * Screen size changed
 */
void sresize PARAMS((SCREEN *t));

/* void chsize(SCREEN *t,int mul,int div)
 * Resize windows: each window is multiplied by the fraction mul/div
 */
void chsize PARAMS(());

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
W *wcreate PARAMS((SCREEN *t, WATOM *watom, W *where, W *target, W *original, int height, unsigned char *huh, int *notify));

/* int wabort(W *w);
 *
 * Kill a window and it's children
 */
int wabort PARAMS((W *w));

/* int wnext(SCREEN *);
 *
 * Switch to next window
 */
int wnext PARAMS((SCREEN *t));

/* int wprev(SCREEN *);
 *
 * Switch to previous window
 */
int wprev PARAMS((SCREEN *t));

/* int wgrow(W *);
 *
 * increase size of window.  Return 0 for success, -1 for fail.
 */
int wgrow PARAMS((W *w));

/* int wshrink(W *);
 *
 * Decrease size of window.  Returns 0 for success, -1 for fail.
 */
int wshrink PARAMS((W *w));

/* void wshowone(W *);
 *
 * Show only one window on the screen
 */
void wshowone PARAMS((W *w));

/* void wshowall(SCREEN *);
 *
 * Show all windows on the screen, including the given one
 */
void wshowall PARAMS((SCREEN *t));

/* void wredraw(W *);
 *
 * Force complete redraw of window
 */
void wredraw PARAMS((W *w));

/* void updall()
 *
 * Redraw all windows
 */
void updall PARAMS((void));

/* void msgnw[t](W *w, char *s);
 * Display a message which will be eliminated on the next keypress.
 * msgnw displays message on bottom line of window
 * msgnwt displays message on top line of window
 */
void msgnw PARAMS((W *w, unsigned char *s));
void msgnwt PARAMS((W *w, unsigned char *s));

#define JOE_MSGBUFSIZE 300
extern unsigned char msgbuf[JOE_MSGBUFSIZE];	/* Message composition buffer for msgnw/msgnwt */

void msgout PARAMS((W *w));			/* Output msgnw/msgnwt messages */

/* Common user functions */

int urtn PARAMS((BASE *b, int k));		/* User hit return */
int utype PARAMS((BASE *b, int k));		/* User types a character */
int uretyp PARAMS((BASE *bw));			/* Refresh the screen */
int ugroww PARAMS((BASE *bw));			/* Grow current window */
int uexpld PARAMS((BASE *bw));			/* Explode current window or show all windows */
int ushrnk PARAMS((BASE *bw));			/* Shrink current window */
int unextw PARAMS((BASE *bw));			/* Goto next window */
int uprevw PARAMS((BASE *bw));			/* Goto previous window */

void scrdel PARAMS((B *b, long int l, long int n, int flg));
void scrins PARAMS((B *b, long int l, long int n, int flg));

#endif
