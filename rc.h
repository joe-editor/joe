/*
 *	*rc file parser
 *	Copyright
 *		(C) 1992 Joseph H. Allen; 
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_RC_H
#define _JOE_RC_H 1

#include "config.h"
#include "types.h"

#include "kbd.h"
#include "macro.h"

extern OPTIONS pdefault;
void setopt PARAMS((OPTIONS *n, char *name));

/* KMAP *getcontext(char *name);
 * Find and return the KMAP for a given context name.  If none is found, an
 * empty kmap is created, bound to the context name, and returned.
 */
KMAP *getcontext PARAMS((char *name));

/* int procrc(CAP *cap, char *name);  Process an rc file
   Returns 0 for success
          -1 for file not found
           1 for syntax error (errors written to stderr)
*/
int procrc PARAMS((CAP *cap, char *name));

int glopt PARAMS((char *s, char *arg, OPTIONS *options, int set));

/* FIXME: cyclic dependency of header files (here on BW struct)
int umode PARAMS((BW *bw));
*/
int umode();

#endif
