/* User edit functions
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

#include <stdio.h>
#include "config.h"
#include "tty.h"
#include "b.h"
#include "w.h"
#include "termcap.h"
#include "vfile.h"
#include "toomany.h"
#include "scrn.h"
#include "vs.h"
#include "bw.h"
#include "pw.h"
#include "tw.h"
#include "zstr.h"
#include "main.h"
#include "edfuncs.h"

int square=0;		/* Rectangle mode */

B *filehist=0;
B *filthist=0;
B *linehist=0;
B *taghist=0;

char *msgs[]=
{
"Error writing file",
"Error opening file",
"Error seeking file",
"Error reading file",
"New File"
};

/****************/
/* Window stuff */
/****************/

void uprevw(w)
W *w;
{
wprev(w->t);
}

void unextw(w)
W *w;
{
wnext(w->t);
}

void ugroww(w)
W *w;
{
wgrow(w);
}

void ushrnk(w)
W *w;
{
wshrink(w);
}

void uexpld(w)
W *w;
{
if(w->t->h-w->t->wind==getgrouph(w)) wshowall(w->t);
else wshowone(w);
}

/******************************/
/* Rectangle mode subroutines */
/******************************/

void pfill(p,to,usetabs)
P *p;
long to;
{
if(usetabs)
 while(p->col<to)
  if(p->col+p->b->tab-p->col%p->b->tab<=to) binsc(p,'\t'), pgetc(p);
  else binsc(p,' '), pgetc(p);
else while(p->col<to) binsc(p,' '), pgetc(p);
}

/* Insert rectangle into buffer
 * returns width of inserted matter
 */

long pinsrect(cur,tmp)
P *cur;
B *tmp;
{
P *p=pdup(cur);
P *q=pdup(tmp->bof);
P *r=pdup(q);
int usetabs=0;
long width=0;
do
 {
 long wid=cur->col;
 while(!piseol(q))
  if(pgetc(q)=='\t') wid+=cur->b->tab-wid%cur->b->tab, usetabs=1;
  else ++wid;
 if(wid-cur->col>width) width=wid-cur->col;
 } while(pgetc(q)!=MAXINT);
if(width)
 {
 pset(q,tmp->bof);
 while(pset(r,q), peol(q), (q->line!=tmp->eof->line || q->col))
  {
  pcol(p,cur->col);
  if(p->col<cur->col) pfill(p,cur->col,usetabs);
  binsb(p,r,q); pfwrd(p,q->byte-r->byte);
  if(p->col<cur->col+width) pfill(p,cur->col+width,usetabs);
  if(!pnextl(p)) binsc(p,'\n'), pgetc(p);
  if(pgetc(q)==MAXINT) break;
  }
 }
prm(p); prm(q); prm(r);
return width;
}

/* Overwrite version of above */

long povrrect(cur,tmp)
P *cur;
B *tmp;
{
P *p=pdup(cur);
P *q=pdup(tmp->bof);
P *r=pdup(q);
P *z=pdup(cur);
int usetabs=0;
long width=0;
long curcol=cur->col;
do
 {
 long wid=curcol;
 while(!piseol(q))
  if(pgetc(q)=='\t') wid+=cur->b->tab-wid%cur->b->tab, usetabs=1;
  else ++wid;
 if(wid-curcol>width) width=wid-curcol;
 } while(pgetc(q)!=MAXINT);
if(width)
 {
 pset(q,tmp->bof);
 while(pset(r,q), peol(q), (q->line!=tmp->eof->line || q->col))
  {
  pcol(p,curcol);
  if(p->col<curcol) pfill(p,curcol,usetabs);
  pset(z,p); pcol(z,curcol+width); bdel(p,z);
  binsb(p,r,q); pfwrd(p,q->byte-r->byte);
  if(p->col<curcol+width) pfill(p,curcol+width,usetabs);
  if(!pnextl(p)) binsc(p,'\n'), pgetc(p);
  if(pgetc(q)==MAXINT) break;
  }
 }
prm(p); prm(q); prm(r); prm(z);
return width;
}

/* Extract rectangle into a buffer */

B *pextrect(up,down,left,right)
P *up, *down;
long left,right;
{
P *p=pdup(up);
P *q=pdup(p);
B *tmp=bmk();
P *z=pdup(tmp->eof);
pbol(p);
do
 {
 pcol(p,left);
 pset(q,p);
 pcol(q,right);
 pset(z,tmp->eof); binsb(z,p,q);
 pset(z,tmp->eof); binsc(z,'\n');
 } while(pnextl(p) && p->line<=down->line);
prm(p); prm(q); prm(z);
return tmp;
}

/* Delete rectangle.  Returns true if tabs were used */

int pdelrect(up,down,left,right,overtype)
P *up, *down;
long left,right;
{
P *p=pdup(up);
P *q=pdup(p);
int usetabs=0;
if(overtype)
 {
 int c;
 pbol(p);
 do
  {
  pcol(p,left);
  pset(q,p);
  pcol(q,right);
  while(p->byte<q->byte) if(pgetc(p)=='\t') { usetabs=1; break; }
  if(usetabs) break;
  } while(pnextl(p) && p->line<=down->line);
 pset(p,up);
 }
pbol(p);
do
 {
 pcol(p,left);
 pset(q,p);
 pcol(q,right);
 bdel(p,q);
 if(overtype) pfill(p,right,usetabs);
 } while(pnextl(p) && p->line<=down->line);
prm(p); prm(q);
return usetabs;
}

/***************/
/* Block stuff */
/***************/

void umarkb(w)
W *w;
{
BW *bw=(BW *)w->object;
pdupown(bw->cursor,&w->t->markb);
updall();
}

void umarkk(w)
W *w;
{
BW *bw=(BW *)w->object;
pdupown(bw->cursor,&w->t->markk);
updall();
}

