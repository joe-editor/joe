/* Highlighted block functions
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
#include "b.h"
#include "bw.h"
#include "scrn.h"
#include "w.h"
#include "pw.h"
#include "qw.h"
#include "uedit.h"
#include "zstr.h"
#include "vs.h"
#include "ublock.h"

/* Global options */

int square=0;				/* Set for rectangle mode */
int lightoff=0;				/* Set if highlighting should turn off
					   after block operations */
extern int marking;

/* Global variables */

P *markb=0;				/* Beginning and end of block */
P *markk=0;

/* Push markb & markk */

typedef struct marksav MARKSAV;
struct marksav
 {
 LINK(MARKSAV) link;
 P *markb, *markk;
 } markstack={{&markstack,&markstack}};
MARKSAV markfree={{&markfree,&markfree}};
int nstack=0;

int upsh(bw) 
BW *bw;
 {
 MARKSAV *m=alitem(&markfree,sizeof(MARKSAV));
 m->markb=0; m->markk=0;
 if(markk) pdupown(markk,&m->markk);
 if(markb) pdupown(markb,&m->markb);
 enqueb(MARKSAV,link,&markstack,m);
 ++nstack;
 return 0;
 }

int upop(bw)
BW *bw;
 {
 MARKSAV *m=markstack.link.prev;
 if(m!=&markstack)
  {
  --nstack;
  prm(markk);
  prm(markb);
  markk=m->markk; if(markk) markk->owner= &markk;
  markb=m->markb; if(markb) markb->owner= &markb;
  demote(MARKSAV,link,&markfree,m);
  if(lightoff) unmark(bw);
  updall();
  return 0;
  }
 else return -1;
 }

/* Return true if markb/markk are valid */

int markv(r)
 {
 if(markb && markk && markb->b==markk->b && markk->byte>markb->byte &&
    (!square || markk->xcol>markb->xcol)) return 1;
 else if(r && markb && markk && markb->b==markk->b &&
         markk->byte<markb->byte && (!square || markk->xcol<markb->xcol))
  {
  P *t=markb;
  markb=markk;
  markk=t;
  markb->owner= &markb;
  markk->owner= &markk;
  return 1;
  }
 else return 0;
 }

/* Rectangle-mode subroutines */

/* B *pextrect(P *org,long height,long left,long right);
 * Copy a rectangle into a new buffer
 *
 * org points to top-left corner of rectangle.
 * height is number of lines in rectangle.
 * right is rightmost column of rectangle + 1
 */

B *pextrect(org,height,right)
P *org;
long height,right;
 {
 P *p=pdup(org);	/* Left part of text to extract */
 P *q=pdup(p);		/* After right part of text to extract */
 B *tmp=bmk(NULL);	/* Buffer to extract to */
 P *z=pdup(tmp->eof);	/* Buffer pointer */
 while(height--)
  {
  pcol(p,org->xcol);
  pset(q,p);
  pcolwse(q,right); 
  peof(z); binsb(z,bcpy(p,q));
  peof(z); binsc(z,'\n');
  pnextl(p);
  }
 prm(p); prm(q); prm(z);
 return tmp;
 }

/* void pdelrect(P *org,long height,long right);
 * Delete a rectangle.
 */

void pdelrect(org,height,right)
P *org;
long height,right;
 {
 P *p=pdup(org);
 P *q=pdup(p);
 while(height--)
  {
  pcol(p,org->xcol);
  pset(q,p);
  pcol(q,right);
  bdel(p,q);
  pnextl(p);
  }
 prm(p); prm(q);
 }

/* void pclrrect(P *org,long height,long right,int usetabs);
 * Blank-out a rectangle.
 */

void pclrrect(org,height,right,usetabs)
P *org;
long height,right;
 {
 P *p=pdup(org);
 P *q=pdup(p);
 while(height--)
  {
  long pos;
  pcol(p,org->xcol);
  pset(q,p);
  pcoli(q,right);
  pos=q->col;
  bdel(p,q);
  pfill(p,pos,usetabs);
  pnextl(p);
  }
 prm(p); prm(q);
 }

/* int ptabrect(P *org,long height,long right)
 * Check if there are any TABs in a rectange
 */

