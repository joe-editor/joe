/* Cruddy terminal interface - should be very portable though
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
#include "async.h"

int have=0;

eputs(s)
char *s;
{
fputs(s,stdout);
}

eputc(c)
{
putchar(c);
}

aopen()
{
fflush(stdout);
system("/bin/stty raw -echo ixon ixoff");
signal(SIGHUP,tsignal);
signal(SIGINT,SIG_IGN);
signal(SIGQUIT,SIG_IGN);
signal(SIGPIPE,SIG_IGN);
signal(SIGALRM,SIG_IGN);
signal(SIGTERM,tsignal);
signal(SIGUSR1,SIG_IGN);
signal(SIGUSR2,SIG_IGN);
}

aclose()
{
fflush(stdout);
signal(SIGHUP,SIG_DFL);
signal(SIGINT,SIG_DFL);
signal(SIGQUIT,SIG_DFL);
signal(SIGPIPE,SIG_DFL);
signal(SIGALRM,SIG_DFL);
signal(SIGTERM,SIG_DFL);
signal(SIGUSR1,SIG_DFL);
signal(SIGUSR2,SIG_DFL);
system("/bin/stty cooked echo");
}

aflush()
{
}

anext()
{
unsigned char c;
fflush(stdout);
while(read(fileno(stdin),&c,1)!=1);
return c;
}

termtype()
{
/*
char entry[1024];
char area[1024];
char *foo=area;
char *x=(char *)getenv("TERM");
if(!x) return;
if(tgetent(entry,x)<1) return;
height=tgetnum("li");
if(height<1) height=24;
width=tgetnum("co");
if(width<2) width=80;
if(!tgetstr("cs",&foo)) scroll=0;
*/
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
