/*
 *	Text editing windows
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_TW_H
#define _JOE_TW_H 1

#include "config.h"
#include "types.h"

BW *wmktw PARAMS((SCREEN *t, B *b));

int usplitw PARAMS((BW *bw));
int uduptw PARAMS((BW *bw));
int utw0 PARAMS((BASE *b));
int utw1 PARAMS((BASE *b));
int uabortbuf PARAMS((BW *bw));
int uabort PARAMS((BW *bw, int k));
int uabort1 PARAMS((BW *bw, int k));
void setline PARAMS((B *b, long int line));
int abortit PARAMS((BW *bw));
extern int staen;

#endif
