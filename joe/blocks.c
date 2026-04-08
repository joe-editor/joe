/*
 *	Fast block move/copy subroutines
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
/* This module requires ALIGNED and SIZEOF_INT to be defined correctly */

#include "types.h"

#define BITS 8

#if SIZEOF_INT == 8
#  define SHFT 3
#  define INTFILL_CHAR 0x0101010101010101
#elif SIZEOF_INT == 4
#  define SHFT 2
#  define INTFILL_CHAR 0x01010101
#elif SIZEOF_INT == 2
#  define SHFT 1
#  define INTFILL_CHAR 0x0101
#else
#  error I do not know how to handle your strange integers
#endif

/* Set 'sz' 'int's beginning at 'd' to the value 'c' */
/* Returns address of block.  Does nothing if 'sz' equals zero */

int *msetI(int *dest, int c, ptrdiff_t sz)
{
	int	*d = dest;
	int	*orgd = dest;

	while (sz >= 16) {
		d[0] = c;
		d[1] = c;
		d[2] = c;
		d[3] = c;
		d[4] = c;
		d[5] = c;
		d[6] = c;
		d[7] = c;
		d[8] = c;
		d[9] = c;
		d[10] = c;
		d[11] = c;
		d[12] = c;
		d[13] = c;
		d[14] = c;
		d[15] = c;
		d += 16;
		sz -= 16;
	}
	switch (sz) {
	case 15:	d[14] = c; FALLTHROUGH
	case 14:	d[13] = c; FALLTHROUGH
	case 13:	d[12] = c; FALLTHROUGH
	case 12:	d[11] = c; FALLTHROUGH
	case 11:	d[10] = c; FALLTHROUGH
	case 10:	d[9] = c; FALLTHROUGH
	case 9:		d[8] = c; FALLTHROUGH
	case 8:		d[7] = c; FALLTHROUGH
	case 7:		d[6] = c; FALLTHROUGH
	case 6:		d[5] = c; FALLTHROUGH
	case 5:		d[4] = c; FALLTHROUGH
	case 4:		d[3] = c; FALLTHROUGH
	case 3:		d[2] = c; FALLTHROUGH
	case 2:		d[1] = c; FALLTHROUGH
	case 1:		d[0] = c; FALLTHROUGH
	case 0:		/* do nothing */;
	}
	return orgd;
}

ptrdiff_t *msetD(ptrdiff_t *dest, ptrdiff_t c, ptrdiff_t sz)
{
	ptrdiff_t	*d = dest;
	ptrdiff_t	*orgd = dest;

	while (sz >= 16) {
		d[0] = c;
		d[1] = c;
		d[2] = c;
		d[3] = c;
		d[4] = c;
		d[5] = c;
		d[6] = c;
		d[7] = c;
		d[8] = c;
		d[9] = c;
		d[10] = c;
		d[11] = c;
		d[12] = c;
		d[13] = c;
		d[14] = c;
		d[15] = c;
		d += 16;
		sz -= 16;
	}
	switch (sz) {
	case 15:	d[14] = c; FALLTHROUGH
	case 14:	d[13] = c; FALLTHROUGH
	case 13:	d[12] = c; FALLTHROUGH
	case 12:	d[11] = c; FALLTHROUGH
	case 11:	d[10] = c; FALLTHROUGH
	case 10:	d[9] = c; FALLTHROUGH
	case 9:		d[8] = c; FALLTHROUGH
	case 8:		d[7] = c; FALLTHROUGH
	case 7:		d[6] = c; FALLTHROUGH
	case 6:		d[5] = c; FALLTHROUGH
	case 5:		d[4] = c; FALLTHROUGH
	case 4:		d[3] = c; FALLTHROUGH
	case 3:		d[2] = c; FALLTHROUGH
	case 2:		d[1] = c; FALLTHROUGH
	case 1:		d[0] = c; FALLTHROUGH
	case 0:		/* do nothing */;
	}
	return orgd;
}

/* Set 'sz' 'int's beginning at 'd' to the value 'c' */
/* Returns address of block.  Does nothing if 'sz' equals zero */

