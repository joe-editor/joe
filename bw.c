/* Edit buffer window generation
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
#include "heap.h"
#include "tty.h"
#include "vfile.h"
#include "toomany.h"
#include "termcap.h"
#include "kbd.h"
#include "b.h"
#include "scrn.h"
#include "w.h"
#include "msgs.h"
#include "bw.h"

/* Display modes */
int dspasis=0;
extern int square;

P *getto(p,cur,top,line)
P *p,*cur,*top;
long line;
{
long dist=MAXLONG;
long d;
P *best;
if(!p)
 {
 if(d=(line-cur->line>=0?line-cur->line:cur->line-line), d<dist)
  dist=d, best=cur;
 if(d=(line-top->line>=0?line-top->line:top->line-line), d<dist)
  dist=d, best=top;
 p=pdup(best);
 pbol(p);
 }
while(line>p->line) if(!pnextl(p)) break;
if(line<p->line)
 {
 while(line<p->line) pprevl(p);
 pboln(p);
 }
return p;
}

/* Scroll window to follow cursor */

int mid=0;

void bwfllw(w)
BW *w;
{
P *newtop;
if(w->cursor->line<w->top->line)
 {
 newtop=pdup(w->cursor);
 pbol(newtop);
 if(mid)
  if(newtop->line>=w->h/2) pline(newtop,newtop->line-w->h/2);
  else pset(newtop,newtop->b->bof);
 if(w->top->line-newtop->line<w->h)
  {
  nscrldn(w->t->t,w->y,w->y+w->h,(int)(w->top->line-newtop->line));
  scrldn(w->t->t->updtab,w->y,w->y+w->h,(int)(w->top->line-newtop->line));
  }
 else msetI(w->t->t->updtab+w->y,1,w->h);
 pset(w->top,newtop);
 prm(newtop);
 }
else if(w->cursor->line>=w->top->line+w->h)
 {
 newtop=pdup(w->top);
 if(mid) newtop=getto(NULL,w->cursor,w->top,w->cursor->line-w->h/2);
 else newtop=getto(NULL,w->cursor,w->top,w->cursor->line-(w->h-1));
 if(newtop->line-w->top->line<w->h)
  {
  nscrlup(w->t->t,
            w->y,
            w->y+w->h,
            (int)(newtop->line-w->top->line));
  scrlup(w->t->t->updtab,w->y,w->y+w->h,(int)(newtop->line-w->top->line));
  }
 else msetI(w->t->t->updtab+w->y,1,w->h);
 pset(w->top,newtop);
 prm(newtop);
 }

/* Adjust column */
if(w->cursor->xcol<w->offset)
 {
 w->offset=w->cursor->xcol;
 msetI(w->t->t->updtab+w->y,1,w->h);
 }
else if(w->cursor->xcol>=w->offset+w->w)
 {
 w->offset=w->cursor->xcol-(w->w-1);
 msetI(w->t->t->updtab+w->y,1,w->h);
 }
}

/* Scroll a buffer window after an insert occured.  'flg' is set to 1 if
 * the first line was split
 */

void bwins(w,l,n,flg)
BW *w;
long l,n;
int flg;
{
if(l+flg+n<w->top->line+w->h && l+flg>=w->top->line && l+flg<=w->b->eof->line)
 {
 int y;
 if(flg) w->t->t->sary[w->y+l-w->top->line]=w->t->t->li;
 nscrldn(w->t->t,(int)(w->y+l+flg-w->top->line),w->y+w->h,(int)n);
 scrldn(w->t->t->updtab,(int)(w->y+l+flg-w->top->line),w->y+w->h,(int)n);
 }
if(l<w->top->line+w->h && l>=w->top->line)
 if(n>=w->h-(l-w->top->line))
  msetI(w->t->t->updtab+w->y+l-w->top->line,1,w->h-(int)(l-w->top->line));
 else
  msetI(w->t->t->updtab+w->y+l-w->top->line,1,(int)n+1);
}

/* Scroll current windows after a delete */

