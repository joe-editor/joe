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
BW *wmkpw PARAMS((W *w, unsigned char *prompt, B **history, int (*func) (), unsigned char *huh, int (*abrt) (), int (*tab) (), void *object, int *notify, struct charmap *map));

int ucmplt PARAMS((BW *bw, int k));

/* Function for TAB completion */

unsigned char **regsub PARAMS((unsigned char **z, int len, unsigned char *s));

void cmplt_ins PARAMS((BW *bw,unsigned char *line));

int cmplt_abrt PARAMS((BW *bw,int x, unsigned char *line));

int cmplt_rtn PARAMS((MENU *m,int x,unsigned char *line));

int simple_cmplt PARAMS((BW *bw,unsigned char **list));

#endif
