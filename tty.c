/* TTY interface
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


/* System include files */

/* These should exist on every system */
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
extern int errno;

/* These are a mess */

#ifdef SYSPARAM
#include <sys/param.h>
#endif

#ifdef SCO
#include <sys/stream.h>
#include <sys/ptem.h>
#endif

#ifdef SYSTIM
#include <sys/time.h>
#endif

#ifdef TIM
#include <time.h>
#endif

/* Depends on tty structure system uses */
#ifdef BSDTTY
#include <sgtty.h>
#endif

#ifdef POSIXTTY
#include <termios.h>
#endif

#ifdef SVTTY
#include <termio.h>
#endif

/* JOE include files */

#include "config.h"
#include "heap.h"
#include "msgs.h"
#include "tty.h"

/* Aliased defines */

#ifndef O_NDELAY
#ifdef O_NONBLOCK
#define O_NDELAY O_NONBLOCK
#endif
#ifdef FNDELAY
#define O_NDELAY FNDELAY
#endif
#endif

#ifndef sigmask
#define sigmask(x) (1<<((x)-1))
#endif

#ifndef SIGCHLD
#ifdef SIGCLD
#define SIGCHLD SIGCLD
#endif
#endif

/* The terminal */

FILE *termin=0;
FILE *termout=0;

/* Original state of tty */

#ifdef BSDTTY
static struct sgttyb oarg;
static struct tchars otarg;
static struct ltchars oltarg;
#endif

#ifdef SVTTY
static struct termio oldterm;
#endif

#ifdef POSIXTTY
struct termios oldterm;
#endif

/* Output buffer, index and size */

char *obuf=0;
int obufp=0;
int obufsiz;

/* The baud rate */

unsigned baud;		/* Bits per second */
unsigned long upc;	/* Microseconds per character */

/* TTY Speed code to baud-rate conversion table */

static int speeds[]=
{
B50,50,B75,75,B110,110,B134,134,B150,150,B200,200,B300,300,B600,600,B1200,1200,
B1800,1800,B2400,2400,B4800,4800,B9600,9600,

#ifdef B19200
B19200,19200,
#else
EXTA,19200,
#endif

#ifdef B38400
B38400,38400
#else
EXTB,38400
#endif
};

/* Input buffer */

int have=0;		/* Set if we have pending input */
static char havec;	/* Character read in during pending input check */
int leave=0;		/* When set, typeahead checking is disabled */

/* TTY mode flag.  1 for open, 0 for closed */

static int ttymode=0;

/* Signal state flag.  1 for joe, 0 for normal */

static int ttysig=0;

/* Stuff for shell windows */

static int ackkbd= -1;		/* Editor acks keyboard client to this */
static int mpxfd;		/* Editor reads packets from this fd */
static int mpxsfd;		/* Clients send packets to this fd */
static int nmpx=0;
static int kbdpid;		/* PID of kbd client */
static int accept=MAXINT;	/* Not MAXINT if we have a packet */
static int deathchk=0;		/* Number of SIGCHLDs */
static int deathcnt=0;		/* Number of SIGCLDs/waits */
struct packet
 {
 MPX *who;
 int size;
 int ch;
 char data[1024];
 } pack;
MPX asyncs[NPROC];

/* Versions of 'read' and 'write' which automatically retry during signals */

int jread(fd,buf,siz)
char *buf;
{
int rt;
do
 rt=read(fd,buf,siz);
while(rt<0 && errno==EINTR);
return rt;
}

int jwrite(fd,buf,siz)
char *buf;
{
int rt;
do
 rt=write(fd,buf,siz);
 while(rt<0 && errno==EINTR);
return rt;
}

/* If we're very posix */

#ifdef REALPOSIX
void signal(a,b)
int a;
void (*b)();
{
struct sigaction action;
sigemptyset(&action.sa_mask);
action.sa_flags=0;
action.sa_handler=b;
sigaction(a,&action,NULL);
}
#endif

