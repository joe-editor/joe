/* Math */

#include <signal.h>
#include "w.h"
#include "bw.h"
#include "pw.h"
#include "tw.h"
#include "vs.h"
#include "zstr.h"
#include "umath.h"

char *merr;

void fperr()
 {
 if(!merr) merr="Float point exception";
 signal(SIGFPE,fperr);
 }

struct var
 {
 char *name;
 int set;
 double val;
 struct var *next;
 } *vars=0;

struct var *get(str)
char *str;
 {
 struct var *v;
 for(v=vars;v;v=v->next) if(!zcmp(v->name,str)) return v;
 v=(struct var *)malloc(sizeof(struct var));
 v->set=0;
 v->next=vars;
 vars=v;
 v->name=zdup(str);
 return v;
 }

char *ptr;
struct var *dumb;

double expr(prec,rtv)
struct var **rtv;
 {
 double x=0.0;
 struct var *v=0;
 while(*ptr==' ' || *ptr=='\t') ++ptr;
 if(*ptr>='a' && *ptr<='z' || *ptr>='A' && *ptr<='Z' || *ptr=='_')
  {
  char *s=ptr, c;
  while(*ptr>='a' && *ptr<='z' || *ptr>='A' && *ptr<='Z' || *ptr=='_' ||
        *ptr>='0' && *ptr<='9') ++ptr;
  c= *ptr; *ptr=0;
  v=get(s);
  x=v->val;
  *ptr=c;
  }
 else if(*ptr>='0' && *ptr<='9' || *ptr=='.')
  {
  sscanf(ptr,"%lf",&x);
  while(*ptr>='0' && *ptr<='9' || *ptr=='.' || *ptr=='e' || *ptr=='E') ++ptr;
  }
 else if(*ptr=='(')
  {
  ++ptr;
  x=expr(0,&v);
  if(*ptr==')') ++ptr;
  else { if(!merr) merr="Missing )"; }
  }
 else if(*ptr=='-')
  {
  ++ptr;
  x= -expr(10,&dumb);
  }
 loop:
 while(*ptr==' ' || *ptr=='\t') ++ptr;
 if(*ptr=='*' && 5>prec)
  {
  ++ptr;
  x*=expr(5,&dumb);
  goto loop;
  }
 else if(*ptr=='/' && 5>prec)
  {
  ++ptr;
  x/=expr(5,&dumb);
  goto loop;
  }
 else if(*ptr=='+' && 4>prec)
  {
  ++ptr;
  x+=expr(4,&dumb);
  goto loop;
  }
 else if(*ptr=='-' && 4>prec)
  {
  ++ptr;
  x-=expr(4,&dumb);
  goto loop;
  }
 else if(*ptr=='=' && 2>=prec)
  {
  ++ptr;
  x=expr(2,&dumb);
  if(v) v->val=x, v->set=1;
  else { if(!merr) merr="Left side of = is not an l-value"; }
  goto loop;
  }
 *rtv=v;
 return x;
 }

double calc(bw,s)
BW *bw;
char *s;
 {
 double result;
 struct var *v;
 BW *tbw=bw->parent->main->object;
 v=get("top"); v->val=tbw->top->line+1; v->set=1;
 v=get("lines"); v->val=tbw->b->eof->line+1; v->set=1;
 v=get("line"); v->val=tbw->cursor->line+1; v->set=1;
 v=get("col"); v->val=tbw->cursor->col+1; v->set=1;
 v=get("byte"); v->val=tbw->cursor->byte+1; v->set=1;
 v=get("height"); v->val=tbw->h; v->set=1;
 v=get("width"); v->val=tbw->w; v->set=1;
 ptr=s;
 merr=0;
 up: result=expr(0,&dumb);
 if(!merr)
  {
  while(*ptr==' ' || *ptr=='\t') ++ptr;
  if(*ptr==';')
   {
   ++ptr;
   while(*ptr==' ' || *ptr=='\t') ++ptr;
   if(*ptr) goto up;
   }
  else if(*ptr) merr="Extra junk after end of expr";
  }
 return result;
 }

int domath(bw,s,object,notify)		/* Main user interface */
BW *bw;
char *s;
void *object;
int *notify;
 {
 double result=calc(bw,s);
 if(notify) *notify=1;
 if(merr) { msgnw(bw,merr); return -1; }
 vsrm(s);
 sprintf(msgbuf,"%lg",result);
 if(bw->parent->watom->what!=TYPETW)
  {
  binsm(bw->cursor,sz(msgbuf));
  pfwrd(bw->cursor,zlen(msgbuf));
  bw->cursor->xcol=piscol(bw->cursor);
  }
 else msgnw(bw,msgbuf);
 return 0;
 }

B *mathhist=0;

int umath(bw)
BW *bw;
 {
 signal(SIGFPE,fperr);
 if(wmkpw(bw,"=",&mathhist,domath,"math",NULL,NULL,NULL,NULL)) return 0;
 else return -1;
 }
