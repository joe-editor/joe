/* Various memory block functions
   Copyright (C) 1991 Joseph H. Allen

This file is part of J (Joe's Editor)

J is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software
Foundation; either version 1, or (at your option) any later version. 

J is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details. 

You should have received a copy of the GNU General Public License
along with GNU Emacs; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */


#include "blocks.h"

/* Set block to zero */

unsigned char *bzero(bk,sz)
unsigned char *bk;
{
unsigned char *b=bk-1;
if(sz) do b[sz]=0; while(--sz);
return bk;
}

/* Set block to unsigned character */

unsigned char *bset(bk,sz,c)
unsigned char *bk;
unsigned char c;
{
unsigned char *b=bk-1;
if(sz) do b[sz]=c; while(--sz);
return bk;
}

/* Move a possibly overlapping block of memory without loosing any data */

unsigned char *bmove(dst,src,sz)
unsigned char *dst;
unsigned char *src;
{
if(src==dst || !sz) return dst;
if(src>dst)
 {
 unsigned x=0;
 do dst[x]=src[x]; while(++x, --sz);
 }
else
 {
 unsigned char *d=dst-1;
 --src;
 do d[sz]=src[sz]; while(--sz);
 } 
return dst;
}

/* Move a block in the forward direction */

unsigned char *bfwrd(dst,src,sz)
unsigned char *dst;
unsigned char *src;
{
if(src!=dst && sz)
 {
 unsigned x=0;
 do dst[x]=src[x]; while(++x,--sz);
 }
return dst;
}

/* Move a block in the backward direction */

unsigned char *bbkwd(dst,src,sz)
unsigned char *dst;
unsigned char *src;
{
unsigned char *s=src-1, *d=dst-1;
if(s!=d && sz) do d[sz]=s[sz]; while(--sz);
return dst;
}

unsigned umin(a,b)
unsigned a,b;
{
return (a>b)?b:a;
}

unsigned umax(a,b)
unsigned a,b;
{
return (a>b)?a:b;
}

int min(a,b)
{
return (a>b)?b:a;
}

int max(a,b)
{
return (a>b)?a:b;
}

/* Compare blocks for equality */

int beq(dst,src,sz)
unsigned char *dst;
unsigned char *src;
{
unsigned char *d=dst-1, *s=src-1;
if(!sz) return 1;
do
 if(d[sz]!=s[sz]) return 0;
while(--sz);
return 1;
}

/* Compare blocks for equality case insensitive */

int bieq(dst,src,sz)
unsigned char *dst;
unsigned char *src;
{
unsigned char *d=dst, *s=src; int cnt=sz;
if(!cnt) return 1;
do
 if(*s>='a' && *s<='z')
  {
  if(*d>='a' && *d<='z') { if(*(d++)!=*(s++)) return 0; }
  else if(*(d++)!=(0x5f&*(s++))) return 0;
  }
 else if(*d>='a' && *d<='z')
  {
  if(*s>='a' && *s<='z') { if(*(d++)!=*(s++)) return 0; }
  else if(*(s++)!=(0x5f&*(d++))) return 0;
  }
 else if(*(d++)!=*(s++)) return 0;
while(--cnt);
return 1;
}

unsigned char *bchr(bk,sz,c)
unsigned char *bk;
unsigned char c;
{
unsigned char *s=bk;
int cnt=sz;
if(cnt)
 do if(*s==c) return s;
 while(++s, --cnt);
return 0;
}

unsigned char *brchr(bk,sz,c)
unsigned char *bk, c;
{
unsigned char *s=bk+sz;
int cnt=sz;
if(cnt)
 do if(*(--s)==c) return s;
 while(--cnt);
return 0;
}
