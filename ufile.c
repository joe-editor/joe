/* User file operations
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
#include <fcntl.h>
#include <sys/stat.h>
#include "config.h"
#include <string.h>

#ifdef UTIME
#include <utime.h>
#define HAVEUTIME 1
#else
#ifdef SYSUTIME
#include <sys/utime.h>
#define HAVEUTIME 1
#endif
#endif

#include "b.h"
#include "bw.h"
#include "scrn.h"
#include "tw.h"
#include "w.h"
#include "pw.h"
#include "qw.h"
#include "ublock.h"
#include "main.h"
#include "zstr.h"
#include "vs.h"
#include "va.h"
#include "menu.h"
#include "path.h"
#include "ublock.h"
#include "tty.h"
#include "tab.h"
#include "uerror.h"
#include "macro.h"
#include "ufile.h"

extern int orphan;
char *backpath=0;			/* Place to store backup files */
static B *filehist=0;			/* History of file names */
int nobackups=0;
int exask=0;

/* Ending message generator */

void genexmsg(bw,saved,name)
BW *bw;
char *name;
 {
 char *s;
 if(bw->b->name && bw->b->name[0]) s=bw->b->name;
 else s="(Unnamed)";

 if(name)
  if(saved)
   sprintf(msgbuf,"File %s saved",name);
  else
   sprintf(msgbuf,"File %s not saved",name);
 else
  if(bw->b->changed && bw->b->count==1)
   sprintf(msgbuf,"File %s not saved",s);
  else if(saved)
   sprintf(msgbuf,"File %s saved",s);
  else
   sprintf(msgbuf,"File %s not changed so no update needed",s);
 msgnw(bw,msgbuf);

 if(!exmsg)
  if(bw->b->changed && bw->b->count==1)
   {
   exmsg=vsncpy(NULL,0,sc("File "));
   exmsg=vsncpy(sv(exmsg),sz(s));
   exmsg=vsncpy(sv(exmsg),sc(" not saved."));
   }
  else if(saved)
   {
   exmsg=vsncpy(NULL,0,sc("File "));
   exmsg=vsncpy(sv(exmsg),sz(s));
   exmsg=vsncpy(sv(exmsg),sc(" saved."));
   }
  else
   {
   exmsg=vsncpy(NULL,0,sc("File "));
   exmsg=vsncpy(sv(exmsg),sz(s));
   exmsg=vsncpy(sv(exmsg),sc(" not changed so no update needed."));
   }
 }

/* Write highlighted block to a file */

int ublksave(bw)
BW *bw;
 {
 if(markb && markk && markb->b==markk->b &&
    (markk->byte-markb->byte)>0 &&
    (!square || piscol(markk)>piscol(markb)))
  {
  if(wmkpw(bw,
           "Name of file to write (^C to abort): ",&filehist,dowrite,"Names",NULL,cmplt,NULL,NULL)) return 0;
  else return -1;
  }
 else return usave(bw);
 }

/* Shell escape */

int ushell(bw)
BW *bw;
 {
 nescape(bw->parent->t->t);
 ttsusp();
 nreturn(bw->parent->t->t);
 return 0;
 }

/* Copy a file */

int cp(from,to)
char *from, *to;
 {
 int f, g, amnt;
 struct stat sbuf;

#ifdef HAVEUTIME
#ifdef NeXT
 time_t utbuf[2];
#else
 struct utimbuf utbuf;
#endif
#endif

 f=open(from,O_RDONLY);
 if(f<0) return -1;
 if(fstat(f,&sbuf)<0) return -1;
 g=creat(to,sbuf.st_mode);
 if(g<0)
  {
  close(f);
  return -1;
  }
 while((amnt=read(f,stdbuf,stdsiz))>0)
  if(amnt!=write(g,stdbuf,amnt)) break;
 close(f); close(g);
 if(amnt) return -1;

#ifdef HAVEUTIME
#ifdef NeXT
 utbuf[0]=(time_t)sbuf.st_atime;
 utbuf[1]=(time_t)sbuf.st_mtime;
#else
 utbuf.actime=sbuf.st_atime;
 utbuf.modtime=sbuf.st_mtime;
#endif
 utime(to,&utbuf);
#endif

 return 0;
 }

/* Make backup file if it needs to be made
 * Returns 0 if backup file was made or didn't need to be made
 * Returns 1 for error
 */

