/*
	Prompt windows
	Copyright (C) 1992 Joseph H. Allen

	This file is part of JOE (Joe's Own Editor)
*/

#ifndef _Ipw
#define _Ipw 1

#include "config.h"
#include "bw.h"
#include "b.h"
#include "w.h"

typedef struct pw PW;

struct pw {
	int (*pfunc) ();	/* Func which gets called when RTN is hit */
	int (*abrt) ();		/* Func which gets called when window is aborted */
	int (*tab) ();		/* Func which gets called when TAB is hit */
	char *prompt;		/* Prompt string */
	int promptlen;		/* Width of prompt string */
	int promptofst;		/* Prompt scroll offset */
	B *hist;		/* History buffer */
	void *object;		/* Object */
};

#define TYPEPW 0x200

/* BW *wmkpw(BW *bw,char *prompt,int (*func)(),char *huh,int (*abrt)(),
             int (*tab)(),void *object,int *notify);
 * Create a prompt window for the given window
 */
BW *wmkpw (W *w, char *prompt, B **history, int (*func) (), char *huh, int (*abrt) (), int (*tab) (), void *object, int *notify);

int ucmplt (BW *bw, int k);

#endif
