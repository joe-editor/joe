/*
	Position history
	Copyright (C) 1992 Joseph H. Allen

	This file is part of JOE (Joe's Own Editor)
*/

#ifndef _Iposhist
#define _Iposhist 1

#include "config.h"
#include "bw.h"
#include "w.h"

void afterpos PARAMS((void));
void aftermove PARAMS((W * w, P * p));
void windie PARAMS((W * w));
int uprevpos PARAMS((BW * bw));
int unextpos PARAMS((BW * bw));

#endif
