/* Editor startup and main edit loop
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
#include "vfile.h"
#include "blocks.h"
#include "tty.h"
#include "toomany.h"
#include "termcap.h"
#include "scrn.h"
#include "b.h"
#include "bw.h"
#include "tw.h"
#include "w.h"
#include "kbd.h"
#include "zstr.h"
#include "macro.h"
#include "tab.h"
#include "pw.h"
#include "qw.h"
#include "edfuncs.h"
#include "poshist.h"
#include "pattern.h"
#include "help.h"
#include "vs.h"
#include "msgs.h"
#include "main.h"

extern int mid, dspasis, dspctrl, force, help, pgamnt, starow, stacol,
           tabwidth, nobackups, lightoff, exask, skiptop;
int help=0;		/* Set to have help on when starting */

char *exmsg=0;		/* Message to display when exiting the editor */

SCREEN *maint;		/* Main edit screen */

/* Command table */

#define EMID 1
#define ECHKXCOL 2
#define EFIXXCOL 4
#define EMINOR 8
#define EPOS 16
#define EMOVE 32

static CMD cmds[]=
{
  { "abort", TYPETW, uaborttw },
  { "aborthelp", TYPEHELP, uhabort },
  { "abortpw", TYPEPW, uabortpw },
  { "abortqw", TYPEQW, uabortqw },
  { "aborttab", TYPETAB, tuabort },
  { "arg", TYPETW+TYPEPW+TYPETAB+TYPEHELP+TYPEQW, uarg },
  { "backs", TYPETW+TYPEPW+ECHKXCOL+EFIXXCOL+EMINOR, ubacks },
  { "backstab", TYPETAB, tbacks },
  { "backw", TYPETW+TYPEPW+ECHKXCOL+EFIXXCOL, ubackw },
  { "bknd", TYPETW+TYPEPW, ubknd },
  { "blkcpy", TYPETW+TYPEPW+EFIXXCOL, ublkcpy },
  { "blkdel", TYPETW+TYPEPW+EFIXXCOL, ublkdel },
  { "blkmove", TYPETW+TYPEPW+EFIXXCOL, ublkmove },
  { "blksave", TYPETW+TYPEPW+0, ublksave },
  { "bof", TYPETW+TYPEPW+EMOVE+EFIXXCOL, ubof },
  { "bofhelp", TYPEHELP, uhbof },
  { "boftab", TYPETAB, tbof },
  { "bol", TYPETW+TYPEPW+EFIXXCOL, ubol },
  { "bolhelp", TYPEHELP, uhbol },
  { "boltab", TYPETAB, tbol },
  { "center", TYPETW+TYPEPW+EFIXXCOL, ucenter },
  { "complete", TYPETW+TYPEPW, ucmplt },
  { "delbol", TYPETW+TYPEPW+EFIXXCOL, udelbl },
  { "delch", TYPETW+TYPEPW+ECHKXCOL+EFIXXCOL+EMINOR, udelch },
  { "deleol", TYPETW+TYPEPW+0, udelel },
  { "dellin", TYPETW+TYPEPW+EFIXXCOL, udelln },
  { "delw", TYPETW+TYPEPW+EFIXXCOL+ECHKXCOL, udelw },
  { "dnarw", TYPETW+TYPEPW+EMOVE, udnarw },
  { "dnarwhelp", TYPEHELP, uhdnarw },
  { "dnarwtab", TYPETAB, tdnarw },
  { "dnslide", TYPETW+EMOVE, udnslide },
  { "edit", TYPETW+TYPEPW, uedit },
  { "eof", TYPETW+TYPEPW+EFIXXCOL+EMOVE, ueof },
  { "eofhelp", TYPEHELP, uheof },
  { "eoftab", TYPETAB, teof },
  { "eol", TYPETW+TYPEPW+EFIXXCOL, ueol },
  { "eolhelp", TYPEHELP, uheol },
  { "eoltab", TYPETAB, teol },
  { "explode", TYPETW+TYPEPW+TYPETAB+TYPEHELP+TYPEQW, uexpld },
  { "exsave", TYPETW+TYPEPW, uexsve },
  { "ffirst", TYPETW+TYPEPW, pffirst },
  { "filt", TYPETW+TYPEPW, ufilt },
  { "fnext", TYPETW+TYPEPW, pfnext },
  { "format", TYPETW+TYPEPW+EFIXXCOL, uformat },
  { "groww", TYPETW, ugroww },
  { "help", TYPETW+TYPEPW+TYPETAB+TYPEQW, uhelp },
  { "iasis", TYPETW+TYPEPW+TYPETAB+TYPEHELP+EFIXXCOL+TYPEQW, uiasis },
  { "iforce", TYPETW+TYPEPW+TYPETAB+TYPEHELP+TYPEQW, uiforce },
  { "iindent", TYPETW+TYPEPW, uiindent },
  { "iindentc", TYPETW+TYPEPW, uicindent },
  { "iistep", TYPETW+TYPEPW, uiistep },
  { "ilmargin", TYPETW+TYPEPW, uilmargin },
  { "imid", TYPETW+TYPEPW+TYPETAB+TYPEHELP+TYPEQW, uimid },
  { "insc", TYPETW+TYPEPW+EFIXXCOL, uinsc },
  { "insf", TYPETW+TYPEPW+0, uinsf },
  { "ipgamnt", TYPETW+TYPEPW, uipgamnt },
  { "irmargin", TYPETW+TYPEPW+TYPETAB, uirmargin },
  { "isquare", TYPETW+TYPEPW, uisquare },
  { "istacol", TYPETW+TYPEPW+TYPETAB+TYPEHELP+TYPEQW, uistacol },
  { "istarow", TYPETW+TYPEPW+TYPETAB+TYPEHELP+TYPEQW, uistarow },
  { "itab", TYPETW+TYPEPW, uitab },
  { "itype", TYPETW+TYPEPW, uitype },
  { "iwrap", TYPETW+TYPEPW, uiwrap },
  { "keyhelp", TYPEHELP, uhkey },
  { "keytab", TYPETAB, tkey },
  { "lindent", TYPETW+TYPEPW+EFIXXCOL, ulindent },
  { "line", TYPETW+TYPEPW, uline },
  { "ltarw", TYPETW+TYPEPW+EFIXXCOL+ECHKXCOL, ultarw },
  { "ltarwhelp", TYPEHELP, uhltarw },
  { "ltarwtab", TYPETAB, tltarw },
  { "markb", TYPETW+TYPEPW+0, umarkb },
  { "markk", TYPETW+TYPEPW+0, umarkk },
  { "nedge", TYPETW+TYPEPW+EFIXXCOL, unedge },
  { "nextpos", TYPETW+TYPEPW+EFIXXCOL+EMID+EPOS, unextpos },
  { "nextw", TYPETW+TYPEPW+TYPETAB+TYPEHELP+TYPEQW, unextw },
  { "nextword", TYPETW+TYPEPW+EFIXXCOL, unxtwrd },
  { "open", TYPETW+TYPEPW+EFIXXCOL, uopen },
  { "pedge", TYPETW+TYPEPW+EFIXXCOL, upedge },
  { "pgdn", TYPETW+EMOVE, upgdn },
  { "pgup", TYPETW+EMOVE, upgup },
  { "play", TYPETW+TYPEPW+TYPETAB+TYPEHELP+TYPEQW, uplay },
  { "prevpos", TYPETW+TYPEPW+EPOS+EMID+EFIXXCOL, uprevpos },
  { "prevw", TYPETW+TYPEPW+TYPETAB+TYPEHELP+TYPEQW, uprevw },
  { "prevword", TYPETW+TYPEPW+EFIXXCOL+ECHKXCOL, uprvwrd },
  { "quote", TYPETW+TYPEPW, uquote },
  { "quote8", TYPETW+TYPEPW, uquote8 },
  { "record", TYPETW+TYPEPW+TYPETAB+TYPEHELP+TYPEQW, urecord },
  { "redo", TYPETW+TYPEPW+EFIXXCOL, uredo },
  { "retype", TYPETW+TYPEPW+TYPETAB+TYPEHELP+TYPEQW, uretyp },
  { "rindent", TYPETW+TYPEPW+EFIXXCOL, urindent },
  { "rtarw", TYPETW+TYPEPW+EFIXXCOL, urtarw },
  { "rtarwhelp", TYPEHELP, uhrtarw },
  { "rtarwtab", TYPETAB, trtarw },
  { "rtn", TYPETW+TYPEPW+EFIXXCOL, urtn },
  { "rtnhelp", TYPEHELP, uhrtn },
  { "rtnpw", TYPEPW+EMID, upromptrtn },
  { "rtntab", TYPETAB, trtn },
  { "save", TYPETW+TYPEPW, usave },
  { "shell", TYPETW+TYPEPW+TYPETAB+TYPEHELP+TYPEQW, ushell },
  { "shrinkw", TYPETW, ushrnk },
  { "splitw", TYPETW, usplitw },
  { "stat", TYPETW+TYPEPW, ustat },
  { "stop", TYPETW+TYPEPW+TYPETAB+TYPEHELP+TYPEQW, ustop },
  { "tag", TYPETW+TYPEPW, utag },
  { "tomarkb", TYPETW+TYPEPW+ECHKXCOL+EFIXXCOL, utomarkb },
  { "tomarkbk", TYPETW+TYPEPW+ECHKXCOL+EFIXXCOL, utomarkbk },
  { "tomarkk", TYPETW+TYPEPW+ECHKXCOL+EFIXXCOL, utomarkk },
  { "tomatch", TYPETW+TYPEPW+ECHKXCOL+EFIXXCOL, utomatch },
  { "type", TYPETW+TYPEPW+EFIXXCOL+EMINOR, utype },
  { "typeqw", TYPEQW, utypeqw },
  { "undo", TYPETW+TYPEPW+EFIXXCOL, uundo },
  { "uparw", TYPETW+TYPEPW+EMOVE, uuparw },
  { "uparwhelp", TYPEHELP, uhuparw },
  { "uparwtab", TYPETAB, tuparw },
  { "upslide", TYPETW+EMOVE, uupslide }
};

