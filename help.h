/*
 *	Help system
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *		(C) 2001 Marek 'Marx' Grac
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_HELP_H
#define _JOE_HELP_H 1

#include "config.h"
#include "types.h"

#include "w.h"				/* definitions of BASE & SCREEN */

void help_display PARAMS((SCREEN *t));		/* display text in help window */
int help_on PARAMS((SCREEN *t));		/* turn help on */
int help_init PARAMS((char *filename));		/* load help file */

int u_help PARAMS((BASE *base));		/* toggle help on/off */
int u_help_next PARAMS((BASE *base));		/* goto next help screen */
int u_help_prev PARAMS((BASE *base));		/* goto prev help screen */

#endif
