/*
	Zero terminated strings
	Copyright
		(C) 1992 Joseph H. Allen; 
		(C) 2001 Marek 'Marx' Grac

	This file is part of JOE (Joe's Own Editor)
*/ 

#ifndef _JOEzstr
#define _JOEzstr 1

#include "config.h"

/* Character type test macros */

/* Remaining legal characters of a C identifier */
#define isalnum_(c) (isalnum(c) || (c==*"_"))

/* Whitespace: tab, space, newline or nul */
#define isspace_eof(c) (isspace(c) || (!c))

/* Minimum and maximum functions */

unsigned uns_min (unsigned a, unsigned b);
int int_min (int a, int b);
long long_max (long a, long b);
long long_min (long a, long b);

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
