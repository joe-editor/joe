/*
 *	Edit buffer window generation
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_BW_H
#define _JOE_BW_H 1

#include "config.h"
#include "types.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include "b.h"
#include "rc.h"
#include "w.h"

extern int dspasis;

extern int mid;
void bwfllw PARAMS((BW *w));
void bwins PARAMS((BW *w, long int l, long int n, int flg));
void bwdel PARAMS((BW *w, long int l, long int n, int flg));
void bwgen PARAMS((BW *w, int linums));
BW *bwmk PARAMS((W *window, B *b, int prompt));
void bwmove PARAMS((BW *w, int x, int y));
void bwresz PARAMS((BW *w, int wi, int he));
void bwrm PARAMS((BW *w));
int ustat PARAMS((BW *bw));
int ucrawll PARAMS((BW *bw));
int ucrawlr PARAMS((BW *bw));
void orphit PARAMS((BW *bw));

#endif
