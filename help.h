/*
	Help system
	Copyright
		(C) 1992 Joseph H. Allen
		(C) 2001 Marek 'Marx' Grac

	This file is part of JOE (Joe's Own Editor)
*/

#ifndef _JOEhelp
#define _JOEhelp 1

#include "config.h"				// ???
#include "w.h"					// definitions of BASE & SCREEN

struct help {					// ???
	char *hlptxt;				// ???
	int hlpsiz;				// ???
	int hlpbsz;				// ???
	int hlplns;				// ???
	char *name;				// ???
	struct help *next;			// ???
};

extern char *hlptxt;				// ???
extern int hlpsiz, hlpbsz, hlplns;		// ???
extern char **help_names;			// ???

extern struct help *help_first;			// first screen of help list
extern struct help **help_structs;		// array of help screens

void help_display (SCREEN *t);			// display text in help window
void help_to_array (void);			// transform help list into array
int help_on (SCREEN *t);			// turn help on

int u_help (BASE *base);			// toggle help on/off
int u_help_next (BASE *base);			// goto next help screen
int u_help_prev (BASE *base);			// goto prev help screen



#endif
