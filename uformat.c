/* User text formatting functions
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
#include "zstr.h"
#include "ublock.h"
#include "uformat.h"

/* Center line cursor is on and move cursor to beginning of next line */

int ucenter(bw)
BW *bw;
 {
 P *p=bw->cursor, *q;
 long endcol, begcol, x;
 int c;
 
 peol(p);
 while(cwhite(c=prgetc(p)));
 if(c=='\n') { pgetc(p); goto done; }
 if(c==MAXINT) goto done;
 pgetc(p); endcol=piscol(p);
 
 pbol(p);
 while(cwhite(c=pgetc(p)));
 if(c=='\n') { prgetc(p); goto done; }
 if(c==MAXINT) goto done;
 prgetc(p); begcol=piscol(p);
 
 if(endcol-begcol>bw->o.rmargin+bw->o.lmargin) goto done;
 
 q=pdup(p); pbol(q); bdel(q,p); prm(q);
 
 for(x=0;x!=(bw->o.lmargin+bw->o.rmargin)/2-(endcol-begcol)/2;++x) binsc(p,' ');
 
 done:
 if(!pnextl(p))
  {
  binsc(p,'\n');
  pgetc(p);
  return -1;
  }
 else return 0;
 }

/* Return true if line is definitly not a paragraph line.
 * Lines which arn't paragraph lines:
 *  1) Blank lines
 *  2) Lines which begin with '.'
 */

int pisnpara(p)
P *p;
 {
 P *q;
 int c;
 if(pisblank(p)) return 1;
 q=pdup(p);
 pbol(q);
 while(cwhite(c=pgetc(q)));
 prm(q);
 if(c=='.') return 1;
 else return 0;
 }

/* Move pointer to beginning of paragraph
 *
 * This function simply moves backwards until it sees:
 *  0) The beginning of the file
 *  1) A blank line
 *  2) A line with indentation greater than that of the line we started with
 *  3) A line with indentation less than that of the starting line, but with
 *     a blank line (or beginning of file) preceeding it.
 */

int within=0;

P *pbop(p)
P *p;
 {
 long indent;
 pbol(p); indent=pisindent(p);
 while(!pisbof(p) && (!within || !markb || p->byte>markb->byte))
  {
  long ind;
  pprevl(p); pbol(p); ind=pisindent(p);
  if(pisnpara(p)) { pnextl(p); break; }
  if(ind>indent) break;
  if(ind<indent)
   {
   if(pisbof(p)) break;
   pprevl(p); pbol(p);
   if(pisnpara(p)) { pnextl(p); break; }
   else { pnextl(p); pnextl(p); break; }
   }
  }
 return p;
 }

/* Move pointer to end of paragraph.  Pointer must already be on first
 * line of paragraph for this to work correctly.
 *
 * This function moves forwards until it sees:
 *  0) The end of the file.
 *  1) A blank line
 *  2) A line with indentation different from the second line of the paragraph
 */

P *peop(p)
P *p;
 {
 long indent;
 if(!pnextl(p) || pisnpara(p) ||
    (within && markk && p->byte>=markk->byte)) return p;
 indent=pisindent(p);
 while(pnextl(p) && (!within || !markk || p->byte<markk->byte))
  {
  long ind=pisindent(p);
  if(ind!=indent || pisnpara(p)) break;
  }
 return p;
 }

/* Motion commands */

int ubop(bw)
BW *bw;
 {
 P *q=pdup(bw->cursor);
 up: while(pisnpara(q) && !pisbof(q) &&
           (!within || !markb || q->byte>markb->byte)) pprevl(q);
 pbop(q);
 if(q->byte!=bw->cursor->byte)
  {
  pset(bw->cursor,q);
  prm(q);
  return 0;
  }
 else if(!pisbof(q))
  {
  prgetc(q);
  goto up;
  }
 else
  {
  prm(q);
  return -1;
  }
 }

