/*
	Single-key query windows
	Copyright (C) 1992 Joseph H. Allen

	This file is part of JOE (Joe's Own Editor)
*/

#ifndef _Iquery
#define _Iquery 1

#include "config.h"
#include "w.h"

typedef struct query QW;
struct query {
	W *parent;		/* Window we're in */
	int (*func) ();		/* Func. which gets called when key is hit */
	int (*abrt) ();
	void *object;
	char *prompt;		/* Prompt string */
	int promptlen;		/* Width of prompt string */
	int promptofst;		/* Prompt scroll offset */
};

#define TYPEQW 0x1000

/* QW *mkqw(BW *bw,char *prompt,int (*func)(),int (*abrt)(),void *object);
 * Create a query window for the given window
 */
QW *mkqw (W *w, char *prompt, int len, int (*func) (), int (*abrt) (), void *object, int *notify);
QW *mkqwna (W *w, char *prompt, int len, int (*func) (), int (*abrt) (), void *object, int *notify);
QW *mkqwnsr (W *w, char *prompt, int len, int (*func) (), int (*abrt) (), void *object, int *notify);

#endif
