/*
 *	UNICODE/ISO-10646 conversion utilities
 *	Copyright
 *		(C) 2004 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

#ifndef _Icharmap
#define _Icharmap 1

#include "config.h"
#include "types.h"

struct pair {
	int first;			/* Unicode */
	int last;			/* Byte */
};

struct charmap {
	struct charmap *next;		/* Linked list of loaded character maps */
	unsigned char *name;		/* Name of this one */
	int to_map[256];		/* Convert byte to unicode */
	struct pair from_map[256];	/* Convert from unicode to byte */
	int from_size;
};

/* Find (load if necessary) a character set */
struct charmap *find_charmap PARAMS((unsigned char *name));

/* Convert byte to unicode */
int to_uni PARAMS((struct charmap *cset, int c));

/* Convert unicode to byte */
int from_uni PARAMS((struct charmap *cset, int c));

#endif