void **msetP(void **d, void *c, ptrdiff_t sz)
{
	void	**orgd = d;

	while (sz >= 16) {
		d[0] = c;
		d[1] = c;
		d[2] = c;
		d[3] = c;
		d[4] = c;
		d[5] = c;
		d[6] = c;
		d[7] = c;
		d[8] = c;
		d[9] = c;
		d[10] = c;
		d[11] = c;
		d[12] = c;
		d[13] = c;
		d[14] = c;
		d[15] = c;
		d += 16;
		sz -= 16;
	}
	switch (sz) {
	case 15:	d[14] = c; FALLTHROUGH
	case 14:	d[13] = c; FALLTHROUGH
	case 13:	d[12] = c; FALLTHROUGH
	case 12:	d[11] = c; FALLTHROUGH
	case 11:	d[10] = c; FALLTHROUGH
	case 10:	d[9] = c; FALLTHROUGH
	case 9:		d[8] = c; FALLTHROUGH
	case 8:		d[7] = c; FALLTHROUGH
	case 7:		d[6] = c; FALLTHROUGH
	case 6:		d[5] = c; FALLTHROUGH
	case 5:		d[4] = c; FALLTHROUGH
	case 4:		d[3] = c; FALLTHROUGH
	case 3:		d[2] = c; FALLTHROUGH
	case 2:		d[1] = c; FALLTHROUGH
	case 1:		d[0] = c; FALLTHROUGH
	case 0:		/* do nothing */;
	}
	return orgd;
}

/* Set 'sz' 'char's beginning at 'd' to the value 'c' */
/* Returns address of block.  Does nothing if 'sz' equals zero */

char *mset(char *dest, char c, ptrdiff_t sz)
{
	char	*d = dest;
	char	*orgd = dest;

	if (sz < 16) {
		switch (sz) {
		case 15:	d[14] = c; FALLTHROUGH
		case 14:	d[13] = c; FALLTHROUGH
		case 13:	d[12] = c; FALLTHROUGH
		case 12:	d[11] = c; FALLTHROUGH
		case 11:	d[10] = c; FALLTHROUGH
		case 10:	d[9] = c; FALLTHROUGH
		case 9:		d[8] = c; FALLTHROUGH
		case 8:		d[7] = c; FALLTHROUGH
		case 7:		d[6] = c; FALLTHROUGH
		case 6:		d[5] = c; FALLTHROUGH
		case 5:		d[4] = c; FALLTHROUGH
		case 4:		d[3] = c; FALLTHROUGH
		case 3:		d[2] = c; FALLTHROUGH
		case 2:		d[1] = c; FALLTHROUGH
		case 1:		d[0] = c; FALLTHROUGH
		case 0:		/* do nothing */;
		}
	} else {
		ptrdiff_t z = SIZEOF_INT - ((ptrdiff_t)d & (SIZEOF_INT - 1));

		if (z != SIZEOF_INT) {
			switch (z) {
			case 7:		d[6] = c; FALLTHROUGH
			case 6:		d[5] = c; FALLTHROUGH
			case 5:		d[4] = c; FALLTHROUGH
			case 4:		d[3] = c; FALLTHROUGH
			case 3:		d[2] = c; FALLTHROUGH
			case 2:		d[1] = c; FALLTHROUGH
			case 1:		d[0] = c; FALLTHROUGH
			case 0:		/* do nothing */;
			}
			d += z;
			sz -= z;
		}
		msetI((int *)d, ((int)c & 255) * INTFILL_CHAR, sz >> SHFT);
		d += sz & ~(SIZEOF_INT - 1);
		switch (sz & (SIZEOF_INT - 1)) {
		case 7:		d[6] = c; FALLTHROUGH
		case 6:		d[5] = c; FALLTHROUGH
		case 5:		d[4] = c; FALLTHROUGH
		case 4:		d[3] = c; FALLTHROUGH
		case 3:		d[2] = c; FALLTHROUGH
		case 2:		d[1] = c; FALLTHROUGH
		case 1:		d[0] = c; FALLTHROUGH
		case 0:		/* do nothing */;
		}
	}
	return orgd;
}