static int backup(bw)
BW *bw;
 {
 if(!bw->b->backup && !nobackups && bw->b->name && bw->b->name[0])
  {
  char tmp[1024];
  char name[1024];
  int x;

#ifdef __MSDOS__

  if(backpath)
   sprintf(name,"%s/%s",backpath,namepart(tmp,bw->b->name));
  else
   sprintf(name,"%s",bw->b->name);
  
  ossep(name);

  for(x=strlen(name);name[--x]!='.';)
   if(name[x]=='\\' || (name[x]==':' && x==1) || x==0)
    {
    x=strlen(name);
    break;
    }

  strcpy(name+x,".bak");

#else

  /* Create backup file name */
  if(backpath)
   sprintf(name,"%s/%s~",backpath,namepart(tmp,bw->b->name));
  else
   sprintf(name,"%s~",bw->b->name);

  /* Attempt to delete backup file first */
  unlink(name);

#endif

  /* Copy original file to backup file */
  if(cp(bw->b->name,name))
   {
   return 1;
   }
  else
   {
   bw->b->backup=1;
   return 0;
   }
  }
 else return 0;
 }

/* Write file */

struct savereq
 {
 int (*callback)();
 char *name;
 };

static int saver(bw,c,req,notify)
BW *bw;
int c;
struct savereq *req;
int *notify;
 {
 int (*callback)();
 int fl;
 callback=req->callback;
 if(c=='n' || c=='N')
  {
  vsrm(req->name);
  free(req);
  if(notify) *notify=1;
  msgnw(bw,"Couldn't make backup file... file not saved");
  if(callback) return callback(bw,-1);
  else return -1;
  }
 if(c!='y' && c!='Y')
  if(mkqw(bw,sc("Could not make backup file.  Save anyway (y,n,^C)? "),saver,NULL,req,notify))
   return 0;
  else
   {
   if(notify) *notify=1;
   return -1;
   }
 if(notify) *notify=1;
 if(bw->b->er== -1 && bw->o.msnew) exemac(bw->o.msnew), bw->b->er= -3;
 if(bw->b->er== 0 && bw->o.msold) exemac(bw->o.msold);
 if(fl=bsave(bw->b->bof,req->name,bw->b->eof->byte))
  {
  msgnw(bw,msgs[fl+5]);
  vsrm(req->name);
  free(req);
  if(callback) return callback(bw,-1);
  else return -1;
  }
 else
  {
  if(!bw->b->name) bw->b->name=joesep(strdup(req->name));
  if(!strcmp(bw->b->name,req->name))
   {
   bw->b->changed=0;
   saverr(bw->b->name);
   }
  genexmsg(bw,1,req->name);
  vsrm(req->name);
  free(req);
  if(callback) return callback(bw,0);
  else return 0;
  }
 }

static int dosave(bw,s,callback,notify)
BW *bw;
char *s;
int (*callback)();
int *notify;
 {
 struct savereq *req=(struct savereq *)malloc(sizeof(struct savereq));
 req->name=s;
 req->callback=callback;
 if(backup(bw)) saver(bw,0,req,notify);
 else saver(bw,'y',req,notify);
 }

static int dosave2(bw,c,s,notify)
BW *bw;
char *s;
int *notify;
 {
 if(c=='y' || c=='Y') return dosave(bw,s,NULL,notify);
 else if(c=='n' || c=='N')
  {
  if(notify) *notify=1;
  genexmsg(bw,0,s);
  vsrm(s);
  return -1;
  }
 else
  if(mkqw(bw,sc("File exists.  Overwrite (y,n,^C)? "),dosave2,NULL,s,notify)) return 0;
  else return -1;
 }

static int dosave1(bw,s,object,notify)
BW *bw;
char *s;
void *object;
int *notify;
 {
 int f;
 if(s[0]!='!' && !(s[0]=='>' && s[1]=='>') && (!bw->b->name || strcmp(s,bw->b->name)))
  {
  f=open(s,O_RDONLY);
  if(f!= -1)
   {
   close(f);
   return dosave2(bw,0,s,notify);
   }
  }
 return dosave(bw,s,object,notify);
 }

int usave(bw)
BW *bw;
 {
 BW *pbw=wmkpw(bw,
                "Name of file to save (^C to abort): ",&filehist,dosave1,"Names",NULL,cmplt,NULL,NULL);
 if(pbw && bw->b->name)
  {
  binss(pbw->cursor,bw->b->name);
  pset(pbw->cursor,pbw->b->eof);
  pbw->cursor->xcol=piscol(pbw->cursor);
  }
 if(pbw) return 0;
 else return -1;
 }

/* Load file to edit */

