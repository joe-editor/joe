/* Terminal interface header file
   Copyright (C) 1991 Joseph H. Allen

This file is part of J (Joe's Editor)

J is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software
Foundation; either version 1, or (at your option) any later version. 

J is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details. 

You should have received a copy of the GNU General Public License
along with GNU Emacs; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

aopen();                /* Set terminal to raw (character at a time) mode */
aclose();               /* Restore terminal to cooked (line at a time) mode */
aflush();               /* Flush the output */
anext();                /* Get next key */

extern int have;	/* Set if there is typeahead */

/* All of the above functions do a fflush(stdout) before doing anything else */

eputs();		/* Write string to terminal */
eputc();		/* Write character to terminal */

shell();		/* Shell escape */
tsignal();              /* E's signal handler.  This function saves the current
                           edit file in .cut.e and then exists some signals
                           might be set up to call this */

termtype();             /* Determine the following terminal parameters */

extern int width;       /* Screen width */
extern int height;      /* Screen height */
extern int scroll;      /* Set=use scrolling regions, Clr=don't use them */

/* If termtype does nothing, these value defualt to: width=80, height=24,
   scroll=1 (true).
 
   Note that the only terminal type supported is ANSI/VT100
 */


/* For Best operation:
 *
 *    acheck  returns true if the user typed ahead
 *
 *    acheck  should sleep (nap) for number*(10000/baud) milliseconds
 *            immediately after its fflush(stdout) call.  Number is
 *            the number of characters the call to fflush(stdout)
 *            wrote.
 *
 *    aopen & aclose should not loose any typeahead when they change modes
 *
 * For example:  see the async*.c files
 */

/* For O.K. operation:
 *
 *    acheck  always returns 0 (false).  If acheck is going to do this, don't
 *            even have it fflush(stdout).
 *
 *    aopen & aclose do system("/bin/stty ...."); so they invariably loose
 *                   any typeahead
 *
 * For example: see the cruddy.c file
 */
