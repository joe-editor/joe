/*
	Various utilities
	
	Copyright
		(C) 1992 Joseph H. Allen
		(C) 2001 Marek 'Marx' Grac

	This file is part of JOE (Joe's Own Editor)
*/ 

#ifndef _JOEutils
#define _JOEutils 1

#include <ctype.h>

/* 
 * Characters which are considered as word characters 
 * 	_ is considered as word character because is often used 
 *	in the names of C/C++ functions
 */
#define isalnum_(c) (isalnum(c) || (c==*"_"))

/* 
 * Whitespace characters are characters like tab, space, ...
 *	Every config line in *rc must be end by whitespace but
 *	EOF is not considered as whitespace by isspace()
 *	This is because I don't want to be forced to end 
 *	*rc file with \n
 */
#define isspace_eof(c) (isspace(c) || (!c))

/*
 * Functions which return minimum/maximum of two numbers  
 */
unsigned uns_min (unsigned a, unsigned b);
int int_min (int a, int b);
long long_max (long a, long b);
long long_min (long a, long b);

#endif