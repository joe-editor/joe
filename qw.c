/* Query windows
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
#include "zstr.h"
#include "vs.h"
#include "qw.h"

static void dispqw(qw)
QW *qw;
 {
 W *w=qw->parent;

 /* Scroll buffer and position prompt */
 if(qw->promptlen>w->w/2+w->w/4) qw->promptofst=qw->promptlen-w->w/2;
 else qw->promptofst=0;

 /* Set cursor position */
 w->curx=qw->promptlen-qw->promptofst;
 w->cury=0;

 /* Generate prompt */
 w->t->t->updtab[w->y]=1;
 gentxt(w->t->t,w->x,w->y,qw->promptofst,qw->prompt,qw->promptlen,1);
 }

static void dispqwn(qw)
QW *qw;
 {
 W *w=qw->parent;

 /* Scroll buffer and position prompt */
 if(qw->promptlen>w->w/2+w->w/4) qw->promptofst=qw->promptlen-w->w/2;
 else qw->promptofst=0;

 /* Set cursor position */
 if(w->win->watom->follow && w->win->object) w->win->watom->follow(w->win->object);
 if(w->win->watom->disp && w->win->object) w->win->watom->disp(w->win->object);
 w->curx=w->win->curx;
 w->cury=w->win->cury+w->win->y-w->y;

 /* Generate prompt */
 w->t->t->updtab[w->y]=1;
 gentxt(w->t->t,w->x,w->y,qw->promptofst,qw->prompt,qw->promptlen,1);
 }

/* When user hits a key in a query window */

int utypeqw(qw,c)
QW *qw;
 {
 W *win;
 W *w=qw->parent;
 int *notify=w->notify;
 int (*func)();
 void *object=qw->object;
 win=qw->parent->win;
 func=qw->func;
 vsrm(qw->prompt);
 free(qw);
 w->object=0;
 w->notify=0;
 wabort(w);
 if(func) return func(win->object,c,object,notify);
 return -1;
 }

static int abortqw(qw)
QW *qw;
 {
 W *win=qw->parent->win;
 void *object=qw->object;
 int (*abrt)()=qw->abrt;
 vsrm(qw->prompt);
 free(qw);
 if(abrt) return abrt(win->object,object);
 else return -1;
 }

static WATOM watomqw=
 {
 "query",
 dispqw,
 0,
 abortqw,
 0,
 utypeqw,
 0,
 0,
 0,
 0,
 TYPEQW
 };

static WATOM watqwn=
 {
 "querya",
 dispqwn,
 0,
 abortqw,
 0,
 utypeqw,
 0,
 0,
 0,
 0,
 TYPEQW
 };

static WATOM watqwsr=
 {
 "querysr",
 dispqwn,
 0,
 abortqw,
 0,
 utypeqw,
 0,
 0,
 0,
 0,
 TYPEQW
 };

/* Create a query window */

QW *mkqw(obw,prompt,len,func,abrt,object,notify)
BASE *obw;
char *prompt;
int (*func)();
int (*abrt)();
void *object;
int *notify;
 {
 W *new;
 QW *qw;
 W *w=obw->parent;
 new=wcreate(w->t,&watomqw,w,w,w->main,1,NULL,notify);
 if(!new)
  {
  if(notify) *notify=1;
  return 0;
  }
 wfit(new->t);
 new->object=(void *)(qw=(QW *)malloc(sizeof(QW)));
 qw->parent=new;
 qw->prompt=vsncpy(NULL,0,prompt,len);
 qw->promptlen=len;
 qw->promptofst=0;
 qw->func=func;
 qw->abrt=abrt;
 qw->object=object;
 w->t->curwin=new;
 return qw;
 }

/* Same as above, but cursor is left in original window */
/* For Ctrl-Meta thing */

QW *mkqwna(obw,prompt,len,func,abrt,object,notify)
BASE *obw;
char *prompt;
int (*func)();
int (*abrt)();
void *object;
int *notify;
 {
 W *new;
 QW *qw;
 W *w=obw->parent;
 new=wcreate(w->t,&watqwn,w,w,w->main,1,NULL,notify);
 if(!new)
  {
  if(notify) *notify=1;
  return 0;
  }
 wfit(new->t);
 new->object=(void *)(qw=(QW *)malloc(sizeof(QW)));
 qw->parent=new;
 qw->prompt=vsncpy(NULL,0,prompt,len);
 qw->promptlen=len;
 qw->promptofst=0;
 qw->func=func;
 qw->abrt=abrt;
 qw->object=object;
 w->t->curwin=new;
 return qw;
 }

/* Same as above, but cursor is left in original window */
/* For search and replace thing */

QW *mkqwnsr(obw,prompt,len,func,abrt,object,notify)
BASE *obw;
char *prompt;
int (*func)();
int (*abrt)();
void *object;
int *notify;
 {
 W *new;
 QW *qw;
 W *w=obw->parent;
 new=wcreate(w->t,&watqwsr,w,w,w->main,1,NULL,notify);
 if(!new)
  {
  if(notify) *notify=1;
  return 0;
  }
 wfit(new->t);
 new->object=(void *)(qw=(QW *)malloc(sizeof(QW)));
 qw->parent=new;
 qw->prompt=vsncpy(NULL,0,prompt,len);
 qw->promptlen=len;
 qw->promptofst=0;
 qw->func=func;
 qw->abrt=abrt;
 qw->object=object;
 w->t->curwin=new;
 return qw;
 }