CMDTAB cmdtab={cmds,sizeof(cmds)/sizeof(CMD)};

/* Make windows follow cursor */

void dofollows()
{
W *w=maint->curwin;
do
 {
 if(w->y!= -1) w->watom->follow(w);
 w=(W *)(w->link.next);
 }
 while(w!=maint->curwin);
}

/* Execute a command */

void execmd(n,k)
{
BW *bw=(BW *)maint->curwin->object;
if((cmdtab.cmd[n].flag&ECHKXCOL) && bw->cursor->xcol!=bw->cursor->col)
 goto skip;
if(!(cmdtab.cmd[n].flag&maint->curwin->watom->what)) goto skip;
cmdtab.cmd[n].func(maint->curwin,k);
if(leave) return;
bw=(BW *)maint->curwin->object;

if(!(cmdtab.cmd[n].flag&EPOS) &&
   (maint->curwin->watom->what&(TYPETW|TYPEPW)))
 afterpos(maint->curwin,bw->cursor);
if(!(cmdtab.cmd[n].flag&(EMOVE|EPOS)) &&
   (maint->curwin->watom->what&(TYPETW|TYPEPW)))
 aftermove(maint->curwin,bw->cursor);

skip:
if(cmdtab.cmd[n].flag&EFIXXCOL) bw->cursor->xcol=bw->cursor->col;
if(cmdtab.cmd[n].flag&EMID)
 {
 int omid=mid; mid=1;
 dofollows();
 mid=omid;
 }
}