/* Set signals for JOE */

void sigjoe()
{
if(ttysig) return;
ttysig=1;
signal(SIGHUP,ttsig);
signal(SIGTERM,ttsig);
signal(SIGINT,SIG_IGN);
signal(SIGPIPE,SIG_IGN);
signal(SIGQUIT,SIG_IGN);
}

/* Restore signals for exiting */

void signrm()
{
if(!ttysig) return;
ttysig=0;
signal(SIGHUP,SIG_DFL);
signal(SIGTERM,SIG_DFL);
signal(SIGINT,SIG_DFL);
signal(SIGPIPE,SIG_DFL);
signal(SIGQUIT,SIG_DFL);
}

/* Open terminal and set signals */

void ttopen()
{
sigjoe();
ttopnn();
}

/* Close terminal and restore signals */

void ttclose()
{
ttclsn();
signrm();
}

int winched=0;

void winchd()
{
++winched;
#ifdef SIGWINCH
signal(SIGWINCH,winchd);
#endif
}

/* Open terminal */

void ttopnn()
{
int x, bbaud;
char *bs;

#ifdef BSDTTY
struct sgttyb arg;
struct tchars targ;
struct ltchars ltarg;
#endif

#ifdef SVTTY
struct termio newterm;
#endif

#ifdef POSIXTTY
struct termios newterm;
#endif

if(!termin)
 if(!(termin=fopen("/dev/tty","r")) || !(termout=fopen("/dev/tty","w")))
  {
  fprintf(stderr,M078);
  exit(1);
  }
 else
  {
#ifdef SIGWINCH
  signal(SIGWINCH,winchd);
#endif
  }

if(ttymode) return;
ttymode=1;
fflush(termout);

#ifdef BSDTTY
ioctl(fileno(termin),TIOCGETP,&arg);
ioctl(fileno(termin),TIOCGETC,&targ);
ioctl(fileno(termin),TIOCGLTC,&ltarg);
oarg=arg; otarg=targ; oltarg=ltarg;
arg.sg_flags=( (arg.sg_flags&~(ECHO|CRMOD|XTABS|ALLDELAY|TILDE) ) | CBREAK) ;
if(getenv("NOXON")) targ.t_startc= -1, targ.t_stopc= -1;
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
ioctl(fileno(termin),TIOCSETN,&arg);
ioctl(fileno(termin),TIOCSETC,&targ);
ioctl(fileno(termin),TIOCSLTC,&ltarg);
bbaud=arg.sg_ospeed;
#endif

#ifdef SVTTY
ioctl(fileno(termin),TCGETA,&oldterm);
newterm=oldterm;
newterm.c_lflag=0;
if(getenv("NOXON"))  newterm.c_iflag&=~(ICRNL|IGNCR|INLCR|IXON);
else newterm.c_iflag&=~(ICRNL|IGNCR|INLCR);
newterm.c_oflag=0;
newterm.c_cc[VMIN]=1;
newterm.c_cc[VTIME]=0;
ioctl(fileno(termin),TCSETAW,&newterm);
bbaud=(newterm.c_cflag&CBAUD);
#endif

#ifdef POSIXTTY
tcgetattr(fileno(termin),&oldterm);
newterm=oldterm;
newterm.c_lflag=0;
if(getenv("NOXON"))  newterm.c_iflag&=~(ICRNL|IGNCR|INLCR|IXON);
else newterm.c_iflag&=~(ICRNL|IGNCR|INLCR);
newterm.c_oflag=0;
newterm.c_cc[VMIN]=1;
newterm.c_cc[VTIME]=0;
tcsetattr(fileno(termin),TCSANOW,&newterm);
bbaud=cfgetospeed(&newterm);
#endif

baud=9600; upc=0;
for(x=0;x!=30;x+=2)
 if(bbaud==speeds[x])
  {
  baud=speeds[x+1];
  break;
  }
bs=getenv("BAUD");
if(bs) sscanf(bs,"%u",&baud);
upc=DIVIDEND/baud;
if(obuf) free(obuf);
if(!(TIMES*upc)) obufsiz=4096;
else
 {
 obufsiz=1000000/(TIMES*upc);
 if(obufsiz>4096) obufsiz=4096;
 }
if(!obufsiz) obufsiz=1;
obuf=(char *)malloc(obufsiz);
}

