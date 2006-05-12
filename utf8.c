/*
 *	UTF-8 Utilities
 *	Copyright
 *		(C) 2004 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "types.h"
#include "utils.h"

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/* nl_langinfo(CODESET) is broken on many systems.  If HAVE_SETLOCALE is undefined,
   JOE uses a limited internal version instead */

#ifndef CODESET
#undef HAVE_SETLOCALE
#endif

/* Under AmigaOS we have setlocale() but don't have langinfo.h and associated stuff,
 * so we have to disable the whole piece of code
 */
#ifdef __amigaos
#undef HAVE_SETLOCALE
#endif

/* Cygwin has CODESET, but it's crummy */
#ifdef __CYGWIN__
#undef HAVE_SETLOCALE
#endif

#if defined(HAVE_LOCALE_H) && defined(HAVE_SETLOCALE)
#	include <locale.h>
#       include <langinfo.h>
#endif

#include "rc.h"
#include "utf8.h"
#include "charmap.h"

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
	} else if(c < 0x10000) {
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
		return -3;
	}
	return -1;
}

/* Initialize state machine */

void utf8_init(struct utf8_sm *utf8_sm)
{
	utf8_sm->ptr = 0;
	utf8_sm->state = 0;
}

/* Decode an entire string */

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

/* Decode and advance
 *
 * Returns: 0 - 7FFFFFFF: decoded character
 *  -2: incomplete sequence
 *  -3: bad start of sequence found.
 *
 * p/plen are always advanced in such a way that repeated called to utf8_decode_fwrd do not cause
 * infinite loops.
 */

int utf8_decode_fwrd(unsigned char **p,int *plen)
{
	struct utf8_sm sm;
	unsigned char *s = *p;
	int len = *plen;
	int c = -2; /* Return this on no more input. */

	utf8_init(&sm);

	while (len) {
		c = utf8_decode(&sm, *s);
		if (c >= 0) {
			/* We've got a character */
			--len;
			++s;
			break;
		} else if (c == -2) {
			/* Bad sequence detected.  Caller should feed rest of string in again. */
			break;
		} else if (c == -3) {
			/* Bad start of UTF-8 sequence.  We need to eat this char to avoid infinite loops. */
			--len;
			++s;
			/* But we should tell the caller that something bad was found. */
			break;
		} else {
			/* If c is -1, utf8_decode accepted the character, so we should get the next one. */
			--len;
			++s;
		}
	}

	*plen = len;
	*p = s;

	return c;
}

/* For systems (BSD) with no nl_langinfo(CODESET) */

/*
 * This is a quick-and-dirty emulator of the nl_langinfo(CODESET)
 * function defined in the Single Unix Specification for those systems
 * (FreeBSD, etc.) that don't have one yet. It behaves as if it had
 * been called after setlocale(LC_CTYPE, ""), that is it looks at
 * the locale environment variables.
 *
 * http://www.opengroup.org/onlinepubs/7908799/xsh/langinfo.h.html
 *
 * Please extend it as needed and suggest improvements to the author.
 * This emulator will hopefully become redundant soon as
 * nl_langinfo(CODESET) becomes more widely implemented.
 *
 * Since the proposed Li18nux encoding name registry is still not mature,
 * the output follows the MIME registry where possible:
 *
 *   http://www.iana.org/assignments/character-sets
 *
 * A possible autoconf test for the availability of nl_langinfo(CODESET)
 * can be found in
 *
 *   http://www.cl.cam.ac.uk/~mgk25/unicode.html#activate
 *
 * Markus.Kuhn@cl.cam.ac.uk -- 2002-03-11
 * Permission to use, copy, modify, and distribute this software
 * for any purpose and without fee is hereby granted. The author
 * disclaims all warranties with regard to this software.
 *
 * Latest version:
 *
 *   http://www.cl.cam.ac.uk/~mgk25/ucs/langinfo.c
 */

