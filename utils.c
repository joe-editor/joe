/*
 *	Various utilities
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *		(C) 2001 Marek 'Marx' Grac
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"

#include <errno.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "i18n.h"
#include "utf8.h"
#include "charmap.h"
#include "blocks.h"
#include "macro.h"
#include "regex.h"
#include "utils.h"


#if 0
int joe_ispunct(int wide,struct charmap *map,int c)
{
	if (joe_isspace(c))
		return 0;

	if (c=='_')
		return 1;

	if (isalnum_(wide,map,c))
		return 0;

	return joe_isprint(wide,map,c);
}
#endif

/*
 * return minimum/maximum of two numbers
 */
unsigned int uns_min(unsigned int a, unsigned int b)
{
	return a < b ? a : b;
}

signed int int_min(signed int a, signed int b)
{
	return a < b ? a : b;
}

signed long int long_max(signed long int a, signed long int b)
{
	return a > b ? a : b;
}

signed long int long_min(signed long int a, signed long int b)
{
	return a < b ? a : b;
}

#if 0
/* 
 * Characters which are considered as word characters 
 * 	_ is considered as word character because is often used 
 *	in the names of C/C++ functions
 */
int isalnum_(int wide,struct charmap *map,int c)
{
	/* Fast... */
	if (c>='0' && c<='9' ||
	    c>='a' && c<='z' ||
	    c>='A' && c<='Z' ||
	    c=='_')
	  return 1;
	else if(c<128)
	  return 0;

	/* Slow... */
	if (wide)
		return joe_iswalpha(c);
	else
		return joe_iswalpha(to_uni(map,c));
}

int isalpha_(int wide,struct charmap *map,int c)
{
	/* Fast... */
	if (c>='a' && c<='z' ||
	    c>='A' && c<='Z' ||
	    c=='_')
	  return 1;
	else if(c<128)
	  return 0;

	/* Slow... */
	if (wide)
		return joe_iswalpha(c);
	else
		return joe_iswalpha(to_uni(map,c));
}
#endif

/* Versions of 'read' and 'write' which automatically retry when interrupted */
ssize_t joe_read(int fd, void *buf, size_t size)
{
	ssize_t rt;

	do {
		rt = read(fd, buf, size);
	} while (rt < 0 && errno == EINTR);
	return rt;
}

ssize_t joe_write(int fd, void *buf, size_t size)
{
	ssize_t rt;

	do {
		rt = write(fd, buf, size);
	} while (rt < 0 && errno == EINTR);
	return rt;
}

/* Heap checking versions of malloc() */

/* #define HEAP_CHECK 1 */

#ifdef HEAP_CHECK

struct mcheck {
	struct mcheck *next;
	int state;		/* 0=malloced, 1=free */
	int size;		/* size in bytes */
	int magic;
};

struct mcheck *first;
struct mcheck *last;

void check_malloc()
{
	struct mcheck *m;
	int y = 0;
	for(m=first;m;m=m->next) {
		unsigned char *ptr = (unsigned char *)m+sizeof(struct mcheck);
		int x;
		if(m->magic!=0x55AA55AA) {
			printf("corrupt heap: head %x\n",ptr);
			*(int *)0=0;
		}
		for(x=0;x!=16384;++x)
			if(ptr[x+m->size]!=0xAA) {
				printf("Corrupt heap: tail %x\n",ptr);
				*(int *)0=0;
			}
		if(m->state)
			for(x=0;x!=m->size;++x)
				if(ptr[x]!=0x41) {
					printf("Corrupt heap: modified free block %x\n",ptr);
					*(int *)0=0;
				}
		++y;
	}
}

void *joe_malloc(size_t size)
{
	struct mcheck *m = (struct mcheck *)malloc(size+sizeof(struct mcheck)+16384);
	unsigned char *ptr = (unsigned char *)m+sizeof(struct mcheck);
	int x;
	m->state = 0;
	m->size = size;
	m->magic=0x55AA55AA;
	m->next = 0;

	if(!size) {
		printf("0 passed to malloc\n");
		*(int *)0=0;
	}
	for(x=0;x!=16384;++x)
		ptr[x+size]=0xAA;
	if(first) {
		last->next = m;
		last = m;
	} else {
		first = last = m;
	}
	check_malloc();
	/* printf("malloc size=%d addr=%x\n",size,ptr); fflush(stdout); */
	return ptr;
}

void *joe_calloc(size_t nmemb, size_t size)
{
	size_t sz = nmemb*size;
	int x;
	unsigned char *ptr;
	ptr = joe_malloc(sz);
	for(x=0;x!=sz;++x)
		ptr[x] = 0;
	return ptr;
}

void *joe_realloc(void *ptr, size_t size)
{
	struct mcheck *n;
	unsigned char *np;
	struct mcheck *m = (struct mcheck *)((char *)ptr-sizeof(struct mcheck));

	if(!size) {
		printf("0 passed to realloc\n");
		*(int *)0=0;
	}

	np = joe_malloc(size);

	n = (struct mcheck *)(np-sizeof(struct mcheck));

	if(m->size>size)
		memcpy(np,ptr,size);
	else
		memcpy(np,ptr,m->size);

	joe_free(ptr);

	return np;
}

