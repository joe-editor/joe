/*
 	Doubly linked list primitives
 	Copyright (C) 1992 Joseph H. Allen
 
 	This file is part of JOE (Joe's Own Editor)
 */

#include "config.h"
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include "queue.h"

void *QUEUE;
void *ITEM;
void *LAST;

void *alitem(void *list, int itemsize)
{
	STDITEM	*freelist = (STDITEM *)list;

	if (qempty(STDITEM, link, freelist)) {
		STDITEM *i = (STDITEM *) malloc(itemsize * 16);
		STDITEM *z = (STDITEM *) ((char *) i + itemsize * 16);

		while (i != z) {
			enquef(STDITEM, link, freelist, i);
			i = (STDITEM *) ((char *) i + itemsize);
		}
	}
	return (void *) deque_f(STDITEM, link, freelist->link.prev);
}

void frchn(void *list, void *ch)
{
	STDITEM *freelist = (STDITEM *)list;
	STDITEM *chn = (STDITEM *)ch;
	STDITEM *i;

	if ((i = chn->link.prev) != chn) {
		deque(STDITEM, link, chn);
		splicef(STDITEM, link, freelist, i);
	}
}
