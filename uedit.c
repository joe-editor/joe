/* Basic user edit functions
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
#include "zstr.h"
#include "vs.h"
#include "uformat.h"
#include "umath.h"
#include "ublock.h"
#include "tw.h"
#include "macro.h"
#include "main.h"
#include "uedit.h"

/* Global options */

int pgamnt= -1;				/* No. of PgUp/PgDn lines to keep */

/* Move cursor to beginning of line */

int ubol(bw)
BW *bw;
 {
 pbol(bw->cursor);
 return 0;
 }

/* Move cursor to end of line */

int ueol(bw)
BW *bw;
 {
 peol(bw->cursor);
 return 0;
 }

/* Move cursor to beginning of file */

int ubof(bw)
BW *bw;
 {
 pbof(bw->cursor);
 return 0;
 }

/* Move cursor to end of file */

int ueof(bw)
BW *bw;
 {
 peof(bw->cursor);
 return 0;
 }

/* Move cursor left */

int ultarw(bw)
BW *bw;
 {
 if(prgetc(bw->cursor)!=MAXINT) return 0;
 else return -1;
 }

/* Move cursor right */

int urtarw(bw)
BW *bw;
 {
 if(pgetc(bw->cursor)!=MAXINT) return 0;
 else return -1;
 }

/* Move cursor to beginning of previous word, previous edge, or beginning
   of line */

int uprvwrd(bw)
BW *bw;
 {
 int c, d;
 if(pisbof(bw->cursor)) return -1;
 /* Move to end of previous word or edge */
 lp:
 d=' ';
 while(c=prgetc(bw->cursor),
       c!= MAXINT && !crest(c) && (!cwhitel(c) || cwhitel(d)))
  d=c; 
 if(c==' ')
  {
  d=prgetc(bw->cursor); if(d!=MAXINT) pgetc(bw->cursor);
  if(!cwhitel(d)) { pgetc(bw->cursor); goto lp; }
  }
 if(c!= MAXINT) pgetc(bw->cursor);
 /* Move to beginning of current word */
 while(crest(c=prgetc(bw->cursor)));
 if(c!= MAXINT) pgetc(bw->cursor);
 return 0;
 }

/* Move cursor to end of next word, next edge, or end of line */

int unxtwrd(bw)
BW *bw;
 {
 int c, d;
 if(piseof(bw->cursor)) return -1;
 /* Move to start of next word or edge */
 lp:
 d=' ';
 while(c=brc(bw->cursor),
       c!= MAXINT && !crest(c) && (!cwhitel(c) || cwhitel(d)))
  d=pgetc(bw->cursor);
 if(c==' ')
  {
  pgetc(bw->cursor); d=brc(bw->cursor); prgetc(bw->cursor);
  if(!cwhitel(d)) goto lp;
  }
 /* Move to end of current word */
 while(c=brc(bw->cursor), crest(c)) pgetc(bw->cursor);
 return 0;
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
 if(pisbol(p)) return -1;
 if(piseol(p)) return  1;
 q=pdup(p);
 pboi(q);
 if(q->byte==p->byte) goto left;
 if(cwhite(c=brc(p)))
  {
  pset(q,p); if(cwhite(prgetc(q))) goto no;
  if(c=='\t') goto right;
  pset(q,p); pgetc(q);
  if(pgetc(q)==' ') goto right;
  goto no;
  }
 else
  {
  pset(q,p); c=prgetc(q);
  if(c=='\t') goto left;
  if(c!=' ') goto no;
  if(prgetc(q)==' ') goto left;
  goto no;
  }

 right: prm(q); return 1;
 left: prm(q); return -1;
 no: prm(q); return 0;
 }

int upedge(bw)
BW *bw;
 {
 if(prgetc(bw->cursor)==MAXINT) return -1;
 while(pisedge(bw->cursor)!= -1) prgetc(bw->cursor);
 return 0;
 }

