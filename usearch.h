/*
 *	Search & Replace system
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_USEARCH_H
#define _JOE_USEARCH_H 1

#include "config.h"
#include "types.h"

SRCH *mksrch PARAMS((char *pattern, char *replacement, int ignore, int backwards, int repeat, int replace, int rest));
void rmsrch PARAMS((SRCH *srch));

int dopfnext PARAMS((BW *bw, SRCH *srch, int *notify));

int pffirst PARAMS((BW *bw));
int pfnext PARAMS((BW *bw));

int pqrepl PARAMS((BW *bw));
int prfirst PARAMS((BW *bw));

#endif
