/*
 *	Simple hash table
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_HASH_H
#define _JOE_HASH_H 1

#include "config.h"
#include "types.h"

unsigned long hash PARAMS((unsigned char *s));
HASH *htmk PARAMS((int len));
void htrm PARAMS((HASH *ht));
void *htadd PARAMS((HASH *ht, unsigned char *name, void *val));
void *htfind PARAMS((HASH *ht, unsigned char *name));

#endif
