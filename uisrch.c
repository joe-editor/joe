/* Incremental search */

#include "bw.h"
#include "qw.h"
#include "vs.h"
#include "usearch.h"
#include "main.h"
#include "uisrch.h"

char *lastpat=0;			/* Previous pattern */

IREC fri={{&fri,&fri}};			/* Free-list of irecs */

IREC *alirec()				/* Allocate an IREC */
 {
 return alitem(&fri,sizeof(IREC));
 }

void frirec(i)				/* Free an IREC */
IREC *i;
 {
 enquef(IREC,link,&fri,i);
 }

void rmisrch(isrch)			/* Eliminate a struct isrch */
struct isrch *isrch;
 {
 if(isrch)
  {
  vsrm(isrch->pattern);
  frchn(&fri,&isrch->irecs);
  free(isrch);
  }
 }

int iabrt(bw,isrch)			/* User hit ^C */
BW *bw;
struct isrch *isrch;
 {
 rmisrch(isrch);
 return -1;
 }

void iappend(bw,isrch,s,len)		/* Append text and search */
BW *bw;
struct isrch *isrch;
char *s;
 { /* Append char and search */
 IREC *i=alirec();
 i->what=len;
 i->disp=bw->cursor->byte;
 isrch->pattern=vsncpy(sv(isrch->pattern),s,len);
 if(!qempty(IREC,link,&isrch->irecs)) pgoto(bw->cursor,isrch->irecs.link.prev->start);
 i->start=bw->cursor->byte;
 if(dopfnext(bw,mksrch(vsncpy(NULL,0,isrch->pattern+isrch->ofst,sLen(isrch->pattern)-isrch->ofst),NULL,0,isrch->dir,-1,0,0),NULL))
  ttputc(7);
 enqueb(IREC,link,&isrch->irecs,i);
 }

int itype(bw,c,isrch,notify)		/* Main user interface */
BW *bw;
struct isrch *isrch;
int *notify;
 {
 IREC *i;
 int omid;
 if(isrch->quote) goto in;
 if(c==8 || c==127)
  { /* Backup */
  if((i=isrch->irecs.link.prev)!=&isrch->irecs)
   {
   pgoto(bw->cursor,i->disp);
   omid=mid; mid=1; dofollows(); mid=omid;
   isrch->pattern=vstrunc(isrch->pattern,sLEN(isrch->pattern)-i->what);
   frirec(deque(IREC,link,i));
   }
  else ttputc(7);
  }
 else if(c=='Q'-'@' || c=='`') isrch->quote=1;
 else if(c=='S'-'@' || c=='\\'-'@' || c=='L'-'@' || c=='R'-'@')
  { /* Repeat */
  if(c=='R'-'@') isrch->dir=1;
  else isrch->dir=0;
  if(qempty(IREC,link,&isrch->irecs))
   {
   if(lastpat && lastpat[0]) iappend(bw,isrch,sv(lastpat));
   }
  else
   {
   i=alirec();
   i->disp=i->start=bw->cursor->byte;
   i->what=0;
   if(dopfnext(bw,mksrch(vsncpy(NULL,0,isrch->pattern+isrch->ofst,sLen(isrch->pattern)-isrch->ofst),NULL,0,isrch->dir,-1,0,0),NULL))
    ttputc(7), frirec(i);
   else
    enqueb(IREC,link,&isrch->irecs,i);
   }
  }
 else if((c<32 || c>=256) && c!=MAXINT)
  { /* Done */
  if(c!='C'-'@') nungetc(c);
  if(notify) *notify=1;
  lastpat=vstrunc(lastpat,0);
  lastpat=vsncpy(lastpat,0,isrch->pattern+isrch->ofst,sLen(isrch->pattern)-isrch->ofst);
  isrch->pattern=0;
  rmisrch(isrch);
  return 0;
  }
 else if(c!=MAXINT)
  { /* Search */
  unsigned char k;
  in: k=c;
  isrch->quote=0;
  iappend(bw,isrch,&k,1);
  }
 omid=mid;
 mid=1;
 bw->cursor->xcol=piscol(bw->cursor);
 dofollows();
 mid=omid;
 if(mkqwnsr(bw,sv(isrch->pattern),itype,iabrt,isrch,notify)) return 0;
 else
  {
  rmisrch(isrch);
  return -1;
  }
 }

int doisrch(bw,dir)			/* Create a struct isrch */
BW *bw;
 {
 struct isrch *isrch=(struct isrch *)malloc(sizeof(struct isrch));
 izque(IREC,link,&isrch->irecs);
 isrch->pattern=vsncpy(NULL,0,sc("I-find: "));
 isrch->ofst=sLen(isrch->pattern);
 isrch->dir=dir;
 isrch->quote=0;
 return itype(bw,MAXINT,isrch,NULL);
 }

int uisrch(bw)
BW *bw;
 {
 return doisrch(bw,0);
 }

int ursrch(bw)
BW *bw;
 {
 return doisrch(bw,1);
 }