void ublkdel(w)
W *w;
{
BW *bw=(BW *)w->object;
if(w->t->markb && w->t->markk &&
   w->t->markb->b==w->t->markk->b &&
   w->t->markk->byte>w->t->markb->byte &&
   (!square || w->t->markb->col<w->t->markk->col))
 if(square)
  {
  pdelrect(w->t->markb,w->t->markk,w->t->markb->col,w->t->markk->col,
           bw->overtype);
  return;
  }
 else
  {
  bdel(w->t->markb,w->t->markk);
  prm(w->t->markb);
  prm(w->t->markk);
  return;
  }
msgnw(w,"No block");
}

void ublkmove(w)
W *w;
{
BW *bw=(BW *)w->object;
long size;
if(w->t->markb && w->t->markk && w->t->markb->b==w->t->markk->b &&
   (size=w->t->markk->byte-w->t->markb->byte)>0 &&
   (!square || w->t->markb->col<w->t->markk->col))
 if(square)
  {
  long height=w->t->markk->line-w->t->markb->line;
  long width;
  long ocol=bw->cursor->col;
  B *tmp=pextrect(w->t->markb,w->t->markk,w->t->markb->col,w->t->markk->col);
  pdelrect(w->t->markb,w->t->markk,w->t->markb->col,w->t->markk->col,
           bw->overtype);
  if(bw->overtype)
   {
   while(bw->cursor->col<ocol)
    if(brc(bw->cursor)==' ') pgetc(bw->cursor);
    else if(bw->cursor->col+bw->b->tab-bw->cursor->col%bw->b->tab<=
            bw->cursor->col) pgetc(bw->cursor);
    else binsc(bw->cursor,' '), pgetc(bw->cursor);
   width=povrrect(bw->cursor,tmp);
   }
  else width=pinsrect(bw->cursor,tmp);
  brm(tmp);
  while(bw->cursor->col<ocol) pgetc(bw->cursor);
  umarkb(w);
  umarkk(w);
  pline(w->t->markk,w->t->markk->line+height);
  pcol(w->t->markk,w->t->markb->col+width);
  return;
  }
 else if(bw->cursor->b!=w->t->markk->b ||
         bw->cursor->byte>w->t->markk->byte ||
         bw->cursor->byte<w->t->markb->byte)
  {
  binsb(bw->cursor,w->t->markb,w->t->markk);
  ublkdel(w);
  umarkb(w);
  umarkk(w);
  pfwrd(w->t->markk,size);
  updall();
  return;
  }
msgnw(w,"No block");
}

void ublkcpy(w)
W *w;
{
BW *bw=(BW *)w->object;
long size;
if(w->t->markb && w->t->markk && w->t->markb->b==w->t->markk->b &&
   (size=w->t->markk->byte-w->t->markb->byte)>0 &&
   (!square || w->t->markb->col < w->t->markk->col))
 if(square)
  {
  B *tmp=pextrect(w->t->markb,w->t->markk,w->t->markb->col,w->t->markk->col);
  long width;
  long height;
  if(bw->overtype) width=povrrect(bw->cursor,tmp);
  else width=pinsrect(bw->cursor,tmp);
  height=w->t->markk->line-w->t->markb->line;
  brm(tmp);
  umarkb(w);
  umarkk(w);
  pline(w->t->markk,w->t->markk->line+height);
  pcol(w->t->markk,w->t->markb->col+width);
  return;
  }
 else
  {
  binsb(bw->cursor,w->t->markb,w->t->markk);
  umarkb(w);
  umarkk(w);
  pfwrd(w->t->markk,size);
  updall();
  return;
  }
msgnw(w,"No block");
}

void ushell(w)
W *w;
{
nescape(w->t->t);
ttsusp();
nreturn(w->t->t);
}

void dowrite(w,s)
W *w;
char *s;
{
long size;
int fl;
if(w->t->markb && w->t->markk && w->t->markb->b==w->t->markk->b &&
   (size=w->t->markk->byte-w->t->markb->byte)>0 &&
   (!square || w->t->markk->col>w->t->markb->col))
 {
 if(square)
  {
  B *tmp=pextrect(w->t->markb,w->t->markk,w->t->markb->col,w->t->markk->col);
  if(fl=bsave(tmp->bof,s,tmp->eof->byte)) msgnw(w,msgs[5+fl]);
  brm(tmp);
  }
 else if(fl=bsave(w->t->markb,s,size)) msgnw(w,msgs[5+fl]);
 }
else msgnw(w,"No block");
vsrm(s);
}

void ublksave(w)
W *w;
{
BW *bw=(BW *)w->object;
if(w->t->markb && w->t->markk && w->t->markb->b==w->t->markk->b &&
   (w->t->markk->byte-w->t->markb->byte)>0 &&
   (!square || w->t->markk->col>w->t->markb->col))
 {
 wmkfpw(w,"Name of file to write (^C to abort): ",&filehist,dowrite,"Names");
 return;
 }
msgnw(w,"No block");
}

long pindent();

void setindent(w)
W *w;
{
BW *bw=(BW *)w->object;
P *p, *q;
long indent;
if(pblank(bw->cursor)) return;

p=pdup(bw->cursor);
q=pdup(p);
indent=pindent(p);

do
 if(!pprevl(p)) goto done;
 else pboln(p);
 while(pindent(p)>=indent && !pblank(p));
pnextl(p);
done:
pboln(p);
if(w->t->markb) prm(w->t->markb);
w->t->markb=p; p->owner=&w->t->markb;

do
 if(!pnextl(q)) break;
 while(pindent(q)>=indent && !pblank(q));
pfcol(q);

if(w->t->markk) prm(w->t->markk);
w->t->markk=q; q->owner=&w->t->markk;

updall();
}

void urindent(w)
W *w;
{
BW *bw=(BW *)w->object;
if(square)
 {
 if(w->t->markb && w->t->markk && w->t->markb->b==w->t->markk->b &&
    w->t->markb->byte<=w->t->markk->byte && w->t->markb->col<=w->t->markk->col)
  {
  P *p=pdup(w->t->markb);
  do
   {
   pcol(p,w->t->markb->col);
   pfill(p,w->t->markb->col+bw->istep,bw->indentc=='\t'?1:0);
   } while(pnextl(p) && p->line<=w->t->markk->line);
  prm(p);
  }
 return;
 }
if(!w->t->markb || !w->t->markk || w->t->markb->b!=w->t->markk->b ||
   bw->cursor->byte<w->t->markb->byte || bw->cursor->byte>w->t->markk->byte)
 {
 setindent(w);
 }
else
 {
 P *p=pdup(w->t->markb);
 while(p->byte<w->t->markk->byte)
  {
  pbol(p);
  while(p->col<bw->istep) binsc(p,bw->indentc), pgetc(p);
  pnextl(p);
  }
 prm(p);
 }
}

