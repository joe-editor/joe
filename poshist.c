/* Position history
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

#include "poshist.h"
#include "queue.h"
#include "b.h"
#include "w.h"
#include "zstr.h"
#include "bw.h"

typedef struct pos POS;

struct pos
 {
 LINK(POS) link;
 P *p;
 W *w;
 };

POS pos={{&pos,&pos}};
POS frpos={{&frpos,&frpos}};
POS *curpos= &pos;
int npos=0;

void markpos(w,p)
W *w;
P *p;
 {
 POS *new=alitem(&frpos,sizeof(POS));
 new->p=0;
 pdupown(p,&new->p);
 poffline(new->p);
 new->w=w;
 enqueb(POS,link,&pos,new);
 if(npos==20)
  {
  new=pos.link.next;
  prm(new->p);
  demote(POS,link,&frpos,new);
  }
 else ++npos;
 }

void afterpos()
 {
 if(curpos!= &pos)
  {
  demote(POS,link,&pos,curpos);
  curpos= &pos;
  }
 }

void aftermove(w,p)
W *w;
P *p;
 {
 if(pos.link.prev!=&pos &&
    pos.link.prev->w==w &&
    pos.link.prev->p &&
    Labs(pos.link.prev->p->line-p->line)<3
   )
  poffline(pset(pos.link.prev->p,p));
 else markpos(w,p);
 }

void windie(w)
W *w;
 {
 POS *n;
 for(n=pos.link.prev;n!=&pos;n=n->link.prev) if(n->w==w) n->w=0;
 }

int unextpos(bw)
BW *bw;
 {
 W *w=bw->parent;
 lp:
 if(curpos->link.next!=&pos && curpos!=&pos)
  {
  curpos=curpos->link.next;
  if(!curpos->p || !curpos->w) goto lp;
  if(w->t->curwin==curpos->w &&
     curpos->p->byte==((BW *)w->t->curwin->object)->cursor->byte) goto lp;
  if(w->t->curwin!=curpos->w)
   {
   w->t->curwin=curpos->w;
   if(w->t->curwin->y== -1) wfit(w->t);
   }
  w=w->t->curwin;
  bw=(BW *)w->object;
  if(bw->cursor->byte!=curpos->p->byte)
   pset(bw->cursor,curpos->p);
  return 0;
  }
 else return -1;
 }

int uprevpos(bw)
BW *bw;
 {
 W *w=bw->parent;
 lp:
 if(curpos->link.prev!=&pos)
  {
  curpos=curpos->link.prev;
  if(!curpos->p || !curpos->w) goto lp;
  if(w->t->curwin==curpos->w &&
     curpos->p->byte==((BW *)w->t->curwin->object)->cursor->byte) goto lp;
  if(w->t->curwin!=curpos->w)
   {
   w->t->curwin=curpos->w;
   if(w->t->curwin->y== -1) wfit(w->t);
   }
  w=w->t->curwin;
  bw=(BW *)w->object;
  if(bw->cursor->byte!=curpos->p->byte)
   pset(bw->cursor,curpos->p);
  return 0;
  }
 else return -1;
 }