void bwdel(w,l,n,flg)
BW *w;
long l,n;
int flg;
{
if(l<w->top->line+w->h && l>=w->top->line)
 w->t->t->updtab[w->y+l-w->top->line]=1;
if(l+n<w->top->line+w->h && l+n>=w->top->line)
 w->t->t->updtab[w->y+l+n-w->top->line]=1;

if(l<w->top->line+w->h &&
   (l+n>=w->top->line+w->h || l+n==w->b->eof->line))
 if(l>=w->top->line)
  scrlup(w->t->t->updtab,w->y+l-w->top->line,w->y+w->h,
         w->y+w->h-(w->y+l-w->top->line));
 else
  scrlup(w->t->t->updtab,w->y,w->y+w->h,w->h);
else if(l+n<w->top->line+w->h &&
        l+n>w->top->line &&
        l+n<w->b->eof->line)
 if(l+flg>=w->top->line)
  {
  nscrlup(w->t->t,(int)(w->y+l+flg-w->top->line),w->y+w->h,(int)n);
  scrlup(w->t->t->updtab,(int)(w->y+l+flg-w->top->line),w->y+w->h,(int)n);
  }
 else
  {
  nscrlup(w->t->t,w->y,w->y+w->h,(int)(l+n-w->top->line));
  scrlup(w->t->t->updtab,w->y,w->y+w->h,(int)(l+n-w->top->line));
  }
}

/* Update a single line */

static int lgen(t,y,screen,x,w,p,scr,from,to)
SCRN *t;
int y;
int *screen;	/* Screen line address */
int w;		/* Window */
P *p;		/* Buffer pointer */
long scr;	/* Starting column to display */
long from,to;	/* Range for marked block */
{
int ox=x;
int done=1;
long col=0;
long byte=p->byte;
char *bp;		/* Buffer pointer, 0 if not set */
int amnt;		/* Amount left in this segment of the buffer */
int c, ta;
unsigned char bc;

/* Initialize bp and amnt from p */
if(p->ofst>=p->hdr->hole)
 {
 bp=p->ptr+p->hdr->ehole+p->ofst-p->hdr->hole;
 amnt=SEGSIZ-p->hdr->ehole-(p->ofst-p->hdr->hole);
 }
else
 {
 bp=p->ptr+p->ofst;
 amnt=p->hdr->hole-p->ofst;
 }

if(col==scr) goto loop;
lp:		/* Display next character */
if(amnt) do
 {
 bc= *bp++;
 if(square)
  {
  if(col>=from && col<to) c=INVERSE;
  else c=0;
  }
 else
  {
  if(byte>=from && byte<to) c=INVERSE;
  else c=0;
  }
 ++byte;
 if(bc=='\t')
  {
  ta=p->b->tab-col%p->b->tab;
  if(ta+col>scr)
   {
   ta-=scr-col;
   goto dota;
   }
  if((col+=ta)==scr) { --amnt; goto loop; }
  }
 else if(bc=='\n') goto eobl;
 else if(++col==scr) { --amnt; goto loop; }
 }
 while(--amnt);
if(bp==p->ptr+SEGSIZ)
 {
 if(pnext(p))
  {
  bp=p->ptr;
  amnt=p->hdr->hole;
  goto lp;
  }
 else
  {
  bp=p->ptr;
  amnt=p->hdr->hole+1;
  }
 }
else
 {
 bp=p->ptr+p->hdr->ehole;
 amnt=SEGSIZ-p->hdr->ehole;
 goto lp;
 }
goto eof;

loop:		/* Display next character */
if(amnt) do
 {
 bc= *bp++;
 if(square)
  if(col+x>=from && col+x<to) c=INVERSE;
  else c=0;
 else
  if(byte>=from && byte<to) c=INVERSE;
  else c=0;
 ++byte;
 if(bc=='\t')
  {
  ta=p->b->tab-((x-ox+scr)%p->b->tab);
  dota:
  do
   {
   if(screen[x]!=' '+c)
    {
    screen[x]=' '+c;
    if(t->ins) clrins(t);
    if(t->x!=x || t->y!=y) cpos(t,x,y);
    if(c!=t->attrib) attr(t,c);
    ttputc(' '); ++t->x;
    if(have) goto bye;
    }
   if(++x==w) goto eosl;
   }
   while(--ta);
  }
 else if(bc=='\n') goto eobl;
 else
  {
  if(!dspasis || bc<128)
   {
   if(bc&128) c^=INVERSE, bc&=127;
   if(bc==127) c|=UNDERLINE, bc='?';
   else if(bc<32) c|=UNDERLINE, bc+='@';
   }
  if(t->haz && bc=='~') bc='\\';
  if(screen[x]!=c+bc)
   {
   screen[x]=c+bc;
   if(t->ins) clrins(t);
   if(t->x!=x || t->y!=y) cpos(t,x,y);
   if(c!=t->attrib) attr(t,c);
   ttputc(bc); ++t->x;
   if(have) goto bye;
   }
  if(++x==w) goto eosl;
  }
 }
 while(--amnt);
if(bp==p->ptr+SEGSIZ)
 {
 if(pnext(p))
  {
  bp=p->ptr;
  amnt=p->hdr->hole;
  goto loop;
  }
 else
  {
  bp=p->ptr;
  amnt=p->hdr->hole+1;
  }
 }
else
 {
 bp=p->ptr+p->hdr->ehole;
 amnt=SEGSIZ-p->hdr->ehole;
 goto loop;
 }
goto eof;

eobl:		/* End of buffer line found.  Erase to end of screen line */
++p->line;
eof:
if(x!=w) done=eraeol(t,x,y);
else done=0;

/* Set p to bp/amnt */
bye:
if(bp-p->ptr<=p->hdr->hole) p->ofst=bp-p->ptr;
else p->ofst=bp-p->ptr-(p->hdr->ehole-p->hdr->hole);
p->byte=byte;
return done;

eosl:
if(bp-p->ptr<=p->hdr->hole) p->ofst=bp-p->ptr;
else p->ofst=bp-p->ptr-(p->hdr->ehole-p->hdr->hole);
p->byte=byte;
pnextl(p);
return 0;
}