void ulindent(w)
W *w;
{
BW *bw=(BW *)w->object;
if(square)
 {
 if(w->t->markb && w->t->markk && w->t->markb->b==w->t->markk->b &&
    w->t->markb->byte<=w->t->markk->byte && w->t->markb->col<=w->t->markk->col)
  {
  P *p=pdup(w->t->markb);
  P *q=pdup(p);
  do
   {
   pcol(p,w->t->markb->col);
   while(p->col<w->t->markb->col+bw->istep)
    {
    int c=pgetc(p);
    if(c!=' ' && c!='\t' && c!=bw->indentc)
     {
     prm(p);
     prm(q);
     return;
     }
    }
   } while(pnextl(p) && p->line<=w->t->markk->line);
  pset(p,w->t->markb);
  do
   {
   pcol(p,w->t->markb->col);
   pset(q,p);
   pcol(q,w->t->markb->col+bw->istep);
   bdel(p,q);
   } while(pnextl(p) && p->line<=w->t->markk->line);
  prm(p); prm(q);
  }
 return;
 }
if(!w->t->markb || !w->t->markk || w->t->markb->b!=w->t->markk->b ||
   bw->cursor->byte<w->t->markb->byte || bw->cursor->byte>w->t->markk->byte)
 {
 setindent(w);
 }
else
 {
 P *p=pdup(w->t->markb);
 P *q=pdup(p);
 pbol(p);
 while(p->byte<w->t->markk->byte)
  {
  if(pindent(p)<bw->istep) { prm(p); return; }
  pnextl(p);
  }
 pset(p,w->t->markb);
 pbol(p);
 while(p->byte<w->t->markk->byte)
  {
  pset(q,p);
  while(q->col<bw->istep) pgetc(q);
  bdel(p,q);
  pnextl(p);
  }
 prm(p); prm(q);
 }
}

static void dofilt(w,s)
W *w;
char *s;
{
BW *bw=(BW *)w->object;
int fr[2];
int fw[2];
char c;

if(w->t->markb && w->t->markk && w->t->markb->b==w->t->markk->b &&
   w->t->markk->byte>w->t->markb->byte &&
   (!square || w->t->markk->col>w->t->markb->col)) goto go;
msgnw(w,"No block");
return;

go:

pipe(fr);
pipe(fw);
nescape(w->t->t);
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
  B *tmp=bmk();
  long width;
  long height;
  pdelrect(w->t->markb,w->t->markk,w->t->markb->col,w->t->markk->col,
           bw->overtype);
  binsfd(tmp->bof,fr[0],MAXLONG);
  if(bw->overtype) width=povrrect(w->t->markb,tmp);
  else width=pinsrect(w->t->markb,tmp);
  if(tmp->eof->col || !tmp->eof->line) height=tmp->eof->line;
  else height=tmp->eof->line-1;
  pdupown(w->t->markb,&w->t->markk);
  pline(w->t->markk,w->t->markk->line+height);
  pcol(w->t->markk,w->t->markb->col+width);
  brm(tmp);
  updall();
  }
 else
  {
  bdel(w->t->markb,w->t->markk);
  szz=w->t->markk->b->eof->byte;
  binsfd(w->t->markk,fr[0],MAXLONG);
  pfwrd(w->t->markk,w->t->markk->b->eof->byte-szz);
  }
 close(fr[0]);
 wait(0);
 wait(0);
 }
else
 {
 if(square)
  {
  B *tmp=pextrect(w->t->markb,w->t->markk,w->t->markb->col,w->t->markk->col);
  bsavefd(tmp->bof,fw[1],tmp->eof->byte);
  }
 else bsavefd(w->t->markb,fw[1],w->t->markk->byte-w->t->markb->byte);
 close(fw[1]);
 _exit(0);
 }
vsrm(s);
ttopnn();
nreturn(w->t->t);
bw->cursor->xcol=bw->cursor->col;
}

void ufilt(w)
W *w;
{
BW *bw=(BW *)w->object;
if(w->t->markb && w->t->markk && w->t->markb->b==w->t->markk->b &&
   (w->t->markk->byte-w->t->markb->byte)>0)
 {
 wmkpw(w,"Command to filter block through (^C to abort): ",
       &filthist,dofilt,NULL);
 return;
 }
msgnw(w,"No block");
}

/****************************/
/* File loading and storing */
/****************************/

static int backup(w)
W *w;
{
BW *bw=(BW *)w->object;
if(!bw->b->backup)
 {
 char *s=0;
 /* Create command string */
 s=vsncpy(s,0,sc("/bin/cp "));
 s=vsncpy(s,sLEN(s),sz(bw->b->name));
 s=vsadd(s,' ');
 s=vsncpy(s,sLEN(s),sz(bw->b->name));
 s=vsncpy(s,sLEN(s),sc("~ 2>/dev/null"));

 if(system(s))
  {
  msgnw(w,"Couldn't make backup file... file not saved");
  vsrm(s);
  return 1;
  }
 else
  {
  bw->b->backup=1;
  vsrm(s);
  return 0;
  }
 }
else return 0;
}

static int dosave(w,s)
W *w;
char *s;
{
BW *bw=(BW *)w->object;
FILE *f;
int fl;
if(backup(w)) { vsrm(s); return 1; }
if(fl=bsave(bw->b->bof,s,bw->b->eof->byte))
 {
 msgnw(w,msgs[fl+5]);
 vsrm(s);
 return 1;
 }
else
 {
 if(!bw->b->name) bw->b->name=zdup(s);
 if(!zcmp(bw->b->name,s)) bw->b->chnged=0;
 vsrm(s);
 return 0;
 }
}

