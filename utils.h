/*
	Various utilities
	
	Copyright
		(C) 1992 Joseph H. Allen
		(C) 2001 Marek 'Marx' Grac

	This file is part of JOE (Joe's Own Editor)
*/

#ifndef _JOEutils
#define _JOEutils 1

#include "config.h"
#include <ctype.h>

/* 
 * Characters which are considered as word characters 
 * 	_ is considered as word character because is often used 
 *	in the names of C/C++ functions
 */
unsigned int isalnum_ (unsigned int c);

/* 
 * Whitespace characters are characters like tab, space, ...
 *	Every config line in *rc must be end by whitespace but
 *	EOF is not considered as whitespace by isspace()
 *	This is because I don't want to be forced to end 
 *	*rc file with \n
 */
unsigned int isspace_eof (unsigned int c);

/*
 * Define function isblank(c) for non-GNU systems
 */
#ifndef __USE_GNU
#define isblank(c) (((c)==32) || ((c)==9))
#endif

/*
 * Functions which return minimum/maximum of two numbers  
 */
unsigned int uns_min (unsigned int a, unsigned int b);
signed int   int_min (signed int a, int signed b);
signed long long_max (signed long a, signed long b);
signed long long_min (signed long a, signed long b);

#endif
