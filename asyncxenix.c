/* Terminal interface for XENIX
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

#include <stdio.h>
#include <signal.h>
#include <termio.h>
#include "async.h"

#define DIVISOR 12000
#define TIMES 2

static struct termio oldterm;

static unsigned char *obuf=0;
static unsigned obufp=0;
static unsigned obufsiz;
static unsigned long ccc;

static unsigned speeds[]=
{
B50,50,B75,75,B110,110,B134,134,B150,150,B200,200,B300,300,B600,600,B1200,1200,
B1800,1800,B2400,2400,B4800,4800,B9600,9600,EXTA,19200,EXTB,38400
};

aopen()
{
int x;
struct termio newterm;
fflush(stdout);
signal(SIGHUP,tsignal);
signal(SIGINT,SIG_IGN);
signal(SIGQUIT,SIG_IGN);
signal(SIGPIPE,SIG_IGN);
signal(SIGALRM,SIG_IGN);
signal(SIGTERM,tsignal);
signal(SIGUSR1,SIG_IGN);
signal(SIGUSR2,SIG_IGN);
signal(SIGPWR,tsignal);
ioctl(fileno(stdin),TCGETA,&oldterm);
newterm=oldterm;
newterm.c_lflag=0;
newterm.c_iflag=IXON|IXOFF|IGNBRK;
newterm.c_oflag=0;
newterm.c_cc[VINTR]= -1;
newterm.c_cc[VQUIT]= -1;
newterm.c_cc[VMIN]=1;
newterm.c_cc[VTIME]=0;
ioctl(fileno(stdin),TCSETAW,&newterm);
ccc=0;
for(x=0;x!=30;x+=2)
 if((newterm.c_cflag&CBAUD)==speeds[x])
  {
  ccc=DIVISOR/speeds[x+1];
  break;
  }
if(!obuf)
 {
 if(!(TIMES*ccc)) obufsiz=4096;
 else
  {
  obufsiz=1000/(TIMES*ccc);
  if(obufsiz>4096) obufsiz=4096;
  }
 if(!obufsiz) obufsiz=1;
 obuf=(unsigned char *)malloc(obufsiz);
 }
}

aclose()
{
aflush();
ioctl(fileno(stdin),TCSETAW,&oldterm);
signal(SIGHUP,SIG_DFL);
signal(SIGINT,SIG_DFL);
signal(SIGQUIT,SIG_DFL);
signal(SIGPIPE,SIG_DFL);
signal(SIGALRM,SIG_DFL);
signal(SIGTERM,SIG_DFL);
signal(SIGUSR1,SIG_DFL);
signal(SIGUSR2,SIG_DFL);
signal(SIGPWR,SIG_DFL);
}

int have=0;

aflush()
{
if(obufp)
 {
 write(fileno(stdout),obuf,obufp);
 nap(obufp*ccc);
 obufp=0;
 }
if(!have) if(rdchk(fileno(stdin))>0) have=1;
}

anext()
{
unsigned char c;
aflush();
if(read(fileno(stdin),&c,1)<1) tsignal();
have=0;
return c;
}

eputc(c)
unsigned char c;
{
obuf[obufp++]=c;
if(obufp==obufsiz) aflush();
}

eputs(s)
char *s;
{
while(*s)
 {
 obuf[obufp++]= *(s++);
 if(obufp==obufsiz) aflush();
 }
}

termtype()
{
unsigned char entry[1024];
unsigned char area[1024];
unsigned char *foo=area;
unsigned char *x=(unsigned char *)getenv("TERM");
if(!x) return;
if(tgetent(entry,x)!=1) return;
height=tgetnum("li");
if(height<2) height=24;
width=tgetnum("co");
if(width<2) width=2;
if(!tgetstr("cs",&foo)) scroll=0;
}

shell(s)
char *s;
{
aclose();
if(fork()) wait(0);
else
 {
 execl(s,s,0);
 _exit(0);
 }
aopen();
}
