/* GPM/xterm mouse functions
   Copyright (C) 1999 Jesse McGrew

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

#include <sys/time.h>

#include "config.h"
#include "b.h"
#include "bw.h"
#include "w.h"
#include "qw.h"
#include "tw.h"
#include "pw.h"
#include "vs.h"
#include "kbd.h"
#include "macro.h"
#include "main.h"
#include "ublock.h"
#include "mouse.h"

int rtbutton=0;			/* use button 3 instead of 1 */
int floatmouse=0;		/* don't fix xcol after tomouse */

static int xtmstate;
static int Cb, Cx, Cy;
static int last_msec=0;		/* time in ms when event occurred */
static int clicks;

int uxtmouse(BW *bw)
 {
 Cb=ttgetc()-32;
 if(Cb<0) return -1;
 Cx=ttgetc()-32;
 if(Cx<=0) return -1;
 Cy=ttgetc()-32;
 if(Cy<=0) return -1;

 if((Cb&3)==3)
  /* button released */
  mouseup(Cx,Cy);
 else if((Cb&3)==(rtbutton?2:0))	/* preferred button */
  if((Cb&32)==0)
   /* button pressed */
   mousedn(Cx,Cy);
  else
   /* drag */
   mousedrag(Cx,Cy);
 }

static void fake_key(int c)
 {
 MACRO *m=dokey(maint->curwin->kbd,c);
 int x=maint->curwin->kbd->x;
 maint->curwin->main->kbd->x=x;
 if(x)
  maint->curwin->main->kbd->seq[x-1]=maint->curwin->kbd->seq[x-1];
 if(m)
  exemac(m);
 }

static int mnow()
 {
 struct timeval tv;
 gettimeofday(&tv, NULL);
 return tv.tv_sec * 1000 + tv.tv_usec / 1000;
 }

void mousedn(int x,int y)
 {
 Cx=x, Cy=y;
 if(last_msec==0 || mnow()-last_msec>MOUSE_MULTI_THRESH)
  {
  /* not a multiple click */
  clicks=1;
  fake_key(KEY_MDOWN);
  }
 else if(clicks==1)
  {
  /* double click */
  clicks=2;
  fake_key(KEY_M2DOWN);
  }
 else if(clicks=2)
  {
  /* triple click */
  clicks=3;
  fake_key(KEY_M3DOWN);
  }
 else
  {
  /* start over */
  clicks=1;
  fake_key(KEY_MDOWN);
  }
}

void mouseup(int x,int y)
 {
 struct timeval tv;
 Cx=x, Cy=y;
 switch(clicks)
  {
  case 1:
  fake_key(KEY_MUP);
  break;
  
  case 2:
  fake_key(KEY_M2UP);
  break;
  
  case 3:
  fake_key(KEY_M3UP);
  break;
  }
 last_msec=mnow();
 }

void mousedrag(int x,int y)
 {
 Cx=x, Cy=y;
 switch(clicks)
  {
  case 1:
  fake_key(KEY_MDRAG);
  break;
  
  case 2:
  fake_key(KEY_M2DRAG);
  break;
  
  case 3:
  fake_key(KEY_M3DRAG);
  break;
  }
 }

int utomouse(BW *xx)
 {
 BW *bw;
 int x=Cx-1,y=Cy-1;
 W *w=watpos(maint,x,y);
 if(!w) return -1;
 maint->curwin=w;
 bw=w->object;
 if(w->watom->what==TYPETW)
  {
  /* window has a status line? */
  if(((TW *)bw->object)->staon)
   /* clicked on it? */
   if(y==w->y) return -1;
   else pline(bw->cursor,y-w->y+bw->top->line-1);
  else pline(bw->cursor,y-w->y+bw->top->line);
  pcol(bw->cursor,x-w->x+bw->offset);
  if(floatmouse) bw->cursor->xcol=x-w->x+bw->offset;
  else bw->cursor->xcol=piscol(bw->cursor);
  return 0;
  }
 else if(w->watom->what==TYPEPW)
  {
  PW *pw=(PW *)bw->object;
  /* only one line in prompt windows */
  pcol(bw->cursor,x-w->x+bw->offset-pw->promptlen+pw->promptofst);
  bw->cursor->xcol=piscol(bw->cursor);
  }
 else return -1;
 }

/* same as utomouse but won't change windows, and always floats. puts the
 * position that utomouse would use into tmspos. */
static int tmspos;