int unedge(bw)
BW *bw;
 {
 if(pgetc(bw->cursor)==MAXINT) return -1;
 while(pisedge(bw->cursor)!= 1) pgetc(bw->cursor);
 return 0;
 }

/* Move cursor to matching delimiter */

int utomatch(bw)
BW *bw;
 {
 int d;
 int c,		/* Character under cursor */
     f,		/* Character to find */
     dir;	/* 1 to search forward, -1 to search backward */
 
 switch(c=brc(bw->cursor))
  {
 case '(':  f=')';  dir=1;   break;
 case '[':  f=']';  dir=1;   break;
 case '{':  f='}';  dir=1;   break;
 case '`':  f='\''; dir=1;   break;
 case '<':  f='>';  dir=1;   break;
 case ')':  f='(';  dir= -1; break;
 case ']':  f='[';  dir= -1; break;
 case '}':  f='{';  dir= -1; break;
 case '\'': f='`';  dir= -1; break;
 case '>':  f='<';  dir= -1; break;
 default:   return -1;
  }
 
 if(dir==1)
  {
  P *p=pdup(bw->cursor);
  int cnt=0;	/* No. levels of delimiters we're in */
  while(d=pgetc(p), d!= MAXINT)
   if(d==c) ++cnt;
   else if(d==f && !--cnt) { prgetc(p); pset(bw->cursor,p); break; }
  prm(p);
  }
 else
  {
  P *p=pdup(bw->cursor);
  int cnt=0;	/* No. levels of delimiters we're in */
  while(d=prgetc(p), d!= MAXINT)
   if(d==c) ++cnt;
   else if(d==f) if(!cnt--) { pset(bw->cursor,p); break; }
  prm(p);
  }
 if(d==MAXINT) return -1;
 else return 0;
 }

/* Move cursor up */

int uuparw(bw)
BW *bw;
 {
 if(bw->cursor->line)
  {
  pprevl(bw->cursor);
  pcol(bw->cursor,bw->cursor->xcol);
  return 0;
  }
 else return -1;
 }

/* Move cursor down */

int udnarw(bw)
BW *bw;
 {
 if(bw->cursor->line!=bw->b->eof->line)
  {
  pnextl(bw->cursor);
  pcol(bw->cursor,bw->cursor->xcol);
  return 0;
  }
 else return -1;
 }

/* Move cursor to top of window */

int utos(bw)
BW *bw;
 {
 long col=bw->cursor->xcol;
 pset(bw->cursor,bw->top);
 pcol(bw->cursor,col);
 bw->cursor->xcol=col;
 return 0;
 }

/* Move cursor to bottom of window */

int ubos(bw)
BW *bw;
 {
 long col=bw->cursor->xcol;
 pline(bw->cursor,bw->top->line+bw->h-1);
 pcol(bw->cursor,col);
 bw->cursor->xcol=col;
 return 0;
 }

/* Scroll buffer window up n lines
 * If beginning of file is close, scrolls as much as it can
 * If beginning of file is on-screen, cursor jumps to beginning of file
 *
 * If flg is set: cursor stays fixed relative to screen edge
 * If flg is clr: cursor stays fixed on the buffer line
 */

void scrup(bw,n,flg)
BW *bw;
 {
 int scrollamnt=0;
 int cursoramnt=0;
 int x;
 
 /* Decide number of lines we're really going to scroll */
 
 if(bw->top->line>=n) scrollamnt=cursoramnt=n;
 else
  if(bw->top->line) scrollamnt=cursoramnt=bw->top->line;
  else
   if(flg) cursoramnt=bw->cursor->line;
   else if(bw->cursor->line>=n) cursoramnt=n;
 
 /* Move top-of-window pointer */
 for(x=0;x!=scrollamnt;++x) pprevl(bw->top);
 pbol(bw->top);
 
 /* Move cursor */
 for(x=0;x!=cursoramnt;++x) pprevl(bw->cursor);
 pbol(bw->cursor);
 pcol(bw->cursor,bw->cursor->xcol);
 
 /* If window is on the screen, give (buffered) scrolling command */
 if(bw->parent->y!= -1) nscrldn(bw->parent->t->t,bw->y,bw->y+bw->h,scrollamnt);
 }