int ueop(bw)
BW *bw;
 {
 P *q=pdup(bw->cursor);
 up: while(pisnpara(q) && !piseof(q)) pnextl(q);
 pbop(q); peop(q);
 if(q->byte!=bw->cursor->byte)
  {
  pset(bw->cursor,q);
  prm(q);
  return 0;
  }
 else if(!piseof(q))
  {
  pnextl(q);
  goto up;
  }
 else
  {
  prm(q);
  return -1;
  }
 }

/* Wrap word */

void wrapword(p,indent,french)
P *p;
long indent;
 {
 P *q;
 int c;
 long to=p->byte;
 /* Get to beginning of word */
 while(!pisbol(p) && piscol(p)>indent && !cwhite(prgetc(p)));
 if(!pisbol(p) && piscol(p)>indent)
  {
  q=pdup(p);
  while(!pisbol(q))
   if(!cwhite(c=prgetc(q)))
    {
    pgetc(q);
    if((c=='.'||c=='?'||c=='!') && q->byte!=p->byte && !french) pgetc(q);
    break;
    }
  pgetc(p);
  to-=p->byte-q->byte;
  bdel(q,p);
  prm(q);
  binsc(p,'\n'), ++to;
#ifdef __MSDOS__
  ++to;
#endif
  pgetc(p);
  if(indent) while(indent--) binsc(p,' '), ++to;
  }
 pfwrd(p,to-p->byte);
 }

/* Reformat paragraph */

int uformat(bw)
BW *bw;
 {
 long indent;
 char *buf, *b;
 int len;
 long curoff;
 int c;
 P *p, *q;
 p=pdup(bw->cursor); pbol(p);
 
 if(pisnpara(p))
  {
  prm(p);
  return 0;
  }
 
 pbop(p);
 curoff=bw->cursor->byte-p->byte;
 pset(bw->cursor,p); peop(bw->cursor);
 
 if(!pisbol(bw->cursor)) binsc(bw->cursor,'\n'), pgetc(bw->cursor);
 
 q=pdup(p); pnextl(q);
 if(q->line!=bw->cursor->line) indent=pisindent(q);
 else indent=pisindent(p);
 prm(q);
 if(bw->o.lmargin>indent) indent=bw->o.lmargin;
 
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
  if(c=='\n' ||
     c=='\r' && len && *b=='\n') { ++len; --b; break; }
  if(cwhite(c))
   {
   char *r=b;
   int rlen=len;
   int z;
   while(rlen--)
    {
    z= *r++;
    if(z=='\n') break;
    if(!cwhite(z)) goto ok;
    }
   ++len; --b; break;
   ok:;
   }
  binsc(p,c); pgetc(p);
  if(piscol(p)>bw->o.rmargin && !cwhite(c))
   {
   wrapword(p,indent,bw->o.french);
   break;
   }
  }
 
 /* Do rest */
 
 while(len>0)
  if(cwhitel(*b) || *b=='\r')
   {
   int f=0;
   if((b[-1]=='.' || b[-1]=='?' || b[-1]=='!') && cwhitel(b[1])) f=1;
   while(len && (cwhitel(*b) || *b=='\r'))
    {
    if(b-buf==curoff) pset(bw->cursor,p);
    ++b, --len;
    }
   if(len)
    {
    if(f && !bw->o.french) binsc(p,' '), pgetc(p);
    binsc(p,' '); pgetc(p);
    }
   }
  else
   {
   if(b-buf==curoff) pset(bw->cursor,p);
   binsc(p,*b++); --len; pgetc(p);
   if(piscol(p)>bw->o.rmargin) wrapword(p,indent,bw->o.french);
   }
 
 binsc(p,'\n');
 prm(p);
 free(buf);
 return 0;
 }

/* Format entire block */

extern int lightoff;

int ufmtblk(bw)
BW *bw;
 {
 if(markv(1) && bw->cursor->byte>=markb->byte && bw->cursor->byte<=markk->byte)
  {
  markk->end=1;
  utomarkk(bw);
  within=1;
  do
   ubop(bw), uformat(bw);
   while(bw->cursor->byte>markb->byte);
  within=0;
  markk->end=0;
  if(lightoff) unmark(bw);
  return 0;
  }
 else return uformat(bw);
 }
