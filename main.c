 /* Editor startup and main edit loop
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
#include <fcntl.h>
#include <string.h>
#include "config.h"
#include "w.h"
#include "tty.h"
#include "help.h"
#include "rc.h"
#include "vfile.h"
#include "b.h"
#include "bw.h"
#include "tw.h"
#include "kbd.h"
#include "macro.h"
#include "vs.h"
#include "path.h"
#include "termcap.h"
#include "main.h"

extern int mid, dspasis, force, help, pgamnt, nobackups, lightoff,
           exask, skiptop, noxon, lines, staen, columns, Baud, dopadding,
           marking, beep;

extern int idleout;	/* Clear to use /dev/tty for screen */
extern char *joeterm;
int help=0;		/* Set to have help on when starting */
int nonotice=0;		/* Set to prevent copyright notice */
int orphan=0;
char *exmsg=0;		/* Message to display when exiting the editor */

SCREEN *maint;		/* Main edit screen */

/* Make windows follow cursor */

void dofollows()
 {
 W *w=maint->curwin;
 do
  {
  if(w->y!= -1 && w->watom->follow && w->object) w->watom->follow(w->object);
  w=(W *)(w->link.next);
  }
  while(w!=maint->curwin);
 }

/* Update screen */

int dostaupd=1;
extern int staupd;

void edupd(flg)
 {
 W *w;
 int wid,hei;
 if(dostaupd) staupd=1, dostaupd=0;
 ttgtsz(&wid,&hei);
 if(wid>=2 && wid!=maint->w ||
    hei>=1 && hei!=maint->h)
  {
  nresize(maint->t,wid,hei);
  sresize(maint);
  }
 dofollows();
 ttflsh();
 nscroll(maint->t);
 dsphlp(maint);
 w=maint->curwin; do
  {
  if(w->y!= -1)
   {
   if(w->object && w->watom->disp) w->watom->disp(w->object,flg);
   msgout(w);
   }
  w=(W *)(w->link.next);
  }
  while(w!=maint->curwin);
 cpos(maint->t,
      maint->curwin->x+maint->curwin->curx,
      maint->curwin->y+maint->curwin->cury);
 staupd=0;
 }

static int ahead=0;
static int ungot=0;
static int ungotc=0;

void nungetc(c)
 {
 if(c!='C'-'@' && c!='M'-'@')
  {
  chmac();
  ungot=1;
  ungotc=c;
  }
 }

int edloop(flg)
 {
 int term=0;
 int ret=0;
 SCRN *n=maint->t;
 if(flg)
  if(maint->curwin->watom->what==TYPETW) return 0;
  else maint->curwin->notify= &term;
 while(!leave && (!flg || !term))
  {
  MACRO *m;
  int c;
  if(exmsg && !flg)
   {
   vsrm(exmsg);
   exmsg=0;
   }
  edupd(1);
  if(!ahead && !have) ahead=1;
  if(ungot) c=ungotc, ungot=0;
  else c=ttgetc();
  if(!ahead && c==10) c=13;
  m=dokey(maint->curwin->kbd,c);
  if(maint->curwin->main && maint->curwin->main!=maint->curwin)
   {
   int x=maint->curwin->kbd->x;
   maint->curwin->main->kbd->x=x;
   if(x)
    maint->curwin->main->kbd->seq[x-1]=maint->curwin->kbd->seq[x-1];
   }
  if(m) ret=exemac(m);
  }
 if(term== -1) return -1;
 else return ret;
 }

#ifdef __MSDOS__
extern void setbreak();
extern int breakflg;
#endif

char **mainenv;

