/* tags file symbol lookup
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
#include "b.h"
#include "bw.h"
#include "w.h"
#include "pw.h"
#include "qw.h"
#include "vs.h"
#include "zstr.h"
#include "usearch.h"
#include "tab.h"
#include "main.h"
#include "utag.h"

static int dotag(bw,s,obj,notify)
BW *bw;
char *s;
void *obj;
int *notify;
 {
 char buf[512];
 FILE *f;
 char *t=0;
 if(notify) *notify=1;
 if(bw->b->name)
  {
  t=vsncpy(t,0,sz(bw->b->name));
  t=vsncpy(sv(t),sc(":"));
  t=vsncpy(sv(t),sv(s));
  }
 f=fopen("tags","r");
 if(!f)
  {
  msgnw(bw,"Couldn't open tags file");
  vsrm(s);
  vsrm(t);
  return -1;
  }
 while(fgets(buf,512,f))
  {
  int x, y, c;
  for(x=0;buf[x] && buf[x]!=' ' && buf[x]!='\t';++x);
  c=buf[x]; buf[x]=0;
  if(!zcmp(s,buf) || t && !zcmp(t,buf))
   {
   buf[x]=c;
   while(buf[x]==' ' || buf[x]=='\t') ++x;
   for(y=x;buf[y] && buf[y]!=' ' && buf[y]!='\t' && buf[y]!='\n';++y);
   if(x!=y)
    {
    B *b;
    c=buf[y]; buf[y]=0;
    if(doedit(bw,vsncpy(NULL,0,sz(buf+x)),NULL,NULL))
     {
     vsrm(s);
     vsrm(t);
     fclose(f);
     return -1;
     }
    bw=(BW *)maint->curwin->object;
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
      if(line>=1)
       {
       int omid=mid;
       mid=1;
       pline(bw->cursor,line-1), bw->cursor->xcol=piscol(bw->cursor);
       dofollows();
       mid=omid;
       }
      else msgnw(bw,"Invalid line number");
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
       vsrm(s);
       vsrm(t);
       fclose(f);
       return dopfnext(bw,mksrch(vsncpy(NULL,0,sz(buf+y)),NULL,0,0,-1,0,0),NULL);
       }
      }
     }
    vsrm(s);
    vsrm(t);
    fclose(f);
    return 0;
    }
   }
  }
 msgnw(bw,"Not found");
 vsrm(s);
 vsrm(t);
 fclose(f);
 return -1;
 }

static B *taghist=0;

int utag(bw)
BW *bw;
 {
 BW *pbw;
 pbw=wmkpw(bw,"Tag search: ",&taghist,dotag,NULL,NULL,cmplt,NULL,NULL);
 if(pbw && crest(brc(bw->cursor)))
  {
  P *p=pdup(bw->cursor);
  P *q=pdup(p);
  int c;
  while(crest(c=prgetc(p))); if(c!=MAXINT) pgetc(p);
  pset(q,p); while(crest(c=pgetc(q))); if(c!=MAXINT) prgetc(q);
  binsb(pbw->cursor,bcpy(p,q));
  pset(pbw->cursor,pbw->b->eof); pbw->cursor->xcol=piscol(pbw->cursor);
  prm(p); prm(q);
  }
 if(pbw) return 0;
 else return -1;
 }
