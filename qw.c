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
#include "heap.h"
#include "w.h"
#include "zstr.h"
#include "qw.h"

CONTEXT cquery={"query",0}, cquerya={"querya",0}, cquerysr={"querysr",0};

/* Move query window */

static void moveqw(w,x,y)
W *w;
int x,y;
{
}

/* Resize query window */

static void resizeqw(w,wi,he)
W *w;
int wi, he;
{
}

/* Abort query window */

static void killqw(w)
W *w;
{
QW *qw=(QW *)w->object;
free(qw->prompt);
free(qw);
}

/* Update query window */

static void followqw(w)
W *w;
{
}

static void dispqw(w)
W *w;
{
int x;
QW *qw=(QW *)w->object;

/* Scroll buffer and position prompt */
if(qw->promptlen>w->w/2+w->w/4) qw->promptofst=qw->promptlen-w->w/2;
else qw->promptofst=0;

/* Set cursor position */
w->curx=qw->promptlen-qw->promptofst;
w->cury=0;

/* Generate prompt */
w->t->t->updtab[w->y]=1;
genfmt(w->t->t,w->x,w->y,qw->promptofst,qw->prompt,1);
}

static void dispqwn(w)
W *w;
{
int x;
QW *qw=(QW *)w->object;

/* Scroll buffer and position prompt */
if(qw->promptlen>w->w/2+w->w/4) qw->promptofst=qw->promptlen-w->w/2;
else qw->promptofst=0;

/* Set cursor position */
w->win->watom->follow(w->win);
w->win->watom->disp(w->win);
w->curx=w->win->curx;
w->cury=w->win->cury+w->win->y-w->y;

/* Generate prompt */
w->t->t->updtab[w->y]=1;
genfmt(w->t->t,w->x,w->y,qw->promptofst,qw->prompt,1);
}

/* When user hits a key in a query window */

void utypeqw(w,c)
W *w;
{
QW *qw=(QW *)w->object;
W *win;
void (*func)();
win=w->win;
func=qw->func;
wabort(w);
func(win,c);
}

/* When user aborts a query window with ^C */

void uabortqw(w)
W *w;
{
wabort(w);
}

void qdumb()
{
}

static WATOM watomqw=
{
&cquery,
dispqw,
followqw,
killqw,
resizeqw,
moveqw,
qdumb,
qdumb,
TYPEQW
};

static WATOM watqwn=
{
&cquerya,
dispqwn,
followqw,
killqw,
resizeqw,
moveqw,
qdumb,
qdumb,
TYPEQW
};

static WATOM watqwsr=
{
&cquerysr,
dispqwn,
followqw,
killqw,
resizeqw,
moveqw,
qdumb,
qdumb,
TYPEQW
};

/* Create a query window */

W *mkqw(w,prompt,func)
W *w;
char *prompt;
void (*func)();
{
W *new;
QW *qw;
new=wcreate(w->t,&watomqw,w,w,w->main,1,NULL);
if(!new) return 0;
new->object=(void *)(qw=(QW *)malloc(sizeof(QW)));
qw->prompt=zdup(prompt);
qw->promptlen=fmtlen(prompt);
qw->promptofst=0;
qw->func=func;
w->t->curwin=new;
return new;
}

/* Same as above, but cursor is left in original window */
/* For Ctrl-Meta thing */

W *mkqwna(w,prompt,func)
W *w;
char *prompt;
void (*func)();
{
W *new;
QW *qw;
new=wcreate(w->t,&watqwn,w,w,w->main,1,NULL);
if(!new) return 0;
new->object=(void *)(qw=(QW *)malloc(sizeof(QW)));
qw->prompt=zdup(prompt);
qw->promptlen=fmtlen(prompt);
qw->promptofst=0;
qw->func=func;
w->t->curwin=new;
return new;
}

/* Same as above, but cursor is left in original window */
/* For search and replace thing */

W *mkqwnsr(w,prompt,func)
W *w;
char *prompt;
void (*func)();
{
W *new;
QW *qw;
new=wcreate(w->t,&watqwsr,w,w,w->main,1,NULL);
if(!new) return 0;
new->object=(void *)(qw=(QW *)malloc(sizeof(QW)));
qw->prompt=zdup(prompt);
qw->promptlen=fmtlen(prompt);
qw->promptofst=0;
qw->func=func;
w->t->curwin=new;
return new;
}