/* Copy a block of integers */
/* Copy from highest address to lowest */

static int *mbkwdI(int *dest, const int *src, ptrdiff_t sz)
{
	int	*d = dest;
	const int *s = src;

	if (d == s)
		return d;
	d += sz;
	s += sz;
	while (sz >= 16) {
		d -= 16;
		s -= 16;
		d[15] = s[15];
		d[14] = s[14];
		d[13] = s[13];
		d[12] = s[12];
		d[11] = s[11];
		d[10] = s[10];
		d[9] = s[9];
		d[8] = s[8];
		d[7] = s[7];
		d[6] = s[6];
		d[5] = s[5];
		d[4] = s[4];
		d[3] = s[3];
		d[2] = s[2];
		d[1] = s[1];
		d[0] = s[0];
		sz -= 16;
	}
	d -= sz;
	s -= sz;
	switch (sz) {
	case 15:	d[14] = s[14]; FALLTHROUGH
	case 14:	d[13] = s[13]; FALLTHROUGH
	case 13:	d[12] = s[12]; FALLTHROUGH
	case 12:	d[11] = s[11]; FALLTHROUGH
	case 11:	d[10] = s[10]; FALLTHROUGH
	case 10:	d[9] = s[9]; FALLTHROUGH
	case 9:		d[8] = s[8]; FALLTHROUGH
	case 8:		d[7] = s[7]; FALLTHROUGH
	case 7:		d[6] = s[6]; FALLTHROUGH
	case 6:		d[5] = s[5]; FALLTHROUGH
	case 5:		d[4] = s[4]; FALLTHROUGH
	case 4:		d[3] = s[3]; FALLTHROUGH
	case 3:		d[2] = s[2]; FALLTHROUGH
	case 2:		d[1] = s[1]; FALLTHROUGH
	case 1:		d[0] = s[0]; FALLTHROUGH
	case 0:		/* do nothing */;
	}
	return d;
}

/* Copy a block of 'int's.  Copy from lowest address to highest */

static int *mfwrdI(int *dest, const int *src, ptrdiff_t sz)
{
	int	*d = dest;
	const int *s = src;
	int	*od = d;

	if (s == d)
		return d;
	while (sz >= 16) {
		d[0] = s[0];
		d[1] = s[1];
		d[2] = s[2];
		d[3] = s[3];
		d[4] = s[4];
		d[5] = s[5];
		d[6] = s[6];
		d[7] = s[7];
		d[8] = s[8];
		d[9] = s[9];
		d[10] = s[10];
		d[11] = s[11];
		d[12] = s[12];
		d[13] = s[13];
		d[14] = s[14];
		d[15] = s[15];
		s += 16;
		d += 16;
		sz -= 16;
	}
	s -= 15 - sz;
	d -= 15 - sz;
	switch (sz) {
	case 15:	d[0] = s[0]; FALLTHROUGH
	case 14:	d[1] = s[1]; FALLTHROUGH
	case 13:	d[2] = s[2]; FALLTHROUGH
	case 12:	d[3] = s[3]; FALLTHROUGH
	case 11:	d[4] = s[4]; FALLTHROUGH
	case 10:	d[5] = s[5]; FALLTHROUGH
	case 9:		d[6] = s[6]; FALLTHROUGH
	case 8:		d[7] = s[7]; FALLTHROUGH
	case 7:		d[8] = s[8]; FALLTHROUGH
	case 6:		d[9] = s[9]; FALLTHROUGH
	case 5:		d[10] = s[10]; FALLTHROUGH
	case 4:		d[11] = s[11]; FALLTHROUGH
	case 3:		d[12] = s[12]; FALLTHROUGH
	case 2:		d[13] = s[13]; FALLTHROUGH
	case 1:		d[14] = s[14]; FALLTHROUGH
	case 0:		/* do nothing */;
	}
	return od;
}

/* Copy the block of 'sz' bytes beginning at 's' to 'd'.  If 'sz' is zero or
 * if 's'=='d', nothing happens.  The bytes at the highest address ('s'+'sz'-1)
 * are copied before the ones at the lowest ('s') are.
 */