/* Scroll buffer window down n lines
 * If end of file is close, scrolls as much as possible
 * If end of file is on-screen, cursor jumps to end of file
 *
 * If flg is set: cursor stays fixed relative to screen edge
 * If flg is clr: cursor stays fixed on the buffer line
 */

void scrdn(bw,n,flg)
BW *bw;
 {
 int scrollamnt=0;
 int cursoramnt=0;
 int x;
 
 /* How much we're really going to scroll... */
 if(bw->top->b->eof->line<bw->top->line+bw->h)
  {
  cursoramnt=bw->top->b->eof->line-bw->cursor->line;
  if(!flg && cursoramnt>n) cursoramnt=n;
  }
 else if(bw->top->b->eof->line-(bw->top->line+bw->h)>=n)
  cursoramnt=scrollamnt=n;
 else
  cursoramnt=scrollamnt=bw->top->b->eof->line-(bw->top->line+bw->h)+1;

 /* Move top-of-window pointer */
 for(x=0;x!=scrollamnt;++x) pnextl(bw->top);
 
 /* Move cursor */
 for(x=0;x!=cursoramnt;++x) pnextl(bw->cursor);
 pcol(bw->cursor,bw->cursor->xcol);
 
 /* If window is on screen, give (buffered) scrolling command to terminal */
 if(bw->parent->y!= -1) nscrlup(bw->parent->t->t,bw->y,bw->y+bw->h,scrollamnt);
 }

/* Page up */

int upgup(bw)
BW *bw;
 {
 bw=(BW *)bw->parent->main->object;
 if(!bw->cursor->line) return -1;
 if(pgamnt<0) scrup(bw,bw->h/2+bw->h%2,1);
 else if(pgamnt<bw->h) scrup(bw,bw->h-pgamnt,1);
 else scrup(bw,1,1);
 return 0;
 }

/* Page down */

int upgdn(bw)
BW *bw;
 {
 bw=(BW *)bw->parent->main->object;
 if(bw->cursor->line==bw->b->eof->line) return -1;
 if(pgamnt<0) scrdn(bw,bw->h/2+bw->h%2,1);
 else if(pgamnt<bw->h) scrdn(bw,bw->h-pgamnt,1);
 else scrdn(bw,1,1);
 return 0;
 }

/* Scroll by a single line.  The cursor moves with the scroll */

int uupslide(bw)
BW *bw;
 {
 bw=(BW *)bw->parent->main->object;
 if(bw->top->line)
  {
  if(bw->top->line+bw->h-1!=bw->cursor->line) udnarw(bw);
  scrup(bw,1,0);
  return 0;
  }
 else return -1;
 }

int udnslide(bw)
BW *bw;
 {
 bw=(BW *)bw->parent->main->object;
 if(bw->top->line+bw->h<=bw->top->b->eof->line)
  {
  if(bw->top->line!=bw->cursor->line) uuparw(bw);
  scrdn(bw,1,0);
  return 0;
  }
 else return -1;
 }

/* Move cursor to specified line number */

static B *linehist=0;	/* History of previously entered line numbers */

static int doline(bw,s,object,notify)
BW *bw;
char *s;
void *object;
int *notify;
 {
 long num=calc(bw,s);
 if(notify) *notify=1;
 vsrm(s);
 if(num>=1 && !merr)
  {
  int tmp=mid;
  if(num>bw->b->eof->line) num=bw->b->eof->line+1;
  pline(bw->cursor,num-1), bw->cursor->xcol=piscol(bw->cursor);
  mid=1; dofollows(); mid=tmp;
  return 0;
  }
 else
  {
  if(merr) msgnw(bw,merr);
  else msgnw(bw,"Invalid line number");
  return -1;
  }
 }

int uline(bw)
BW *bw;
 {
 if(wmkpw(bw,"Go to line (^C to abort): ",&linehist,doline,NULL,NULL,NULL,NULL,NULL)) return 0;
 else return -1;
 }

