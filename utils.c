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
#include "blocks.h"
#include "utils.h"

int joe_isspace(int c)
{
	if (c==' ' || c=='\t' || c=='\f' || c=='\n' || c=='\r' || c=='\v')
		return 1;
	if (c<128)
		return 0;

	/* Check in character set? */
	return 0;
}

/*
 * Whitespace characters are characters like tab, space, ...
 *      Every config line in *rc must be end by whitespace but
 *      EOF is not considered as whitespace by isspace()
 *      This is because I don't want to be forced to end
 *      *rc file with \n
 */
int isspace_eof(int c)
{
	return(joe_isspace(c) || (!c));
}

/*
 * Define function isblank(c)
 *	!!! code which uses isblank() assumes tested char is evaluated
 *	only once, so it musn't be a macro
 */
/* GNU is blank does not work properly for wide characters */

int joe_isblank(int c)
{
	return((c == 32) || (c == 9));
}

int joe_isprint(int wide,struct charmap *map,int c)
{
	if (c<32)
		return 0;
	if (c==127)
		return 0;
	if (c<128)
		return 1;

	if (wide)
		return joe_iswprint(c);
	else {
		c = to_uni(map,c);

		if (c==-1)
			return 0;

		return joe_iswprint(c);
	}
}

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

unsigned char *lowerize(unsigned char *s)
{
	unsigned char *t;
	for (t=s;*t;t++)
		*t = joe_tolower(locale_map,*t);
	return s;
}

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

/* wrappers to *alloc routines */
void *joe_malloc(size_t size)
{
	return malloc(size);
}

void *joe_calloc(size_t nmemb, size_t size)
{
	return calloc(nmemb, size);
}

void *joe_realloc(void *ptr, size_t size)
{
	return realloc(ptr, size);
}

void joe_free(void *ptr)
{
	free(ptr);
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
	if (isalpha_(0,locale_map,*p)) {
		while(len && isalnum_(0,locale_map,*p))
			*buf++= *p++, --len;
		*buf=0;
		while(isalnum_(0,locale_map,*p))
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
	if(!*kw && !isalnum_(0,locale_map,*p)) {
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
