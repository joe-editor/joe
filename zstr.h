/*
	Zero terminated strings
	Copyright (C) 1992 Joseph H. Allen

	This file is part of JOE (Joe's Own Editor)
*/ 

#ifndef _Izstr
#define _Izstr 1

#include "config.h"

#define _upp 1
#define _low 2
#define _und 4
#define _bin 8
#define _oct 16
#define _dec 32
#define _hex 64
#define _flo 128

#define _whi 1
#define _eol 2
#define _eos 4

extern char _ctaB[], _ctaA[];

/* Character type test macros */

/* Remaining legal characters of a C identifier */
#define crest(c)  (_ctaB[(unsigned char)(c)]&(_low|_upp|_und|_bin|_oct|_dec))

/* Whitespace: tab or space */
#define cwhite(c)  (_ctaA[(unsigned char)(c)]&(_whi))

/* Whitespace: tab, space or newline */
#define cwhitel(c) (_ctaA[(unsigned char)(c)]&(_whi|_eol))

/* Whitespace: tab, space, newline or nul */
#define cwhitef(c) (_ctaA[(unsigned char)(c)]&(_whi|_eol|_eos))

/*************/
/* unsigned Umin(unsigned a,unsigned b); Return the smaller unsigned integer */
unsigned Umin();

/* unsigned Umax(unsigned a,unsigned b); Return the larger unsigned integer */
unsigned Umax();

/* int Imin(int a,int b); Return the lower integer */
int Imin();

/* int Imax(int a,int b); Return the higher integer */
int Imax();

/* long Lmax(long a,long b); Return the higher long */
long Lmax();

/* long Lmin(long a,long b); Return the smaller long */
long Lmin();
/*****************/

/* int zlen(char *s); Return length of z-string */
int zlen();

/* char *zchr(char *s,char c); Return address of first 'c' in 's', or NULL if
 * the end of 's' was found first */
char *zchr();

/* char *zrchr(char *s,char c); Return address of last 'c' in 's', or NULL if
 * there are none.
 */
char *zrchr();

/* char *zcpy(char *d,char *s); Copy z-string at s to d */
char *zcpy();

/* char *zcat(char *d,char *s); Append s onto d */
char *zcat();

/* char *zdup(char *s); Duplicate z-string into an malloc block */
char *zdup();

/* int zcmp(char *l,char *r); Compare the strings.  Return -1 if l is
 * less than r; return 0 if l equals r; and return 1 if l is greater than r.
 */
int zcmp();

/* int zicmp(char *l,char *r); Compare the strings, case insensitive.  Return
 * -1 if l is less than r; return 0 if l equals r; and return 1 if l is greater
 * than r.
 */
int zicmp();

/* int fields(char *s,char **fields,char sep); Break up z-string containing
 * fields into its componant fields.  This is done by setting the field
 * seperator characters to zero- thereby changing the fields into z-strings,
 * and by storing the starting address of each field in the array 'fields'
 * (which must be large enough to store the field pointers).
 *
 * The number of fields which were found in s is returned.
 */
int fields();

/* int nfields(char *s,char sep); Assuming 'sep' is the field seperator
 * character, count the number of fields in z-string s.  If no 'sep'
 * characters are in 's', the number of fields is 1.
 */
int nfields();

#endif
