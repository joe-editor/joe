/*
 *	UTF-8 Utilities
 *	Copyright
 *		(C) 2004 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "config.h"
#include "types.h"

#include <string.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if defined(HAVE_LOCALE_H) && defined(HAVE_SETLOCALE)
#	include <locale.h>
#       include <langinfo.h>
#	include <iconv.h>
#endif

#include "utf8.h"

/* UTF-8 Encoder
 *
 * c is unicode character.
 * buf is 7 byte buffer- utf-8 coded character is written to this followed by a 0 termination.
 * returns length (not including terminator).
 */

int utf8_encode(unsigned char *buf,int c)
{
	if (c < 0x80) {
		buf[0] = c;
		buf[1] = 0;
		return 1;
	} else if(c < 0x800) {
		buf[0] = (0xc0|(c>>6));
		buf[1] = (0x80|(c&0x3F));
		buf[2] = 0;
		return 2;
	} else if(c < 0x20000) {
		buf[0] = (0xe0|(c>>12));
		buf[1] = (0x80|((c>>6)&0x3f));
		buf[2] = (0x80|((c)&0x3f));
		buf[3] = 0;
		return 3;
	} else if(c < 0x200000) {
		buf[0] = (0xf0|(c>>18));
		buf[1] = (0x80|((c>>12)&0x3f));
		buf[2] = (0x80|((c>>6)&0x3f));
		buf[3] = (0x80|((c)&0x3f));
		buf[4] = 0;
		return 4;
	} else if(c < 0x4000000) {
		buf[0] = (0xf8|(c>>24));
		buf[1] = (0x80|((c>>18)&0x3f));
		buf[2] = (0x80|((c>>12)&0x3f));
		buf[3] = (0x80|((c>>6)&0x3f));
		buf[4] = (0x80|((c)&0x3f));
		buf[5] = 0;
		return 5;
	} else {
		buf[0] = (0xfC|(c>>30));
		buf[1] = (0x80|((c>>24)&0x3f));
		buf[2] = (0x80|((c>>18)&0x3f));
		buf[3] = (0x80|((c>>12)&0x3f));
		buf[4] = (0x80|((c>>6)&0x3f));
		buf[5] = (0x80|((c)&0x3f));
		buf[6] = 0;
		return 6;
	}
}

/* UTF-8 Decoder
 *
 * Returns 0 - 7FFFFFFF: decoded character
 *                   -1: character accepted, nothing decoded yet.
 *                   -2: incomplete sequence
 *                   -3: no sequence started, but character is between 128 - 191, 254 or 255
 */

int utf8_decode(struct utf8_sm *utf8_sm,unsigned char c)
{
	if (utf8_sm->state) {
		if ((c&0xC0)==0x80) {
			utf8_sm->buf[utf8_sm->ptr++] = c;
			--utf8_sm->state;
			utf8_sm->accu = ((utf8_sm->accu<<6)|(c&0x3F));
			if(!utf8_sm->state)
				return utf8_sm->accu;
		} else {
			utf8_sm->state = 0;
			return -2;
		}
	} else if ((c&0xE0)==0xC0) {
		/* 192 - 223 */
		utf8_sm->buf[0] = c;
		utf8_sm->ptr = 1;
		utf8_sm->state = 1;
		utf8_sm->accu = (c&0x1F);
	} else if ((c&0xF0)==0xE0) {
		/* 224 - 239 */
		utf8_sm->buf[0] = c;
		utf8_sm->ptr = 1;
		utf8_sm->state = 2;
		utf8_sm->accu = (c&0x0F);
	} else if ((c&0xF8)==0xF0) {
		/* 240 - 247 */
		utf8_sm->buf[0] = c;
		utf8_sm->ptr = 1;
		utf8_sm->state = 3;
		utf8_sm->accu = (c&0x07);
	} else if ((c&0xFC)==0xF8) {
		/* 248 - 251 */
		utf8_sm->buf[0] = c;
		utf8_sm->ptr = 1;
		utf8_sm->state = 4;
		utf8_sm->accu = (c&0x03);
	} else if ((c&0xFE)==0xFC) {
		/* 252 - 253 */
		utf8_sm->buf[0] = c;
		utf8_sm->ptr = 1;
		utf8_sm->state = 5;
		utf8_sm->accu = (c&0x01);
	} else if ((c&0x80)==0x00) {
		/* 0 - 127 */
		utf8_sm->buf[0] = c;
		utf8_sm->ptr = 1;
		utf8_sm->state = 0;
		return c;
	} else {
		/* 128 - 191, 254, 255 */
		utf8_sm->ptr = 0;
		utf8_sm->state = 0;
		return c;
	}
	return -1;
}

/* Initialize state machine */

void utf8_init(struct utf8_sm *utf8_sm)
{
	utf8_sm->ptr = 0;
	utf8_sm->state = 0;
}

/* Decode a string */

int utf8_decode_string(unsigned char *s)
{
	struct utf8_sm sm;
	int x;
	int c;
	utf8_init(&sm);
	for(x=0;s[x];++x)
		c = utf8_decode(&sm,s[x]);
	return c;
}

/* Initialize locale for JOE */

int utf8;		/* Set if terminal is UTF-8 */
unsigned char *codeset;	/* Codeset of terminal */

unsigned char *non_utf8_codeset;
			/* Codeset of local language non-UTF-8 */
			/* What if it is UTF-8? */

#ifdef HAVE_SETLOCALE
iconv_t to_utf;
iconv_t from_utf;
#endif

void joe_locale()
{
	unsigned char *s, *t;

	s=(unsigned char *)getenv("LC_ALL");
	if (!s) {
		s=(unsigned char *)getenv("LC_CTYPE");
		if (!s) {
			s=(unsigned char *)getenv("LANG");
		}
	}

	if (s)
		s=(unsigned char *)strdup((char *)s);

	if (t=(unsigned char *)strrchr((char *)s,'.'))
		*t = 0;

	setlocale(LC_ALL,s);
	non_utf8_codeset = (unsigned char *)strdup(nl_langinfo(CODESET));

	setlocale(LC_ALL,"");
	codeset = (unsigned char *)strdup(nl_langinfo(CODESET));

	if(!strcmp((char *)codeset,"UTF-8"))
		utf8 = 1;

	to_utf = iconv_open("UTF-8", non_utf8_codeset);
	from_utf = iconv_open(non_utf8_codeset, "UTF-8");
}

void to_utf8(unsigned char *s,int c)
{
	unsigned char buf[10];
	unsigned char *bp;
	int ibuf_sz=1;
	int obuf_sz= 10;
	buf[0]=c;
	buf[1]=0;
	bp = buf;

	iconv(to_utf,(char **)&bp,&ibuf_sz,(char **)&s,&obuf_sz);
	*s = 0;
}

int from_utf8(unsigned char *s)
{
	int ibuf_sz=10;
	unsigned char *ibufp=s;
	
	int obuf_sz=10;
	unsigned char obuf[10];
	unsigned char *obufp = obuf;

	iconv(from_utf,(char **)&s,&ibuf_sz,(char **)&obufp,&obuf_sz);
	return obuf[0];
}
