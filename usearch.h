#ifndef _Isrch
#define _Isrch 1

#include "config.h"
#include "queue.h"
#include "b.h"

typedef struct search SRCH;
typedef struct srchrec SRCHREC;

struct srchrec
 {
 LINK(SRCHREC) link;	/* Linked list of search & replace locations */
 int yn;		/* Did we replace? */
 long addr;		/* Where we were */
 };

struct search
 {
 char *pattern;		/* Search pattern */
 char *replacement;	/* Replacement string */
 int backwards;		/* Set if search should go backwards */
 int ignore;		/* Set if we should ignore case */
 int repeat;		/* Set with repeat count (or -1 for no repeat count) */
 int replace;		/* Set if this is search & replace */
 int rest;		/* Set to do remainder of search & replace w/o query */
 char *entire;		/* Entire matched string */
 char *pieces[26];	/* Peices of the matched string */
 int flg;		/* Set after prompted for first replace */
 SRCHREC recs;		/* Search & replace position history */
 P *markb, *markk;	/* Original marks */
 long addr;		/* Addr of last replacement or -1 for none */
 int restrict;		/* Search restricted to marked block */
 };

SRCH *mksrch();
void rmsrch();

int dopfnext();

int pffirst();
int pfnext();

int pqrepl();
int prfirst();

#endif