/* Generate line into an array */

static int lgena(t,y,screen,x,w,p,scr,from,to)
SCRN *t;
int y;
int *screen;	/* Screen line address */
int w;		/* Window */
P *p;		/* Buffer pointer */
long scr;	/* Starting column to display */
long from,to;	/* Range for marked block */
{
int ox=x;
int done=1;
long col=0;
long byte=p->byte;
char *bp;		/* Buffer pointer, 0 if not set */
int amnt;		/* Amount left in this segment of the buffer */
int c, ta;
unsigned char bc;

/* Initialize bp and amnt from p */
if(p->ofst>=p->hdr->hole)
 {
 bp=p->ptr+p->hdr->ehole+p->ofst-p->hdr->hole;
 amnt=SEGSIZ-p->hdr->ehole-(p->ofst-p->hdr->hole);
 }
else
 {
 bp=p->ptr+p->ofst;
 amnt=p->hdr->hole-p->ofst;
 }

if(col==scr) goto loop;
lp:		/* Display next character */
if(amnt) do
 {
 bc= *bp++;
 if(square)
  {
  if(col>=from && col<to) c=INVERSE;
  else c=0;
  }
 else
  {
  if(byte>=from && byte<to) c=INVERSE;
  else c=0;
  }
 ++byte;
 if(bc=='\t')
  {
  ta=p->b->tab-col%p->b->tab;
  if(ta+col>scr)
   {
   ta-=scr-col;
   goto dota;
   }
  if((col+=ta)==scr) { --amnt; goto loop; }
  }
 else if(bc=='\n') goto eobl;
 else if(++col==scr) { --amnt; goto loop; }
 }
 while(--amnt);
if(bp==p->ptr+SEGSIZ)
 {
 if(pnext(p))
  {
  bp=p->ptr;
  amnt=p->hdr->hole;
  goto lp;
  }
 else
  {
  bp=p->ptr;
  amnt=p->hdr->hole+1;
  }
 }
else
 {
 bp=p->ptr+p->hdr->ehole;
 amnt=SEGSIZ-p->hdr->ehole;
 goto lp;
 }
goto eobl;

loop:		/* Display next character */
if(amnt) do
 {
 bc= *bp++;
 if(square)
  if(col+x>=from && col+x<to) c=INVERSE;
  else c=0;
 else
  if(byte>=from && byte<to) c=INVERSE;
  else c=0;
 ++byte;
 if(bc=='\t')
  {
  ta=p->b->tab-((x-ox+scr)%p->b->tab);
  dota:
  do
   {
   screen[x]=' '+c;
   if(++x==w) goto eosl;
   }
   while(--ta);
  }
 else if(bc=='\n') goto eobl;
 else
  {
  if(!dspasis || bc<128)
   {
   if(bc&128) c^=INVERSE, bc&=127;
   if(bc==127) c|=UNDERLINE, bc='?';
   else if(bc<32) c|=UNDERLINE, bc+='@';
   }
  screen[x]=c+bc;
  if(++x==w) goto eosl;
  }
 }
 while(--amnt);
if(bp==p->ptr+SEGSIZ)
 {
 if(pnext(p))
  {
  bp=p->ptr;
  amnt=p->hdr->hole;
  goto loop;
  }
 else
  {
  bp=p->ptr;
  amnt=p->hdr->hole+1;
  }
 }
else
 {
 bp=p->ptr+p->hdr->ehole;
 amnt=SEGSIZ-p->hdr->ehole;
 goto loop;
 }
goto eof;
eobl:		/* End of buffer line found.  Erase to end of screen line */
++p->line;
eof:
while(x!=w) screen[x++]=' ';
done=0;

/* Set p to bp/amnt */
bye:
if(bp-p->ptr<=p->hdr->hole) p->ofst=bp-p->ptr;
else p->ofst=bp-p->ptr-(p->hdr->ehole-p->hdr->hole);
p->byte=byte;
return done;

eosl:
if(bp-p->ptr<=p->hdr->hole) p->ofst=bp-p->ptr;
else p->ofst=bp-p->ptr-(p->hdr->ehole-p->hdr->hole);
p->byte=byte;
pnextl(p);
return 0;
}

