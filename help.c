/* Help system
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
#include "tty.h"
#include "toomany.h"
#include "b.h"
#include "termcap.h"
#include "kbd.h"
#include "scrn.h"
#include "w.h"
#include "vs.h"
#include "menu.h"
#include "help.h"

/* The loaded help screen */

char *hlptxt=0;
int hlpsiz=0;
int hlpbsz=0;
int hlplns=0;

static int uphelp=0;

/* Display help text */

void dsphlp(t)
SCREEN *t;
{
char *str=hlptxt;
int y,x,c;
int atr=0;
for(y=skiptop;y!=t->wind;++y)
 {
 if(t->t->updtab[y])
  {
  for(x=0;x!=t->w-1;++x)
   if(*str=='\n' || !*str)
    if(eraeol(t->t,x,y)) return;
    else break;
   else
    {
    if(*str=='\\')
     if(str[1]=='i' || str[1]=='u' || str[1]=='I' || str[1]=='U')
      {
      if(str[1]=='i' || str[1]=='I') atr^=INVERSE;
      else atr^=UNDERLINE;
      str+=2;
      --x; continue;
      }
     else if(str[1]=='\\') c='\\'+atr, str+=2;
     else { ++str; --x; continue; }
    else c= (unsigned char)*str++ +atr;
    if(t->t->scrn[x+y*t->w]!=c)
     {
     if(have) return;
     t->t->scrn[x+y*t->w]=c;
     outatr(t->t,x,y,c);
     }
    }
  atr=0; t->t->updtab[y]=0;
  }
 while(*str && *str!='\n') ++str;
 if(*str=='\n') ++str;
 }
}

/* Create the help window */

void helpon(t)
SCREEN *t;
{
if(!hlptxt) return;
t->wind=hlplns+skiptop;
if(t->h-t->wind<FITHEIGHT) t->wind=t->h-FITHEIGHT;
if(t->wind<0)
 {
 t->wind=skiptop;
 return;
 }
chsize(t,t->h-t->wind,t->h);
msetI(t->t->updtab+skiptop,1,t->wind);
}

/* Eliminate the help window */

void helpoff(t)
SCREEN *t;
{
int z=t->wind;
t->wind=skiptop;
chsize(t,t->h,t->h-z);
}

/* Toggle help on/off */

void uhelp(w)
W *w;
{
struct help *h;
if(w->huh) if(h=get_help(w->huh))
 {
 if(w->t->wind!=skiptop) helpoff(w->t);
 hlptxt=h->hlptxt;
 hlpsiz=h->hlpsiz;
 hlpbsz=h->hlpbsz;
 hlplns=h->hlplns;
 helpon(w->t);
 return;
 }
uhelpme(w);
}

/* Help selection menu */

void uheol(w) W *w; { MENU *m=(MENU *)w->object; meol(m); }
void uhbol(w) W *w; { MENU *m=(MENU *)w->object; mbol(m); }
void uheof(w) W *w; { MENU *m=(MENU *)w->object; meof(m); }
void uhbof(w) W *w; { MENU *m=(MENU *)w->object; mbof(m); }
void uhdnarw(w) W *w; { MENU *m=(MENU *)w->object; mdnarw(m); }
void uhuparw(w) W *w; { MENU *m=(MENU *)w->object; muparw(m); }
void uhltarw(w) W *w; { MENU *m=(MENU *)w->object; mltarw(m); }
void uhrtarw(w) W *w; { MENU *m=(MENU *)w->object; mrtarw(m); }
void hdumb() {}
void movehelp(w,x,y) W *w; { MENU *m=(MENU *)w->object; menumove(m,x,y); }
void resizehelp(w,x,y) W *w; { MENU *m=(MENU *)w->object; menuresz(m,x,y); }
void disphelp(w) W *w;
 {
 MENU *m=(MENU *)w->object;
 menugen(m);
 w->cury=0;
 w->curx=(m->cursor-m->top)*(m->width+1);
 }
void followhelp(w) W *w; { MENU *m=(MENU *)w->object; menufllw(m); }
void wkillhelp(w) W *w; { MENU *m=(MENU *)w->object; if(m) menurm(m); }

int prevcursor=0;

void uhrtn(w)
W *w;
{
MENU *m=(MENU *)w->object;
hlptxt=help_structs[m->cursor]->hlptxt;
hlpsiz=help_structs[m->cursor]->hlpsiz;
hlpbsz=help_structs[m->cursor]->hlpbsz;
hlplns=help_structs[m->cursor]->hlplns;
prevcursor=m->cursor;
wabort(w);
helpon(w->t);
}

void uhkey(w,c)
W *w;
{
MENU *m=(MENU *)w->object;
int x;
int n=0;
c=toup(c);
for(x=0;x!=sLEN(m->list);++x) if(toup(m->list[x][0])==c) ++n;
if(!n) return;
if(n==1)
 for(x=0;x!=sLEN(m->list);++x)
  if(toup(m->list[x][0])==c)
   {
   m->cursor=x;
   uhrtn(w);
   return;
   }
do
 {
 ++m->cursor;
 if(m->cursor==sLEN(m->list)) m->cursor=0;
 } while(toup(m->list[m->cursor][0])!=c);
}

void uhabort(w)
W *w;
{
MENU *m=(MENU *)w->object;
prevcursor=m->cursor;
wabort(w);
}

CONTEXT cthelp= {"help",0};

static WATOM watomhelp=
{
&cthelp,
disphelp,
followhelp,
wkillhelp,
resizehelp,
movehelp,
hdumb,
hdumb,
TYPEHELP
};

void uhelpme(w)
W *w;
{
W *new;
MENU *m;
if(w->t->wind!=skiptop)
 {
 helpoff(w->t);
 return;
 }
if(!help_names[0]) return;
if(!help_names[1])
 {
 helpon(w->t);
 return;
 }
if(!(new=wcreate(w->t,&watomhelp,w,w,w->main,1,NULL))) return;
new->object=(void *)(m=mkmenu(new->t,help_names,new->x,new->y,new->w,new->h));
m->cursor=prevcursor;
w->t->curwin=new;
}