/* Move cursor to specified column number */

static B *colhist=0;	/* History of previously entered column numbers */

static int docol(bw,s,object,notify)
BW *bw;
char *s;
void *object;
int *notify;
 {
 long num=calc(bw,s);
 if(notify) *notify=1;
 vsrm(s);
 if(num>=1 && !merr)
  {
  int tmp=mid;
  pcol(bw->cursor,num-1), bw->cursor->xcol=piscol(bw->cursor);
  mid=1; dofollows(); mid=tmp;
  return 0;
  }
 else
  {
  if(merr) msgnw(bw,merr);
  else msgnw(bw,"Invalid column number");
  return -1;
  }
 }

int ucol(bw)
BW *bw;
 {
 if(wmkpw(bw,"Go to column (^C to abort): ",&colhist,docol,NULL,NULL,NULL,NULL,NULL)) return 0;
 else return -1;
 }

/* Move cursor to specified byte number */

static B *bytehist=0;	/* History of previously entered byte numbers */

static int dobyte(bw,s,object,notify)
BW *bw;
char *s;
void *object;
int *notify;
 {
 long num=calc(bw,s);
 if(notify) *notify=1;
 vsrm(s);
 if(num>=0 && !merr)
  {
  int tmp=mid;
  pgoto(bw->cursor,num), bw->cursor->xcol=piscol(bw->cursor);
  mid=1; dofollows(); mid=tmp;
  return 0;
  }
 else
  {
  if(merr) msgnw(bw,merr);
  else msgnw(bw,"Invalid byte number");
  return -1;
  }
 }

int ubyte(bw)
BW *bw;
 {
 if(wmkpw(bw,"Go to byte (^C to abort): ",&bytehist,dobyte,NULL,NULL,NULL,NULL,NULL)) return 0;
 else return -1;
 }

/* Delete character under cursor
 * or write ^D to process if we're at end of file in a shell window
 */

int udelch(bw)
BW *bw;
 {
 if(bw->pid && bw->cursor->byte==bw->b->eof->byte)
  {
  char c=4;
  write(bw->pid,&c,0);
  }
 else
  {
  P *p;
  if(piseof(bw->cursor)) return -1;
  pgetc(p=pdup(bw->cursor));
  bdel(bw->cursor,p);
  prm(p);
  }
 return 0;
 }

/* Backspace, or if cursor is at end of file in a shell window, send backspace
 * to shell */

int ubacks(bw,k)
BW *bw;
 {
 if(bw->pid && bw->cursor->byte==bw->b->eof->byte)
  {
  char c=k;
  write(bw->out,&c,1);
  }
 else if(bw->parent->watom->what==TYPETW || !pisbol(bw->cursor))
  {
  P *p;
  int c;
  if(pisbof(bw->cursor)) return -1;
  p=pdup(bw->cursor);
  if((c=prgetc(bw->cursor))!= MAXINT)
   if(!bw->o.overtype || c=='\t' || pisbol(p) || piseol(p))
    bdel(bw->cursor,p);
  prm(p);
  }
 else return -1;
 return 0;
 }

/* Delete from cursor to end of word it's on, to beginning of next word if
 * it's on whitespace or a single character if it's on neither a word nor
 * on whitespace
 */

int udelw(bw)
BW *bw;
 {
 P *p=pdup(bw->cursor);
 int c=brc(p);
 if(crest(c))
  while(c=brc(p), crest(c)) pgetc(p);
 else if(cwhitel(c) || c=='\r')
  while(c=brc(p), (cwhitel(c) || c=='\r')) pgetc(p);
 else pgetc(p);
 if(p->byte==bw->cursor->byte) { prm(p); return -1; }
 bdel(bw->cursor,p);
 prm(p);
 return 0;
 }

/* Delete from cursor to beginning of word it's in or immediately after,
 * to start of whitespace, or a single character
 */

