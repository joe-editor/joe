#ifndef _Iuisrch
#define _Iuisrch 1

#include "queue.h"

typedef struct irec IREC;
struct irec
 {
 LINK(IREC) link;
 int what;				/* 0 repeat, >0 append n chars */
 long start;				/* Cursor search position */
 long disp;				/* Original cursor position */
 };

struct isrch
 {
 IREC irecs;				/* Linked list of positions */
 char *pattern;				/* Search pattern string/prompt */
 int ofst;				/* Offset in pattern past prompt */
 int dir;				/* 0=fwrd, 1=bkwd */
 int quote;				/* Set to quote next char */
 };

int uisrch();
int ursrch();

#endif
