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
	LINK (H) link;			/* ??? */
	long seg;			/* ??? */
	int hole;			/* ??? */
	int ehole;			/* ??? */
	int nlines;			/* ??? */
};

struct point {
	LINK (P) link;			/* ??? */
		
	B *b;				/* ??? */
	int ofst;			/* ??? */
	char *ptr;			/* ??? */
	H *hdr;				/* ??? */

	long byte;			/* ??? */
	long line;			/* ??? */
	long col;			/* ??? */
	long xcol;			/* ??? */
	int valcol;			/* ??? */
	int end;			/* ??? */

	P **owner;			/* ??? */
};

struct buffer {
	LINK (B) link;
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

B *bmk ();
void brm ();

B *bfind ();

P *pdup ();
P *pdupown ();
P *poffline ();
P *ponline ();
B *bonline ();
B *boffline ();

void prm ();
P *pset ();

P *p_goto_bof (P *p);		/* move cursor to begging of file */
P *p_goto_eof (P *p);		/* move cursor to end of file */
P *p_goto_bol (P *p);		/* move cursor to begging of line */
P *p_goto_eol (P *p);		/* move cursor to end of line */

int pisbof ();
int piseof ();
int piseol ();
int pisbol ();
int pisbow ();
int piseow ();

#define piscol(p) ((p)->valcol?(p)->col:(pfcol(p),(p)->col))

int pisblank ();

long pisindent ();

int pnext ();
int pprev ();

int pgetc ();

P *pfwrd ();

int prgetc ();

P *pbkwd ();
P *pgoto ();

P *pfcol ();


P *pnextl ();

P *pprevl ();

P *pline ();

P *pcolwse ();
P *pcol ();
P *pcoli ();
void pbackws ();
void pfill ();

P *pfind ();
P *pifind ();
P *prfind ();
P *prifind ();

/* B *bcpy(P *from,P *to);
 * Copy text between from and to into a new buffer
 */
B *bcpy ();

void pcoalesce ();

void bdel ();

/* P *binsb(P *p,B *b);
 * Insert an entire buffer 'b' into another buffer at 'p'
 */
P *binsb ();

/* P *binsm(P *p,char *blk,int amnt);
 * Insert a block 'blk' of size 'amnt' into buffer at 'p'
 */
P *binsm ();

/* P *binsc(P *p,char c);
 * Insert character into buffer at P
 */
P *binsc ();

/* P *binss(P *p,char *s);
 * Insert zero terminated string into buffer at P
 */
P *binss ();

/* B *bload(char *s);
 * Load a file into a new buffer
 *
 * Returns with errno set to 0 for success,
 * -1 for new file (file doesn't exist)
 * -2 for read error
 * -3 for seek error
 * -4 for open error
 */
B *bread ();
B *bload ();
B *bfind ();
B *borphan ();

/* int bsave(P *p,char *s,long size);
 * Save 'size' bytes beginning at 'p' into file with name in 's'
 */
int bsavefd ();
int bsave ();

char *parsens ();

/* int brc(P *p);
 * Get character at pointer or return MAXINT if pointer is at end of buffer
 */
int brc ();

/* char *brmem(P *p,char *blk,int size);
 * Copy 'size' bytes from a buffer beginning at p into block 'blk'
 */
char *brmem ();

/* char *brs(P *p,int size);
 * Copy 'size' bytes from a buffer beginning at p into a zero-terminated
 * C-string in an malloc block.
 */
char *brs ();

/* char *brvs(P *p,int size);
 * Copy 'size' bytes from a buffer beginning at p into a variable length
 * string.
 */
char *brvs ();

B *bnext ();
B *bprev ();

#define error berror
extern int berror;

char **getbufs ();

#endif
