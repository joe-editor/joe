/*
 	Doubly linked list primitives
 	Copyright (C) 1992 Joseph H. Allen
 
 	This file is part of JOE (Joe's Own Editor)
 */

#include "config.h"
#include "queue.h"

void *QUEUE;
void *ITEM;
void *LAST;

void *ALITEM (STDITEM *freelist, int itemsize) {
	if (qempty (STDITEM, link, freelist)) {
		STDITEM *i = (STDITEM *) malloc (itemsize * 16);
		STDITEM *z = (STDITEM *) ((char *) i + itemsize * 16);
		while (i != z) {
			enquef (STDITEM, link, freelist, i);
			i = (STDITEM *) ((char *) i + itemsize);
		}
	}
	return (void *) deque (STDITEM, link, freelist->link.prev);
}

void FRCHN (STDITEM *freelist, STDITEM *chn) {
	STDITEM *i;

	if ((i = chn->link.prev) != chn) {
		  deque (STDITEM, link, chn);
		  splicef (STDITEM, link, freelist, i);
	}
}
