/*
	Various utilities
	Copyright
		(C) 1992 Joseph H. Allen
		(C) 2001 Marek 'Marx' Grac

	This file is part of JOE (Joe's Own Editor)
*/

#include "utils.h"

/*
 * return minimum/maximum of two numbers
 */
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
