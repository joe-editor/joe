/*
 *	Doubly linked list primitives
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_QUEUE
#define _JOE_QUEUE 1 

#include "config.h"

extern void *ITEM;
extern void *QUEUE;
extern void *LAST;

#define izque(type,member,item) do { \
	QUEUE = (void *)(item); \
	((type *)QUEUE)->member.prev = (type *)QUEUE; \
	((type *)QUEUE)->member.next = (type *)QUEUE; \
	} while(0)

#define deque(type,member,item) do { \
	ITEM = (void *)(item); \
	((type *)ITEM)->member.prev->member.next = ((type *)ITEM)->member.next; \
	((type *)ITEM)->member.next->member.prev = ((type *)ITEM)->member.prev; \
	} while(0)

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

#define enquef(type,member,queue,item) do { \
	ITEM = (void *)(item); \
	QUEUE = (void *)(queue); \
	((type *)ITEM)->member.next = ((type *)QUEUE)->member.next; \
	((type *)ITEM)->member.prev = (type *)QUEUE; \
	((type *)QUEUE)->member.next->member.prev = (type *)ITEM; \
	((type *)QUEUE)->member.next = (type *)ITEM; \
	} while(0)

#define enqueb(type,member,queue,item) do { \
	ITEM = (void *)(item); \
	QUEUE = (void *)(queue); \
	((type *)ITEM)->member.next = (type *)QUEUE; \
	((type *)ITEM)->member.prev = ((type *)QUEUE)->member.prev; \
	((type *)QUEUE)->member.prev->member.next = (type *)ITEM; \
	((type *)QUEUE)->member.prev = (type *)ITEM; \
	} while(0)

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
	enquef(type,member,(queue),deque_f(type,member,(item)))

#define demote(type,member,queue,item) \
	enqueb(type,member,(queue),deque_f(type,member,(item)))

#define splicef(type,member,queue,chain) do { \
	ITEM = (void *)(chain); \
	LAST = (void *)((type *)ITEM)->member.prev; \
	QUEUE = (void *)(queue); \
	((type *)LAST)->member.next = ((type *)QUEUE)->member.next; \
	((type *)ITEM)->member.prev = (type *)QUEUE; \
	((type *)QUEUE)->member.next->member.prev = (type *)LAST; \
	((type *)QUEUE)->member.next = (type *)ITEM; \
	} while(0)

#define spliceb(type,member,queue,chain) do { \
	ITEM = (void *)(chain); \
	LAST = (void *)((type *)ITEM)->member.prev; \
	QUEUE = (void *)(queue); \
	((type *)LAST)->member.next = (type *)QUEUE; \
	((type *)ITEM)->member.prev = ((type *)QUEUE)->member.prev; \
	((type *)QUEUE)->member.prev->member.next = (type *)ITEM; \
	((type *)QUEUE)->member.prev = (type *)LAST; \
	} while(0)

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