void bwgen(w)
BW *w;
{
int *screen;
P *p=0;
P *q=pdup(w->cursor);
int bot=w->h+w->y;
int y;
int dosquare=0;
long from,to;
from=to=0;
if(w->t->markb && w->t->markk && w->t->markb->b==w->t->markk->b &&
   w->t->markb->b==w->b)
 if(square) from=w->t->markb->col, to=w->t->markk->col, dosquare=1;
 else from=w->t->markb->byte, to=w->t->markk->byte;

y=w->cursor->line-w->top->line+w->y;
for(screen=w->t->t->scrn+y*w->t->w;y!=bot; ++y, screen+=w->t->w)
 {
 if(have) break;
 if(w->t->t->updtab[y])
  {
  p=getto(p,w->cursor,w->top,w->top->line+y-w->y);
  if(w->t->t->insdel && !w->x)
   {
   pset(q,p);
   if(dosquare)
    if(w->top->line+y-w->y>=w->t->markb->line &&
       w->top->line+y-w->y<=w->t->markk->line)
     lgena(w->t->t,y,w->t->t->compose,w->x,w->x+w->w,q,w->offset,from,to);
    else
     lgena(w->t->t,y,w->t->t->compose,w->x,w->x+w->w,q,w->offset,0L,0L);
   else
    lgena(w->t->t,y,w->t->t->compose,w->x,w->x+w->w,q,w->offset,from,to);
   magic(w->t->t,y,screen,w->t->t->compose,
         (int)(w->cursor->xcol-w->offset+w->x));
   }
  if(dosquare)
   if(w->top->line+y-w->y>=w->t->markb->line &&
      w->top->line+y-w->y<=w->t->markk->line)
    w->t->t->updtab[y]=lgen(w->t->t,y,screen,w->x,w->x+w->w,p,w->offset,
                            from,to);
   else
    w->t->t->updtab[y]=lgen(w->t->t,y,screen,w->x,w->x+w->w,p,w->offset,
                            0L,0L);
  else
   w->t->t->updtab[y]=lgen(w->t->t,y,screen,w->x,w->x+w->w,p,w->offset,
                           from,to);
  }
 }
 
y=w->y;
for(screen=w->t->t->scrn+w->y*w->t->w; y!=w->y+w->cursor->line-w->top->line;
    ++y, screen+=w->t->w)
 {
 if(have) break;
 if(w->t->t->updtab[y])
  {
  p=getto(p,w->cursor,w->top,w->top->line+y-w->y);
  if(w->t->t->insdel && !w->x)
   {
   pset(q,p);
   if(dosquare)
    if(w->top->line+y-w->y>=w->t->markb->line &&
       w->top->line+y-w->y<=w->t->markk->line)
     lgena(w->t->t,y,w->t->t->compose,w->x,w->x+w->w,q,w->offset,from,to);
    else
     lgena(w->t->t,y,w->t->t->compose,w->x,w->x+w->w,q,w->offset,0L,0L);
   else
    lgena(w->t->t,y,w->t->t->compose,w->x,w->x+w->w,q,w->offset,from,to);
   magic(w->t->t,y,screen,w->t->t->compose,
         (int)(w->cursor->xcol-w->offset+w->x));
   }
  if(dosquare)
   if(w->top->line+y-w->y>=w->t->markb->line &&
      w->top->line+y-w->y<=w->t->markk->line)
    w->t->t->updtab[y]=lgen(w->t->t,y,screen,w->x,w->x+w->w,p,w->offset,
                            from,to);
   else
    w->t->t->updtab[y]=lgen(w->t->t,y,screen,w->x,w->x+w->w,p,w->offset,
                            0L,0L);
  else
   w->t->t->updtab[y]=lgen(w->t->t,y,screen,w->x,w->x+w->w,p,w->offset,
                           from,to);
  }
 }
prm(q);
if(p) prm(p);
}

