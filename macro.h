/*
 *	Keyboard macros
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_MACRO_H
#define _JOE_MACRO_H 1

#include "config.h"
#include "types.h"

#include "cmd.h"

/* Set when macro is recording: for status line */
extern struct recmac *recmac;

/* Macro construction functions */
MACRO *mkmacro PARAMS((int k, int arg, int n, CMD *cmd));
void addmacro PARAMS((MACRO *macro, MACRO *m));
MACRO *dupmacro PARAMS((MACRO *mac));
void rmmacro PARAMS((MACRO *macro));
MACRO *macstk PARAMS((MACRO *m, int k));
MACRO *macsta PARAMS((MACRO *m, int a));

void chmac PARAMS((void));

/* Text to macro / Macro to text */
MACRO *mparse PARAMS((MACRO *m, char *buf, int *sta));
char *mtext PARAMS((char *s, MACRO *m));

/* Execute a macro */
extern MACRO *curmacro;
int exemac PARAMS((MACRO *m));
int exmacro PARAMS((MACRO *m, int u));

/* Keyboard macros user interface */
/* FIXME: cyclic dependency of header files (here on BW struct)
int uplay PARAMS((BW *bw, int c));
int urecord PARAMS((BW *bw, int c));
int uquery PARAMS((BW *bw));
int umacros PARAMS((BW *bw));
*/
int uplay();
int ustop PARAMS((void));
int urecord();
int uquery();
int umacros();

/* Repeat prefix user command */
/* FIXME: cyclic dependency of header files (here on BW struct)
int uarg PARAMS((BW *bw));
int uuarg PARAMS((BW *bw, int c));
*/
int uarg();
int uuarg();

#endif
