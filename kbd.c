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

#include "config.h"
#include "macro.h"
#include "termcap.h"
#include "vs.h"
#include "kbd.h"

/* Create a KBD */

KBD *mkkbd(kmap)
KMAP *kmap;
 {
 KBD *kbd=(KBD *)malloc(sizeof(KBD));
 kbd->topmap=kmap;
 kbd->curmap=kmap;
 kbd->x=0;
 return kbd;
 }

/* Eliminate a KBD */

void rmkbd(k)
KBD *k;
 {
 free(k);
 }

/* Process next key for KBD */

void *dokey(kbd,n)
KBD *kbd;
 {
 void *bind=0;

 /* If we were passed a negative character */
 if(n<0) n+=256;

 /* If we're starting from scratch, clear the keymap sequence buffer */
 if(kbd->curmap==kbd->topmap) kbd->x=0;

 if(kbd->curmap->keys[n].k==1)
  { /* A prefix key was found */
  kbd->seq[kbd->x++]=n;
  kbd->curmap=kbd->curmap->keys[n].value.submap;
  }
 else
  { /* A complete key sequence was entered or an unbound key was found */
  bind=kbd->curmap->keys[n].value.bind;
/*  kbd->seq[kbd->x++]=n; */
  kbd->x=0;
  kbd->curmap=kbd->topmap;
  }
 return bind;
 }

/* Return key code for key name or -1 for syntax error */

static int keyval(s)
char *s;
 {
 if(s[0]=='^' && s[1] && !s[2])
  if(s[1]=='?') return 127;
  else return s[1]&0x1F;
 else if((s[0]=='S'||s[0]=='s') && (s[1]=='P'||s[1]=='p') && !s[2]) return ' ';
 else if(s[1] || !s[0]) return -1;
 else return (unsigned char)s[0];
 }

/* Create an empty keymap */

KMAP *mkkmap()
 {
 KMAP *kmap=(KMAP *)calloc(sizeof(KMAP),1);
 return kmap;
 }

/* Eliminate a keymap */

void rmkmap(kmap)
KMAP *kmap;
 {
 int x;
 if(!kmap) return;
 for(x=0;x!=KEYS;++x) if(kmap->keys[x].k==1) rmkmap(kmap->keys[x].value.submap);
 free(kmap);
 }

/* Parse a range */

static char *range(seq,vv,ww)
char *seq;
int *vv, *ww;
 {
 char c;
 int x, v, w;
 for(x=0;seq[x] && seq[x]!=' ';++x);	/* Skip to a space */
 c=seq[x]; seq[x]=0;			/* Zero terminate the string */
 v=keyval(seq);				/* Get key */
 w=v;
 if(w<0) return 0;
 seq[x]=c;				/* Restore the space or 0 */
 for(seq+=x;*seq==' ';++seq);		/* Skip over spaces */

 /* Check for 'TO ' */
 if((seq[0]=='T' || seq[0]=='t') &&
    (seq[1]=='O' || seq[1]=='o') && seq[2]==' ')
  {
  for(seq+=2;*seq==' ';++seq);			/* Skip over spaces */
  for(x=0;seq[x] && seq[x]!=' ';++x);		/* Skip to space */
  c=seq[x]; seq[x]=0;				/* Zero terminate the string */
  w=keyval(seq);				/* Get key */
  if(w<0) return 0;
  seq[x]=c;					/* Restore the space or 0 */
  for(seq+=x;*seq==' ';++seq);			/* Skip over spaces */
  }

 if(v>w) return 0;

 *vv=v; *ww=w;
 return seq;
 }

/* Add a binding to a keymap */

