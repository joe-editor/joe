/*
	Zero terminated strings
	Copyright
		(C) 1992 Joseph H. Allen; 
		(C) 2001 Marek 'Marx' Grac

	This file is part of JOE (Joe's Own Editor)
*/ 

#include <ctype.h>
#include <string.h>
#include "zstr.h"

char _ctaB[]=
 {
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,_flo,0,_bin,_bin,_oct,_oct,_oct,_oct,_oct,_oct,_dec,_dec,0,0,0,0,0,0,
 0,_upp|_hex,_upp|_hex,_upp|_hex,_upp|_hex,_upp|_hex|_flo,_upp|_hex,_upp,_upp,_upp,_upp,_upp,_upp,_upp,_upp,_upp,_upp,_upp,_upp,_upp,_upp,_upp,_upp,_upp,_upp,_upp,_upp,0,0,0,0,_und,
 0,_low|_hex,_low+_hex,_low|_hex,_low|_hex,_low|_hex|_flo,_low|_hex,_low,_low,_low,_low,_low,_low,_low,_low,_low,_low,_low,_low,_low,_low,_low,_low,_low,_low,_low,_low,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
 };

char _ctaA[]=
 {
 _eos,0,0,0,0,0,0,0,0,_whi,_eol,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 _whi,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
 };

unsigned Umin(a,b) unsigned a,b; { return a<b?a:b; }
int Imin(a,b) { return a<b?a:b; }
long Lmax(a,b) long a,b; { return a>b?a:b; }
long Lmin(a,b) long a,b; { return a<b?a:b; }

int fields(s,fld,sep)
char *s, **fld, sep;
 {
 int y=1;
 for(fld[0]=s;s=strchr(s,sep);*s=0, fld[y++]= ++s);
 return y;
 }

int nfields(s,sep)
char *s, sep;
 {
 int y=1;
 while(s=strchr(s,sep)) ++y, ++s;
 return y;
 }