MACRO *curmacro=0;
int macroptr;

void exmacro(m)
MACRO *m;
{
int arg=maint->arg;
int flg=0;
maint->arg=0;

if(!arg) arg=1;

if( m->steps ||
    arg!=1 ||
    !(cmdtab.cmd[m->n].flag&EMINOR)
  ) flg=1;

if(flg) umclear();
while(arg-- && !leave)
 if(m->steps)
  {
  MACRO *tmpmac=curmacro;
  int tmpptr=macroptr;
  int x=0;
  while(m && x!=m->n && !leave)
   {
   MACRO *d;
   d=m->steps[x++];
   curmacro=m;
   macroptr=x;
   exmacro(d);
   m=curmacro;
   x=macroptr;
   }
  curmacro=tmpmac;
  macroptr=tmpptr;
  }
 else execmd(m->n,m->k);
if(leave) return;
if(flg) umclear();

undomark();
}

/* Execute a macro */

void exemac(m)
MACRO *m;
{
record(m);
exmacro(m);
}

static CONTEXT *cntxts[]=
 { &cmain,&cterm,&cprmpt,&cttab,&cfprmpt,&cthelp,&cquery,&cquerya,
   &cquerysr,0 };

/* Update screen */

void edupd()
{
W *w;
int wid,hei;
ttgtsz(&wid,&hei);
if(wid>=2 && wid!=maint->w ||
   hei>=1 && hei!=maint->h)
 {
 nresize(maint->t,wid,hei);
 sresize(maint);
 }
dofollows();
ttflsh();
nscroll(maint->t);
dsphlp(maint);
w=maint->curwin; do
 {
 if(w->y!= -1)
  {
  w->watom->disp(w);
  if(w->msgb)
   {
   msgout(w->t->t,w->y+w->h-1,w->msgb);
   w->msgb=0;
   w->t->t->updtab[w->y+w->h-1]=1;
   }
  if(w->msgt)
   {
   int y=w->h>1?1:0;
   msgout(w->t->t,w->y+y,w->msgt);
   w->msgt=0;
   w->t->t->updtab[w->y+y]=1;
   }
  }
 w=(W *)(w->link.next);
 }
 while(w!=maint->curwin);
cpos(maint->t,
     maint->curwin->x+maint->curwin->curx,
     maint->curwin->y+maint->curwin->cury);
}