void joe_free(void *ptr)
{
	struct mcheck *m = (struct mcheck *)((char *)ptr-sizeof(struct mcheck));
	int x;
	if (m->magic!=0x55AA55AA) {
		printf("Free non-malloc block %x\n",ptr);
		*(int *)0=0x0;
	}
	if (m->state) {
		printf("Double-free %x\n",ptr);
		*(int *)0=0x0;
	}
	for(x=0;x!=m->size;++x)
		((unsigned char *)ptr)[x]=0x41;
	m->state = 1;
	check_malloc();
	/* printf("free=%x\n",ptr); */
}

#else

/* Normal malloc() */

void *joe_malloc(size_t size)
{
	return malloc(size);
}

void *joe_calloc(size_t nmemb,size_t size)
{
	return calloc(nmemb,size);
}

void *joe_realloc(void *ptr,size_t size)
{
	return realloc(ptr,size);
}

void joe_free(void *ptr)
{
	free(ptr);
}

#endif

unsigned char *joe_strdup(unsigned char *bf)
{
	int size = strlen((char *)bf);
	unsigned char *p = (unsigned char *)joe_malloc(size+1);
	memcpy(p,bf,size+1);
	return p;
}

#ifndef SIG_ERR
#define SIG_ERR ((sighandler_t) -1)
#endif

/* wrapper to hide signal interface differrencies */
int joe_set_signal(int signum, sighandler_t handler)
{
	int retval;
#ifdef HAVE_SIGACTION
	struct sigaction sact;

	mset(&sact, 0, sizeof(sact));
	sact.sa_handler = handler;
#ifdef SA_INTERRUPT
	sact.sa_flags = SA_INTERRUPT;
#endif
	retval = sigaction(signum, &sact, NULL);
#elif defined(HAVE_SIGVEC)
	struct sigvec svec;

	mset(&svec, 0, sizeof(svec));
	svec.sv_handler = handler;
#ifdef HAVE_SV_INTERRUPT
	svec.sv_flags = SV_INTERRUPT;
#endif
	retval = sigvec(signum, &svec, NULL);
#else
	retval = (signal(signum, handler) != SIG_ERR) ? 0 : -1;
#ifdef HAVE_SIGINTERRUPT
	siginterrupt(signum, 1);
#endif
#endif
	return(retval);
}

/* Helpful little parsing utilities */

/* Skip whitespace and return first non-whitespace character */

int parse_ws(unsigned char **pp,int cmt)
{
	unsigned char *p = *pp;
	while (*p==' ' || *p=='\t')
		++p;
	if (*p=='\r' || *p=='\n' || *p==cmt)
		*p = 0;
	*pp = p;
	return *p;
}

/* Parse an identifier into a buffer.  Identifier is truncated to a maximum of len chars. */

int parse_ident(unsigned char **pp, unsigned char *buf, int len)
{
	unsigned char *p = *pp;
	if (joe_isalpha_(locale_map,*p)) {
		while(len && joe_isalnum_(locale_map,*p))
			*buf++= *p++, --len;
		*buf=0;
		while(joe_isalnum_(locale_map,*p))
			++p;
		*pp = p;
		return 0;
	} else
		return -1;
}

/* Parse to next whitespace */

int parse_tows(unsigned char **pp, unsigned char *buf)
{
	unsigned char *p = *pp;
	while (*p && *p!=' ' && *p!='\t' && *p!='\n' && *p!='\r' && *p!='#')
		*buf++ = *p++;

	*pp = p;
	*buf = 0;
	return 0;
}

/* Parse a keyword */

int parse_kw(unsigned char **pp, unsigned char *kw)
{
	unsigned char *p = *pp;
	while(*kw && *kw==*p)
		++kw, ++p;
	if(!*kw && !joe_isalnum_(locale_map,*p)) {
		*pp = p;
		return 0;
	} else
		return -1;
}

/* Parse a field */

int parse_field(unsigned char **pp, unsigned char *kw)
{
	unsigned char *p = *pp;
	while(*kw && *kw==*p)
		++kw, ++p;
	if(!*kw && (!*p || *p==' ' || *p=='\t' || *p=='#' || *p=='\n' || *p=='\r')) {
		*pp = p;
		return 0;
	} else
		return -1;
}

/* Parse a character */

int parse_char(unsigned char **pp, unsigned char c)
{
	unsigned char *p = *pp;
	if (*p == c) {
		*pp = p+1;
		return 0;
	} else
		return -1;
}

/* Parse an integer.  Returns 0 for success. */

int parse_int(unsigned char **pp, int *buf)
{
	unsigned char *p = *pp;
	if (*p>='0' && *p<='9' || *p=='-') {
		*buf = atoi((char *)p);
		if(*p=='-')
			++p;
		while(*p>='0' && *p<='9')
			++p;
		*pp = p;
		return 0;
	} else
		return -1;
}

