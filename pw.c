/* Prompt windows
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
#include "w.h"
#include "tw.h"
#include "vfile.h"
#include "termcap.h"
#include "b.h"
#include "kbd.h"
#include "scrn.h"
#include "bw.h"
#include "zstr.h"
#include "help.h"
#include "tab.h"
#include "undo.h"
#include "uedit.h"
#include "pw.h"

static void disppw(bw,flg)
BW *bw;
 {
 W *w=bw->parent;
 PW *pw=(PW *)bw->object;
 if(!flg) return;
 /* Scroll buffer and position prompt */
 if(pw->promptlen>w->w/2+w->w/4)
  {
  pw->promptofst=pw->promptlen-w->w/2;
  if(piscol(bw->cursor)<w->w-(pw->promptlen-pw->promptofst))
   bw->offset=0;
  else
   bw->offset=piscol(bw->cursor)-(w->w-(pw->promptlen-pw->promptofst)-1);
  }
 else
  { 
  if(piscol(bw->cursor)<w->w-pw->promptlen) pw->promptofst=0, bw->offset=0;
  else if(piscol(bw->cursor)>=w->w)
   pw->promptofst=pw->promptlen, bw->offset=piscol(bw->cursor)-(w->w-1);
  else
   pw->promptofst=pw->promptlen-(w->w-piscol(bw->cursor)-1),
   bw->offset=piscol(bw->cursor)-(w->w-(pw->promptlen-pw->promptofst)-1);
  }

 /* Set cursor position */
 w->curx=piscol(bw->cursor)-bw->offset+pw->promptlen-pw->promptofst;
 w->cury=0;

 /* Generate prompt */
 w->t->t->updtab[w->y]=1;
 genfmt(w->t->t,w->x,w->y,pw->promptofst,pw->prompt,0);

 /* Position and size buffer */
 bwmove(bw,w->x+pw->promptlen-pw->promptofst,w->y);
 bwresz(bw,w->w-(pw->promptlen-pw->promptofst),1);

 /* Generate buffer */
 bwgen(bw,0);
 }

/* When user hits return in a prompt window */

extern int dostaupd;

int rtnpw(bw)
BW *bw;
 {
 W *w=bw->parent;
 PW *pw=(PW *)bw->object;
 char *s;
 W *win;
 int *notify;
 int (*pfunc)();
 void *object;
 long byte;
 peol(bw->cursor);
 byte=bw->cursor->byte;
 pbol(bw->cursor);
 s=brvs(bw->cursor,(int)(byte-bw->cursor->byte));
 if(pw->hist)
  if(bw->b->changed)
   {
   P *q=pdup(pw->hist->eof);
   binsm(q,s,(int)(byte-bw->cursor->byte));
   peof(q);
   binsc(q,'\n');
   prm(q);
   }
  else
   {
   P *q=pdup(pw->hist->bof);
   P *r;
   P *t;
   pline(q,bw->cursor->line);
   r=pdup(q);
   pnextl(r);
   t=pdup(pw->hist->eof);
   binsb(t,bcpy(q,r));
   bdel(q,r);
   prm(q); prm(r); prm(t);
   }
 win=w->win;
 pfunc=pw->pfunc;
 object=pw->object;
 bwrm(bw);
 free(pw->prompt);
 free(pw);
 w->object=0;
 notify=w->notify;
 w->notify=0;
 wabort(w);
 dostaupd=1;
 if(pfunc) return pfunc(win->object,s,object,notify);
 else return -1;
 }

int ucmplt(bw,k)
BW *bw;
 {
 PW *pw=(PW *)bw->object;
 if(pw->tab) return pw->tab(bw,k);
 else return -1;
 }

static void inspw(bw,b,l,n,flg)
BW *bw;
B *b;
long l,n;
int flg;
 {
 if(b==bw->b) bwins(bw,l,n,flg);
 }

static void delpw(bw,b,l,n,flg)
BW *bw;
B *b;
long l,n;
int flg;
 {
 if(b==bw->b) bwdel(bw,l,n,flg);
 }

static int abortpw(b)
BW *b;
 {
 PW *pw=b->object;
 void *object=pw->object;
 int (*abrt)()=pw->abrt;
 W *win=b->parent->win;
 bwrm(b);
 free(pw->prompt);
 free(pw);
 if(abrt) return abrt(win->object,object);
 else return -1;
 }

static WATOM watompw=
 {
 "prompt",
 disppw,
 bwfllw,
 abortpw,
 rtnpw,
 utypebw,
 0,
 0,
 inspw,
 delpw,
 TYPEPW
 };

/* Create a prompt window */

BW *wmkpw(obw,prompt,history,func,huh,abrt,tab,object,notify)
BASE *obw;
char *prompt;
B **history;
int (*func)();
char *huh;
int (*abrt)();
int (*tab)();
void *object;
int *notify;
 {
 W *new;
 PW *pw;
 BW *bw;
 W *w=obw->parent;
 new=wcreate(w->t,&watompw,w,w,w->main,1,huh,notify);
 if(!new)
  {
  if(notify) *notify=1;
  return 0;
  }
 wfit(new->t);
 new->object=(void *)(bw=bwmk(new,bmk(NULL),1));
 bw->object=(void *)(pw=(PW *)malloc(sizeof(PW)));
 pw->abrt=abrt;
 pw->tab=tab;
 pw->object=object;
 pw->prompt=zdup(prompt);
 pw->promptlen=fmtlen(prompt);
 pw->promptofst=0;
 pw->pfunc=func;
 if(history)
  {
  if(!*history) *history=bmk(NULL);
  pw->hist= *history;
  binsb(bw->cursor,bcpy(pw->hist->bof,pw->hist->eof));
  bw->b->changed=0;
  peof(bw->cursor); peof(bw->top); pbol(bw->top);
  }
 else pw->hist=0;
 w->t->curwin=new;
 return bw;
 }
