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

unsigned long hash (char *s);
HASH *htmk (int len);
void htrm (HASH *ht);
void *htadd (HASH *ht, char *name, void *val);
void *htfind (HASH *ht, char *name);

#endif