int ptabrect(org,height,right)
P *org;
long height,right;
 {
 P *p=pdup(org);
 while(height--)
  {
  int c;
  pcol(p,org->xcol);
  while(c=pgetc(p), c!=MAXINT && c!='\n')
   if(c=='\t')
    {
    prm(p); return 1;
    }
   else if(piscol(p)>right) break;
  if(c!='\n') pnextl(p);
  }
 prm(p); return 0;
 }

/* Insert rectangle */

void pinsrect(cur,tmp,width,usetabs)
P *cur;
B *tmp;
long width;
 {
 P *p=pdup(cur);		/* We insert at & move this pointer */
 P *q=pdup(tmp->bof);		/* These are for scanning through 'tmp' */
 P *r=pdup(q);
 if(width)
  while(pset(r,q), peol(q), (q->line!=tmp->eof->line || piscol(q)))
   {
   pcol(p,cur->xcol);
   if(piscol(p)<cur->xcol) pfill(p,cur->xcol,usetabs);
   binsb(p,bcpy(r,q)); pfwrd(p,q->byte-r->byte);
   if(piscol(p)<cur->xcol+width) pfill(p,cur->xcol+width,usetabs);
   if(piseol(p)) pbackws(p);
   if(!pnextl(p)) binsc(p,'\n'), pgetc(p);
   if(pgetc(q)==MAXINT) break;
   }
 prm(p); prm(q); prm(r);
 }

/* Block functions */

/* Set beginning */

int umarkb(bw)
BW *bw;
 {
 pdupown(bw->cursor,&markb);
 markb->xcol=bw->cursor->xcol;
 updall();
 return 0;
 }

int udrop(bw)
BW *bw;
 {
 prm(markk);
 if(marking && markb)
  prm(markb);
 else
  umarkb(bw);
 return 0;
 }

/* Set end */

int umarkk(bw)
BW *bw;
 {
 pdupown(bw->cursor,&markk);
 markk->xcol=bw->cursor->xcol;
 updall();
 return 0;
 }

/* Unset marks */

int unmark(bw)
BW *bw;
 {
 prm(markb);
 prm(markk);
 updall();
 return 0;
 }

/* Mark line */

int umarkl(bw)
BW *bw;
 {
 pbol(bw->cursor);
 umarkb(bw);
 pnextl(bw->cursor);
 umarkk(bw);
 utomarkb(bw);
 pcol(bw->cursor,bw->cursor->xcol);
 return 0;
 }

int utomarkb(bw)
BW *bw;
 {
 if(markb && markb->b==bw->b) { pset(bw->cursor,markb); return 0; }
 else return -1;
 }

int utomarkk(bw)
BW *bw;
 {
 if(markk && markk->b==bw->b) { pset(bw->cursor,markk); return 0; }
 else return -1;
 }

int uswap(bw)
BW *bw;
 {
 if(markb && markb->b==bw->b)
  {
  P *q=pdup(markb);
  umarkb(bw);
  pset(bw->cursor,q); prm(q);
  return 0;
  }
 else return -1;
 }

int utomarkbk(bw)
BW *bw;
 {
 if(markb && markb->b==bw->b && bw->cursor->byte!=markb->byte)
  { pset(bw->cursor,markb); return 0; }
 else if(markk && markk->b==bw->b && bw->cursor->byte!=markk->byte)
  { pset(bw->cursor,markk); return 0; }
 else return -1;
 }

/* Delete block */

int ublkdel(bw)
BW *bw;
 {
 if(markv(1))
  {
  if(square)
   if(bw->o.overtype)
    {
    long ocol=markk->xcol;
    pclrrect(markb,markk->line-markb->line+1,markk->xcol,
             ptabrect(markb,markk->line-markb->line+1,markk->xcol));
    pcol(markk,ocol); markk->xcol=ocol;
    }
   else
    pdelrect(markb,markk->line-markb->line+1,markk->xcol);
  else
   bdel(markb,markk);
  if(lightoff) unmark(bw);
  }
 else { msgnw(bw,"No block"); return -1; }
 return 0;
 }

/* Move highlighted block */