int main(argc,argv,envv)
int argc;
char *argv[];
char *envv[];
 {
 CAP *cap;
 char *s;
 char *run;
 char *rundir;
 SCRN *n;
 int opened=0;
 int omid;
 int backopt;
 int c;

 mainenv=envv;

#ifdef __MSDOS__
 _fmode=O_BINARY;
 strcpy(stdbuf,argv[0]);
 joesep(stdbuf);
 run=namprt(stdbuf);
 rundir=dirprt(stdbuf);
 for(c=0;run[c];++c)
  if(run[c]=='.')
   {
   run=vstrunc(run,c);
   break;
   }
#else
 run=namprt(argv[0]);
#endif 

 if(s=getenv("LINES")) sscanf(s,"%d",&lines);
 if(s=getenv("COLUMNS")) sscanf(s,"%d",&columns);
 if(s=getenv("BAUD")) sscanf(s,"%u",&Baud);
 if(getenv("DOPADDING")) dopadding=1;
 if(getenv("NOXON")) noxon=1;
 if(s=getenv("JOETERM")) joeterm=s;

#ifndef __MSDOS__
 if(!(cap=getcap(NULL,9600,NULL,NULL)))
  {
  fprintf(stderr,"Couldn't load termcap/terminfo entry\n");
  return 1;
  }
#endif

#ifdef __MSDOS__

 s=vsncpy(NULL,0,sv(run));
 s=vsncpy(sv(s),sc("rc"));
 c=procrc(cap,s);
 if(c==0) goto donerc;
 if(c==1)
  {
  char buf[8];
  fprintf(stderr,"There were errors in '%s'.  Use it anyway?",s);
  fflush(stderr);
  fgets(buf,8,stdin);
  if(buf[0]=='y' || buf[0]=='Y') goto donerc;
  }

 vsrm(s);
 s=vsncpy(NULL,0,sv(rundir));
 s=vsncpy(sv(s),sv(run));
 s=vsncpy(sv(s),sc("rc"));
 c=procrc(cap,s);
 if(c==0) goto donerc;
 if(c==1)
  {
  char buf[8];
  fprintf(stderr,"There were errors in '%s'.  Use it anyway?",s);
  fflush(stderr);
  fgets(buf,8,stdin);
  if(buf[0]=='y' || buf[0]=='Y') goto donerc;
  }

#else

 s=vsncpy(NULL,0,sc("."));
 s=vsncpy(sv(s),sv(run));
 s=vsncpy(sv(s),sc("rc"));
 c=procrc(cap,s);
 if(c==0) goto donerc;
 if(c==1)
  {
  char buf[8];
  fprintf(stderr,"There were errors in '%s'.  Use it anyway?",s);
  fflush(stderr);
  fgets(buf,8,stdin);
  if(buf[0]=='y' || buf[0]=='Y') goto donerc;
  }

 vsrm(s);
 s=getenv("HOME");
 if(s)
  {
  s=vsncpy(NULL,0,sz(s));
  s=vsncpy(sv(s),sc("/."));
  s=vsncpy(sv(s),sv(run));
  s=vsncpy(sv(s),sc("rc"));
  c=procrc(cap,s);
  if(c==0) goto donerc;
  if(c==1)
   {
   char buf[8];
   fprintf(stderr,"There were errors in '%s'.  Use it anyway?",s);
   fflush(stderr);
   fgets(buf,8,stdin);
   if(buf[0]=='y' || buf[0]=='Y') goto donerc;
   }
  }

 vsrm(s);
 s=vsncpy(NULL,0,sc(JOERC));
 s=vsncpy(sv(s),sv(run));
 s=vsncpy(sv(s),sc("rc"));
 c=procrc(cap,s);
 if(c==0) goto donerc;
 if(c==1)
  {
  char buf[8];
  fprintf(stderr,"There were errors in '%s'.  Use it anyway?",s);
  fflush(stderr);
  fgets(buf,8,stdin);
  if(buf[0]=='y' || buf[0]=='Y') goto donerc;
  }

#endif

 fprintf(stderr,"Couldn't open '%s'\n",s);
 return 1;

 donerc:
 izhelp();
 for(c=1;argv[c];++c)
  if(argv[c][0]=='-')
   if(argv[c][1])
    switch(glopt(argv[c]+1,argv[c+1],NULL,1))
     {
     case 0: fprintf(stderr,"Unknown option '%s'\n",argv[c]); break;
     case 1: break;
     case 2: ++c; break;
     }
   else idleout=0;

 if(!(n=nopen(cap))) return 1;
 maint=screate(n);
 vmem=vtmp();

 for(c=1,backopt= 0;argv[c];++c)
  if(argv[c][0]=='+' && argv[c][1])
   {
   if(!backopt) backopt=c;
   }
  else if(argv[c][0]=='-' && argv[c][1])
   {
   if(!backopt) backopt=c;
   if(glopt(argv[c]+1,argv[c+1],NULL,0)==2) ++c;
   }
  else
   {
   B *b=bfind(argv[c]);
   BW *bw=0;
   int er=error;
   if(!orphan || !opened)
    {
    bw=wmktw(maint,b);
    if(er) msgnwt(bw,msgs[5+er]);
    }
   else b->orphan=1;
   if(bw)
    {
    long lnum=0;
    bw->o.readonly=bw->b->rdonly;
    if(backopt) while(backopt!=c)
     if(argv[backopt][0]=='+')
      {
      sscanf(argv[backopt]+1,"%ld",&lnum);
      ++backopt;
      }
     else
      if(glopt(argv[backopt]+1,argv[backopt+1],&bw->o,0)==2) backopt+=2;
      else backopt+=1;
    bw->b->o=bw->o;
    bw->b->rdonly=bw->o.readonly;
    maint->curwin=bw->parent;
    if(er== -1 && bw->o.mnew) exemac(bw->o.mnew);
    if(er==0 && bw->o.mold) exemac(bw->o.mold);
    if(lnum>0) pline(bw->cursor,lnum-1);
    }
   opened=1;
   backopt=0;
   }

 if(opened)
  {
  wshowall(maint);
  omid=mid; mid=1;
  dofollows();
  mid=omid;
  }
 else
  {
  BW *bw=wmktw(maint,bfind(""));
  if(bw->o.mnew) exemac(bw->o.mnew);
  }
 maint->curwin=maint->topwin;
 if(help) helpon(maint);
 if(!nonotice)
  msgnw(lastw(maint)->object,"\\i** Joe's Own Editor v2.8 ** Copyright (C) 1995 Joseph H. Allen **\\i");
 edloop(0);
 vclose(vmem);
 nclose(n);
 if(exmsg) fprintf(stderr,"\n%s\n",exmsg);
 return 0;
 }