void usave(w)
W *w;
{
BW *bw=(BW *)w->object;
W *pw=wmkfpw(w,"Name of file to save (^C to abort): ",&filehist,dosave,"Names");
if(pw && bw->b->name)
 {
 BW *pbw=(BW *)pw->object;
 binss(pbw->cursor,bw->b->name);
 pset(pbw->cursor,pbw->b->eof); pbw->cursor->xcol=pbw->cursor->col;
 }
}

static void doedit(w,s)
W *w;
char *s;
{
BW *bw=(BW *)w->object;
void *object=bw->object;
B *b=bfind(s);
if(!b)
 {
 b=bmk();
 if(!zlen(s))
  msgnwt(w,"New file ");
 else
  {
  int fl;
  if(fl=bload(b,s)) msgnwt(w,msgs[fl+5]);
  }
 }
bwrm(bw);
w->object=(void *)(bw=bwmk(w->t,b,w->x,w->y+1,w->w,w->h-1));
wredraw(w);
setoptions(bw,s);
bw->object=object;
vsrm(s);
}

void uedit(w)
W *w;
{
BW *bw=(BW *)w->object;
if(bw->b->count==1 && bw->b->chnged)
 {
 int c=query(w,"Do you really want to throw away this file (y,n)? ");
 if(c!='y' && c!='Y') return;
 }
wmkfpw(w,"Name of file to edit (^C to abort): ",&filehist,doedit,"Names");
}

static void doinsf(w,s)
W *w;
char *s;
{
BW *bw=(BW *)w->object;
int fl;
if(square)
 {
 B *tmp=bmk();
 if(fl=binsf(tmp->bof,s)) msgnw(w,msgs[fl+5]);
 else
  if(bw->overtype) povrrect(bw->cursor,tmp);
  else pinsrect(bw->cursor,tmp);
 brm(tmp);
 }
else
 if(fl=binsf(bw->cursor,s)) msgnw(w,msgs[fl+5]);
vsrm(s);
bw->cursor->xcol=bw->cursor->col;
}

void uinsf(w)
W *w;
{
BW *bw=(BW *)w->object;
wmkfpw(w,"Name of file to insert (^C to abort): ",&filehist,doinsf,"Names");
}

extern char *exmsg;

static void doex(w,s)
W *w;
char *s;
{
BW *bw=(BW *)w->object;
bw->b->name=zdup(s); 
if(dosave(w,s)) { free(bw->b->name); bw->b->name=0; return; }
exmsg=vsncpy(NULL,0,sc("File "));
exmsg=vsncpy(exmsg,sLEN(exmsg),sz(bw->b->name));
exmsg=vsncpy(exmsg,sLEN(exmsg),sc(" saved."));
bw->b->chnged=0;
wabort(w);
}

void uexsve(w)
W *w;
{
BW *bw=(BW *)w->object;
if(!bw->b->chnged)
 {
 exmsg=vsncpy(NULL,0,sc("File "));
 exmsg=vsncpy(exmsg,sLEN(exmsg),sz(bw->b->name));
 exmsg=vsncpy(exmsg,sLEN(exmsg),sc(" not changed so no updated needed."));
 wabort(w);
 return;
 }
if(bw->b->name)
 {
 if(dosave(w,vsncpy(NULL,0,sz(bw->b->name)))) return;
 exmsg=vsncpy(NULL,0,sc("File "));
 exmsg=vsncpy(exmsg,sLEN(exmsg),sz(bw->b->name));
 exmsg=vsncpy(exmsg,sLEN(exmsg),sc(" saved."));
 bw->b->chnged=0;
 wabort(w);
 } 
else wmkfpw(w,"Name of file to save (^C to abort): ",&filehist,doex,"Names");
}

/*************/
/* Goto line */
/*************/

static void doline(w,s)
W *w;
char *s;
{
BW *bw=(BW *)w->object;
long num=0;
sscanf(s,"%ld",&num);
if(num>=1) pline(bw->cursor,num-1), bw->cursor->xcol=bw->cursor->col;
else msgnw(w,"Invalid line number");
vsrm(s);
}

void uline(w)
W *w;
{
wmkpw(w,"Goto line (^C to abort): ",&linehist,doline,NULL);
}

/************************/
/* Basic edit functions */
/************************/

void uquote(w)
W *w;
{
BW *bw=(BW *)w->object;
int c=queryn(w,"Ctrl-");
if((c>=0x40 && c<=0x5F) || (c>='a' && c<='z')) c&=0x1F;
if(c=='?') c=127;
utype(w,c);
}

void uquote8(w)
W *w;
{
BW *bw=(BW *)w->object;
int c=queryn(w,"Meta-");
if(c=='`')
 {
 c=queryn(w,"Meta-Ctrl-");
 if((c>=0x40 && c<=0x5F) || (c>='a' && c<='z')) c&=0x1F;
 if(c=='?') c=127;
 }
c|=128;
utype(w,c);
}

void uretyp(w)
W *w;
{
BW *bw=(BW *)w->object;
nredraw(w->t->t);
}

P *pboi(p)
P *p;
{
pbol(p);
while(cwhite(brc(p))) pgetc(p);
return p;
}

int pisedge(p)
P *p;
{
P *q;
int c;
if(pisbol(p)) return 1;
if(piseol(p)) return 1;
q=pdup(p);
pboi(q);
if(q->byte==p->byte) goto yes;
if(cwhite(c=brc(p)))
 {
 pset(q,p); if(cwhite(prgetc(q))) goto no;
 if(c=='\t') goto yes;
 pset(q,p); pgetc(q);
 if(pgetc(q)==' ') goto yes;
 goto no;
 }
else
 {
 pset(q,p); c=prgetc(q);
 if(c=='\t') goto yes;
 if(c!=' ') goto no;
 if(prgetc(q)==' ') goto yes;
 goto no;
 }

yes: prm(q); return 1;
no:  prm(q); return 0;
}

void upedge(w)
W *w;
{
BW *bw=(BW *)w->object;
prgetc(bw->cursor);
while(!pisedge(bw->cursor)) prgetc(bw->cursor);
}

