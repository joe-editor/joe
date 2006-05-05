/*
 * 	Shell-window functions
 *	Copyright (C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_USHELL_H
#define _JOE_USHELL_H 1

#include "config.h"
#include "types.h"

int ubknd PARAMS((BW *bw));
int ukillpid PARAMS((BW *bw));
int urun PARAMS((BW *bw));
int ubuild PARAMS((BW *bw));
int ugrep PARAMS((BW *bw));
int cstart PARAMS((BW *bw, unsigned char *name, unsigned char **s, void *obj, int *notify, int build, int out_only));

extern B *runhist;
extern B *buildhist;
extern B *grephist;

#endif