void bwmove(w,x,y)
BW *w;
int x,y;
{
w->x=x;
w->y=y;
}

void bwresz(w,wi,he)
BW *w;
int wi, he;
{
if(he>w->h && w->y!= -1) msetI(w->t->t->updtab+w->y+w->h,1,he-w->h);
w->w=wi;
w->h=he;
}

BW *bwmk(t,b,x,y,wi,h)
SCREEN *t;
B *b;
int x,y,wi,h;
{
BW *w=(BW *)malloc(sizeof(BW));
w->pid=0;
w->out= -1;
w->b=b;
w->x=x;
w->y=y;
w->w=wi;
w->h=h;

w->lmargin=0;
w->rmargin=76;
w->autoindent=0;
w->wordwrap=0;
w->overtype=0;
w->indentc=' ';
w->istep=1;

w->top=pdup(b->bof);
w->cursor=pdup(w->top);
w->t=t; 
w->object=NULL;
w->offset=0;
return w;
}

void bwrm(w)
BW *w;
{
prm(w->top);
prm(w->cursor);
brm(w->b);
free(w);
}

void ustat(w)
W *w;
{
BW *bw=(BW *)w->object;
static char buf[80];
unsigned c=brc(bw->cursor);
if(c==MAXINT)
 sprintf(buf,M007,
         bw->cursor->line+1,bw->cursor->col+1,bw->cursor->byte,
         bw->cursor->byte,bw->b->eof->line+1,bw->b->eof->byte+1);
else
 sprintf(buf,M008,
         bw->cursor->line+1,bw->cursor->col+1,bw->cursor->byte,
         bw->cursor->byte,c,c,c);
msgnw(w,buf);
}