void unedge(w)
W *w;
{
BW *bw=(BW *)w->object;
pgetc(bw->cursor);
while(!pisedge(bw->cursor)) pgetc(bw->cursor);
}

void ubol(w)
W *w;
{
BW *bw=(BW *)w->object;
pbol(bw->cursor);
}

void ueol(w)
W *w;
{
BW *bw=(BW *)w->object;
peol(bw->cursor);
}

void ubof(w)
W *w;
{
BW *bw=(BW *)w->object;
pbof(bw->cursor);
}

void ueof(w)
W *w;
{
BW *bw=(BW *)w->object;
peof(bw->cursor);
}

void ultarw(w)
W *w;
{
BW *bw=(BW *)w->object;
prgetc(bw->cursor);
}


void urtarw(w)
W *w;
{
BW *bw=(BW *)w->object;
pgetc(bw->cursor);
}

void uprvwrd(w)
W *w;
{
BW *bw=(BW *)w->object;
int c, d;

/* Move to end of previous word or edge */
lp:
d=' ';
while(c=prgetc(bw->cursor),
      c!= MAXINT && !cword(c) && (!cwhitel(c) || cwhitel(d)))
 d=c; 
if(c==' ')
 {
 d=prgetc(bw->cursor); if(d!=MAXINT) pgetc(bw->cursor);
 if(!cwhitel(d)) { pgetc(bw->cursor); goto lp; }
 }
if(c!= MAXINT) pgetc(bw->cursor);


/* Move to beginning of current word */
while(cword(c=prgetc(bw->cursor)));
if(c!= MAXINT) pgetc(bw->cursor);
}

void unxtwrd(w)
W *w;
{
BW *bw=(BW *)w->object;
int c, d;
/* Move to start of next word or edge */
lp:
d=' ';
while(c=brc(bw->cursor),
      c!= MAXINT && !cword(c) && (!cwhitel(c) || cwhitel(d)))
 d=pgetc(bw->cursor);
if(c==' ')
 {
 pgetc(bw->cursor); d=brc(bw->cursor); prgetc(bw->cursor);
 if(!cwhitel(d)) goto lp;
 }

/* Move to end of current word */
while(c=brc(bw->cursor), cword(c)) pgetc(bw->cursor);
}

void utomatch(w)
W *w;
{
BW *bw=(BW *)w->object;
int c, f, dir, cnt, d;
P *p;
c=brc(bw->cursor);
f= MAXINT; dir=1;
if(c=='(') f=')';
if(c=='[') f=']';
if(c=='{') f='}';
if(c=='`') f='\'';
if(c=='<') f='>';
if(c==')') f='(', dir= -1;
if(c==']') f='[', dir= -1;
if(c=='}') f='{', dir= -1;
if(c=='\'') f='`', dir= -1;
if(c=='>') f='<', dir= -1;
if(f== MAXINT) return;
if(dir==1)
 {
 p=pdup(bw->cursor);
 cnt=0;
 pgetc(p);
 while(d=pgetc(p), d!= MAXINT)
  if(d==c) ++cnt;
  else if(d==f) if(!cnt--) break;
 if(d!= MAXINT)
  {
  prgetc(p);
  pset(bw->cursor,p);
  }
 prm(p);
 }
else
 {
 p=pdup(bw->cursor);
 cnt=0;
 while(d=prgetc(p), d!= MAXINT)
  if(d==c) ++cnt;
  else if(d==f) if(!cnt--) break;
 if(d!= MAXINT) pset(bw->cursor,p);
 prm(p);
 }
}

void uuparw(w)
W *w;
{
BW *bw=(BW *)w->object;
long col=bw->cursor->xcol;
pprevl(bw->cursor);
pboln(bw->cursor);
pcol(bw->cursor,bw->cursor->xcol=col);
}

void udnarw(w)
W *w;
{
BW *bw=(BW *)w->object;
long col=bw->cursor->xcol;
if(!pnextl(bw->cursor)) pboln(bw->cursor);
pcol(bw->cursor,bw->cursor->xcol=col);
}

void scrup(w,n,flg)
W *w;
{
BW *bw=(BW *)w->object;
int scrollamnt=0;
int cursoramnt=0;
int x;
long col=bw->cursor->xcol;
if(bw->top->line>=n) scrollamnt=cursoramnt=n;
else if(bw->top->line) scrollamnt=cursoramnt=bw->top->line;
else
 if(flg) cursoramnt=bw->cursor->line;
 else if(bw->cursor->line>=n) cursoramnt=n;

for(x=0;x!=scrollamnt;++x) pprevl(bw->top);
pboln(bw->top);
for(x=0;x!=cursoramnt;++x) pprevl(bw->cursor);
pboln(bw->cursor); pcol(bw->cursor,bw->cursor->xcol=col);
if(scrollamnt && scrollamnt<bw->h)
 {
 nscrldn(w->t->t,bw->y,bw->y+bw->h,scrollamnt);
 scrldn(w->t->t->updtab,bw->y,bw->y+bw->h,scrollamnt);
 }
else if(scrollamnt)
 {
 scrldn(w->t->t->updtab,bw->y,bw->y+bw->h,bw->h);
 }
}

void scrdn(w,n,flg)
W *w;
{
BW *bw=(BW *)w->object;
int scrollamnt=0;
int cursoramnt=0;
int x;
long col=bw->cursor->xcol;

if(bw->top->b->eof->line<bw->top->line+bw->h)
 {
 cursoramnt=bw->top->b->eof->line-bw->cursor->line;
 if(!flg && cursoramnt>n) cursoramnt=n;
 }
else if(bw->top->b->eof->line-(bw->top->line+bw->h)>=n)
 cursoramnt=scrollamnt=n;
else
 cursoramnt=scrollamnt=bw->top->b->eof->line-(bw->top->line+bw->h)+1;

for(x=0;x!=scrollamnt;++x) pnextl(bw->top);
for(x=0;x!=cursoramnt;++x) pnextl(bw->cursor);
pcol(bw->cursor,bw->cursor->xcol=col);
if(scrollamnt && scrollamnt<bw->h)
 {
 int x;
 nscrlup(w->t->t,bw->y,bw->y+bw->h,scrollamnt);
 scrlup(w->t->t->updtab,bw->y,bw->y+bw->h,scrollamnt);
 }
else if(scrollamnt) scrlup(w->t->t->updtab,bw->y,bw->y+bw->h,bw->h);
}

