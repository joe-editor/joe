/*
	Doubly linked list primitives
	Copyright (C) 1992 Joseph H. Allen

	This file is part of JOE (Joe's Own Editor)
*/

#ifndef _Iqueue
#define _Iqueue 1

#include "config.h"

extern void *ITEM;
extern void *QUEUE;
extern void *LAST;

#define LINK(type) struct { type *next; type *prev; }

typedef struct stditem STDITEM;
struct stditem {
	LINK(STDITEM) link;
};


#define izque(type,member,item) \
	( \
	QUEUE=(void *)(item), \
	((type *)QUEUE)->member.prev=(type *)QUEUE, \
	((type *)QUEUE)->member.next=(type *)QUEUE \
	)

#define deque(type,member,item) \
	( \
	ITEM=(void *)(item), \
	((type *)ITEM)->member.prev->member.next=((type *)ITEM)->member.next, \
	((type *)ITEM)->member.next->member.prev=((type *)ITEM)->member.prev\
	)

#define deque_f(type,member,item) \
	( \
	ITEM=(void *)(item), \
	((type *)ITEM)->member.prev->member.next=((type *)ITEM)->member.next, \
	((type *)ITEM)->member.next->member.prev=((type *)ITEM)->member.prev, \
	(type *)ITEM \
	)

#define qempty(type,member,item) \
	( \
	QUEUE=(void *)(item), \
	(type *)QUEUE==((type *)QUEUE)->member.next \
	)

#define enquef(type,member,queue,item) \
	( \
	ITEM=(void *)(item), \
	QUEUE=(void *)(queue), \
	((type *)ITEM)->member.next=((type *)QUEUE)->member.next, \
	((type *)ITEM)->member.prev=(type *)QUEUE, \
	((type *)QUEUE)->member.next->member.prev=(type *)ITEM, \
	((type *)QUEUE)->member.next=(type *)ITEM \
	)

#define enqueb(type,member,queue,item) \
	( \
	ITEM=(void *)(item), \
	QUEUE=(void *)(queue), \
	((type *)ITEM)->member.next=(type *)QUEUE, \
	((type *)ITEM)->member.prev=((type *)QUEUE)->member.prev, \
	((type *)QUEUE)->member.prev->member.next=(type *)ITEM, \
	((type *)QUEUE)->member.prev=(type *)ITEM \
	)

#define enqueb_f(type,member,queue,item) \
	( \
	ITEM=(void *)(item), \
	QUEUE=(void *)(queue), \
	((type *)ITEM)->member.next=(type *)QUEUE, \
	((type *)ITEM)->member.prev=((type *)QUEUE)->member.prev, \
	((type *)QUEUE)->member.prev->member.next=(type *)ITEM, \
	((type *)QUEUE)->member.prev=(type *)ITEM, \
	(type *)ITEM \
	)

#define promote(type,member,queue,item) \
	( \
	enquef(type,member,(queue),deque_f(type,member,(item))) \
	)

#define demote(type,member,queue,item) \
	( \
	enqueb(type,member,(queue),deque_f(type,member,(item))) \
	)

#define splicef(type,member,queue,chain) \
	( \
	ITEM=(void *)(chain), \
	LAST=(void *)((type *)ITEM)->member.prev, \
	QUEUE=(void *)(queue), \
	((type *)LAST)->member.next=((type *)QUEUE)->member.next, \
	((type *)ITEM)->member.prev=(type *)QUEUE, \
	((type *)QUEUE)->member.next->member.prev=(type *)LAST, \
	((type *)QUEUE)->member.next=(type *)ITEM \
	)

#define spliceb(type,member,queue,chain) \
	( \
	ITEM=(void *)(chain), \
	LAST=(void *)((type *)ITEM)->member.prev, \
	QUEUE=(void *)(queue), \
	((type *)LAST)->member.next=(type *)QUEUE, \
	((type *)ITEM)->member.prev=((type *)QUEUE)->member.prev, \
	((type *)QUEUE)->member.prev->member.next=(type *)ITEM, \
	((type *)QUEUE)->member.prev=(type *)LAST \
	)

#define spliceb_f(type,member,queue,chain) \
	( \
	ITEM=(void *)(chain), \
	LAST=(void *)((type *)ITEM)->member.prev, \
	QUEUE=(void *)(queue), \
	((type *)LAST)->member.next=(type *)QUEUE, \
	((type *)ITEM)->member.prev=((type *)QUEUE)->member.prev, \
	((type *)QUEUE)->member.prev->member.next=(type *)ITEM, \
	((type *)QUEUE)->member.prev=(type *)LAST, \
	(type *)ITEM \
	)

#define snip(type,member,first,last) \
	( \
	ITEM=(void *)(first), \
	LAST=(void *)(last), \
	((type *)LAST)->member.next->member.prev=((type *)ITEM)->member.prev, \
	((type *)ITEM)->member.prev->member.next=((type *)LAST)->member.next, \
	((type *)ITEM)->member.prev=(type *)LAST, \
	((type *)LAST)->member.next=(type *)ITEM, \
	(type *)ITEM \
	)

void *alitem PARAMS((void *list, int itemsize));
void frchn PARAMS((void *list, void *ch));

#endif