/* Close terminal */

void ttclsn()
{
int oleave;

if(ttymode) ttymode=0;
else return;

oleave=leave; leave=1;

ttflsh();

#ifdef BSDTTY
ioctl(fileno(termin),TIOCSETN,&oarg);
ioctl(fileno(termin),TIOCSETC,&otarg);
ioctl(fileno(termin),TIOCSLTC,&oltarg);
#endif

#ifdef SVTTY
ioctl(fileno(termin),TCSETAW,&oldterm);
#endif

#ifdef POSIXTTY
tcsetattr(fileno(termin),TCSANOW,&oldterm);
#endif

leave=oleave;
}

/* Flush output and check for typeahead */

static int yep;
static void dosig() { yep=1; } 

int ttflsh()
{
/* Flush output */
if(obufp)
 {
 unsigned long usec=obufp*upc;		/* No. usecs this write should take */
#ifdef BSDTIMER
 if(usec>=500000/HZ && baud<38400)
  {
  struct itimerval a,b;
  a.it_value.tv_sec=usec/1000000;
  a.it_value.tv_usec=usec%1000000;
  a.it_interval.tv_usec=0;
  a.it_interval.tv_sec=0;
  signal(SIGALRM,dosig); yep=0;
  sigsetmask(sigmask(SIGALRM));
  setitimer(ITIMER_REAL,&a,&b);
  jwrite(fileno(termout),obuf,obufp);
  while(!yep) sigpause(0);
  signal(SIGALRM,SIG_DFL);
  }
 else write(fileno(termout),obuf,obufp);

#else

 write(fileno(termout),obuf,obufp);

#ifdef XENIX
 if(baud<38400 && usec/1000) nap(usec/1000);
#endif

#endif

 obufp=0;
 }

/* Ack previous packet */
if(ackkbd!= -1 && accept!=MAXINT && !have)
 {
 accept=MAXINT;
 pack.ch=0;
 pack.size=0;
 if(pack.who)
  {
  if(pack.who->func) write(pack.who->ackfd,&pack,sizeof(struct packet)-1024);
  }
 else write(ackkbd,&pack,sizeof(struct packet)-1024);
 }

/* Check for typeahead or next packet */

if(!have && !leave)
 if(ackkbd!= -1)
  {
  fcntl(mpxfd,F_SETFL,O_NDELAY);
  if(read(mpxfd,&pack,sizeof(struct packet)-1024)>0)
   {
   jread(mpxfd,pack.data,pack.size);
   have=1, accept=pack.ch;
   }
  fcntl(mpxfd,F_SETFL,0);
  }
 else
  {
  /* Set terminal input to non-blocking */
  fcntl(fileno(termin),F_SETFL,O_NDELAY);
 
  /* Try to read */
  if(read(fileno(termin),&havec,1)==1) have=1;
 
  /* Set terminal back to blocking */
  fcntl(fileno(termin),F_SETFL,0);
  }
return 0;
}

/* Read next character from input */

void mpxdied();
void death();