static void *mbkwd(REGISTER void *orgd, REGISTER const void *orgs, REGISTER ptrdiff_t sz)
{
	char *d = (char *)orgd;
	const char *s = (const char *)orgs;
	if (s == d)
		return d;
	s += sz;
	d += sz;
#ifdef ALIGNED
	if (sz >= 16)
#else
	if (((ptrdiff_t)s & (SIZEOF_INT - 1)) == ((ptrdiff_t)d & (SIZEOF_INT - 1)) && sz >= 16)
#endif
	{
		ptrdiff_t z = ((ptrdiff_t)s & (SIZEOF_INT - 1));

		s -= z;
		d -= z;
		switch (z) {
		case 7:		d[6] = s[6]; FALLTHROUGH
		case 6:		d[5] = s[5]; FALLTHROUGH
		case 5:		d[4] = s[4]; FALLTHROUGH
		case 4:		d[3] = s[3]; FALLTHROUGH
		case 3:		d[2] = s[2]; FALLTHROUGH
		case 2:		d[1] = s[1]; FALLTHROUGH
		case 1:		d[0] = s[0]; FALLTHROUGH
		case 0:		/* do nothing */;
		}
		sz -= z;
		mbkwdI((int *)(d - (sz & ~(SIZEOF_INT - 1))), (const int *)(s - (sz & ~(SIZEOF_INT - 1))), sz >> SHFT);
		d -= sz;
		s -= sz;
		switch (sz & (SIZEOF_INT - 1)) {
		case 7:		d[6] = s[6]; FALLTHROUGH
		case 6:		d[5] = s[5]; FALLTHROUGH
		case 5:		d[4] = s[4]; FALLTHROUGH
		case 4:		d[3] = s[3]; FALLTHROUGH
		case 3:		d[2] = s[2]; FALLTHROUGH
		case 2:		d[1] = s[1]; FALLTHROUGH
		case 1:		d[0] = s[0]; FALLTHROUGH
		case 0:		/* do nothing */;
		}
	} else {
		while (sz >= 16) {
			d -= 16;
			s -= 16;
			d[15] = s[15];
			d[14] = s[14];
			d[13] = s[13];
			d[12] = s[12];
			d[11] = s[11];
			d[10] = s[10];
			d[9] = s[9];
			d[8] = s[8];
			d[7] = s[7];
			d[6] = s[6];
			d[5] = s[5];
			d[4] = s[4];
			d[3] = s[3];
			d[2] = s[2];
			d[1] = s[1];
			d[0] = s[0];
			sz -= 16;
		}
		d -= sz;
		s -= sz;
		switch (sz) {
		case 15:	d[14] = s[14]; FALLTHROUGH
		case 14:	d[13] = s[13]; FALLTHROUGH
		case 13:	d[12] = s[12]; FALLTHROUGH
		case 12:	d[11] = s[11]; FALLTHROUGH
		case 11:	d[10] = s[10]; FALLTHROUGH
		case 10:	d[9] = s[9]; FALLTHROUGH
		case 9:		d[8] = s[8]; FALLTHROUGH
		case 8:		d[7] = s[7]; FALLTHROUGH
		case 7:		d[6] = s[6]; FALLTHROUGH
		case 6:		d[5] = s[5]; FALLTHROUGH
		case 5:		d[4] = s[4]; FALLTHROUGH
		case 4:		d[3] = s[3]; FALLTHROUGH
		case 3:		d[2] = s[2]; FALLTHROUGH
		case 2:		d[1] = s[1]; FALLTHROUGH
		case 1:		d[0] = s[0]; FALLTHROUGH
		case 0:		/* do nothing */;
		}
	}
	return d;
}

/* Copy the block of 'sz' bytes beginning at 's' to 'd'.  If 'sz' is zero or
 * if 's'=='d', nothing happens.  The bytes at the lowest address ('s')
 * are copied before the ones at the highest ('s'+'sz'-1) are.
 */

