/* Command execution */

#include "config.h"
#include "vs.h"
#include "va.h"
#include "w.h"
#include "tw.h"
#include "bw.h"
#include "pw.h"
#include "tab.h"
#include "qw.h"
#include "menu.h"
#include "help.h"
#include "ublock.h"
#include "uedit.h"
#include "ufile.h"
#include "uformat.h"
#include "undo.h"
#include "usearch.h"
#include "uisrch.h"
#include "ushell.h"
#include "utag.h"
#include "poshist.h"
#include "macro.h"
#include "hash.h"
#include "rc.h"
#include "umath.h"
#include "uerror.h"
#include "path.h"
#include "cmd.h"

extern int smode;
int beep=0;
int uexecmd();

/* Command table */

CMD cmds[]=
{
  { "abort", TYPETW+TYPEPW+TYPEMENU+TYPEQW, uabort, 0, 0 },
  { "abortbuf", TYPETW, uabortbuf, 0, 0 },
  { "arg", TYPETW+TYPEPW+TYPEMENU+TYPEQW, uarg, 0, 0 },
  { "ask", TYPETW+TYPEPW, uask, 0, 0 },
  { "uarg", TYPETW+TYPEPW+TYPEMENU+TYPEQW, uuarg, 0, 0 },
  { "backs", TYPETW+TYPEPW+ECHKXCOL+EFIXXCOL+EMINOR+EKILL+EMOD, ubacks, 1, "delch" },
  { "backsmenu", TYPEMENU, umbacks, 1, 0 },
  { "backw", TYPETW+TYPEPW+ECHKXCOL+EFIXXCOL+EKILL+EMOD, ubackw, 1, "delw" },
  { "bknd", TYPETW+TYPEPW, ubknd, 0, 0 },
  { "bkwdc", TYPETW+TYPEPW+EFIXXCOL, ubkwdc, 1, "fwrdc" },
  { "blkcpy", TYPETW+TYPEPW+EFIXXCOL+EMOD, ublkcpy, 1, 0 },
  { "blkdel", TYPETW+TYPEPW+EFIXXCOL+EKILL+EMOD, ublkdel, 0, 0 },
  { "blkmove", TYPETW+TYPEPW+EFIXXCOL+EMOD, ublkmove, 0, 0 },
  { "blksave", TYPETW+TYPEPW+0, ublksave, 0, 0 },
  { "bof", TYPETW+TYPEPW+EMOVE+EFIXXCOL, ubof, 0, 0 },
  { "bofmenu", TYPEMENU, umbof, 0, 0 },
  { "bol", TYPETW+TYPEPW+EFIXXCOL, ubol, 0, 0 },
  { "bolmenu", TYPEMENU, umbol, 0, 0 },
  { "bop", TYPETW+TYPEPW+EFIXXCOL, ubop, 1, "eop" },
  { "bos", TYPETW+TYPEPW+EMOVE, ubos, 0, 0 },
  { "bufed", TYPETW, ubufed, 0, 0 },
  { "byte", TYPETW+TYPEPW, ubyte, 0, 0 },
  { "center", TYPETW+TYPEPW+EFIXXCOL+EMOD, ucenter, 1, 0 },
  { "ctrl", TYPETW+TYPEPW+EMOD, uctrl, 0, 0 },
  { "col", TYPETW+TYPEPW, ucol, 0, 0 },
  { "complete", TYPETW+TYPEPW+EMINOR+EMOD, ucmplt, 0, 0 },
  { "copy", TYPETW+TYPEPW, ucopy, 0, 0 },
  { "crawll", TYPETW+TYPEPW, ucrawll, 1, "crawlr" },
  { "crawlr", TYPETW+TYPEPW, ucrawlr, 1, "crawll" },
  { "delbol", TYPETW+TYPEPW+EFIXXCOL+EKILL+EMOD, udelbl, 1, "deleol" },
  { "delch", TYPETW+TYPEPW+ECHKXCOL+EFIXXCOL+EMINOR+EKILL+EMOD, udelch, 1, "backs" },
  { "deleol", TYPETW+TYPEPW+EKILL+EMOD, udelel, 1, "delbol" },
  { "dellin", TYPETW+TYPEPW+EFIXXCOL+EKILL+EMOD, udelln, 1, 0 },
  { "delw", TYPETW+TYPEPW+EFIXXCOL+ECHKXCOL+EKILL+EMOD, udelw, 1, "backw" },
  { "dnarw", TYPETW+TYPEPW+EMOVE, udnarw, 1, "uparw" },
  { "dnarwmenu", TYPEMENU, umdnarw, 1, "uparwmenu" },
  { "dnslide", TYPETW+TYPEPW+TYPEMENU+TYPEQW+EMOVE, udnslide, 1, "upslide" },
  { "drop", TYPETW+TYPEPW, udrop, 0, 0 },
  { "dupw", TYPETW, uduptw, 0, 0 },
  { "edit", TYPETW+TYPEPW, uedit, 0, 0 },
  { "eof", TYPETW+TYPEPW+EFIXXCOL+EMOVE, ueof, 0, 0 },
  { "eofmenu", TYPEMENU, umeof, 0, 0 },
  { "eol", TYPETW+TYPEPW+EFIXXCOL, ueol, 0, 0 },
  { "eolmenu", TYPEMENU, umeol, 0, 0 },
  { "eop", TYPETW+TYPEPW+EFIXXCOL, ueop, 1, "bop" },
  { "execmd", TYPETW+TYPEPW, uexecmd, 0, 0 },
  { "explode", TYPETW+TYPEPW+TYPEMENU+TYPEQW, uexpld, 0, 0 },
  { "exsave", TYPETW+TYPEPW, uexsve, 0, 0 },
  { "ffirst", TYPETW+TYPEPW, pffirst, 0, 0 },
  { "filt", TYPETW+TYPEPW+EMOD, ufilt, 0, 0 },
  { "fnext", TYPETW+TYPEPW, pfnext, 1, 0 },
  { "format", TYPETW+TYPEPW+EFIXXCOL+EMOD, uformat, 1, 0 },
  { "fmtblk", TYPETW+EMOD+EFIXXCOL, ufmtblk, 1, 0 },
  { "fwrdc", TYPETW+TYPEPW+EFIXXCOL, ufwrdc, 1, "bkwdc" },
  { "gomark", TYPETW+TYPEPW+EMOVE, ugomark, 0, 0 },
  { "groww", TYPETW, ugroww, 1, "shrinkw" },
  { "isrch", TYPETW+TYPEPW, uisrch, 0, 0 },
  { "killproc", TYPETW+TYPEPW, ukillpid, 0, 0 },
  { "help", TYPETW+TYPEPW+TYPEQW, uhelp, 0, 0 },
  { "hnext", TYPETW+TYPEPW+TYPEQW, uhnext, 0, 0 },
  { "hprev", TYPETW+TYPEPW+TYPEQW, uhprev, 0, 0 },
  { "insc", TYPETW+TYPEPW+EFIXXCOL+EMOD, uinsc, 1, "delch" },
  { "insf", TYPETW+TYPEPW+EMOD, uinsf, 0, 0 },
  { "lindent", TYPETW+TYPEPW+EFIXXCOL+EMOD, ulindent, 1, "rindent" },
  { "line", TYPETW+TYPEPW, uline, 0, 0 },
  { "lose", TYPETW+TYPEPW, ulose, 0, 0 },
  { "ltarw", TYPETW+TYPEPW+EFIXXCOL+ECHKXCOL, ultarw, 1, "rtarw" },
  { "ltarwmenu", TYPEMENU, umltarw, 1, "rtarwmenu" },
  { "markb", TYPETW+TYPEPW+0, umarkb, 0, 0 },
  { "markk", TYPETW+TYPEPW+0, umarkk, 0, 0 },
  { "markl", TYPETW+TYPEPW, umarkl, 0, 0 },
  { "math", TYPETW+TYPEPW, umath, 0, 0 },
  { "mode", TYPETW+TYPEPW+TYPEQW, umode, 0, 0 },
  { "msg", TYPETW+TYPEPW+TYPEQW+TYPEMENU, umsg, 0, 0 },
  { "nbuf", TYPETW, unbuf, 1, "upbuf" },
  { "nedge", TYPETW+TYPEPW+EFIXXCOL, unedge, 1, "pedge" },
  { "nextpos", TYPETW+TYPEPW+EFIXXCOL+EMID+EPOS, unextpos, 1, "prevpos" },
  { "nextw", TYPETW+TYPEPW+TYPEMENU+TYPEQW, unextw, 1, "prevw" },
  { "nextword", TYPETW+TYPEPW+EFIXXCOL, unxtwrd, 1, "prevword" },
  { "nmark", TYPETW+TYPEPW, unmark, 0, 0 },
  { "notmod", TYPETW, unotmod, 0, 0 },
  { "nxterr", TYPETW, unxterr, 1, "prverr" },
  { "open", TYPETW+TYPEPW+EFIXXCOL+EMOD, uopen, 1, "deleol" },
  { "parserr", TYPETW, uparserr, 0, 0 },
  { "pbuf", TYPETW, upbuf, 1, "unbuf" },
  { "pedge", TYPETW+TYPEPW+EFIXXCOL, upedge, 1, "nedge" },
  { "pgdn", TYPETW+TYPEPW+TYPEMENU+TYPEQW+EMOVE, upgdn, 1, "pgup" },
  { "pgup", TYPETW+TYPEPW+TYPEMENU+TYPEQW+EMOVE, upgup, 1, "pgdn" },
  { "play", TYPETW+TYPEPW+TYPEMENU+TYPEQW, uplay, 1, 0 }, /* EFIXX? */
  { "prevpos", TYPETW+TYPEPW+EPOS+EMID+EFIXXCOL, uprevpos, 1, "nextpos" },
  { "prevw", TYPETW+TYPEPW+TYPEMENU+TYPEQW, uprevw, 1, "nextw" },
  { "prevword", TYPETW+TYPEPW+EFIXXCOL+ECHKXCOL, uprvwrd, 1, "nextword" },
  { "prverr", TYPETW, uprverr, 1, "nxterr" },
  { "psh", TYPETW+TYPEPW+TYPEMENU+TYPEQW, upsh, 0, 0 },
  { "pop", TYPETW+TYPEPW+TYPEMENU+TYPEQW, upop, 0, 0 },
  { "qrepl", TYPETW+TYPEPW+EMOD, pqrepl, 0, 0 },
  { "query", TYPETW+TYPEPW+TYPEMENU+TYPEQW, uquery, 0, 0 },
  { "quote", TYPETW+TYPEPW+EMOD, uquote, 0, 0 },
  { "quote8", TYPETW+TYPEPW+EMOD, uquote8, 0, 0 },
  { "record", TYPETW+TYPEPW+TYPEMENU+TYPEQW, urecord, 0, 0 },
  { "redo", TYPETW+TYPEPW+EFIXXCOL, uredo, 1, "undo" },
  { "retype", TYPETW+TYPEPW+TYPEMENU+TYPEQW, uretyp, 0, 0 },
  { "rfirst", TYPETW+TYPEPW, prfirst, 0, 0 },
  { "rindent", TYPETW+TYPEPW+EFIXXCOL+EMOD, urindent, 1, "lindent" },
  { "run", TYPETW+TYPEPW, urun, 0, 0 },
  { "rsrch", TYPETW+TYPEPW, ursrch, 0, 0 },
  { "rtarw", TYPETW+TYPEPW+EFIXXCOL, urtarw, 1, "ltarw" },
  { "rtarwmenu", TYPEMENU, umrtarw, 1, "ltarwmenu" },
  { "rtn", TYPETW+TYPEPW+TYPEMENU+TYPEQW+EMOD, urtn, 1, 0 },
  { "save", TYPETW+TYPEPW, usave, 0, 0 },
  { "setmark", TYPETW+TYPEPW, usetmark, 0, 0 },
  { "shell", TYPETW+TYPEPW+TYPEMENU+TYPEQW, ushell, 0, 0 },
  { "shrinkw", TYPETW, ushrnk, 1, "groww" },
  { "splitw", TYPETW, usplitw, 0, 0 },
  { "stat", TYPETW+TYPEPW, ustat, 0, 0 },
  { "stop", TYPETW+TYPEPW+TYPEMENU+TYPEQW, ustop, 0, 0 },
  { "swap", TYPETW+TYPEPW+EFIXXCOL, uswap, 0, 0 },
  { "tag", TYPETW+TYPEPW, utag, 0, 0 },
  { "tomarkb", TYPETW+TYPEPW+EFIXXCOL, utomarkb, 0, 0 },
  { "tomarkbk", TYPETW+TYPEPW+EFIXXCOL, utomarkbk, 0, 0 },
  { "tomarkk", TYPETW+TYPEPW+EFIXXCOL, utomarkk, 0, 0 },
  { "tomatch", TYPETW+TYPEPW+EFIXXCOL, utomatch, 0, 0 },
  { "tos", TYPETW+TYPEPW+EMOVE, utos, 0, 0 },
  { "tw0", TYPETW+TYPEPW+TYPEQW+TYPEMENU, utw0, 0, 0 },
  { "tw1", TYPETW+TYPEPW+TYPEQW+TYPEMENU, utw1, 0, 0 },
  { "txt", TYPETW+TYPEPW, utxt, 0, 0 },
  { "type", TYPETW+TYPEPW+TYPEQW+TYPEMENU+EMINOR+EMOD, utype, 1, "backs" },
  { "undo", TYPETW+TYPEPW+EFIXXCOL, uundo, 1, "redo" },
  { "uparw", TYPETW+TYPEPW+EMOVE, uuparw, 1, "dnarw" },
  { "uparwmenu", TYPEMENU, umuparw, 1, "dnarwmenu" },
  { "upslide", TYPETW+TYPEPW+TYPEMENU+TYPEQW+EMOVE, uupslide, 1, "dnslide" },
  { "yank", TYPETW+TYPEPW+EFIXXCOL+EMOD, uyank, 1, 0 },
  { "yapp", TYPETW+TYPEPW+EKILL, uyapp, 0, 0 },
  { "yankpop", TYPETW+TYPEPW+EFIXXCOL+EMOD, uyankpop, 1, 0 }
};