int ublkmove(bw)
BW *bw;
 {
 if(markv(1))
  {
  if(markb->b->rdonly) { msgnw(bw,"Read only"); return -1; }
  if(square)
   {
   long height=markk->line-markb->line+1;
   long width=markk->xcol-markb->xcol;
   int usetabs=ptabrect(markb,height,markk->xcol);
   long ocol=piscol(bw->cursor);
   B *tmp=pextrect(markb,height,markk->xcol);
   ublkdel(bw);
   if(bw->o.overtype)
    {
    /* If cursor was in block, blkdel moves it to left edge of block, so fix it
     * back to its original place here */
    pcol(bw->cursor,ocol);
    pfill(bw->cursor,ocol,0);
    pdelrect(bw->cursor,height,piscol(bw->cursor)+width);
    }
   else if(bw->cursor->xcol>=markk->xcol &&
           bw->cursor->line>=markb->line &&
           bw->cursor->line<=markk->line)
    /* If cursor was to right of block, xcol was not properly updated */
    bw->cursor->xcol-=width;
   pinsrect(bw->cursor,tmp,width,usetabs);
   brm(tmp);
   if(lightoff) unmark(bw);
   else
    {
    umarkb(bw);
    umarkk(bw);
    pline(markk,markk->line+height-1);
    pcol(markk,markb->xcol+width); markk->xcol=markb->xcol+width;
    }
   return 0;
   }
  else if(bw->cursor->b!=markk->b ||
          bw->cursor->byte>markk->byte ||
          bw->cursor->byte<markb->byte)
   {
   long size=markk->byte-markb->byte;
   binsb(bw->cursor,bcpy(markb,markk));
   bdel(markb,markk);
   if(lightoff) unmark(bw);
   else
    {
    umarkb(bw);
    umarkk(bw);
    pfwrd(markk,size);
    }
   updall();
   return 0;
   }
  }
 msgnw(bw,"No block");
 return -1;
 }

/* Duplicate highlighted block */

int ublkcpy(bw)
BW *bw;
 {
 if(markv(1))
  if(square)
   {
   long height=markk->line-markb->line+1;
   long width=markk->xcol-markb->xcol;
   int usetabs=ptabrect(markb,height,markk->xcol);
   B *tmp=pextrect(markb,height,markk->xcol);
   if(bw->o.overtype) pdelrect(bw->cursor,height,piscol(bw->cursor)+width);
   pinsrect(bw->cursor,tmp,width,usetabs);
   brm(tmp);
   if(lightoff) unmark(bw);
   else
    {
    umarkb(bw);
    umarkk(bw);
    pline(markk,markk->line+height-1);
    pcol(markk,markb->xcol+width); markk->xcol=markb->xcol+width;
    }
   return 0;
   }
  else
   {
   long size=markk->byte-markb->byte;
   binsb(bw->cursor,bcpy(markb,markk));
   if(lightoff) unmark(bw);
   else
    {
    umarkb(bw);
    umarkk(bw);
    pfwrd(markk,size);
    }
   updall();
   return 0;
   }
 else { msgnw(bw,"No block"); return -1; }
 }

/* Write highlighted block to a file */
/* This is called by ublksave in ufile.c */

int dowrite(bw,s,object,notify)
BW *bw;
char *s;
void *object;
int *notify;
 {
 if(notify) *notify=1;
 if(markv(1))
  if(square)
   {
   int fl;
   int ret=0;
   B *tmp=pextrect(markb,markk->line-markb->line+1,markk->xcol);
   if(fl=bsave(tmp->bof,s,tmp->eof->byte)) msgnw(bw,msgs[5+fl]), ret= -1;
   brm(tmp);
   if(lightoff) unmark(bw);
   vsrm(s);
   return ret;
   }
  else
   {
   int fl;
   int ret=0;
   if(fl=bsave(markb,s,markk->byte-markb->byte)) msgnw(bw,msgs[5+fl]), ret= -1;
   if(lightoff) unmark(bw);
   vsrm(s);
   return ret;
   }
 else
  {
  vsrm(s);
  msgnw(bw,"No block");
  return -1;
  }
 }

/* Set highlighted block on a program block */

void setindent(bw)
BW *bw;
 {
 P *p, *q;
 long indent;
 if(pisblank(bw->cursor)) return;
 
 p=pdup(bw->cursor);
 q=pdup(p);
 indent=pisindent(p);
 
 do
  if(!pprevl(p)) goto done;
  else pbol(p);
  while(pisindent(p)>=indent && !pisblank(p));
 pnextl(p);
 done:
 pbol(p);
 p->xcol=piscol(p);
 if(markb) prm(markb);
 markb=p; p->owner= &markb;
 
 do
  if(!pnextl(q)) break;
  while(pisindent(q)>=indent && !pisblank(q));
 
 if(markk) prm(markk);
 q->xcol=piscol(q);
 markk=q; q->owner= &markk;
 
 updall();
 }