int ubackw(bw)
BW *bw;
 {
 P *p=pdup(bw->cursor);
 int c=prgetc(bw->cursor);
 if(crest(c))
  {
  while(c=prgetc(bw->cursor), crest(c));
  if(c!= MAXINT) pgetc(bw->cursor);
  }
 else if(cwhitel(c))
  {
  while(c=prgetc(bw->cursor), cwhitel(c));
  if(c!= MAXINT) pgetc(bw->cursor);
  }
 if(bw->cursor->byte==p->byte) { prm(p); return -1; }
 bdel(bw->cursor,p);
 prm(p);
 return 0;
 }

/* Delete from cursor to end of line, or if there's nothing to delete,
 * delete the line-break
 */

int udelel(bw)
BW *bw;
 {
 P *p=peol(pdup(bw->cursor));
 if(bw->cursor->byte==p->byte) { prm(p); return udelch(bw); }
 else bdel(bw->cursor,p);
 prm(p);
 return 0;
 }

/* Delete to beginning of line, or if there's nothing to delete,
 * delete the line-break
 */

int udelbl(bw)
BW *bw;
 {
 P *p=pbol(pdup(bw->cursor));
 if(p->byte==bw->cursor->byte) { prm(p); return ubacks(bw,MAXINT); }
 else bdel(p,bw->cursor);
 prm(p);
 return 0;
 }

/* Delete entire line */

int udelln(bw)
BW *bw;
 {
 P *p=pdup(bw->cursor);
 pbol(bw->cursor);
 pnextl(p);
 if(bw->cursor->byte==p->byte) { prm(p); return -1; }
 bdel(bw->cursor,p);
 prm(p);
 return 0;
 }

/* Insert a space */

int uinsc(bw)
BW *bw;
 {
 binsc(bw->cursor,' ');
 return 0;
 }

/* Type a character into the buffer (deal with left margin, overtype mode and
 * word-wrap), if cursor is at end of shell window buffer, just send character
 * to process.
 */

int utypebw(bw,k)
BW *bw;
int k;
 {
 if(bw->pid && bw->cursor->byte==bw->b->eof->byte)
  {
  char c=k;
  write(bw->out,&c,1);
  }
 else if(k=='\t' && bw->o.spaces)
  {
  long n=piscol(bw->cursor);
  n=bw->o.tab-n%bw->o.tab;
  while(n--) utypebw(bw,' ');
  }
 else
  {
  int upd=bw->parent->t->t->updtab[bw->y+bw->cursor->line-bw->top->line];
  int simple=1;
  if(pisblank(bw->cursor))
   while(piscol(bw->cursor)<bw->o.lmargin)
    binsc(bw->cursor,' '), pgetc(bw->cursor);
  binsc(bw->cursor,k), pgetc(bw->cursor);
  if(bw->o.wordwrap && piscol(bw->cursor)>bw->o.rmargin && !cwhite(k))
   wrapword(bw->cursor,(long)bw->o.lmargin,bw->o.french,NULL), simple=0;
  else if(bw->o.overtype && !piseol(bw->cursor) && k!='\t') udelch(bw);
  bw->cursor->xcol=piscol(bw->cursor);
#ifndef __MSDOS__
  if(bw->cursor->xcol-bw->offset-1<0 || bw->cursor->xcol-bw->offset-1>=bw->w)
   simple=0;
  if(bw->cursor->line<bw->top->line ||
     bw->cursor->line>=bw->top->line+bw->h) simple=0;
  if(simple &&
     bw->parent->t->t->sary[bw->y+bw->cursor->line-bw->top->line]) simple=0;
  if(simple && k!='\t' && k!='\n' && !curmacro)
   {
   int c=0;
   SCRN *t=bw->parent->t->t;
   int y=bw->y+bw->cursor->line-bw->top->line;
   int x=bw->cursor->xcol-bw->offset+bw->x-1;
   int *screen=t->scrn+y*t->co;
   if(!upd && piseol(bw->cursor)) t->updtab[y]=0;
   if(markb && markk && markb->b==bw->b && markk->b==bw->b &&
      (!square && bw->cursor->byte>=markb->byte && bw->cursor->byte<markk->byte ||
       square && bw->cursor->line>=markb->line && bw->cursor->line<=markk->line &&
       piscol(bw->cursor)>=markb->xcol && piscol(bw->cursor)<markk->xcol))
    c=INVERSE;
   xlat(c,k);
   outatr(t,screen+x,x,y,k,c);
   }
#endif
  }
 return 0;
 }