static int tomousestay()
 {
 BW *bw;
 int x=Cx-1,y=Cy-1;
 W *w=watpos(maint,x,y);
 if(!w || w!=maint->curwin) return -1;
 bw=w->object;
 if(w->watom->what==TYPETW)
  {
  /* window has a status line? */
  if(((TW *)bw->object)->staon)
   /* clicked on it? */
   if(y==w->y) return -1;
   else pline(bw->cursor,y-w->y+bw->top->line-1);
  else pline(bw->cursor,y-w->y+bw->top->line);
  pcol(bw->cursor,x-w->x+bw->offset);
  tmspos=bw->cursor->xcol=x-w->x+bw->offset;
  if(!floatmouse) tmspos=piscol(bw->cursor);
  return 0;
  }
 else if(w->watom->what==TYPEPW)
  {
  PW *pw=(PW *)bw->object;
  /* only one line in prompt windows */
  pcol(bw->cursor,x-w->x+bw->offset-pw->promptlen+pw->promptofst);
  tmspos=bw->cursor->xcol=piscol(bw->cursor);
  }
 else return -1;
 }

static long anchor;		/* byte where mouse was originally pressed */
static long anchorn;		/* near side of the anchored word */
static int marked;		/* mark was set by defmdrag? */
static int reversed;		/* mouse was dragged above the anchor? */

int udefmdown(BW *xx)
 {
 BW *bw;
 if(utomouse(xx)) return -1;
 if((maint->curwin->watom->what&(TYPEPW|TYPETW))==0) return 0;
 bw=(BW *)maint->curwin->object;
 anchor=bw->cursor->byte;
 marked=reversed=0;
 }

int udefmdrag(BW *xx)
 {
 BW *bw=(BW *)maint->curwin->object;
 if(!marked) marked++, umarkb(bw);
 if(tomousestay()) return -1;
 if(reversed) umarkb(bw);
 else umarkk(bw);
 if((!reversed && bw->cursor->byte<anchor) ||
    (reversed && bw->cursor->byte>anchor))
  {
  P *q=pdup(markb);
  int tmp=markb->xcol;
  pset(markb,markk);
  pset(markk,q);
  markb->xcol=markk->xcol;
  markk->xcol=tmp;
  prm(q);
  reversed=!reversed;
  }
 bw->cursor->xcol=tmspos;
 }

int udefmup(BW *bw)
 {
 }

int udefm2down(BW *xx)
 {
 BW *bw;
 if(utomouse(xx)) return -1;
 if((maint->curwin->watom->what&(TYPEPW|TYPETW))==0) return 0;
 bw=(BW *)maint->curwin->object;
 /* set anchor to left side, anchorn to right side */
 u_goto_prev(bw); anchor=bw->cursor->byte; umarkb(bw); markb->xcol=piscol(markb);
 u_goto_next(bw); anchorn=bw->cursor->byte; umarkk(bw); markk->xcol=piscol(markk);
 reversed=0;
 bw->cursor->xcol=piscol(bw->cursor);
 }

int udefm2drag(BW *xx)
{
	BW *bw=(BW *)maint->curwin->object;
	if(tomousestay()) return -1;
	if(!reversed && bw->cursor->byte<anchor)
	 {
	 pgoto(markk,anchorn);
	 markk->xcol=piscol(markk);
	 reversed=1;
	 }
	else if(reversed && bw->cursor->byte>anchorn)
	 {
	 pgoto(markb,anchor);
	 markb->xcol=piscol(markb);
	 reversed=0;
	 }
	bw->cursor->xcol=piscol(bw->cursor);
	if(reversed)
	 {
	 if(!pisbol(bw->cursor)) u_goto_prev(bw), bw->cursor->xcol=piscol(bw->cursor);
	 umarkb(bw);
	 }
	else
	 {
	 if(!piseol(bw->cursor)) u_goto_next(bw), bw->cursor->xcol=piscol(bw->cursor);
	 umarkk(bw);
	 }
}

int udefm2up(BW *bw)
{
}

int udefm3down(BW *xx)
{
	BW *bw;
	if(utomouse(xx)) return -1;
	if((maint->curwin->watom->what&(TYPEPW|TYPETW))==0) return 0;
	bw=(BW *)maint->curwin->object;
	/* set anchor to beginning of line, anchorn to beginning of next line */
	p_goto_bol(bw->cursor); bw->cursor->xcol=piscol(bw->cursor);
	anchor=bw->cursor->byte; umarkb(bw);
	umarkk(bw); pnextl(markk); anchorn=markk->byte;
	reversed=0;
	bw->cursor->xcol=piscol(bw->cursor);
}

int udefm3drag(BW *xx)
{
	BW *bw=(BW *)maint->curwin->object;
	if(tomousestay()) return -1;
	if(!reversed && bw->cursor->byte<anchor) {
		pgoto(markk,anchorn);
		markk->xcol=piscol(markk);
		reversed=1;
	} else if(reversed && bw->cursor->byte>anchorn) {
		pgoto(markb,anchor);
		markb->xcol=piscol(markb);
		reversed=0;
	}
	p_goto_bol(bw->cursor);
	bw->cursor->xcol=piscol(bw->cursor);
	if(reversed) umarkb(bw), markb->xcol=piscol(markb);
	else umarkk(bw), pnextl(markk), markk->xcol=markk->xcol=piscol(markk);
}

int udefm3up(BW *bw)
{
}
