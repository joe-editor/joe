#ifndef _Ib
#define _Ib 1

#include "config.h"
#include "queue.h"
#include "rc.h"
#include "vfile.h"

#define stdsiz 8192
/* 31744 */
extern char stdbuf[stdsiz];

typedef struct buffer B;
typedef struct point P;
typedef struct header H;

struct header {
	LINK(H) link;		/* LINK ??? */
	long seg;		/* ??? */
	int hole;		/* ??? */
	int ehole;		/* ??? */
	int nlines;		/* ??? */
};

struct point {
	LINK(P) link;		/* ?LINK ??? */

	B *b;			/* ?B ??? */
	int ofst;		/* ??? */
	char *ptr;		/* ??? */
	H *hdr;			/* ?H ??? */

	long byte;		/* ??? */
	long line;		/* ??? */
	long col;		/* ??? */
	long xcol;		/* ??? */
	int valcol;		/* ??? */
	int end;		/* ??? */

	P **owner;		/* ??? */
};

struct buffer {
	LINK(B) link;
	P *bof;
	P *eof;
	char *name;
	int orphan;
	int count;
	int changed;
	int backup;
	void *undo;
	P *marks[10];		/* Bookmarks */
	OPTIONS o;		/* Options */
	P *oldcur;		/* Last cursor position before orphaning */
	P *oldtop;		/* Last top screen position before orphaning */
	int rdonly;		/* Set for read-only */
	int internal;		/* Set for internal buffers */
	int er;			/* Error code when file was loaded */
};

extern int force;		/* Set to have final '\n' added to file */
extern int tabwidth;		/* Default tab width */

extern VFILE *vmem;		/* Virtual memory file used for buffer system */

extern char *msgs[];

B *bmk PARAMS((B *prop));
void brm PARAMS((B *b));

B *bfind PARAMS((char *s));

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

#define piscol(p) ((p)->valcol?(p)->col:(pfcol(p),(p)->col))

int pisblank PARAMS((P *p));

long pisindent PARAMS((P *p));

int pnext PARAMS((P *p));
int pprev PARAMS((P *p));

int pgetc PARAMS((P *p));

P *pfwrd PARAMS((P *p, long int n));

int prgetc PARAMS((P *p));

P *pbkwd PARAMS((P *p, long int n));
P *pgoto PARAMS((P *p, long int loc));

P *pfcol PARAMS((P *p));


P *pnextl PARAMS((P *p));

P *pprevl PARAMS((P *p));

P *pline PARAMS((P *p, long int line));

P *pcolwse PARAMS((P *p, long int goalcol));
P *pcol PARAMS((P *p, long int goalcol));
P *pcoli PARAMS((P *p, long int goalcol));
void pbackws PARAMS((P *p));
void pfill PARAMS((P *p, long int to, int usetabs));

P *pfind PARAMS((P *p, char *s, int len));
P *pifind PARAMS((P *p, char *s, int len));
P *prfind PARAMS((P *p, char *s, int len));
P *prifind PARAMS((P *p, char *s, int len));

/* B *bcpy(P *from,P *to);
 * Copy text between from and to into a new buffer
 */
B *bcpy PARAMS((P *from, P *to));

void pcoalesce PARAMS((P *p));

void bdel PARAMS((P *from, P *to));

/* P *binsb(P *p,B *b);
 * Insert an entire buffer 'b' into another buffer at 'p'
 */
P *binsb PARAMS((P *p, B *b));

/* P *binsm(P *p,char *blk,int amnt);
 * Insert a block 'blk' of size 'amnt' into buffer at 'p'
 */
P *binsm PARAMS((P *p, char *blk, int amnt));

/* P *binsc(P *p,char c);
 * Insert character into buffer at P
 */
P *binsc PARAMS((P *p, char c));

/* P *binss(P *p,char *s);
 * Insert zero terminated string into buffer at P
 */
P *binss PARAMS((P *p, char *s));

/* B *bload(char *s);
 * Load a file into a new buffer
 *
 * Returns with errno set to 0 for success,
 * -1 for new file (file doesn't exist)
 * -2 for read error
 * -3 for seek error
 * -4 for open error
 */
B *bread PARAMS((int fi, long int max));
B *bload PARAMS((char *s));
B *bfind PARAMS((char *s));
B *borphan PARAMS((void));

/* int bsave(P *p,char *s,long size);
 * Save 'size' bytes beginning at 'p' into file with name in 's'
 */
int bsavefd PARAMS((P *p, int fd, long int size));
int bsave PARAMS((P *p, char *s, long int size));

char *parsens PARAMS((char *s, long int *skip, long int *amnt));

/* int brc(P *p);
 * Get character at pointer or return MAXINT if pointer is at end of buffer
 */
int brc PARAMS((P *p));

/* char *brmem(P *p,char *blk,int size);
 * Copy 'size' bytes from a buffer beginning at p into block 'blk'
 */
char *brmem PARAMS((P *p, char *blk, int size));

/* char *brs(P *p,int size);
 * Copy 'size' bytes from a buffer beginning at p into a zero-terminated
 * C-string in an malloc block.
 */
char *brs PARAMS((P *p, int size));

/* char *brvs(P *p,int size);
 * Copy 'size' bytes from a buffer beginning at p into a variable length
 * string.
 */
char *brvs PARAMS((P *p, int size));

B *bnext PARAMS((void));
B *bprev PARAMS((void));

#define error berror
extern int berror;

char **getbufs PARAMS((void));

#endif
