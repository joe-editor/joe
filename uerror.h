/*
 *	Compiler error handler
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_UERROR_H
#define _JOE_UERROR_H 1

#include "config.h"
#include "types.h"

int unxterr PARAMS((BW *bw));
int uprverr PARAMS((BW *bw));
int uparserr PARAMS((BW *bw));
void inserr PARAMS((unsigned char *name, long int where, long int n, int bol));
void delerr PARAMS((unsigned char *name, long int where, long int n));
void abrerr PARAMS((unsigned char *name));
void saverr PARAMS((unsigned char *name));

#endif
