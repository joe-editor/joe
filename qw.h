/*
 *	Single-key query windows
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_QW_H
#define _JOE_QW_H 1

#include "config.h"
#include "types.h"

/* QW *mkqw(W *w, char *prompt, int (*func)(), int (*abrt)(), void *object);
 * Create a query window for the given window
 */
/* FIXME: ??? ----> */
QW *mkqw PARAMS((W *w, char *prompt, int len, int (*func) (/* ??? */), int (*abrt) (/* ??? */), void *object, int *notify));
QW *mkqwna PARAMS((W *w, char *prompt, int len, int (*func) (/* ??? */), int (*abrt) (/* ??? */), void *object, int *notify));
QW *mkqwnsr PARAMS((W *w, char *prompt, int len, int (*func) (/* ??? */), int (*abrt) (/* ??? */), void *object, int *notify));

#endif