int pgamnt= -1;

void upgup(w)
W *w;
{
BW *bw=(BW *)w->object;
if(pgamnt== -1) scrup(w,bw->h/2+(bw->h&1),1);
else if(pgamnt<bw->h) scrup(w,bw->h-pgamnt,1);
else scrup(w,1,1);
}

void upgdn(w)
W *w;
{
BW *bw=(BW *)w->object;
if(pgamnt== -1) scrdn(w,bw->h/2+(bw->h&1),1);
else if(pgamnt<bw->h) scrdn(w,bw->h-pgamnt,1);
else scrdn(w,1,1);
}

void uupslide(w)
W *w;
{
scrup(w,1,0);
}

void udnslide(w)
W *w;
{
scrdn(w,1,0);
}

void udelch(w)
W *w;
{
BW *bw=(BW *)w->object;
P *p;
p=pdup(bw->cursor);
pgetc(p);
bdel(bw->cursor,p);
prm(p);
}

void ubacks(w)
W *w;
{
BW *bw=(BW *)w->object;
P *p;
int c;
if(bw->overtype && !pisbol(bw->cursor) && !piseol(bw->cursor))
 {
 if((c=prgetc(bw->cursor))!='\t') return;
 else if(c!=MAXINT) pgetc(bw->cursor);
 }
p=pdup(bw->cursor);
if((c=prgetc(bw->cursor))!= MAXINT) bdel(bw->cursor,p);
prm(p);
}

void udelw(w)
W *w;
{
BW *bw=(BW *)w->object;
P *p;
int c;
p=pdup(bw->cursor);
c=brc(p);
if(cword(c))
 while(c=brc(p), cword(c)) pgetc(p);
else if(cwhitel(c))
 while(c=brc(p), cwhitel(c)) pgetc(p);
else pgetc(p);
bdel(bw->cursor,p);
prm(p);
}

void ubackw(w)
W *w;
{
BW *bw=(BW *)w->object;
P *p;
int c;
p=pdup(bw->cursor);
c=prgetc(bw->cursor);
if(cword(c))
 {
 while(c=prgetc(bw->cursor), cword(c));
 if(c!= MAXINT) pgetc(bw->cursor);
 }
else if(cwhitel(c))
 {
 while(c=prgetc(bw->cursor), cwhitel(c));
 if(c!= MAXINT) pgetc(bw->cursor);
 }
bdel(bw->cursor,p);
prm(p);
}

void udelel(w)
W *w;
{
BW *bw=(BW *)w->object;
P *p;
p=pdup(bw->cursor);
peol(p);
if(bw->cursor->byte==p->byte)
 {
 prm(p);
 udelch(w);
 }
else
 {
 bdel(bw->cursor,p);
 prm(p);
 }
}

void udelbl(w)
W *w;
{
BW *bw=(BW *)w->object;
P *p;
p=pdup(bw->cursor);
pbol(p);
bdel(p,bw->cursor);
prm(p);
}

void udelln(w)
W *w;
{
BW *bw=(BW *)w->object;
P *p;
pbol(bw->cursor);
p=pdup(bw->cursor);
pnextl(p);
bdel(bw->cursor,p);
prm(p);
}

void ucenter(w)
W *w;
{
BW *bw=(BW *)w->object;
P *p=bw->cursor, *q;
long endcol, begcol, x;
int c;

peol(p);
while(!pisbol(p) && cwhite(c=prgetc(p)));
if(pisbol(p)) return;
pgetc(p); endcol=p->col;

pbol(p);
while(!piseol(p) && cwhite(c=pgetc(p)));
if(piseol(p)) return;
prgetc(p); begcol=p->col;

if(endcol-begcol>bw->rmargin+bw->lmargin) return;

q=pdup(p); pbol(q); bdel(q,p); prm(q);

for(x=0;x!=(bw->lmargin+bw->rmargin)/2-(endcol-begcol)/2;++x) binsc(p,' ');

if(!pnextl(p))
 {
 binsc(p,'\n');
 pgetc(p);
 }
}

/* Paragraph stuff */

/* Determine if line pointer is on is blank */

int pblank(p)
P *p;
{
P *q=pdup(p);
int rtval;
pbol(q);
while(cwhite(brc(q))) pgetc(q);
rtval=piseol(q);
prm(q);
return rtval;
}

/* Determine indentation level of line pointer is on */

long pindent(p)
P *p;
{
P *q=pdup(p);
long col;
pbol(q);
while(cwhite(brc(q))) pgetc(q);
col=q->col;
prm(q);
return col;
}

/* Move pointer to beginning of paragraph */

P *pbop(p)
P *p;
{
long indent;
pbol(p);
indent=pindent(p);
while(!pisbof(p))
 {
 long ind;
 pprevl(p); pboln(p);
 ind=pindent(p);
 if(pblank(p) || ind<indent)
  {
  pnextl(p);
  break;
  }
 if(ind>indent) break;
 }
return p;
}

/* Move pointer to end of paragraph */

P *peop(p)
P *p;
{
long indent;
pbol(p);
indent=pindent(p);
while(!piseof(p))
 {
 long ind;
 pnextl(p);
 ind=pindent(p);
 if(ind>indent || pblank(p)) break;
 if(ind<indent) indent=ind;
 }
if(piseof(p))  peol(p);
return p;
}

/* Wrap word */

void wrapword(p,indent)
P *p;
long indent;
{
int c;
long to=p->byte;
while(!pisbol(p) && !cwhite(c=prgetc(p)));
if(!pisbol(p))
 {
 pgetc(p);
 binsc(p,'\n'), ++to;
 pgetc(p);
 if(indent) while(indent--) binsc(p,' '), ++to;
 }
pfwrd(p,to-p->byte);
}

/* Reformat paragraph */

