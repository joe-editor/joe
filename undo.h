#ifndef _Iundo
#define _Iundo 1

#include "config.h"
#include "queue.h"
#include "b.h"

#define UNDOKEEP 100

typedef struct undo UNDO;
typedef struct undorec UNDOREC;

struct undorec
 {
 LINK(UNDOREC) link;
 UNDOREC *unit;
 int min;
 int changed;		/* Status of modified flag before this record */
 long where;		/* Buffer address of this record */
 long len;		/* Length of insert or delete */
 int del;		/* Set if this is a delete */
 B *big;		/* Set to buffer containing a large amount of deleted data */
 char *small;		/* Set to malloc block containg a small amount of deleted data */
 };

struct undo
 {
 LINK(UNDO) link;
 B *b;
 int nrecs;
 UNDOREC recs;
 UNDOREC *ptr;
 UNDOREC *first;
 UNDOREC *last;
 };

extern int inundo;
extern int justkilled;

UNDO *undomk();
void undorm();
int uundo();
int uredo();
void umclear();
void undomark();
void undoins();
void undodel();
int uyank();
int uyankpop();
int uyapp();
int unotmod();
int ucopy();

#endif