static KMAP *kbuild(cap,kmap,seq,bind,err,capseq,seql)
CAP *cap;
KMAP *kmap;
char *seq;
void *bind;
int *err;
char *capseq;
 {
 int v, w;

 if(!seql && seq[0]=='.' && seq[1])
  {
  int x, c;
  char *s;
  for(x=0;seq[x] && seq[x]!=' ';++x);
  c=seq[x]; seq[x]=0;
#ifdef __MSDOS__
  if(!zcmp(seq+1,"ku")) capseq="\0H", seql=2;
  else if(!zcmp(seq+1,"kd")) capseq="\0P", seql=2;
  else if(!zcmp(seq+1,"kl")) capseq="\0K", seql=2;
  else if(!zcmp(seq+1,"kr")) capseq="\0M", seql=2;
  else if(!zcmp(seq+1,"kI")) capseq="\0R", seql=2;
  else if(!zcmp(seq+1,"kD")) capseq="\0S", seql=2;
  else if(!zcmp(seq+1,"kh")) capseq="\0G", seql=2;
  else if(!zcmp(seq+1,"kH")) capseq="\0O", seql=2;
  else if(!zcmp(seq+1,"kP")) capseq="\0I", seql=2;
  else if(!zcmp(seq+1,"kN")) capseq="\0Q", seql=2;
  else if(!zcmp(seq+1,"k1")) capseq="\0;", seql=2;
  else if(!zcmp(seq+1,"k2")) capseq="\0<", seql=2;
  else if(!zcmp(seq+1,"k3")) capseq="\0=", seql=2;
  else if(!zcmp(seq+1,"k4")) capseq="\0>", seql=2;
  else if(!zcmp(seq+1,"k5")) capseq="\0?", seql=2;
  else if(!zcmp(seq+1,"k6")) capseq="\0@", seql=2;
  else if(!zcmp(seq+1,"k7")) capseq="\0A", seql=2;
  else if(!zcmp(seq+1,"k8")) capseq="\0B", seql=2;
  else if(!zcmp(seq+1,"k9")) capseq="\0C", seql=2;
  else if(!zcmp(seq+1,"k0")) capseq="\0D", seql=2;
  seq[x]=c;
  if(seql)
   {
   for(seq+=x;*seq==' ';++seq);
   }
#else
  s=jgetstr(cap,seq+1);
  seq[x]=c;
  if(s && (s=tcompile(cap,s)) && (sLEN(s)>1 || s[0]<0))
   {
   capseq=s;
   seql=sLEN(s);
   for(seq+=x;*seq==' ';++seq);
   }
#endif
  else
   {
   *err= -2;
   return kmap;
   }
  }

 if(seql)
  {
  v=w= (unsigned char)*capseq++;
  --seql;
  }
 else
  {
  seq=range(seq,&v,&w);
  if(!seq)
   {
   *err= -1;
   return kmap;
   }
  }

 if(!kmap) kmap=mkkmap();	/* Create new keymap if 'kmap' was NULL */

 /* Make bindings between v and w */
 while(v<=w)
  {
  if(*seq || seql)
   {
   if(kmap->keys[v].k==0) kmap->keys[v].value.submap=0;
   kmap->keys[v].k=1;
   kmap->keys[v].value.submap=kbuild(cap,kmap->keys[v].value.bind,seq,bind,err,capseq,seql);
   }
  else
   {
   if(kmap->keys[v].k==1) rmkmap(kmap->keys[v].value.submap);
   kmap->keys[v].k=0;
   kmap->keys[v].value.bind=
    /* This bit of code sticks the key value in the macro */
    (v==w?macstk(bind,v):dupmacro(macstk(bind,v)));
   }
  ++v;
  }
 return kmap;
 }

int kadd(cap,kmap,seq,bind)
CAP *cap;
KMAP *kmap;
char *seq;
void *bind;
 {
 int err=0;
 kbuild(cap,kmap,seq,bind,&err,NULL,0);
 return err;
 }

void kcpy(dest,src)
KMAP *dest, *src;
 {
 int x;
 for(x=0;x!=KEYS;++x)
  if(src->keys[x].k==1)
   {
   if(dest->keys[x].k!=1)
    {
    dest->keys[x].k=1;
    dest->keys[x].value.submap=mkkmap();
    }
   kcpy(dest->keys[x].value.submap,src->keys[x].value.submap);
   }
  else if(src->keys[x].k==0 && src->keys[x].value.bind)
   {
   if(dest->keys[x].k==1) rmkmap(dest->keys[x].value.submap);
   dest->keys[x].value.bind=src->keys[x].value.bind;
   dest->keys[x].k=0;
   }
 }

/* Remove a binding from a keymap */

int kdel(kmap,seq)
KMAP *kmap;
char *seq;
 {
 int err=1;
 int v, w;

 seq=range(seq,&v,&w);
 if(!seq) return -1;

 /* Clear bindings between v and w */
 while(v<=w)
  {
  if(*seq)
   {
   if(kmap->keys[v].k==1)
    {
    int r=kdel(kmap->keys[v].value.submap,seq);
    if(err!= -1) err=r;
    }
   }
  else
   {
   if(kmap->keys[v].k==1) rmkmap(kmap->keys[v].value.submap);
   kmap->keys[v].k=0;
   kmap->keys[v].value.bind=0;
   if(err!= -1) err=0;
   }
  ++v;
  }

 return err;
 }
