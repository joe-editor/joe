/* DOS TTY INTERFACE */

#include <stdio.h>
#include <dos.h>
#include <bios.h>
#include "tty.h"

int obufp;
int obufsiz;
char *obuf;
unsigned long upc;
unsigned baud=38400;
int have=0;
int leave=0;
void tickon() {}
void tickoff() {}
int noxon;
int Baud;

int jread(fd,buf,siz)
char *buf;
 {
 return read(fd,buf,siz);
 }

int jwrite(fd,buf,siz)
char *buf;
 {
 return write(fd,buf,siz);
 }

int fork() {}
int pipe() {}
int wait() {}
int kill() {}

int setbreak(stat)
 {
 int prv;
 _AX=0x3300;
 geninterrupt(0x21);
 prv=_DL;
 _DX=stat;
 _AX=0x3301;
 geninterrupt(0x21);
 return prv;
 }

int breakflg;

void ttopen()
 {
 obuf=malloc(4096);
 obufsiz=4096;
 obufp=0;
 breakflg=setbreak(0);
 }

void ttopnn()
 {
 ttflsh();
 }

void ttclose()
 {
 ttflsh();
 setbreak(breakflg);
 }

void ttclsn()
 {
 ttflsh();
 }

int prefix=0;
int prefixc;

int ttgetc()
 {
 unsigned c;
 ttflsh();
 if(prefix)
  {
  prefix=0;
  return prefixc;
  }
 c=bioskey(0);
 if((c&255)==0)
  {
  prefix=1;
  prefixc=(c>>8);
  return 0;
  }
 else return (c&255);
 }

ttflsh()
 {
 if(obufp) _write(fileno(stdout),obuf,obufp);
 obufp=0;
 }

void ttputs(s)
char *s;
 {
 while(*s) ttputc(*s++);
 }

void ttshell()
 {
 }

void ttsusp()
 {
 system(getenv("COMSPEC"));
 }

void ttgtsz(x,y) int *x, *y; { *x=0; *y=0; }

void sigjoe() {}

void signrm() {}

char *getcwd();
char *pwd()
 {
 static char buf[128];
 return getcwd(buf,128);
 }

MPX *mpxmk()
 {
 }

int subshell()
 {
 }
