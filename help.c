/* Help system
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

#include <string.h>
#include "config.h"
#include "tty.h"
#include "b.h"
#include "termcap.h"
#include "kbd.h"
#include "scrn.h"
#include "w.h"
#include "vs.h"
#include "menu.h"
#include "zstr.h"
#include "tw.h"
#include "blocks.h"
#include "help.h"

/* The loaded help screen */

char *hlptxt=0;
int hlpsiz=0;
int hlpbsz=0;
int hlplns=0;

char **help_names;
struct help **help_structs;
struct help *first_help;

int helpcursor=0;

int get_help(name)
char *name;
 {
 int x;
 for(x=0;help_structs[x];++x) if(!strcmp(help_structs[x]->name,name)) break;
 if(help_structs[x]) return x;
 else return -1;
 }

/* Display help text */

void dsphlp(t)
SCREEN *t;
 {
 char *str=hlptxt;
 int y,x,c;
 int atr=0;
 for(y=skiptop;y!=t->wind;++y)
  {
  if(t->t->updtab[y])
   {
   for(x=0;x!=t->w-1;++x)
    if(*str=='\n' || !*str)
     if(eraeol(t->t,x,y)) return;
     else break;
    else
     {
     if(*str=='\\')
      switch(*++str)
       {
       case 'i': case 'I': atr^=INVERSE; ++str; --x; goto cont;
       case 'u': case 'U': atr^=UNDERLINE; ++str; --x; goto cont;
       case 'd': case 'D': atr^=DIM; ++str; --x; goto cont;
       case 'b': case 'B': atr^=BOLD; ++str; --x; goto cont;
       case 'f': case 'F': atr^=BLINK; ++str; --x; goto cont;
       case 0: --x; goto cont;
       default: c=(unsigned char)*str++;
       }
     else c= (unsigned char)*str++;
     outatr(t->t,t->t->scrn+x+y*t->w,x,y,c,atr);
     cont:;
     }
   atr=0; t->t->updtab[y]=0;
   }
  while(*str && *str!='\n') ++str;
  if(*str=='\n') ++str;
  }
 }

/* Create the help window */

int helpon(t)
SCREEN *t;
 {
 struct help *h=help_structs[helpcursor];
 hlptxt=h->hlptxt;
 hlpsiz=h->hlpsiz;
 hlpbsz=h->hlpbsz;
 hlplns=h->hlplns;
 if(!hlptxt) return -1;
 t->wind=hlplns+skiptop;
 if(t->h-t->wind<FITHEIGHT) t->wind=t->h-FITHEIGHT;
 if(t->wind<0)
  {
  t->wind=skiptop;
  return -1;
  }
 wfit(t);
 msetI(t->t->updtab+skiptop,1,t->wind);
 return 0;
 }

/* Eliminate the help window */

void helpoff(t)
SCREEN *t;
 {
 int z=t->wind;
 t->wind=skiptop;
 wfit(t);
 }

/* Toggle help on/off */

int uhelp(base)
BASE *base;
 {
 W *w=base->parent;
 int h;
 if(w->huh && (h=get_help(w->huh))>-1)
  {
  if(w->t->wind!=skiptop) helpoff(w->t);
  helpcursor=h;
  return helpon(w->t);
  }
 else if(w->t->wind==skiptop)
  return helpon(w->t);
 else
  {
  helpoff(w->t);
  return 0;
  }
 }

/* Goto next/prev help screen */

int uhnext(base)
BASE *base;
 {
 W *w=base->parent;
 if(help_structs[helpcursor+1])
  {
  if(w->t->wind!=skiptop) helpoff(w->t);
  ++helpcursor;
  return helpon(w->t);
  }
 else return -1;
 }

int uhprev(base)
BASE *base;
 {
 W *w=base->parent;
 if(helpcursor)
  {
  if(w->t->wind!=skiptop) helpoff(w->t);
  --helpcursor;
  return helpon(w->t);
  }
 else return -1;
 }