unsigned char *joe_getcodeset(unsigned char *l)
{
  static unsigned char buf[16];
  unsigned char *p;
  
  if (((l = (unsigned char *)getenv("LC_ALL"))   && *l) ||
      ((l = (unsigned char *)getenv("LC_CTYPE")) && *l) ||
      ((l = (unsigned char *)getenv("LANG"))     && *l)) {

    /* check standardized locales */
    if (!zcmp(l, US "C") || !zcmp(l, US "POSIX"))
      return US "ascii";

    /* check for encoding name fragment */
    if (zstr(l, US "UTF") || zstr(l, US "utf"))
      return US "UTF-8";

    if ((p = zstr(l, US "8859-"))) {
      memcpy((char *)buf, "ISO-8859-\0\0", 12);
      p += 5;
      if (*p >= '0' && *p <= '9') {
	buf[9] = *p++;
	if (*p >= '0' && *p <= '9') buf[10] = *p++;
	return buf;
      }
    }

    if (zstr(l, US "KOI8-R")) return US "KOI8-R";
    if (zstr(l, US "KOI8-U")) return US "KOI8-U";
    if (zstr(l, US "620")) return US "TIS-620";
    if (zstr(l, US "2312")) return US "GB2312";
    if (zstr(l, US "HKSCS")) return US "Big5HKSCS";   /* no MIME charset */
    if (zstr(l, US "Big5") || zstr(l, US "BIG5")) return US "Big5";
    if (zstr(l, US "GBK")) return US "GBK";           /* no MIME charset */
    if (zstr(l, US "18030")) return US "GB18030";     /* no MIME charset */
    if (zstr(l, US "Shift_JIS") || zstr(l, US "SJIS")) return US "Shift_JIS";
    /* check for conclusive modifier */
    if (zstr(l, US "euro")) return US "ISO-8859-15";
    /* check for language (and perhaps country) codes */
    if (zstr(l, US "zh_TW")) return US "Big5";
    if (zstr(l, US "zh_HK")) return US "Big5HKSCS";   /* no MIME charset */
    if (zstr(l, US "zh")) return US "GB2312";
    if (zstr(l, US "ja")) return US "EUC-JP";
    if (zstr(l, US "ko")) return US "EUC-KR";
    if (zstr(l, US "ru")) return US "KOI8-R";
    if (zstr(l, US "uk")) return US "KOI8-U";
    if (zstr(l, US "pl") || zstr(l, US "hr") ||
	zstr(l, US "hu") || zstr(l, US "cs") ||
	zstr(l, US "sk") || zstr(l, US "sl")) return US "ISO-8859-2";
    if (zstr(l, US "eo") || zstr(l, US "mt")) return US "ISO-8859-3";
    if (zstr(l, US "el")) return US "ISO-8859-7";
    if (zstr(l, US "he")) return US "ISO-8859-8";
    if (zstr(l, US "tr")) return US "ISO-8859-9";
    if (zstr(l, US "th")) return US "TIS-620";      /* or ISO-8859-11 */
    if (zstr(l, US "lt")) return US "ISO-8859-13";
    if (zstr(l, US "cy")) return US "ISO-8859-14";
    if (zstr(l, US "ro")) return US "ISO-8859-2";   /* or ISO-8859-16 */
    if (zstr(l, US "am") || zstr(l, US "vi")) return US "UTF-8";
    /* Send me further rules if you like, but don't forget that we are
     * *only* interested in locale naming conventions on platforms
     * that do not already provide an nl_langinfo(CODESET) implementation. */
    return US "ISO-8859-1"; /* should perhaps be "UTF-8" instead */
  }
  return US "ascii";
}

/* Initialize locale for JOE */

unsigned char *codeset;	/* Codeset of terminal */

unsigned char *non_utf8_codeset;
			/* Codeset of local language non-UTF-8 */

struct charmap *locale_map;
			/* Character map of terminal, default map for new files */

struct charmap *locale_map_non_utf8;
			/* Old, non-utf8 version of locale */

void joe_locale()
{
	unsigned char *s, *t, *u;

	int x;

	s=(unsigned char *)getenv("LC_ALL");
	if (!s) {
		s=(unsigned char *)getenv("LC_CTYPE");
		if (!s) {
			s=(unsigned char *)getenv("LANG");
		}
	}

	if (s)
		s=zdup(s);
	else
		s=US "ascii";

	u = zdup(s);

	if (t=zrchr(s,'.'))
		*t = 0;


#ifdef HAVE_SETLOCALE
	setlocale(LC_ALL,(char *)s);
	non_utf8_codeset = zdup((unsigned char *)nl_langinfo(CODESET));
#else
	non_utf8_codeset = joe_getcodeset(s);
#endif


#ifdef HAVE_SETLOCALE
	setlocale(LC_ALL,"");
	codeset = zdup((unsigned char *)nl_langinfo(CODESET));
#else
	codeset = joe_getcodeset(u);
#endif

	locale_map = find_charmap(codeset);
	if (!locale_map)
		locale_map = find_charmap(US "ascii");

	locale_map_non_utf8 = find_charmap(non_utf8_codeset);
	if (!locale_map_non_utf8)
		locale_map_non_utf8 = find_charmap(US "ascii");

	fdefault.charmap = locale_map;
	pdefault.charmap = locale_map;

/*
	printf("Character set is %s\n",locale_map->name);

	for(x=0;x!=128;++x)
		printf("%x	space=%d blank=%d alpha=%d alnum=%d punct=%d print=%d\n",
		       x,joe_isspace(locale_map,x), joe_isblank(locale_map,x), joe_isalpha_(locale_map,x),
		       joe_isalnum_(locale_map,x), joe_ispunct(locale_map,x), joe_isprint(locale_map,x));
*/

/* Use iconv() */
#ifdef junk
	to_utf = iconv_open("UTF-8", (char *)non_utf8_codeset);
	from_utf = iconv_open((char *)non_utf8_codeset, "UTF-8");
#endif

/* For no locale support */
#ifdef junk
	locale_map = find_charmap("ascii");
	fdefault.charmap = locale_map;
	pdefault.charmap = locale_map;
#endif
}

