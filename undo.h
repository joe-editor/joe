/*
 *	UNDO system
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_UNDO_H
#define _JOE_UNDO_H 1

struct undorec {
	LINK(UNDOREC)	link;
	UNDOREC	*unit;
	int	min;
	int	changed;	/* Status of modified flag before this record */
	long	where;		/* Buffer address of this record */
	long	len;		/* Length of insert or delete */
	int	del;		/* Set if this is a delete */
	B	*big;		/* Set to buffer containing a large amount of deleted data */
	unsigned char	*small;		/* Set to malloc block containg a small amount of deleted data */
};

struct undo {
	LINK(UNDO)	link;
	B	*b;
	int	nrecs;
	UNDOREC	recs;
	UNDOREC	*ptr;
	UNDOREC	*first;
	UNDOREC	*last;
};

extern int inundo; /* Set if inserts/deletes are part of an undo operation */
extern int justkilled; /* Last edit was a delete, so store data in yank buffer */

UNDO *undomk PARAMS((B *b));
void undorm PARAMS((UNDO *undo));
int uundo PARAMS((BW *bw));
int uredo PARAMS((BW *bw));
void umclear PARAMS((void));
void undomark PARAMS((void));
void undoins PARAMS((UNDO *undo, P *p, long int size));
void undodel PARAMS((UNDO *undo, long int where, B *b));
int uyank PARAMS((BW *bw));
int uyankpop PARAMS((BW *bw));
int uyapp PARAMS((BW *bw));
int unotmod PARAMS((BW *bw));
int ucopy PARAMS((BW *bw));

void load_yank PARAMS((FILE *f));
void save_yank PARAMS((FILE *f));
void bw_unlock PARAMS((BW *bw));

#endif
