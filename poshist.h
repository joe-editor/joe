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

void afterpos(void);
void aftermove(W * w, P * p);
void windie(W * w);
int uprevpos(BW * bw);
int unextpos(BW * bw);

#endif
