/* Simple hash table */

#ifndef _Ihash
#define _Ihash 1

#include "config.h"

#define hnext(accu,c) (((accu)<<4)+((accu)>>28)+(c))

typedef struct entry HENTRY;
struct entry {
	char *name;
	HENTRY *next;
	void *val;
};

typedef struct hash HASH;
struct hash {
	int len;
	HENTRY **tab;
};

unsigned long hash PARAMS((char *s));
HASH *htmk PARAMS((int len));
void htrm PARAMS((HASH * ht));
void *htadd PARAMS((HASH * ht, char *name, void *val));
void *htfind PARAMS((HASH * ht, char *name));

#endif