int doedit(bw,s,obj,notify)
BW *bw;
char *s;
void *obj;
int *notify;
 {
 int ret=0;
 int er;
 void *object;
 W *w;
 B *b;
 if(notify) *notify=1;
 if(bw->pid)
  {
  msgnw(bw,"Process running in this window");
  return -1;
  }
 b=bfind(s);
 er=error;
 if(bw->b->count==1 && (bw->b->changed || bw->b->name))
  if(orphan) orphit(bw);
  else
   {
   if(uduptw(bw))
    {
    brm(b);
    return -1;
    }
   bw=(BW *)maint->curwin->object;
   }
 if(er)
  {
  msgnwt(bw,msgs[er+5]);
  if(er!= -1) ret= -1;
  }
 object=bw->object;
 w=bw->parent;
 bwrm(bw);
 w->object=(void *)(bw=bwmk(w,b,0));
 wredraw(bw->parent);
 bw->object=object;
 vsrm(s);
 if(er== -1 && bw->o.mnew)
  exemac(bw->o.mnew);
 if(er==0 && bw->o.mold)
  exemac(bw->o.mold);
 return ret;
 }

int okrepl(bw)
BW *bw;
 {
 if(bw->b->count==1 && bw->b->changed)
  {
  msgnw(bw,"Can't replace modified file");
  return -1;
  }
 else return 0;
 }

int uedit(bw)
BW *bw;
 {
 if(wmkpw(bw,"Name of file to edit (^C to abort): ",&filehist,doedit,"Names",NULL,cmplt,NULL,NULL)) return 0;
 else return -1;
 }

/* Load file into buffer: can result in an orphaned buffer */

int dorepl(bw,s,obj,notify)
BW *bw;
char *s;
void *obj;
int *notify;
 {
 void *object=bw->object;
 int ret=0;
 int er;
 W *w=bw->parent;
 B *b;
 if(notify) *notify=1;
 if(bw->pid)
  {
  msgnw(bw,"Process running in this window");
  return -1;
  }
 b=bfind(s);
 er=error;
 if(error)
  {
  msgnwt(bw,msgs[error+5]);
  if(error!= -1) ret= -1;
  }
 if(bw->b->count==1 && (bw->b->changed || bw->b->name))
  orphit(bw);
 bwrm(bw);
 w->object=(void *)(bw=bwmk(w,b,0));
 wredraw(bw->parent);
 bw->object=object;
 vsrm(s);
 if(er== -1 && bw->o.mnew)
  exemac(bw->o.mnew);
 if(er==0 && bw->o.mold)
  exemac(bw->o.mold);
 return ret;
 }

/* Switch to next buffer in window */

int unbuf(bw)
BW *bw;
 {
 void *object=bw->object;
 W *w=bw->parent;
 B *b;
 if(bw->pid)
  {
  msgnw(bw,"Process running in this window");
  return -1;
  }
 b=bnext();
 if(b==bw->b) b=bnext();
 if(b==bw->b) return -1;
 if(!b->orphan) ++b->count;
 else b->orphan=0;
 if(bw->b->count==1) orphit(bw);
 bwrm(bw);
 w->object=(void *)(bw=bwmk(w,b,0));
 wredraw(bw->parent);
 bw->object=object;
 return 0;
 }

int upbuf(bw)
BW *bw;
 {
 void *object=bw->object;
 W *w=bw->parent;
 B *b;
 if(bw->pid)
  {
  msgnw(bw,"Process running in this window");
  return -1;
  }
 b=bprev();
 if(b==bw->b) b=bprev();
 if(b==bw->b) return -1;
 if(!b->orphan) ++b->count;
 else b->orphan=0;
 if(bw->b->count==1) orphit(bw);
 bwrm(bw);
 w->object=(void *)(bw=bwmk(w,b,0));
 wredraw(bw->parent);
 bw->object=object;
 return 0;
 }

int uinsf(bw)
BW *bw;
 {
 if(wmkpw(bw,
           "Name of file to insert (^C to abort): ",&filehist,doinsf,"Names",NULL,cmplt,NULL,NULL)) return 0;
 else return -1;
 }

/* Save and exit */

static int exdone(bw,flg)
BW *bw;
 {
 if(flg)
  {
  if(bw->b->name) free(bw->b->name);
  bw->b->name=0;
  return -1;
  }
 else
  {
  bw->b->changed=0;
  saverr(bw->b->name);
  return uabort(bw,MAXINT);
  }
 }

static int exdone1(bw,flg)
BW *bw;
 {
 if(flg)
  {
  return -1;
  }
 else
  {
  bw->b->changed=0;
  saverr(bw->b->name);
  return uabort(bw,MAXINT);
  }
 }

static int doex(bw,s,object,notify)
BW *bw;
char *s;
void *object;
int *notify;
 {
 bw->b->name=joesep(strdup(s)); 
 return dosave(bw,s,exdone,notify);
 }