/* Quoting */

int quotestate;
int quoteval;

int doquote(bw,c,object,notify)
BW *bw;
void *object;
int *notify;
 {
 char buf[40];
 if(c<0 || c>=256)
  {
  nungetc(c);
  return -1;
  }
 switch(quotestate)
  {
  case 0:
  if(c>='0' && c<='9')
   {
   quoteval=c-'0';
   quotestate=1;
   sprintf(buf,"ASCII %c--",c);
   if(!mkqwna(bw,sz(buf),doquote,NULL,NULL,notify)) return -1;
   else return 0;
   }
  else if(c=='x' || c=='X')
   {
   quotestate=3;
   if(!mkqwna(bw,sc("ASCII 0x--"),doquote,NULL,NULL,notify)) return -1;
   else return 0;
   }
  else if(c=='o' || c=='O')
   {
   quotestate=5;
   if(!mkqwna(bw,sc("ASCII 0---"),doquote,NULL,NULL,notify)) return -1;
   else return 0;
   }
  else
   {
   if((c>=0x40 && c<=0x5F) || (c>='a' && c<='z')) c&=0x1F;
   if(c=='?') c=127;
   utypebw(bw,c);
   bw->cursor->xcol=piscol(bw->cursor);
   }
  break;

  case 1:
  if(c>='0' && c<='9')
   {
   sprintf(buf,"ASCII %c%c-",quoteval+'0',c);
   quoteval=quoteval*10+c-'0';
   quotestate=2;
   if(!mkqwna(bw,sz(buf),doquote,NULL,NULL,notify)) return -1;
   else return 0;
   }
  break;

  case 2:
  if(c>='0' && c<='9')
   {
   quoteval=quoteval*10+c-'0';
   utypebw(bw,quoteval);
   bw->cursor->xcol=piscol(bw->cursor);
   }
  break;

  case 3:
  if(c>='0' && c<='9')
   {
   sprintf(buf,"ASCII 0x%c-",c);
   quoteval=c-'0';
   quotestate=4;
   if(!mkqwna(bw,sz(buf),doquote,NULL,NULL,notify)) return -1;
   else return 0;
   }
  else if(c>='a' && c<='f')
   {
   sprintf(buf,"ASCII 0x%c-",c+'A'-'a');
   quoteval=c-'a'+10;
   quotestate=4;
   if(!mkqwna(bw,sz(buf),doquote,NULL,NULL,notify)) return -1;
   else return 0;
   }
  else if(c>='A' && c<='F')
   {
   sprintf(buf,"ASCII 0x%c-",c);
   quoteval=c-'A'+10;
   quotestate=4;
   if(!mkqwna(bw,sz(buf),doquote,NULL,NULL,notify)) return -1;
   else return 0;
   }
  break;

  case 4:
  if(c>='0' && c<='9')
   {
   quoteval=quoteval*16+c-'0';
   utypebw(bw,quoteval);
   bw->cursor->xcol=piscol(bw->cursor);
   }
  else if(c>='a' && c<='f')
   {
   quoteval=quoteval*16+c-'a'+10;
   utypebw(bw,quoteval);
   bw->cursor->xcol=piscol(bw->cursor);
   }
  else if(c>='A' && c<='F')
   {
   quoteval=quoteval*16+c-'A'+10;
   utypebw(bw,quoteval);
   bw->cursor->xcol=piscol(bw->cursor);
   }
  break;

  case 5:
  if(c>='0' && c<='7')
   {
   sprintf(buf,"ASCII 0%c--",c);
   quoteval=c-'0';
   quotestate=6;
   if(!mkqwna(bw,sz(buf),doquote,NULL,NULL,notify)) return -1;
   else return 0;
   }
  break;

  case 6:
  if(c>='0' && c<='7')
   {
   sprintf(buf,"ASCII 0%c%c-",quoteval+'0',c);
   quoteval=quoteval*8+c-'0';
   quotestate=7;
   if(!mkqwna(bw,sz(buf),doquote,NULL,NULL,notify)) return -1;
   else return 0;
   }
  break;

  case 7:
  if(c>='0' && c<='7')
   {
   quoteval=quoteval*8+c-'0';
   utypebw(bw,quoteval);
   bw->cursor->xcol=piscol(bw->cursor);
   }
  break;
  }
 if(notify) *notify=1;
 return 0;
 }

