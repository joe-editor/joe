/*
 *	Editor engine
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_B_H
#define _JOE_B_H 1

#include "config.h"
#include "types.h"

/* 31744 */
extern unsigned char stdbuf[stdsiz];

extern int force;		/* Set to have final '\n' added to file */
extern int tabwidth;		/* Default tab width */

extern VFILE *vmem;		/* Virtual memory file used for buffer system */

extern unsigned char *msgs[];

B *bmk PARAMS((B *prop));
void brm PARAMS((B *b));

B *bfind PARAMS((unsigned char *s));

P *pdup PARAMS((P *p));
P *pdupown PARAMS((P *p, P **o));
P *poffline PARAMS((P *p));
P *ponline PARAMS((P *p));
B *bonline PARAMS((B *b));
B *boffline PARAMS((B *b));

void prm PARAMS((P *p));
P *pset PARAMS((P *n, P *p));

P *p_goto_bof PARAMS((P *p));		/* move cursor to begging of file */
P *p_goto_eof PARAMS((P *p));		/* move cursor to end of file */
P *p_goto_bol PARAMS((P *p));		/* move cursor to begging of line */
P *p_goto_eol PARAMS((P *p));		/* move cursor to end of line */

int pisbof PARAMS((P *p));
int piseof PARAMS((P *p));
int piseol PARAMS((P *p));
int pisbol PARAMS((P *p));
int pisbow PARAMS((P *p));
int piseow PARAMS((P *p));

#define piscol(p) ((p)->valcol ? (p)->col : (pfcol(p), (p)->col))

int pisblank PARAMS((P *p));
int piseolblank PARAMS((P *p));

long pisindent PARAMS((P *p));

int pnext PARAMS((P *p));
int pprev PARAMS((P *p));

int pgetb PARAMS((P *p));
int prgetb PARAMS((P *p));

int pgetc PARAMS((P *p));
int prgetc PARAMS((P *p));

P *pgoto PARAMS((P *p, long int loc));
P *pfwrd PARAMS((P *p, long int n));
P *pbkwd PARAMS((P *p, long int n));

P *pfcol PARAMS((P *p));

P *pnextl PARAMS((P *p));
P *pprevl PARAMS((P *p));

P *pline PARAMS((P *p, long int line));

P *pcolwse PARAMS((P *p, long int goalcol));
P *pcol PARAMS((P *p, long int goalcol));
P *pcoli PARAMS((P *p, long int goalcol));
void pbackws PARAMS((P *p));
void pfill PARAMS((P *p, long int to, int usetabs));

P *pfind PARAMS((P *p, unsigned char *s, int len));
P *pifind PARAMS((P *p, unsigned char *s, int len));
P *prfind PARAMS((P *p, unsigned char *s, int len));
P *prifind PARAMS((P *p, unsigned char *s, int len));

/* copy text between 'from' and 'to' into new buffer */
B *bcpy PARAMS((P *from, P *to));	

void pcoalesce PARAMS((P *p));

void bdel PARAMS((P *from, P *to));

/* insert buffer 'b' into another at 'p' */
P *binsb PARAMS((P *p, B *b));
/* insert a block 'blk' of size 'amnt' into buffer at 'p' */
P *binsm PARAMS((P *p, unsigned char *blk, int amnt)); 

/* insert character 'c' into buffer at 'p' */
P *binsc PARAMS((P *p, int c));

/* insert byte 'c' into buffer at at 'p' */
P *binsbyte PARAMS((P *p, unsigned char c));

/* insert zero term. string 's' into buffer at 'p' */
P *binss PARAMS((P *p, unsigned char *s));

/* B *bload(char *s);
 * Load a file into a new buffer
 *
 * Returns with errno set to 0 for success,
 * -1 for new file (file doesn't exist)
 * -2 for read error
 * -3 for seek error
 * -4 for open error
 */
B *bload PARAMS((unsigned char *s));
B *bread PARAMS((int fi, long int max));
B *bfind PARAMS((unsigned char *s));
B *borphan PARAMS((void));

/* Save 'size' bytes beginning at 'p' into file with name in 's' */
int bsave PARAMS((P *p, unsigned char *s, long int size));
int bsavefd PARAMS((P *p, int fd, long int size));

unsigned char *parsens PARAMS((unsigned char *s, long int *skip, long int *amnt));

/* Get byte at pointer or return NO_MORE_DATA if pointer is at end of buffer */
int brc PARAMS((P *p));

/* Get character at pointer or return NO_MORE_DATA if pointer is at end of buffer */
int brch PARAMS((P *p));

/* Copy 'size' bytes from a buffer beginning at p into block 'blk' */
unsigned char *brmem PARAMS((P *p, unsigned char *blk, int size));

/* Copy 'size' bytes from a buffer beginning at p into a zero-terminated
 * C-string in an malloc block.
 */
unsigned char *brs PARAMS((P *p, int size));

/* Copy 'size' bytes from a buffer beginning at p into a variable length string. */
unsigned char *brvs PARAMS((P *p, int size));

B *bnext PARAMS((void));
B *bprev PARAMS((void));

#define error berror
extern int berror;

unsigned char **getbufs PARAMS((void));

#endif