/* Parse a string into a buffer.  Returns 0 for success.
   Leaves escape sequences in string. */

int parse_string(unsigned char **pp, unsigned char *buf, int len)
{
	unsigned char *p= *pp;
	if(*p=='\"') {
		++p;
		while(len && *p && *p!='\"')
			if(*p=='\\' && p[1] && len>2) {
				*buf++ = *p++;
				*buf++ = *p++;
				len-=2;
			} else {
				*buf++ = *p++;
				--len;
			}
		*buf = 0;
		while(*p && *p!='\"')
			if(*p=='\\' && p[1])
				p+=2;
			else
				p++;
		if(*p=='\"') {
			*pp= p+1;
			return 0;
		}
	}
	return -1;
}

/* Parse string escape sequences (fixes them in place- returns new string length) */

int parse_escapes(unsigned char *buf,int len)
{
	int x;
	unsigned char *p;
	x=0;
	for(p=buf;len;) {
		int c = escape(0,&p,&len);
		buf[x++] = c;
	}
	return x;
}

/* Emit a string with escape sequences */

void emit_string(FILE *f,unsigned char *s,int len)
{
	unsigned char buf[8];
	unsigned char *p, *q;
	fputc('\"',f);
	while(len) {
		p = unescape(buf,*s++);
		for(q=buf;q!=p;++q)
			fputc(*q,f);
		--len;
	}
	fputc('\"',f);
}

/* Parse an HDLC string (returns length or -1 for error) */

int parse_hdlc(unsigned char **pp, unsigned char *buf, int len)
{
#if 0
	unsigned char *p= *pp;
	int x = 0;
	if(*p=='~') {
		++p;
		while(len && *p && *p!='~')
			if(*p=='}' && p[1]) {
				++p;
				buf[x++] = 0x20 + *p++;
				--len;
			} else {
				buf[x++] = *p++;
				--len;
			}
		buf[x] = 0;
		while(*p && *p!='~')
			++p;
		if(*p=='~') {
			*pp= p+1;
			return x;
		}
	}
	return -1;
#else
	unsigned char *p= *pp;
	int x = 0;
	if(*p=='"') {
		++p;
		while (len && *p && *p!='"')
			if (*p=='\\') {
				if (p[1]=='\\' || p[1]=='"') {
					++p;
					buf[x++] = *p++;
					--len;
				} else if (p[1]=='n') {
					p+=2;
					buf[x++] = '\n';
					--len;
				} else if (p[1]=='r') {
					p+=2;
					buf[x++] = '\r';
					--len;
				} else if (p[1]=='0' && p[2]=='0' && p[3]=='0') {
					p+=4;
					buf[x++] = 0;
					--len;
				} else
					return -1;
			} else {
				buf[x++] = *p++;
				--len;
			}
		buf[x] = 0;
		while (*p && *p!='"')
			if (*p=='\\') {
				if (p[1]=='\\' || p[1]=='"') {
					p+=2;
				} else if (p[1]=='n') {
					p+=2;
				} else if (p[1]=='r') {
					p+=2;
				} else if (p[1]=='0' && p[2]=='0' && p[3]=='0') {
					p+=4;
				} else
					return -1;
			} else {
				++p;
			}
		if(*p=='"') {
			*pp= p+1;
			return x;
		}
	}
	return -1;
#endif
}

/* Emit an HDLC string */

void emit_hdlc(FILE *f,unsigned char *s,int len)
{
#if 0
	fputc('~',f);
	while(len) {
		if(*s=='~' || *s=='}' || *s==0 || *s=='\n')
			fputc('}',f), fputc(*s-0x20,f);
		else
			fputc(*s,f);
		++s;
		--len;
	}
	fputc('~',f);
#else
	fputc('"',f);
	while(len) {
		if (*s=='"' || *s=='\\')
			fputc('\\',f), fputc(*s,f);
		else if(*s=='\n')
			fputc('\\',f), fputc('n',f);
		else if(*s=='\r')
			fputc('\\',f), fputc('r',f);
		else if(*s==0)
			fputc('\\',f), fputc('0',f), fputc('0',f), fputc('0',f);
		else
			fputc(*s,f);
		++s;
		--len;
	}
	fputc('"',f);
#endif
}

/* Parse a character range: a-z */

int parse_range(unsigned char **pp, int *first, int *second)
{
	unsigned char *p= *pp;
	int a, b;
	if(!*p)
		return -1;
	if(*p=='\\' && p[1]) {
		++p;
		if(*p=='n')
			a = '\n';
		else if(*p=='t')
  			a = '\t';
		else
			a = *p;
		++p;
	} else
		a = *p++;
	if(*p=='-' && p[1]) {
		++p;
		if(*p=='\\' && p[1]) {
			++p;
			if(*p=='n')
				b = '\n';
			else if(*p=='t')
				b = '\t';
			else
				b = *p;
			++p;
		} else
			b = *p++;
	} else
		b = a;
	*first = a;
	*second = b;
	*pp = p;
	return 0;
}