static void *mfwrd(REGISTER void *orgd, REGISTER const void *orgs, REGISTER ptrdiff_t sz)
{
	char *d = (char *)orgd;
	const char *s = (const char *)orgs;

	if (d == s)
		return d;
#ifdef ALIGNED
	if (sz >= 16)
#else
	if (((ptrdiff_t)d & (SIZEOF_INT - 1)) == ((ptrdiff_t)s & (SIZEOF_INT - 1)) && sz >= 16)
#endif
	{
		ptrdiff_t z = ((ptrdiff_t)s & (SIZEOF_INT - 1));

		if (z) {
			s -= z;
			d -= z;
			switch (SIZEOF_INT - z) {
#if SIZEOF_INT == 8
			case 7:		d[1] = s[1]; FALLTHROUGH
			case 6:		d[2] = s[2]; FALLTHROUGH
			case 5:		d[3] = s[3]; FALLTHROUGH
			case 4:		d[4] = s[4]; FALLTHROUGH
			case 3:		d[5] = s[5]; FALLTHROUGH
			case 2:		d[6] = s[6]; FALLTHROUGH
			case 1:		d[7] = s[7]; FALLTHROUGH
			case 0:		/* do nothing */;
#else
#if SIZEOF_INT == 4
			case 3:		d[1] = s[1]; FALLTHROUGH
			case 2:		d[2] = s[2]; FALLTHROUGH
			case 1:		d[3] = s[3]; FALLTHROUGH
			case 0:		/* do nothing */;
#else
#if SIZEOF_INT == 2
			case 1:		d[1] = s[1]; FALLTHROUGH
			case 0:		/* do nothing */;
#endif
#endif
#endif
			}
			s += SIZEOF_INT;
			d += SIZEOF_INT;
			sz -= SIZEOF_INT - z;
		}
		mfwrdI((int *)d, (const int *)s, sz >> SHFT);
		s += sz - (SIZEOF_INT - 1);
		d += sz - (SIZEOF_INT - 1);
		switch (sz & (SIZEOF_INT - 1)) {
#if SIZEOF_INT == 8
		case 7:		d[0] = s[0]; FALLTHROUGH
		case 6:		d[1] = s[1]; FALLTHROUGH
		case 5:		d[2] = s[2]; FALLTHROUGH
		case 4:		d[3] = s[3]; FALLTHROUGH
		case 3:		d[4] = s[4]; FALLTHROUGH
		case 2:		d[5] = s[5]; FALLTHROUGH
		case 1:		d[6] = s[6]; FALLTHROUGH
		case 0:		/* do nothing */;
#else
#if SIZEOF_INT == 4
		case 3:		d[0] = s[0]; FALLTHROUGH
		case 2:		d[1] = s[1]; FALLTHROUGH
		case 1:		d[2] = s[2]; FALLTHROUGH
		case 0:		/* do nothing */;
#else
#if SIZEOF_INT == 2
		case 1:		d[0] = s[0]; FALLTHROUGH
		case 0:		/* do nothing */;
#endif
#endif
#endif
		}
	} else {
		while (sz >= 16) {
			d[0] = s[0];
			d[1] = s[1];
			d[2] = s[2];
			d[3] = s[3];
			d[4] = s[4];
			d[5] = s[5];
			d[6] = s[6];
			d[7] = s[7];
			d[8] = s[8];
			d[9] = s[9];
			d[10] = s[10];
			d[11] = s[11];
			d[12] = s[12];
			d[13] = s[13];
			d[14] = s[14];
			d[15] = s[15];
			s += 16;
			d += 16;
			sz -= 16;
		}
		s -= 15 - sz;
		d -= 15 - sz;
		switch (sz) {
		case 15:	d[0] = s[0]; FALLTHROUGH
		case 14:	d[1] = s[1]; FALLTHROUGH
		case 13:	d[2] = s[2]; FALLTHROUGH
		case 12:	d[3] = s[3]; FALLTHROUGH
		case 11:	d[4] = s[4]; FALLTHROUGH
		case 10:	d[5] = s[5]; FALLTHROUGH
		case 9:		d[6] = s[6]; FALLTHROUGH
		case 8:		d[7] = s[7]; FALLTHROUGH
		case 7:		d[8] = s[8]; FALLTHROUGH
		case 6:		d[9] = s[9]; FALLTHROUGH
		case 5:		d[10] = s[10]; FALLTHROUGH
		case 4:		d[11] = s[11]; FALLTHROUGH
		case 3:		d[12] = s[12]; FALLTHROUGH
		case 2:		d[13] = s[13]; FALLTHROUGH
		case 1:		d[14] = s[14]; FALLTHROUGH
		case 0:		/* do nothing */;
		}
	}
	return orgd;
}