void to_utf8(struct charmap *map,unsigned char *s,int c)
{
	int d = to_uni(map,c);

	if (d==-1)
		utf8_encode(s,'?');
	else
		utf8_encode(s,d);
#ifdef junk
	/* Iconv() way */
	unsigned char buf[10];
	unsigned char *ibufp = buf;
	unsigned char *obufp = s;
	int ibuf_sz=1;
	int obuf_sz= 10;
	buf[0]=c;
	buf[1]=0;

	if (to_utf==(iconv_t)-1 ||
	    iconv(to_utf,(char **)&ibufp,&ibuf_sz,(char **)&obufp,&obuf_sz)==(size_t)-1) {
		s[0]='?';
		s[1]=0;
	} else {
		*obufp = 0;
	}
#endif
}

int from_utf8(struct charmap *map,unsigned char *s)
{
	int d = utf8_decode_string(s);
	int c = from_uni(map,d);
	if (c==-1)
		return '?';
	else
		return c;

#ifdef junk
	/* Iconv() way */
	int ibuf_sz=zlen(s);
	unsigned char *ibufp=s;
	int obuf_sz=10;
	unsigned char obuf[10];
	unsigned char *obufp = obuf;


	if (from_utf==(iconv_t)-1 ||
	    iconv(from_utf,(char **)&ibufp,&ibuf_sz,(char **)&obufp,&obuf_sz)==((size_t)-1))
		return '?';
	else
		return obuf[0];
#endif
}

void my_iconv(unsigned char *dest,struct charmap *dest_map,
              unsigned char *src,struct charmap *src_map)
{
	if (dest_map == src_map) {
		zcpy (dest, src);
		return;
	}

	if (src_map->type) {
		/* src is UTF-8 */
		if (dest_map->type) {
			/* UTF-8 to UTF-8? */
			zcpy (dest, src);
		} else {
			/* UTF-8 to non-UTF-8 */
			while (*src) {
				int len = -1;
				int c = utf8_decode_fwrd(&src, &len);
				if (c >= 0) {
					int d = from_uni(dest_map, c);
					if (d >= 0)
						*dest++ = d;
					else
						*dest++ = '?';
				} else
					*dest++ = 'X';
			}
			*dest = 0;
		}
	} else {
		/* src is not UTF-8 */
		if (!dest_map->type) {
			/* Non UTF-8 to non-UTF-8 */
			while (*src) {
				int c = to_uni(src_map, *src++);
				int d;
				if (c >= 0) {
					d = from_uni(dest_map, c);
					if (d >= 0)
						*dest++ = d;
					else
						*dest++ = '?';
				} else
					*dest++ = '?';
			}
			*dest = 0;
		} else {
			/* Non-UTF-8 to UTF-8 */
			while (*src) {
				int c = to_uni(src_map, *src++);
				if (c >= 0)
					dest += utf8_encode(dest, c);
				else
					*dest++ = '?';
			}
			*dest = 0;
		}
	}
}

/* Guess character set */

struct charmap *guess_map(unsigned char *buf, int len)
{
	unsigned char *p;
	int plen;
	int c;
	int flag;

	/* No info? Use default */
	if (!len)
		return locale_map;

	/* Do we UTF-8? */
	if (!locale_map->type)
		/* Nope, use default */
		return locale_map;

	/* Does it look like UTF-8? */
	p = buf;
	plen = len;
	c = 0;
	flag = 0;
	while (plen) {
		if (*p >= 128)
			flag = 1;
		c = utf8_decode_fwrd(&p, &plen);
		if (c < 0)
			break;
	}
	if (c >= 0 && flag) {
		/* No decoding errors but there are utf-8 sequences */
		return find_charmap(US "utf-8");
	}

	/* Well, it might be non-utf8 version of the locale */
	return locale_map_non_utf8;
}