/* Execute a command n with key k */

int execmd(n,k)
 {
 BW *bw=(BW *)maint->curwin->object;
 int ret= -1;

 /* We don't execute if we have to fix the column position first
  * (i.e., left arrow when cursor is in middle of nowhere) */
 if((cmds[n].flag&ECHKXCOL) && bw->cursor->xcol!=piscol(bw->cursor))
  goto skip;

 /* Don't execute command if we're in wrong type of window */
 if(!(cmds[n].flag&maint->curwin->watom->what)) goto skip;

 if((maint->curwin->watom->what&TYPETW) && bw->b->rdonly &&
    (cmds[n].flag&EMOD))
  {
  msgnw(bw,"Read only");
  if(beep) ttputc(7);
  goto skip;
  }

 /* Execute command */
 ret=cmds[n].func(maint->curwin->object,k);

 if(smode) --smode;

 /* Don't update anything if we're going to leave */
 if(leave) return 0;

 bw=(BW *)maint->curwin->object;

 /* Maintain position history */
 /* If command was not a positioning command */
 if(!(cmds[n].flag&EPOS) &&
    (maint->curwin->watom->what&(TYPETW|TYPEPW)))
  afterpos();

 /* If command was not a movement */
 if(!(cmds[n].flag&(EMOVE|EPOS)) &&
    (maint->curwin->watom->what&(TYPETW|TYPEPW)))
  aftermove(maint->curwin,bw->cursor);

 if(cmds[n].flag&EKILL) justkilled=1;
 else justkilled=0;

 skip:

 /* Make dislayed cursor column equal the actual cursor column
  * for commands which arn't simple vertical movements */
 if(cmds[n].flag&EFIXXCOL) bw->cursor->xcol=piscol(bw->cursor);

 /* Recenter cursor to middle of screen */
 if(cmds[n].flag&EMID)
  {
  int omid=mid; mid=1;
  dofollows();
  mid=omid;
  }

 if(beep && ret) ttputc(7);
 return ret;
 }

