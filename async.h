/* Terminal interface header file
   Copyright (C) 1991 Joseph H. Allen

This file is part of JOE (Joe's Own Editor)

JOE is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software
Foundation; either version 1, or (at your option) any later version.  

JOE is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.  

You should have received a copy of the GNU General Public License
along with JOE; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

int aopen();		/* fflush(stdout) and then set terminal to
			   character at a time mode */
int aclose();		/* Flush output and then restore original tty mode */
int aflush();		/* Flush the output and sleep for the amount of time
			   the output will need to get to the terminal (I.E.,
			   depends on the baud rate).  Then check if there's
			   any typeahead and set 'have' if there is */
int anext();		/* Call aflush() and then return next char from
			   terminal */
int sigjoe();		/* Set signal handling for JOE */
int signorm();		/* Set signal handling back to default */

extern int have;	/* Set if there is typeahead */

int eputs();		/* Write string to terminal */
int eputc();		/* Write character to terminal */
			/* If the output buffer gets full, these call
			   aflush() */

int shell();		/* Shell escape */
int susp();		/* Suspend */

int termtype();             /* Determine the following terminal parameters */

extern int width;       /* Screen width */
extern int height;      /* Screen height */
extern int scroll;      /* Set=use scrolling regions, Clr=don't use them */
extern int record;
extern unsigned char *take;
		/* String to use as input instead of keyboard */

int getsize();		/* Set width and height again with TIOCGSIZE */

/* If termtype does nothing, these values defualt to: width=80, height=24,
   scroll=1 (true).
 
   Note that the only terminal type supported is ANSI/VT100
 */