int uquote(bw)
BW *bw;
 {
 quotestate=0;
 if(mkqwna(bw,
           sc("Ctrl- (or 0-9 for dec. ascii, x for hex, or o for octal)"),doquote,NULL,NULL,NULL)) return 0;
 else return -1;
 }

int doquote9(bw,c,object,notify)
BW *bw;
void *object;
int *notify;
 {
 if(notify) *notify=1;
 if((c>=0x40 && c<=0x5F) || (c>='a' && c<='z')) c&=0x1F;
 if(c=='?') c=127;
 c|=128;
 utypebw(bw,c);
 bw->cursor->xcol=piscol(bw->cursor);
 return 0;
 }

int doquote8(bw,c,object,notify)
BW *bw;
void *object;
int *notify;
 {
 if(c=='`')
  {
  if(mkqwna(bw,sc("Meta-Ctrl-"),doquote9,NULL,NULL,notify)) return 0;
  else return -1;
  }
 if(notify) *notify=1;
 c|=128;
 utypebw(bw,c);
 bw->cursor->xcol=piscol(bw->cursor);
 return 0;
 }

int uquote8(bw)
BW *bw;
 {
 if(mkqwna(bw,sc("Meta-"),doquote8,NULL,NULL,NULL)) return 0;
 else return -1;
 }

extern char srchstr[];

int doctrl(bw,c,object,notify)
BW *bw;
void *object;
int *notify;
 {
 int org=bw->o.overtype;
 if(notify) *notify=1;
 bw->o.overtype=0;
 if(bw->parent->huh==srchstr && c=='\n')
   utypebw(bw,'\\'), utypebw(bw,'n');
 else
  utype(bw,c);
 bw->o.overtype=org;
 bw->cursor->xcol=piscol(bw->cursor);
 return 0;
 }

int uctrl(bw)
BW *bw;
 {
 if(mkqwna(bw,sc("Quote"),doctrl,NULL,NULL,NULL)) return 0;
 else return -1;
 }

/* User hit Return.  Deal with autoindent.  If cursor is at end of shell
 * window buffer, send the return to the shell
 */

int rtntw(bw)
BW *bw;
 {
 if(bw->pid && bw->cursor->byte==bw->b->eof->byte)
  {
  write(bw->out,"\n",1);
  }
 else
  {
  P *p=pdup(bw->cursor);
  char c;
  binsc(bw->cursor,'\n'), pgetc(bw->cursor);
  if(bw->o.autoindent)
   {
   pbol(p);
   while(cwhite(c=pgetc(p)))
    binsc(bw->cursor,c), pgetc(bw->cursor);
   }
  prm(p);
  bw->cursor->xcol=piscol(bw->cursor);
  }
 return 0;
 }

/* Open a line */

int uopen(bw)
BW *bw;
 {
 P *q=pdup(bw->cursor);
 rtntw(bw);
 pset(bw->cursor,q);
 prm(q);
 return 0;
 }

/* Set book-mark */