int ttgetc()
{
int stat, x;
loop:
while(deathchk)
 {
 int pid=wait(NULL);
 for(x=0;x!=NPROC;++x)
  if(asyncs[x].pid==pid)
   {
   asyncs[x].pid=0;
   kill(asyncs[x].kpid,9);
   ++deathcnt;
   break;
   }
 --deathchk;
 }
while(deathcnt)
 {
 for(x=0;x!=NPROC;++x)
  if(!asyncs[x].pid && asyncs[x].func) { mpxdied(asyncs+x); break; }
 --deathcnt;
 }
ttflsh();
if(ackkbd!= -1)
 {
 if(!have)					/* Wait for input */
  {
  stat=read(mpxfd,&pack,sizeof(struct packet)-1024);
  if(pack.size && stat>0) jread(mpxfd,pack.data,pack.size);
  if(stat<=0)
   {
   if(winched) while(winched) { winched=0; edupd(); ttflsh(); }
   goto loop;
   }
  accept=pack.ch;
  }
 have=0;
 if(pack.who)					/* Got bknd input */
  {
  if(accept!=MAXINT)
   {
   if(pack.who->func)
    pack.who->func(pack.who->object,pack.data,pack.size),
    edupd();
   }
  else mpxdied(pack.who);
  goto loop;
  }
 else
  {
  if(accept!=MAXINT) return pack.ch;
  else { ttsig(0); return 0; }
  }
 }
if(have) have=0;
else
 {
 lop:
 if(read(fileno(termin),&havec,1)<1)
  {
  if(winched)
   {
   while(winched) { winched=0; edupd(); ttflsh(); }
   goto lop;
   }
  ttsig(0);
  }
 }
return havec;
}

/* Write string to output */

void ttputs(s)
char *s;
{
while(*s)
 {
 obuf[obufp++]= *s++;
 if(obufp==obufsiz) ttflsh();
 }
}

/* Get window size */

void ttgtsz(x,y)
int *x, *y;
{
#ifdef TIOCGSIZE
struct ttysize getit;
#else
#ifdef TIOCGWINSZ
struct winsize getit;
#else
#ifdef WIOCGETD
struct uwdata uwdat;
#endif
#endif
#endif

*x=0; *y=0;

#ifdef TIOCGSIZE
if(ioctl(fileno(termout),TIOCGSIZE,&getit)!= -1)
 {
 *x=getit.ts_cols;
 *y=getit.ts_lines;
 }
#else
#ifdef TIOCGWINSZ
if(ioctl(fileno(termout),TIOCGWINSZ,&getit)!= -1)
 {
 *x=getit.ws_col;
 *y=getit.ws_row;
 }
#else
#ifdef WIOCGETD
if(ioctl(fileno(termout),WIOCGETD,&uwdat))
 {
 *x=uwdat.uw_width/uwdat.uw_hs;
 *y=uwdat.uw_height/uwdat.uw_vs;
 }
#endif
#endif
#endif
}

void ttshell(cmd)
char *cmd;
{
int x,omode=ttymode;
char *s=getenv("SHELL");
if(!s) return;
ttclsn();
if(x=fork())
 {
 if(x!= -1) wait(0);
 if(omode) ttopnn();
 }
else
 {
 signrm();
 if(cmd) execl(s,s,"-c",cmd,NULL);
 else
  {
  fprintf(stderr,M079);
  execl(s,s,NULL);
  }
 _exit(0);
 }
}

#ifdef REALBSD
char *getwd();
char *pwd()
{
static buf[1024];
return getwd(buf);
}
#else
char *getcwd();
char *pwd()
{
static char buf[1024];
return getcwd(buf,1024);
}
#endif

void death()
{
#ifdef SIGCLD
int x;
int pid=wait(NULL);
for(x=0;x!=NPROC;++x)
 if(asyncs[x].pid==pid)
  {
  asyncs[x].pid=0;
  kill(asyncs[x].kpid,9);
  ++deathcnt;
  break;
  }
#else
++deathchk;
#endif

#ifdef SIGCHLD
signal(SIGCHLD,death);
#endif
}

