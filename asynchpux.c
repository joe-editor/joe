/* Terminal interface for HPUX
   Copyright (C) 1991 Joseph H. Allen

This file is part of JOE (Joe's Own Editor)

JOE is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software
Foundation; either version 1, or (at your option) any later version. 

JOE is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.  

You should have received a copy of the GNU General Public License
along with JOE; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <sys/param.h>
#include <termio.h>
#include "async.h"

#define DIVISOR 12000000
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

void tsignal();

sigjoe()
{
signal(SIGHUP,tsignal);
signal(SIGTERM,tsignal);
signal(SIGINT,SIG_IGN);
signal(SIGPIPE,SIG_IGN);
signal(SIGQUIT,SIG_IGN);
}

signorm()
{
signal(SIGHUP,SIG_DFL);
signal(SIGTERM,SIG_DFL);
signal(SIGINT,SIG_DFL);
signal(SIGPIPE,SIG_DFL);
signal(SIGQUIT,SIG_DFL);
}

aopen()
{
int x;
struct termio newterm;
fflush(stdout);
ioctl(fileno(stdin),TCGETA,&oldterm);
newterm=oldterm;
newterm.c_lflag=0;
newterm.c_iflag&=~(ICRNL|IGNCR|INLCR);
newterm.c_oflag=0;
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
if(obuf) free(obuf);
if(!(TIMES*ccc)) obufsiz=4096;
else
 {
 obufsiz=1000000/(TIMES*ccc);
 if(obufsiz>4096) obufsiz=4096;
 }
if(!obufsiz) obufsiz=1;
obuf=(unsigned char *)malloc(obufsiz);
}

aclose()
{
aflush();
ioctl(fileno(stdin),TCSETAW,&oldterm);
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
 if(usec>=500000/10 /* HZ */)
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
if(!have && !leave)
 {
 fcntl(fileno(stdin),F_SETFL,O_NDELAY);
 if(read(fileno(stdin),&havec,1)==1) have=1;
 fcntl(fileno(stdin),F_SETFL,0);
 }
}

unsigned char *take=0;

anext()
{
if(take)
 if(*take)
  {
  int c;
  if(*take!='\\') return *take++;
  ++take;
  if(!*take) return '\\';
  else if(*take=='r') c='\r';
  else if(*take=='b') c=8;
  else if(*take=='n') c=10;
  else if(*take=='f') c=12;
  else if(*take=='a') c=7;
  else if(*take=='\"') c='\"';
  else if(*take>='0' && *take<='7')
        {
        c= *take++-'0';
        if(*take>='0' && *take<='7')
         {
         c=c*8+*take++-'0';
         if(*take>='0' && *take<='7') c=c*8+*take++-'0';
         }
        --take;
        }
  else c= *take;
  ++take;
  return c;
  }
 else take=0;
aflush();
if(have) have=0;
else if(read(fileno(stdin),&havec,1)<1) tsignal(0);
if(record) macroadd(havec);
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

getsize()
{
#ifdef TIOCGSIZE
struct ttysize getit;
#else
#ifdef TIOCGWINSZ
struct winsize getit;
#else
char *p;
#endif
#endif
#ifdef TIOCGSIZE
if(ioctl(fileno(stdout),TIOCGSIZE,&getit)!= -1)
 {
 if(getit.ts_lines>=3) height=getit.ts_lines;
 if(getit.ts_cols>=2) width=getit.ts_cols;
 }
#else
#ifdef TIOCGWINSZ
if(ioctl(fileno(stdout),TIOCGWINSZ,&getit)!= -1)
 {
 if(getit.ws_row>=3) height=getit.ws_row;
 if(getit.ws_col>=2) width=getit.ws_col;
 }
#else
if(p=getenv("ROWS")) sscanf(p,"%d",&height);
if(p=getenv("COLS")) sscanf(p,"%d",&width);
if(height<3) height=24;
if(width<2) width=80;
#endif
#endif
}

termtype()
{
unsigned char entry[1024];
unsigned char area[1024];
unsigned char *foo=area;
unsigned char *x=(unsigned char *)getenv("TERM");
if(!x) goto down;
if(tgetent(entry,x)!=1) goto down;
height=tgetnum("li");
if(height<3) height=24;
width=tgetnum("co");
if(width<2) width=80;
if(!tgetstr("cs",&foo)) scroll=0;
down:
getsize();
}

shell()
{
int x;
char *s=(char *)getenv("SHELL");
if(!s)
 {
 puts("\nSHELL variable not set");
 return;
 }
eputs("\nYou are at the command shell.  Type 'exit' to continue editing\r\n");
aclose();
if(x=fork())
 {
 if(x!= -1) wait(0);
 }
else
 {
 signorm();
 execl(s,s,0);
 _exit(0);
 }
aopen();
}

susp()
{
#ifdef SIGCONT
eputs("\nThe editor has been suspended.  Type 'fg' to continue editing\r\n");
yep=0;
aclose();
signal(SIGCONT,dosig);
sigsetmask(sigmask(SIGCONT));
kill(0,SIGTSTP);
while(!yep) sigpause(0);
signal(SIGCONT,SIG_DFL);
aopen();
#else
shell();
#endif
}