/* Return command table index for given command name */

HASH *cmdhash=0;

int findcmd(s)
char *s;
 {
 CMD *cmd;
 if(!cmdhash)
  {
  int x;
  cmdhash=htmk(256);
  for(x=0;x!=sizeof(cmds)/sizeof(CMD);++x) htadd(cmdhash,cmds[x].name,cmds+x);
  }
 cmd=(CMD *)htfind(cmdhash,s);
 if(cmd) return cmd-(CMD *)cmds;
 else return -1;
 }

char **getcmds()
 {
 char **s=vatrunc(NULL,sizeof(cmds)/sizeof(CMD));
 int x;
 for(x=0;x!=sizeof(cmds)/sizeof(CMD);++x)
  s[x]=vsncpy(NULL,0,sz(cmds[x].name));
 return s;
 }

/* Command line */

char **scmds=0;

char **regsub(z,len,s)
char **z;
char *s;
 {
 char **lst=0;
 int x;
 for(x=0;x!=len;++x) if(rmatch(s,z[x])) lst=vaadd(lst,vsncpy(NULL,0,sz(z[x])));
 return lst;
 }

void inscmd(bw,line)
BW *bw;
char *line;
 {
 P *p=pdup(bw->cursor); pbol(p);
 peol(bw->cursor);
 bdel(p,bw->cursor);
 binsm(bw->cursor,sv(line)); peol(bw->cursor);
 prm(p);
 bw->cursor->xcol=piscol(bw->cursor);
 }

