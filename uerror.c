/* Compiler error handler */

#include "queue.h"
#include "b.h"
#include "vs.h" 
#include "ufile.h"
#include "w.h"
#include "bw.h"
#include "zstr.h"
#include "ufile.h"
#include "main.h"
#include "uerror.h"

/* Error database */

typedef struct error ERROR;

struct error
 {
 LINK(ERROR) link;	/* Linked list of errors */
 long line;			/* Target line number */
 long org;			/* Original target line number */
 char *file;			/* Target file name */
 long src;			/* Error-file line number */
 char *msg;			/* The message */
 } errors={{&errors,&errors}};
ERROR *errptr= &errors;	/* Current error row */

B *errbuf=0;		/* Buffer with error messages */

/* Insert and delete notices */

void inserr(name,where,n,bol)
char *name;
long where;
long n;
 {
 ERROR *e;
 if(name)
  for(e=errors.link.next;e!=&errors;e=e->link.next)
   if(!zcmp(e->file,name))
    if(e->line>where) e->line+=n;
    else if(e->line==where && bol) e->line+=n;
 }

void delerr(name,where,n)
char *name;
long where;
long n;
 {
 ERROR *e;
 if(name)
  for(e=errors.link.next;e!=&errors;e=e->link.next)
   if(!zcmp(e->file,name))
    if(e->line>where+n) e->line-=n;
    else if(e->line>where) e->line=where;
 }

/* Abort notice */

void abrerr(name)
char *name;
 {
 ERROR *e;
 if(name)
  for(e=errors.link.next;e!=&errors;e=e->link.next)
   if(!zcmp(e->file,name)) e->line=e->org;
 }

/* Save notice */

void saverr(name)
char *name;
 {
 ERROR *e;
 if(name)
  for(e=errors.link.next;e!=&errors;e=e->link.next)
   if(!zcmp(e->file,name)) e->org=e->line;
 }

/* Pool of free error nodes */
ERROR errnodes={{&errnodes,&errnodes}};

/* Free an error node */

void freeerr(n)
ERROR *n;
 {
 vsrm(n->file);
 vsrm(n->msg);
 enquef(ERROR,link,&errnodes,n);
 }

/* Free all errors */

void freeall()
 {
 while(!qempty(ERROR,link,&errors))
  freeerr(deque(ERROR,link,errors.link.next));
 errptr= &errors;
 }

/* Parse error messages into database */

int parseit(s,row)
char *s;
long row;
 {
 int x, y;
 char *name=0;
 long line= -1;
 ERROR *err;

 /* Skip to first word */
 for(x=0;s[x] && !crest(s[x]);++x);
 /* Skip to end of first word */
 for(y=x;crest(s[y]) || s[y]=='.';++y);

 /* Save file name */ 
 if(x!=y) name=vsncpy(NULL,0,s+x,y-x);
 
 /* Skip to first number */
 for(x=y;s[x] && (s[x]<'0' || s[x]>'9');++x);

 /* Skip to end of first number */
 for(y=x;s[y]>='0' && s[y]<='9';++y);
 
 /* Save line number */
 if(x!=y) sscanf(s+x,"%ld",&line);
 if(line!= -1) --line;

 if(name)
  if(line!= -1)
   {
   /* We have an error */
   err=(ERROR *)alitem(&errnodes,sizeof(ERROR));
   err->file=name;
   err->org=err->line=line;
   err->src=row;
   err->msg=vsncpy(NULL,0,sc("\\i"));
   err->msg=vsncpy(sv(err->msg),sv(s));
   enqueb(ERROR,link,&errors,err);
   return 1;
   }
  else vsrm(name);
 return 0;
 }

/* Parse the error output contained in a buffer */

long parserr(b)
B *b;
 {
 P *p=pdup(b->bof);
 P *q=pdup(p);
 long nerrs=0;
 freeall();
 do
  {
  char *s;
  pset(q,p); peol(p);
  s=brvs(q,(int)(p->byte-q->byte));
  if(s)
   {
   nerrs+=parseit(s,q->line);
   vsrm(s);
   }
  } while(pgetc(p)!= MAXINT);
 prm(p); prm(q);
 return nerrs;
 }

int uparserr(bw)
BW *bw;
 {
 errbuf=bw->b;
 freeall();
 sprintf(msgbuf,"Parsed %ld lines",parserr(bw->b));
 msgnw(bw,msgbuf);
 return 0;
 }

int unxterr(bw)
BW *bw;
 {
 int omid;
 if(errptr->link.next==&errors)
  {
  msgnw(bw,"No more errors");
  return -1;
  }
 errptr=errptr->link.next;
 if(!bw->b->name || zcmp(errptr->file,bw->b->name))
  {
  if(doedit(bw,vsdup(errptr->file),NULL,NULL)) return -1;
  bw=(BW *)maint->curwin->object;
  }
 omid=mid; mid=1;
 pline(bw->cursor,errptr->line);
 setline(errbuf,errptr->src);
 dofollows();
 mid=omid;
 bw->cursor->xcol=piscol(bw->cursor);
 msgnw(bw,errptr->msg);
 return 0;
 }

int uprverr(bw)
BW *bw;
 {
 int omid;
 if(errptr->link.prev==&errors)
  {
  msgnw(bw,"No more errors");
  return -1;
  }
 errptr=errptr->link.prev;
 if(!bw->b->name || zcmp(errptr->file,bw->b->name))
  {
  if(doedit(bw,vsdup(errptr->file),NULL,NULL)) return -1;
  bw=(BW *)maint->curwin->object;
  }
 omid=mid; mid=1;
 pline(bw->cursor,errptr->line);
 setline(errbuf,errptr->src);
 dofollows();
 mid=omid;
 bw->cursor->xcol=piscol(bw->cursor);
 msgnw(bw,errptr->msg);
 return 0;
 }
