/*
 *	Prompt windows
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_PW_H
#define _JOE_PW_H 1

#include "config.h"
#include "types.h"

/* BW *wmkpw(BW *bw,char *prompt,int (*func)(),char *huh,int (*abrt)(),
             int (*tab)(),void *object,int *notify);
 * Create a prompt window for the given window
 */
BW *wmkpw PARAMS((W *w, char *prompt, B **history, int (*func) (), char *huh, int (*abrt) (), int (*tab) (), void *object, int *notify));

int ucmplt PARAMS((BW *bw, int k));

#endif