/* Indent more */

int urindent(bw)
BW *bw;
 {
 if(square)
  {
  if(markb && markk && markb->b==markk->b &&
     markb->byte<=markk->byte && markb->xcol<=markk->xcol)
   {
   P *p=pdup(markb);
   do
    {
    pcol(p,markb->xcol);
    pfill(p,markb->xcol+bw->o.istep,bw->o.indentc=='\t'?1:0);
    } while(pnextl(p) && p->line<=markk->line);
   prm(p);
   }
  }
 else
  {
  if(!markb || !markk || markb->b!=markk->b ||
     bw->cursor->byte<markb->byte || bw->cursor->byte>markk->byte ||
     markb->byte==markk->byte)
   setindent(bw);
  else
   {
   P *p=pdup(markb);
   while(p->byte<markk->byte)
    {
    pbol(p);
    if(!piseol(p)) while(piscol(p)<bw->o.istep) binsc(p,bw->o.indentc), pgetc(p);
    pnextl(p);
    }
   prm(p);
   }
  }
 return 0;
 }

/* Indent less */

int ulindent(bw)
BW *bw;
 {
 if(square)
  {
  if(markb && markk && markb->b==markk->b &&
     markb->byte<=markk->byte && markb->xcol<=markk->xcol)
   {
   P *p=pdup(markb);
   P *q=pdup(p);
   do
    {
    pcol(p,markb->xcol);
    while(piscol(p)<markb->xcol+bw->o.istep)
     {
     int c=pgetc(p);
     if(c!=' ' && c!='\t' && c!=bw->o.indentc)
      {
      prm(p);
      prm(q);
      return -1;
      }
     }
    } while(pnextl(p) && p->line<=markk->line);
   pset(p,markb);
   do
    {
    pcol(p,markb->xcol);
    pset(q,p);
    pcol(q,markb->xcol+bw->o.istep);
    bdel(p,q);
    } while(pnextl(p) && p->line<=markk->line);
   prm(p); prm(q);
   }
  }
 else
  {
  if(!markb || !markk || markb->b!=markk->b ||
     bw->cursor->byte<markb->byte || bw->cursor->byte>markk->byte ||
     markb->byte==markk->byte)
   setindent(bw);
  else
   {
   P *p=pdup(markb);
   P *q=pdup(p);
   pbol(p);
   while(p->byte<markk->byte)
    {
    if(!piseol(p)) while(piscol(p)<bw->o.istep)
     {
     int c=pgetc(p);
     if(c!=' ' && c!='\t' && c!=bw->o.indentc)
      {
      prm(p);
      prm(q);
      return -1;
      }
     }
    pnextl(p);
    }
   pset(p,markb);
   pbol(p);
   while(p->byte<markk->byte)
    {
    if(!piseol(p))
     {
     pset(q,p);
     while(piscol(q)<bw->o.istep) pgetc(q);
     bdel(p,q);
     }
    pnextl(p);
    }
   prm(p); prm(q);
   }
  }
 return 0;
 }

/* Insert a file */

int doinsf(bw,s,object,notify)
BW *bw;
char *s;
void *object;
int *notify;
 {
 if(notify) *notify=1;
 if(square)
  if(markv(1))
   {
   B *tmp;
   long width=markk->xcol-markb->xcol;
   long height;
   int usetabs=ptabrect(markb,markk->line-markb->line+1,markk->xcol);
   tmp=bload(s); 
   if(error)
    {
    msgnw(bw,msgs[error+5]);
    brm(tmp);
    return -1;
    }
   if(piscol(tmp->eof)) height=tmp->eof->line+1;
   else height=tmp->eof->line;
   if(bw->o.overtype)
    {
    pclrrect(markb,Lmax(markk->line-markb->line+1,height),markk->xcol,usetabs);
    pdelrect(markb,height,width+markb->xcol);
    }
   pinsrect(markb,tmp,width,usetabs);
   pdupown(markb,&markk);
   markk->xcol=markb->xcol;
   if(height)
    {
    pline(markk,markk->line+height-1);
    pcol(markk,markb->xcol+width); markk->xcol=markb->xcol+width;
    }
   brm(tmp);
   updall();
   return 0;
   }
  else { msgnw(bw,"No block"); return -1; }
 else
  {
  int ret=0;
  B *tmp=bload(s);
  if(error) msgnw(bw,msgs[error+5]), brm(tmp), ret= -1;
  else binsb(bw->cursor,tmp);
  vsrm(s);
  bw->cursor->xcol=piscol(bw->cursor);
  return ret;
  }
 }


