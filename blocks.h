/*
	Fast block move/copy subroutines
	Copyright (C) 1992 Joseph H. Allen

	This file is part of JOE (Joe's Own Editor)
*/

#ifndef _Iblocks
#define _Iblocks 1

#include "config.h"

/* Warning: Don't use mfwrd or mbkwd for filling an area.  Use them only
 * for copying, inserting or deleting.  Since they copy ints at a time, they
 * will not work for filling.
 */

/* char *mfwrd(char *d,char *s,int sz); Copy 'sz' bytes from 's' to 'd'.
   The copy occures in the forward direction (use for deletes)
   Nothing happens if 'd'=='s' or 'sz'==0
   Returns original value of 'd'
 */
char *mfwrd PARAMS((register char *d, register char *s, register int sz));

/* char *mbkwd(char *d,char *s,int sz); Copy 'sz' bytes from 's' to 'd'.
   The copy occures in the backward direction (use for inserts)
   Nothing happens if 'd'=='s' or 'sz'==0
   Returns original value of 'd'
 */
char *mbkwd PARAMS((register char *d, register char *s, register int sz));

/* char *mmove(char *d,char *s,int sz); Copy 'sz' bytes from 's' to 'd'.
 * Chooses either mbkwd or mfwrd to do this such that the data won't get
 * clobbered.
 */
void *mmove PARAMS((void *d, void *s, int sz));

/* Use this for non-overlapping copies */
#define mcpy mbkwd

/* char *mset(char *d,char c,int sz); Set 'sz' bytes at 'd' to 'c'.
 * If 'sz'==0 nothing happens
 * Return original value of 'd'
 */
char *mset PARAMS((register void *dest, register unsigned char c, register int sz));

/* int *msetI(int *d,int c,int sz); Set 'sz' ints at 'd' to 'c'.
 * If 'sz'==0 nothing happens
 * Returns orininal value of 'd'
 */
int *msetI PARAMS((void *dest, int c, int sz));

/* int mcnt(char *blk,char c,int size);
 *
 * Count the number of occurances a character appears in a block
 */
int mcnt PARAMS((register char *blk, register char c, int size));

#ifdef junk
/* char *mchr(char *s,char c);
 *
 * Return address of first 'c' following 's'.
 */
char *mchr PARAMS(());
#endif
#endif