void uformat(w)
W *w;
{
BW *bw=(BW *)w->object;
long indent;
char *buf, *b;
int len;
long curoff;
int c;
P *p, *q;
p=pdup(bw->cursor); pbol(p);

if(pblank(p))
 {
 prm(p);
 return;
 }

pbop(p);
curoff=bw->cursor->byte-p->byte;
peop(bw->cursor);

if(bw->cursor->lbyte) binsc(bw->cursor,'\n'), pgetc(bw->cursor);
if(piseof(bw->cursor)) binsc(bw->cursor,'\n');

indent=pindent(p);
q=pdup(p); pnextl(q);
if(q->line!=bw->cursor->line) indent=pindent(q);
prm(q);
if(bw->lmargin>indent) indent=bw->lmargin;

buf=(char *)malloc(len=(bw->cursor->byte-p->byte));
brmem(p,buf,len);
bdel(p,bw->cursor);
prm(p);

/* text is in buffer.  insert it at cursor */

/* Do first line */
b=buf;
p=pdup(bw->cursor);

while(len--)
 {
 if(b-buf==curoff) pset(bw->cursor,p);
 c= *b++;
 if(c=='\n') { ++len; --b; break; }
 if(cwhite(c))
  {
  char *r=b;
  int rlen=len;
  int z;
  while(rlen--)
   {
   z=*r++;
   if(z=='\n') break;
   if(!cwhite(z)) goto ok;
   }
  ++len; --b; break;
  ok:;
  }
 binsc(p,c); pgetc(p);
 if(p->col>bw->rmargin && !cwhite(c))
  {
  wrapword(p,indent);
  break;
  }
 }

while(len>0)
 if(cwhitel(*b))
  {
  if(b[-1]=='.' || b[-1]=='?' || b[-1]=='!') binsc(p,' '), pgetc(p);
  binsc(p,' '); pgetc(p);
  while(len && cwhitel(*b))
   {
   if(b-buf==curoff) pset(bw->cursor,p);
   ++b, --len;
   }
  }
 else
  {
  if(b-buf==curoff) pset(bw->cursor,p);
  binsc(p,*b++); --len; pgetc(p);
  if(p->col>bw->rmargin) wrapword(p,indent);
  }

binsc(p,'\n');
prm(p);
free(buf);
}

void uinsc(w)
W *w;
{
BW *bw=(BW *)w->object;
binsc(bw->cursor,' ');
}

void utype(w,c)
W *w;
int c;
{
BW *bw=(BW *)w->object;
P *p;
if(pblank(bw->cursor))
 while(bw->cursor->col<bw->lmargin) binsc(bw->cursor,' '), pgetc(bw->cursor);
binsc(bw->cursor,c);
pgetc(bw->cursor);
if(bw->wordwrap && bw->cursor->col>bw->rmargin && !cwhite(c))
 wrapword(bw->cursor,bw->lmargin);
else if(bw->overtype && !piseol(bw->cursor) && c!='\t') udelch(w);
}

void urtn(w)
W *w;
{
BW *bw=(BW *)w->object;
P *p;
int c;
p=pdup(bw->cursor);
binsc(bw->cursor,'\n');
pgetc(bw->cursor);
if(bw->autoindent)
 {
 pbol(p);
 while(cwhite(c=pgetc(p))) binsc(bw->cursor,c), pgetc(bw->cursor);
 }
prm(p);
}

void uopen(w)
W *w;
{
BW *bw=(BW *)w->object;
P *q=pdup(bw->cursor);
urtn(w);
pset(bw->cursor,q);
prm(q);
}

void dotag(w,s)
W *w;
char *s;
{
BW *bw=(BW *)w->object;
char buf[512];
FILE *f;
void *object=bw->object;
f=fopen("tags","r");
while(fgets(buf,512,f))
 {
 int x, y, c;
 for(x=0;buf[x] && buf[x]!=' ' && buf[x]!='\t';++x);
 c=buf[x]; buf[x]=0;
 if(!zcmp(s,buf))
  {
  buf[x]=c;
  while(buf[x]==' ' || buf[x]=='\t') ++x;
  for(y=x;buf[y] && buf[y]!=' ' && buf[y]!='\t' && buf[y]!='\n';++y);
  if(x!=y)
   {
   B *b;
   c=buf[y]; buf[y]=0;
   b=bfind(buf+x);
   if(!b)
    {
    int fl;
    b=bmk();
    if(fl=bload(b,buf+x))
     { msgnwt(w,msgs[fl+5]); brm(b); vsrm(s); fclose(f); return; }
    }
   bwrm(bw);
   w->object=(void *)(bw=bwmk(w->t,b,w->x,w->y+1,w->w,w->h-1));
   wredraw(w);
   setoptions(bw,buf+x);
   bw->object=object;
   
   buf[y]=c;
   while(buf[y]==' ' || buf[y]=='\t') ++y;
   for(x=y;buf[x] && buf[x]!='\n';++x);
   buf[x]=0;
   if(x!=y)
    {
    long line=0;
    if(buf[y]>='0' && buf[y]<='9')
     {
     sscanf(buf+y,"%ld",&line);
     if(line>=1) pline(bw->cursor,line-1), bw->cursor->xcol=bw->cursor->col;
     else msgnw(w,"Invalid line number");
     }
    else
     {
     if(buf[y]=='/' || buf[y]=='?')
      {
      ++y;
      if(buf[y]=='^') buf[--y]='\\';
      }
     if(buf[x-1]=='/' || buf[x-1]=='?')
      {
      --x;
      buf[x]=0;
      if(buf[x-1]=='$')
       {
       buf[x-1]='\\';
       buf[x]='$';
       ++x;
       buf[x]=0;
       }
      }
     if(x!=y)
      {
      w->t->pattern=vsncpy(NULL,0,sz(buf+y));
      w->t->options=0;
      w->t->repeat= -1;
      pfnext(w);
      }
     }
    }
   vsrm(s);
   fclose(f);
   return;
   }
  }
 }
msgnw(w,"Not found");
vsrm(s);
fclose(f);
}