int cmdabrt(bw,x,line)
BW *bw;
char *line;
 {
 if(line) inscmd(bw,line), vsrm(line);
 return -1;
 }

int cmdrtn(m,x,line)
MENU *m;
char *line;
 {
 inscmd(m->parent->win->object,m->list[x]);
 vsrm(line);
 m->object=0;
 wabort(m->parent);
 return 0;
 }

int cmdcmplt(bw)
BW *bw;
 {
 MENU *m;
 P *p, *q; 
 char *line;
 char *line1;
 char **lst;
 if(!scmds) scmds=getcmds();
 p=pdup(bw->cursor); pbol(p);
 q=pdup(bw->cursor); peol(q);
 line=brvs(p,(int)(q->byte-p->byte)); /* Assumes short lines :-) */
 prm(p); prm(q);
 m=mkmenu(bw,NULL,cmdrtn,cmdabrt,NULL,0,line,NULL);
 if(!m) return -1;
 line1=vsncpy(NULL,0,sv(line));
 line1=vsadd(line1,'*');
 lst=regsub(scmds,aLEN(scmds),line1);
 vsrm(line1);
 ldmenu(m,lst,0);
 if(!lst)
  {
  wabort(m->parent);
  ttputc(7);
  return -1;
  }
 else
  {
  if(aLEN(lst)==1) return cmdrtn(m,0,line);
  else if(smode || isreg(line)) return 0;
  else
   {
   char *com=mcomplete(m);
   vsrm(m->object);
   m->object=com;
   wabort(m->parent);
   smode=2;
   ttputc(7);
   return 0;
   }
  }
 }

int docmd(bw,s,object,notify)
BW *bw;
char *s;
void *object;
int *notify;
 {
 MACRO *mac;
 int ret=findcmd(s);
 if(ret<0)
  msgnw(bw,"No such command");
 else
  {
  mac=mkmacro(MAXINT,0,ret);
  ret=exmacro(mac,1);
  rmmacro(mac);
  }
 if(notify) *notify=1;
 return ret;
 }

B *cmdhist=0;

int uexecmd(bw)
BW *bw;
 {
 if(wmkpw(bw,"cmd: ",&cmdhist,docmd,"cmd",NULL,cmdcmplt,NULL,NULL)) return 0;
 else return -1;
 }