int uexsve(bw)
BW *bw;
 {
 if(!bw->b->changed) { uabort(bw,MAXINT); return 0; }
 else if(bw->b->name && !exask)
  {
  return dosave(bw,vsncpy(NULL,0,sz(bw->b->name)),exdone1,NULL);
  } 
 else
  {
  BW *pbw=wmkpw(bw,
                "Name of file to save (^C to abort): ",&filehist,doex,"Names",NULL,cmplt,NULL,NULL);
  if(pbw && bw->b->name)
   {
   binss(pbw->cursor,bw->b->name);
   pset(pbw->cursor,pbw->b->eof);
   pbw->cursor->xcol=piscol(pbw->cursor);
   }
  if(pbw) return 0;
  else return -1;
  }
 }

/* If buffer is modified, prompt for saving */

static int nask(bw,c,object,notify)
BW *bw;
void *object;
int *notify;
 {
 if(c=='y' || c=='Y')
  if(bw->b->name) return dosave1(bw,vsncpy(NULL,0,sz(bw->b->name)),object,notify);
  else
   {
   BW *pbw=wmkpw(bw,
                 "Name of file to save (^C to abort): ",&filehist,dosave1,"Names",NULL,cmplt,object,notify);
   if(pbw) return 0;
   else return -1;
   }
 else if(c=='n' || c=='N')
  {
  genexmsg(bw,0,NULL);
  if(notify) *notify=1;
  return 0;
  }
 else
  if(bw->b->count==1 && bw->b->changed)
   {
   if(mkqw(bw,sc("Save changes to this file (y,n,^C)? "),nask,NULL,object,notify)) return 0;
   else return -1;
   }
  else
   {
   if(notify) *notify=1;
   return 0;
   }
 }

int uask(bw)
BW *bw;
 {
 return nask(bw,0,NULL,NULL);
 }

/* Ask to save file if it is modified.  If user answers yes, run save */

static int nask2(bw,c,object,notify)
BW *bw;
void *object;
int *notify;
 {
 if(c=='y' || c=='Y')
  {
  BW *pbw=wmkpw(bw,
                "Name of file to save (^C to abort): ",&filehist,dosave1,"Names",NULL,cmplt,object,notify);
  if(pbw) return 0;
  else return -1;
  }
 else if(c=='n' || c=='N')
  {
  genexmsg(bw,0,NULL);
  if(notify) *notify=1;
  return 0;
  }
 else
  if(bw->b->count==1 && bw->b->changed)
   {
   if(mkqw(bw,sc("Save changes to this file (y,n,^C)? "),nask,NULL,object,notify)) return 0;
   else return -1;
   }
  else
   {
   if(notify) *notify=1;
   return 0;
   }
 }

int uask2(bw)
BW *bw;
 {
 return nask2(bw,0,NULL,NULL);
 }

/* If buffer is modified, ask if it's ok to lose changes */

int dolose(bw,c,object,notify)
BW *bw;
void *object;
int *notify;
 {
 W *w;
 if(notify) *notify=1;
 if(c!='y' && c!='Y') return -1;
 genexmsg(bw,0,NULL);
 if(bw->b->count==1) bw->b->changed=0;
 object=bw->object; w=bw->parent;
 bwrm(bw);
 w->object=(void *)(bw=bwmk(w,bfind(""),0));
 wredraw(bw->parent);
 bw->object=object;
 if(bw->o.mnew) exemac(bw->o.mnew);
 return 0;
 }

int ulose(bw)
BW *bw;
 {
 msgnw(bw,NULL);
 if(bw->pid) return ukillpid(bw);
 if(bw->b->count==1 && bw->b->changed)
  if(mkqw(bw,sc("Lose changes to this file (y,n,^C)? "),dolose,NULL,NULL,NULL)) return 0;
  else return -1;
 else return dolose(bw,'y',NULL,NULL);
 }

/* Buffer list */

int dobuf(m,x,s)
MENU *m;
char **s;
 {
 char *name;
 BW *bw=m->parent->win->object;
 int *notify=m->parent->notify;
 m->parent->notify=0;
 name=vsdup(s[x]);
 uabort(m,MAXINT);
 return dorepl(bw,name,NULL,notify);
 }

int abrtb(m,x,s)
MENU *m;
char **s;
 {
 varm(s);
 return -1;
 }

int ubufed(bw)
BW *bw;
 {
 char **s=getbufs();
 vasort(av(s));
 if(mkmenu(bw,s,dobuf,abrtb,NULL,0,s,NULL)) return 0;
 else
  {
  varm(s);
  return -1;
  }
 }