int main(argc,argv)
int argc;
char *argv[];
{
char *s;
SCRN *n;
W *w;
int opened=0;
long lnum;
int omid;
int c;
if(prokbd(".joerc",cntxts))
 {
 s=getenv("HOME");
 if(!s) goto in;
 s=vsncpy(NULL,0,sz(s));
 s=vsncpy(s,sLEN(s),sc("/.joerc"));
 if(prokbd(s,cntxts))
  {
  in:;
  if(prokbd(s=JOERC,cntxts))
   {
   fprintf(stderr,M068,s);
   return 1;
   }
  }
 }
if(!(n=nopen())) return 1;
maint=screate(n);

for(c=1,lnum=0;argv[c];++c)
 if(argv[c][0]=='+' && argv[c][1])
  {
  lnum=0;
  sscanf(argv[c]+1,"%ld",&lnum);
  if(lnum) --lnum;
  }
 else if(!zcmp(argv[c],"-nobackups")) nobackups=1;
 else if(!zcmp(argv[c],"-asis")) dspasis=1;
 else if(!zcmp(argv[c],"-stacol")) stacol=1;
 else if(!zcmp(argv[c],"-starow")) starow=1;
 else if(!zcmp(argv[c],"-force")) force=1;
 else if(!zcmp(argv[c],"-help")) help=1;
 else if(!zcmp(argv[c],"-lightoff")) lightoff=1;
 else if(!zcmp(argv[c],"-exask")) exask=1;
 else
  {
  B *b=bfind(argv[c]);
  BW *bw;
  int fl=0;
  if(!b)
   {
   b=bmk(1);
   fl=bload(b,argv[c]);
   }
  w=wmktw(maint,b);
  if(fl) w->msgt=msgs[5+fl];
  bw=(BW *)w->object;
  setoptions(bw,argv[c]);
  pline(bw->cursor,lnum);
  lnum=0;
  opened=1;
  }
if(opened)
 {
 wshowall(maint);
 omid=mid; mid=1;
 dofollows();
 mid=omid;
 }
else
 {
 W *w=wmktw(maint,bmk(1));
 BW *bw=(BW *)w->object;
 setoptions(bw,"");
 }
if(help) helpon(maint);
msgnw(lastw(maint),M069);
do
 {
 MACRO *m=dokey(maint->curwin->kbd,(edupd(),ngetc(n)));
 if(m) exemac(m);
 }
 while(!leave);
nclose(n);
if(exmsg) fprintf(stderr,"\n%s\n",exmsg);
return 0;
}
