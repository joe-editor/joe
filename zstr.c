/*
	Zero terminated strings
	Copyright
		(C) 1992 Joseph H. Allen; 
		(C) 2001 Marek 'Marx' Grac

	This file is part of JOE (Joe's Own Editor)
*/

#include <string.h>
#include "zstr.h"

unsigned uns_min (unsigned a, unsigned b) {
	return a < b ? a : b;
}
int int_min (int a, int b) {
	return a < b ? a : b;
}
long long_max (long a, long b) {
	return a > b ? a : b;
}
long long_min (long a, long b) {
	return a < b ? a : b;
}

int fields (s, fld, sep)
     char *s, **fld, sep;
{
	int y = 1;
	for (fld[0] = s; s = strchr (s, sep); *s = 0, fld[y++] = ++s);
	return y;
}

int nfields (s, sep)
     char *s, sep;
{
	int y = 1;
	while (s = strchr (s, sep))
		++y, ++s;
	return y;
}