void *mmove(void *d, const void *s, ptrdiff_t sz)
{
	if (d > s)
		mbkwd(d, s, sz);
	else
		mfwrd(d, s, sz);
	return d;
}

/* Utility to count number of lines within a segment */

ptrdiff_t mcnt(REGISTER const char *blk, REGISTER char c, ptrdiff_t size)
{
	REGISTER ptrdiff_t nlines = 0;

	while (size >= 16) {
		if (blk[0] == c) ++nlines;
		if (blk[1] == c) ++nlines;
		if (blk[2] == c) ++nlines;
		if (blk[3] == c) ++nlines;
		if (blk[4] == c) ++nlines;
		if (blk[5] == c) ++nlines;
		if (blk[6] == c) ++nlines;
		if (blk[7] == c) ++nlines;
		if (blk[8] == c) ++nlines;
		if (blk[9] == c) ++nlines;
		if (blk[10] == c) ++nlines;
		if (blk[11] == c) ++nlines;
		if (blk[12] == c) ++nlines;
		if (blk[13] == c) ++nlines;
		if (blk[14] == c) ++nlines;
		if (blk[15] == c) ++nlines;
		blk += 16;
		size -= 16;
	}
	switch (size) {
	case 15:	if (blk[14] == c) ++nlines; FALLTHROUGH
	case 14:	if (blk[13] == c) ++nlines; FALLTHROUGH
	case 13:	if (blk[12] == c) ++nlines; FALLTHROUGH
	case 12:	if (blk[11] == c) ++nlines; FALLTHROUGH
	case 11:	if (blk[10] == c) ++nlines; FALLTHROUGH
	case 10:	if (blk[9] == c) ++nlines; FALLTHROUGH
	case 9:		if (blk[8] == c) ++nlines; FALLTHROUGH
	case 8:		if (blk[7] == c) ++nlines; FALLTHROUGH
	case 7:		if (blk[6] == c) ++nlines; FALLTHROUGH
	case 6:		if (blk[5] == c) ++nlines; FALLTHROUGH
	case 5:		if (blk[4] == c) ++nlines; FALLTHROUGH
	case 4:		if (blk[3] == c) ++nlines; FALLTHROUGH
	case 3:		if (blk[2] == c) ++nlines; FALLTHROUGH
	case 2:		if (blk[1] == c) ++nlines; FALLTHROUGH
	case 1:		if (blk[0] == c) ++nlines; FALLTHROUGH
	case 0:		/* do nothing */;
	}
	return nlines;
}

#if 0

char *mchr(REGISTER char *blk, char c)
{
    loop:
	if (blk[0] == c) return blk + 0;
	if (blk[1] == c) return blk + 1;
	if (blk[2] == c) return blk + 2;
	if (blk[3] == c) return blk + 3;
	if (blk[4] == c) return blk + 4;
	if (blk[5] == c) return blk + 5;
	if (blk[6] == c) return blk + 6;
	if (blk[7] == c) return blk + 7;
	if (blk[8] == c) return blk + 8;
	if (blk[9] == c) return blk + 9;
	if (blk[10] == c) return blk + 10;
	if (blk[11] == c) return blk + 11;
	if (blk[12] == c) return blk + 12;
	if (blk[13] == c) return blk + 13;
	if (blk[14] == c) return blk + 14;
	if (blk[15] == c) return blk + 15;
	blk += 15;
	goto loop;
}

#endif