/* Filter highlighted block through a UNIX command */

static int filtflg=0;

static int dofilt(bw,s,object,notify)
BW *bw;
char *s;
void *object;
int *notify;
 {
 int fr[2];
 int fw[2];
 if(notify) *notify=1; 
 if(markb && markk && !square &&
    markb->b==bw->b && markk->b==bw->b && markb->byte==markk->byte) goto ok;
 if(!markv(1))
  {
  msgnw(bw,"No block");
  return -1;
  }
 ok:
 
 pipe(fr);
 pipe(fw);
 npartial(bw->parent->t->t);
 ttclsn();
 if(!fork())
  {
  signrm();
  close(0);
  close(1);
  dup(fw[0]);
  dup(fr[1]);
  close(fw[0]);
  close(fr[1]);
  close(fw[1]);
  close(fr[0]);
  execl("/bin/sh","/bin/sh","-c",s,NULL);
  _exit(0);
  }
 close(fr[1]);
 close(fw[0]);
 if(fork())
  {
  long szz;
  close(fw[1]);
  if(square)
   {
   B *tmp;
   long width=markk->xcol-markb->xcol;
   long height;
   int usetabs=ptabrect(markb,markk->line-markb->line+1,markk->xcol);
   tmp=bread(fr[0],MAXLONG);
   if(piscol(tmp->eof)) height=tmp->eof->line+1;
   else height=tmp->eof->line;
   if(bw->o.overtype)
    {
    pclrrect(markb,markk->line-markb->line+1,markk->xcol,usetabs);
    pdelrect(markb,Lmax(height,markk->line-markb->line+1),width+markb->xcol);
    }
   else pdelrect(markb,markk->line-markb->line+1,markk->xcol);
   pinsrect(markb,tmp,width,usetabs);
   pdupown(markb,&markk);
   markk->xcol=markb->xcol;
   if(height)
    {
    pline(markk,markk->line+height-1);
    pcol(markk,markb->xcol+width); markk->xcol=markb->xcol+width;
    }
   if(lightoff) unmark(bw);
   brm(tmp);
   updall();
   }
  else
   {
   bdel(markb,markk);
   szz=markk->b->eof->byte;
   binsb(markk,bread(fr[0],MAXLONG));
   pfwrd(markk,markk->b->eof->byte-szz);
   if(lightoff) unmark(bw);
   }
  close(fr[0]);
  wait(0);
  wait(0);
  }
 else
  {
  if(square)
   {
   B *tmp=pextrect(markb,markk->line-markb->line+1,markk->xcol);
   bsavefd(tmp->bof,fw[1],tmp->eof->byte);
   }
  else bsavefd(markb,fw[1],markk->byte-markb->byte);
  close(fw[1]);
  _exit(0);
  }
 vsrm(s);
 ttopnn();
 if(filtflg) umarkb(bw), umarkk(bw);
 bw->cursor->xcol=piscol(bw->cursor);
 return 0;
 }

static B *filthist=0;

void markall(bw)
BW *bw;
 {
 pdupown(bw->cursor->b->bof,&markb); markb->xcol=0;
 pdupown(bw->cursor->b->eof,&markk); markk->xcol=piscol(markk);
 updall();
 }

int checkmark(bw)
BW *bw;
 {
 if(!markv(1))
  if(square) return 2;
  else
   {
   markall(bw), filtflg=1;
   return 1;
   }
 else
  {
  filtflg=0;
  return 0;
  }
 }

int ufilt(bw)
BW *bw;
 {
#ifdef __MSDOS__
 msgnw(bw,"Sorry, no sub-processes in DOS (yet)");
 return -1;
#else
 switch(checkmark(bw))
  {
  case 0:
  if(wmkpw(bw,
           "Command to filter block through (^C to abort): ",
           &filthist,dofilt,NULL,NULL,utypebw,NULL,NULL)) return 0;
  else return -1;

  case 1:
  if(wmkpw(bw,
           "Command to filter file through (^C to abort): ",
           &filthist,dofilt,NULL,NULL,utypebw,NULL,NULL)) return 0;
  else return -1;

  case 2:
  msgnw(bw,"No block");
  return -1;
  }
#endif
 }
