/*
	Various utilities
	Copyright
		(C) 1992 Joseph H. Allen
		(C) 2001 Marek 'Marx' Grac

	This file is part of JOE (Joe's Own Editor)
*/

#include "config.h"
#include "utils.h"

#include <ctype.h>

/*
 * return minimum/maximum of two numbers
 */
unsigned int uns_min (unsigned int a, unsigned int b) {
	return a < b ? a : b;
}

signed int int_min (signed int a, signed int b) {
	return a < b ? a : b;
}

signed long int long_max (signed long int a, signed long int b) {
	return a > b ? a : b;
}

signed long int long_min (signed long int a, signed long int b) {
	return a < b ? a : b;
}

/* 
 * Characters which are considered as word characters 
 * 	_ is considered as word character because is often used 
 *	in the names of C/C++ functions
 */
unsigned int isalnum_ (unsigned int c) {
	return (isalnum (c) || ( c == 95 ));
}

/* 
 * Whitespace characters are characters like tab, space, ...
 *	Every config line in *rc must be end by whitespace but
 *	EOF is not considered as whitespace by isspace()
 *	This is because I don't want to be forced to end 
 *	*rc file with \n
 */
unsigned int isspace_eof (unsigned int c) {
	return (isspace(c) || (!(c)));
}