void ttsusp()
{
#ifdef SIGTSTP
int omode=ttymode;
ttclsn();
fprintf(stderr,M080);
if(ackkbd!= -1)
 {
#ifdef SIGCHLD
 signal(SIGCHLD,SIG_IGN);
#endif
 }
kill(0,SIGTSTP);
if(ackkbd!= -1)
 {
 kill(kbdpid,SIGCONT);
#ifdef SIGCHLD
 signal(SIGCHLD,death);
#endif
 }
if(omode) ttopnn();
#else
ttshell(NULL);
#endif
}

void mpxstart()
{
int fds[2];
pipe(fds);
mpxfd=fds[0];
mpxsfd=fds[1];
pipe(fds);
accept=MAXINT; have=0;
#ifdef SIGCHLD
signal(SIGCHLD,death);
#endif
if(!(kbdpid=fork()))
 {
 close(fds[1]);
 do
  {
  char c;
  int sta;
  pack.who=0;
  jread(fileno(termin),&c,1);
  if(sta==0) pack.ch=MAXINT;
  else pack.ch=c;
  pack.size=0;
  jwrite(mpxsfd,&pack,sizeof(struct packet)-1024);
  while(jread(fds[0],&pack,sizeof(struct packet)-1024)<1);
  }
  while(1);
 _exit(0);
 }
close(fds[0]);
ackkbd=fds[1];
}

void mpxend()
{
ttflsh();
#ifdef SIGCHLD
signal(SIGCHLD,SIG_DFL);
#endif
kill(kbdpid,9); wait(NULL);
close(ackkbd); ackkbd= -1;
close(mpxfd);
close(mpxsfd);
if(have) havec=pack.ch;
}

MPX *mpxmk(fd,pid,func,object,die,dieobj)
void (*func)();
void *object;
void (*die)();
void *dieobj;
{
int fds[2];
int x;
MPX *m;
for(x=0;x!=NPROC;++x) 
 if(!asyncs[x].func) { m=asyncs+x; goto ok; }
return 0;
ok:
ttflsh();
++nmpx;
if(ackkbd== -1) mpxstart();
m->func=func;
m->object=object;
m->die=die;
m->pid=pid;
m->dieobj=dieobj;
pipe(fds);
m->ackfd=fds[1];
if(!(m->kpid=fork()))
 {
 close(fds[1]);
 loop:
 pack.who=m;
 pack.ch=0;
 pack.size=jread(fd,pack.data,1024);
 if(pack.size>0)
  {
  jwrite(mpxsfd,&pack,sizeof(struct packet)-1024+pack.size);
  if(jread(fds[0],&pack,sizeof(struct packet)-1024)>0) goto loop;
  goto loop;
  }
 pack.size=0;
 pack.ch=MAXINT;
 write(mpxsfd,&pack,sizeof(struct packet)-1024);
 _exit(0);
 }
close(fds[0]);
return m;
}

void mpxdied(m)
MPX *m;
{
if(!--nmpx) mpxend();
if(m->die) m->die(m->dieobj);
m->func=0;
edupd();
}

int subshell(fd,name)
char *name;
{
int pid;
int x;
if(!(pid=fork()))
 {
 signrm();
 close(fd);

#ifdef SIDCALL
 setsid();
#else
#ifdef GRPCALL
#ifdef TIOCNOTTY
 x=open("/dev/tty",2);
 ioctl(x,TIOCNOTTY,0);
#endif
 setpgrp(0,0);
#endif
#endif

 for(x=0;x!=32;++x) close(x);

 x=open(name,2); if(x== -1) goto bye;
 dup(x); dup(x);

#ifdef BSDTTY
 ioctl(0,TIOCSETN,&oarg);
 ioctl(0,TIOCSETC,&otarg);
 ioctl(0,TIOCSLTC,&oltarg);
#endif

#ifdef SVTTY
 ioctl(0,TCSETAW,&oldterm);
#endif

#ifdef POSIXTTY
 tcsetattr(0,TCSANOW,&oldterm);
#endif

 execl(getenv("SHELL"),"-","-i",NULL);

 bye:
 sleep(5); _exit(0);
 }
return pid;
}
