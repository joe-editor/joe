/*
 *	UNDO system
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_UNDO_H
#define _JOE_UNDO_H 1

#include "config.h"
#include "types.h"

extern int inundo;
extern int justkilled;

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

#endif