void utag(w)
W *w;
{
BW *bw=(BW *)w->object;
W *pw;
if(bw->b->count==1 && bw->b->chnged)
 {
 int c=query(w,"Do you really want to throw away this file (y,n)? ");
 if(c!='y' && c!='Y') return;
 }
pw=wmkfpw(w,"Tag string to find (^C to abort): ",&taghist,dotag,NULL);
if(pw && cword(brc(bw->cursor)))
 {
 BW *pbw=(BW *)pw->object;
 P *p=pdup(bw->cursor);
 P *q=pdup(p);
 int c;
 while(cword(c=prgetc(p))); if(c!=MAXINT) pgetc(p);
 pset(q,p); while(cword(c=pgetc(q))); if(c!=MAXINT) prgetc(q);
 binsb(pbw->cursor,p,q);
 pset(pbw->cursor,pbw->b->eof); pbw->cursor->xcol=pbw->cursor->col;
 prm(p); prm(q);
 }
}

/* Mode commands */

void uisquare(w)
W *w;
{
square= !square;
if(square) msgnw(w,"Rectangle mode selected.  Effects block commands & Insert file");
else msgnw(w,"Normal text-stream mode selected");
updall();
}

void uiindent(w)
W *w;
{
BW *bw=(BW *)w->object;
bw->autoindent= !bw->autoindent;
if(bw->autoindent) msgnw(w,"Autoindent enabled");
else msgnw(w,"Autoindent disabled");
}

void uiwrap(w)
W *w;
{
BW *bw=(BW *)w->object;
bw->wordwrap= !bw->wordwrap;
if(bw->wordwrap) msgnw(w,"Word wrap enabled");
else msgnw(w,"Word wrap disabled");
}

void uitype(w)
W *w;
{
BW *bw=(BW *)w->object;
bw->overtype= !bw->overtype;
if(bw->overtype==0) msgnw(w,"Insert mode");
else msgnw(w,"Overtype mode");
}

void uimid(w)
W *w;
{
mid= !mid;
if(mid) msgnw(w,"Cursor will be recentered after a scroll");
else msgnw(w,"Cursor will not be recentered after a scroll");
}

void uiforce(w)
W *w;
{
force= !force;
if(force) msgnw(w,"Last line forced to have LF when file saved");
else msgnw(w,"Last line not forced to have LF");
}

void uiasis(w)
W *w;
{
dspasis= !dspasis;
if(dspasis) msgnw(w,"Characters above 127 displayed as-is");
else msgnw(w,"Characters above 127 remapped");
refigure();
updall();
}

void uistacol(w)
W *w;
{
stacol= !stacol;
}

void uistarow(w)
W *w;
{
starow= !starow;
}

static void dolmar(w,s)
W *w;
char *s;
{
BW *bw=(BW *)w->object;
long v=bw->lmargin;
sscanf(s,"%ld",&v);
vsrm(s);
bw->lmargin=v;
}

void uilmargin(w)
W *w;
{
BW *bw=(BW *)w->object;
char buf[80];
sprintf(buf,"Left margin %ld (^C to abort): ",bw->lmargin);
wmkpw(w,buf,NULL,dolmar,NULL);
}

static void docindent(w,s)
W *w;
char *s;
{
BW *bw=(BW *)w->object;
int v=bw->indentc;
sscanf(s,"%d",&v);
vsrm(s);
bw->indentc=v;
}

void uicindent(w)
W *w;
{
BW *bw=(BW *)w->object;
char buf[80];
sprintf(buf,"Indent char %d (SPACE=32, TAB=9, ^C to abort): ",
        bw->indentc);
wmkpw(w,buf,NULL,docindent,NULL);
}

static void doistep(w,s)
W *w;
char *s;
{
BW *bw=(BW *)w->object;
long v=bw->istep;
sscanf(s,"%ld",&v);
vsrm(s);
bw->istep=v;
}

void uiistep(w)
W *w;
{
BW *bw=(BW *)w->object;
char buf[80];
sprintf(buf,"Indent step %ld (^C to abort): ",bw->istep);
wmkpw(w,buf,NULL,doistep,NULL);
}

static void dormar(w,s)
W *w;
char *s;
{
BW *bw=(BW *)w->object;
long v=bw->rmargin;
sscanf(s,"%ld",&v);
vsrm(s);
bw->rmargin=v;
}

void uirmargin(w)
W *w;
{
BW *bw=(BW *)w->object;
char buf[80];
sprintf(buf,"Right margin %ld (^C to abort): ",bw->rmargin);
wmkpw(w,buf,NULL,dormar,NULL);
}

static void dotab(w,s)
W *w;
char *s;
{
BW *bw=(BW *)w->object;
int v=bw->b->tab;
sscanf(s,"%ld",&v);
if(v<0 || v>256) v=8;
vsrm(s);
bw->b->tab=v;
refigure();
updall();
}

void uitab(w)
W *w;
{
BW *bw=(BW *)w->object;
char buf[80];
sprintf(buf,"Tab width %d (^C to abort): ",bw->b->tab);
wmkpw(w,buf,NULL,dotab,NULL);
}

static void dopgamnt(w,s)
W *w;
char *s;
{
BW *bw=(BW *)w->object;
long v=pgamnt;
sscanf(s,"%ld",&v);
vsrm(s);
if(v<-1) v= -1;
pgamnt=v;
}

void uipgamnt(w)
W *w;
{
char buf[80];
sprintf(buf,"Lines to keep for pgup/pgdn or -1 for 1/2 (%ld): ",pgamnt);
wmkpw(w,buf,NULL,dopgamnt,NULL);
}

/* Argument setting */

void uarg(w,c)
W *w;
{
char buf[80];
if(c>='1' && c<='9') w->t->arg=(c&0xF);
else w->t->arg=0;
sprintf(buf,"%d",w->t->arg); msgnw(w,buf);
while(c=edgetc(), c>='0' && c<='9')
 {
 w->t->arg=w->t->arg*10+(c&0xf);
 sprintf(buf,"%d",w->t->arg); msgnw(w,buf);
 }
if(c==3) w->t->arg=0;
else eungetc(c);
}
