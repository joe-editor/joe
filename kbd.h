/* Key-map handler
   Copyright (C) 1992 Joseph H. Allen

This file is part of JOE (Joe's Own Editor)

JOE is free software; you can redistribute it and/or modify it under the 
terms of the GNU General Public License as published by the Free Software 
Foundation; either version 1, or (at your option) any later version.  

JOE is distributed in the hope that it will be useful, but WITHOUT ANY 
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more 
details.  

You should have received a copy of the GNU General Public License along with 
JOE; see the file COPYING.  If not, write to the Free Software Foundation, 
675 Mass Ave, Cambridge, MA 02139, USA.  */ 

#ifndef _Ikbd
#define _Ikbd 1

#include "config.h"
#include "scrn.h"

typedef struct kmap KMAP;
typedef struct kbd KBD;
#define KEYS 256

typedef struct key KEY;

/* A key binding */

struct key
 {
 int k;			/* Flag: 0=binding, 1=submap */
 union
  {
  void *bind;		/* What key is bound to */
  KMAP *submap;		/* Sub KMAP address (for prefix keys) */
  } value;
 };

/* A map of keycode to command/sub-map bindings */

struct kmap
 {
 KEY keys[KEYS];	/* KEYs */
 };

/** A keyboard handler **/

struct kbd
 {
 KMAP *curmap;		/* Current keymap */
 KMAP *topmap;		/* Top-level keymap */
 int seq[16];		/* Current sequence of keys */
 int x;			/* What we're up to */
 };

/* KMAP *mkkmap(void);
 * Create an empty keymap
 */
KMAP *mkkmap();

/* void rmkmap(KMAP *kmap);
 * Free a key map
 */
void rmkmap();

/* int kadd(KMAP *kmap,char *seq,void *bind);
 * Add a key sequence binding to a key map
 *
 * Returns 0 for success
 *        -1 for for invalid key sequence
 *
 * A valid key sequence is one or more keys seperated with spaces.  A key
 * is a single character or one of the following strings:
 *
 *    ^?	                   127 (DEL)
 *
 *    ^@   -   ^_                  Control characters
 *
 *    SP                           32 (space character)
 *
 *    UP, DOWN, LEFT, RIGHT,
 *    F0 - F10, DEL, INS, HOME,
 *    END, PGUP, PGDN              termcap special characters
 *
 * In addition, the last key of a key sequence may be replaced with
 * a range-fill of the form: <KEY> TO <KEY>
 *
 * So for example, if the sequence: ^K A TO Z
 * is speicified, then the key sequences
 * ^K A, ^K B, ^K C, ... ^K Z are all bound.
 */
int kadd();

/* void kcpy(KMAP *dest,KMAP *src);
 * Copy all of the entries in the 'src' keymap into the 'dest' keymap
 */
void kcpy();

/* int kdel(KMAP *kmap,char *seq);
 * Delete a binding from a keymap
 *
 * Returns 0 for success
 *        -1 if the given key sequence was invalid
 *         1 if the given key sequence did not exist
 */
int kdel();

/* KBD *mkkbd(KMAP *kmap);
   Create a keyboard handler which uses the given keymap
*/
KBD *mkkbd();

/* void rmkbd(KBD *);
 *
 * Eliminate a keyboard handler
 */
void rmkbd();

/* void *dokey(KBD *kbd,int k);
   Handle a key for a KBD:

     Returns 0 for invalid or prefix keys

     Returns binding for a completed key sequence
*/
void *dokey();

#endif
