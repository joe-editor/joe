/*
	Zero terminated strings
	Copyright
		(C) 1992 Joseph H. Allen; 
		(C) 2001 Marek 'Marx' Grac

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

/* Whitespace: tab, space, newline or nul */
#define cwhitef(c) (_ctaA[(unsigned char)(c)]&(_whi|_eol|_eos))

/* Minimum and maximum functions */

/* unsigned Umin(unsigned a,unsigned b); Return the smaller unsigned integer */
unsigned Umin();

/* int Imin(int a,int b); Return the lower integer */
int Imin();

/* long Lmax(long a,long b); Return the higher long */
long Lmax();

/* long Lmin(long a,long b); Return the smaller long */
long Lmin();

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