int dosetmark(bw,c,object,notify)
BW *bw;
void *object;
int *notify;
 {
 if(notify) *notify=1;
 if(c>='0' && c<='9')
  {
  pdupown(bw->cursor,bw->b->marks+c-'0');
  poffline(bw->b->marks[c-'0']);
  sprintf(msgbuf,"Mark %d set",c-'0');
  msgnw(bw,msgbuf);
  return 0;
  }
 else
  {
  nungetc(c);
  return -1;
  }
 }

int usetmark(bw,c)
BW *bw;
 {
 if(c>='0' && c<='9') return dosetmark(bw,c,NULL,NULL);
 else
  if(mkqwna(bw,sc("Set mark (0-9):"),dosetmark,NULL,NULL,NULL)) return 0;
  else return -1;
 }

/* Goto book-mark */

int dogomark(bw,c,object,notify)
BW *bw;
void *object;
int *notify;
 {
 if(notify) *notify=1;
 if(c>='0' && c<='9')
  if(bw->b->marks[c-'0'])
   {
   pset(bw->cursor,bw->b->marks[c-'0']);
   bw->cursor->xcol=piscol(bw->cursor);
   return 0;
   }
  else
   {
   sprintf(msgbuf,"Mark %d not set",c-'0');
   msgnw(bw,msgbuf);
   return -1;
   }
 else
  {
  nungetc(c);
  return -1;
  }
 }

int ugomark(bw,c)
BW *bw;
 {
 if(c>='0' && c<='9') return dogomark(bw,c,NULL,NULL);
 else
  if(mkqwna(bw,sc("Goto bookmark (0-9):"),dogomark,NULL,NULL,NULL)) return 0;
  else return -1;
 }

/* Goto next instance of character */

static int dobkwdc;

int dofwrdc(bw,k,object,notify)
BW *bw;
void *object;
int *notify;
 {
 int c;
 P *q;
 if(notify) *notify=1;
 if(k<0 || k>=256)
  {
  nungetc(k);
  return -1;
  }
 q=pdup(bw->cursor);
 if(dobkwdc) { while((c=prgetc(q))!=MAXINT) if(c==k) break; }
 else { while((c=pgetc(q))!=MAXINT) if(c==k) break; }
 if(c==MAXINT)
  {
  msgnw(bw,"Not found");
  prm(q);
  return -1;
  }
 else
  {
  pset(bw->cursor,q);
  prm(q);
  return 0;
  }
 }

int ufwrdc(bw,k)
BW *bw;
 {
 dobkwdc=0;
 if(k>=0 && k<256) return dofwrdc(bw,k,NULL,NULL);
 else if(mkqw(bw,sc("Fwrd to char: "),dofwrdc,NULL,NULL,NULL)) return 0;
 else return -1;
 }

int ubkwdc(bw,k)
BW *bw;
 {
 dobkwdc=1;
 if(k>=0 && k<256) return dofwrdc(bw,k,NULL,NULL);
 else if(mkqw(bw,sc("Bkwd to char: "),dofwrdc,NULL,NULL,NULL)) return 0;
 else return -1;
 }

/* Display a message */

int domsg(b,s,object,notify)
BASE *b;
char *s;
void *object;
int *notify;
 {
 if(notify) *notify=1;
 zcpy(msgbuf,s);
 vsrm(s);
 msgnw(b,msgbuf);
 return 0;
 }

int umsg(b)
BASE *b;
 {
 if(wmkpw(b,"Msg (^C to abort): ",NULL,domsg,NULL,NULL,NULL,NULL,NULL)) return 0;
 else return -1;
 }

/* Insert text */

int dotxt(bw,s,object,notify)
BW *bw;
char *s;
void *object;
int *notify;
 {
 int x;
 if(notify) *notify=1;
 for(x=0;x!=sLEN(s);++x) utypebw(bw,s[x]);
 vsrm(s);
 return 0;
 }

int utxt(bw)
BASE *bw;
 {
 if(wmkpw(bw,"Insert (^C to abort): ",NULL,dotxt,NULL,NULL,utypebw,NULL,NULL)) return 0;
 else return -1;
 }
