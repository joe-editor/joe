/* Terminal interface for BSD
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

#include <sgtty.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include "async.h"

#define HZ 10			/* Clock ticks/second */

#define DIVISOR 11000000	/* The baud rate divided into this should
				   give the number of microseconds per
				   character.  It should attempt to
				   reflect the true throughput, which is
				   usually slower than the best possible
				   for a given baud rate */

#define TIMES 3			/* Times per second that we check for
				   typeahead */

static struct sgttyb oarg;
static struct tchars otarg;
static struct ltchars oltarg;

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
struct sgttyb arg;
struct tchars targ;
struct ltchars ltarg;
fflush(stdout);
signal(SIGHUP,tsignal);
signal(SIGINT,SIG_IGN);
signal(SIGQUIT,SIG_IGN);
signal(SIGPIPE,SIG_IGN);
signal(SIGALRM,SIG_IGN);
signal(SIGTERM,tsignal);
signal(SIGUSR1,SIG_IGN);
signal(SIGUSR2,SIG_IGN);
ioctl(fileno(stdin),TIOCGETP,&arg);
ioctl(fileno(stdin),TIOCGETC,&targ);
ioctl(fileno(stdin),TIOCGLTC,&ltarg);
oarg=arg; otarg=targ; oltarg=ltarg;
arg.sg_flags=( (arg.sg_flags&~(ECHO|CRMOD) ) | CBREAK) ;
targ.t_intrc= -1;
targ.t_quitc= -1;
targ.t_eofc= -1;
targ.t_brkc= -1;
ltarg.t_suspc= -1;
ltarg.t_dsuspc= -1;
ltarg.t_rprntc= -1;
ltarg.t_flushc= -1;
ltarg.t_werasc= -1;
ltarg.t_lnextc= -1;
ioctl(fileno(stdin),TIOCSETN,&arg);
ioctl(fileno(stdin),TIOCSETC,&targ);
ioctl(fileno(stdin),TIOCSLTC,&ltarg);
ccc=0;
for(x=0;x!=30;x+=2)
 if(arg.sg_ospeed==speeds[x])
  {
  ccc=DIVISOR/speeds[x+1];
  break;
  }
if(!obuf)
 {
 if(!(TIMES*ccc)) obufsiz=4096;
 else
  {
  obufsiz=1000000/(TIMES*ccc);
  if(obufsiz>4096) obufsiz=4096;
  }
 if(!obufsiz) obufsiz=1;
 obuf=(unsigned char *)malloc(obufsiz);
 }
}

aclose()
{
aflush();
ioctl(fileno(stdin),TIOCSETN,&oarg);
ioctl(fileno(stdin),TIOCSETC,&otarg);
ioctl(fileno(stdin),TIOCSLTC,&oltarg);
signal(SIGHUP,SIG_DFL);
signal(SIGINT,SIG_DFL);
signal(SIGQUIT,SIG_DFL);
signal(SIGPIPE,SIG_DFL);
signal(SIGALRM,SIG_DFL);
signal(SIGTERM,SIG_DFL);
signal(SIGUSR1,SIG_DFL);
signal(SIGUSR2,SIG_DFL);
}

int have=0;
static unsigned char havec;
static int yep;

static dosig()
{
yep=1;
}

aflush()
{
if(obufp)
 {
 struct itimerval a,b;
 unsigned long usec=obufp*ccc;
 if(usec>=500000/HZ)
  {
  a.it_value.tv_sec=usec/1000000;
  a.it_value.tv_usec=usec%1000000;
  a.it_interval.tv_usec=0;
  a.it_interval.tv_sec=0;
  signal(SIGALRM,dosig);
  yep=0;
  sigsetmask(sigmask(SIGALRM));
  setitimer(ITIMER_REAL,&a,&b);
  write(fileno(stdout),obuf,obufp);
  while(!yep) sigpause(0);
  signal(SIGALRM,SIG_DFL);
  }
 else write(fileno(stdout),obuf,obufp);
 obufp=0;
 }
if(!have)
 {
 fcntl(fileno(stdin),F_SETFL,FNDELAY);
 if(read(fileno(stdin),&havec,1)==1) have=1;
 fcntl(fileno(stdin),F_SETFL,0);
 }
}

anext()
{
aflush();
if(have) have=0;
else if(read(fileno(stdin),&havec,1)<1) tsignal();
return havec;
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
if(width<2) width=80;
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
