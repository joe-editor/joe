/* rc file parser */

#include <stdio.h>
#include "zstr.h"
#include "macro.h"
#include "cmd.h"
#include "bw.h"
#include "help.h"
#include "vs.h"
#include "va.h"
#include "menu.h"
#include "umath.h"
#include "uedit.h"
#include "pw.h"
#include "path.h"
#include "w.h"
#include "tw.h"
#include "termcap.h"
#include "rc.h"

static struct context
 {
 struct context *next;
 char *name;
 KMAP *kmap;
 } *contexts=0;		/* List of named contexts */

/* Find a context of a given name- if not found, one with an empty kmap
 * is created.
 */

KMAP *getcontext(name)
char *name;
 {
 struct context *c;
 for(c=contexts;c;c=c->next) if(!zcmp(c->name,name)) return c->kmap;
 c=(struct context *)malloc(sizeof(struct context));
 c->next=contexts;
 c->name=zdup(name);
 contexts=c;
 return c->kmap=mkkmap();
 }

OPTIONS *options=0;
extern int mid, dspasis, dspctrl, force, help, pgamnt, square, csmode,
           nobackups, lightoff, exask, skiptop, noxon, lines, staen,
           columns, Baud, dopadding, orphan, marking, beep, keepup,
           nonotice;
extern char *backpath;

OPTIONS pdefault=
 { 0, 0, 0, 0, 76, 0, 0, 8, ' ', 1, 0, 0, 0, 0, 0 };

OPTIONS fdefault=
 { 0, 0, 0, 0, 76, 0, 0, 8, ' ', 1, "main", "\\i%n %m %M", " %S Ctrl-K H for help", 0, 0 };

void setopt(n,name)
OPTIONS *n;
char *name;
 {
 OPTIONS *o;
 for(o=options;o;o=o->next)
  if(rmatch(o->name,name))
   {
   *n= *o;
   return;
   }
 *n=fdefault;
 }

/* Set a global or local option 
 * returns 0 for no such option,
 *         1 for option accepted
 *         2 for option + argument accepted
 */

struct glopts
 {
 char *name;	/* Option name */
 int type;	/* 0 for global option flag
                   1 for global option numeric
                   2 for global option string
                   4 for local option flag
                   5 for local option numeric
                   6 for local option string
                   7 for local option numeric+1
                */
 int *set;	/* Address of global option */
 char *addr;	/* Local options structure member address */
 char *yes;	/* Message if option was turned on, or prompt string */
 char *no;	/* Message if option was turned off */
 char *menu;	/* Menu string */
 int ofst;	/* Local options structure member offset */
 } glopts[]=
 {
  {"overwrite", 4, 0, (char *)&fdefault.overtype,
   "Overtype mode",
   "Insert mode",
   "T Overtype "},

  {"autoindent", 4, 0, (char *)&fdefault.autoindent,
   "Autoindent enabled",
   "Autoindent disabled",
   "I Autoindent "},

  {"wordwrap", 4, 0, (char *)&fdefault.wordwrap,
   "Wordwrap enabled",
   "Wordwrap disabled",
   "Word wrap "},

  {"tab", 5, 0, (char *)&fdefault.tab,
   "Tab width (%d): ",
   0,
   "D Tab width "},

  {"lmargin", 7, 0, (char *)&fdefault.lmargin,
   "Left margin (%d): ",
   0,
   "Left margin "},

  {"rmargin", 7, 0, (char *)&fdefault.rmargin,
   "Right margin (%d): ",
   0,
   "Right margin "},
  
  {"square", 0, &square, 0,
   "Rectangle mode",
   "Text-stream mode",
   "X Rectangle mode " },

  {"indentc", 5, 0, (char *)&fdefault.indentc,
   "Indent char %d (SPACE=32, TAB=9, ^C to abort): ",
   0,
   " Indent char "},

  {"istep", 5, 0, (char *)&fdefault.istep,
   "Indent step %d (^C to abort): ",
   0,
   " Indent step "},

  {"french", 4, 0, (char *)&fdefault.french,
   "One space after periods for paragraph reformat",
   "Two spaces after periods for paragraph reformat",
   " french spacing "},

  {"spaces", 4, 0, (char *)&fdefault.spaces,
   "Inserting spaces when tab key is hit",
   "Inserting tabs when tab key is hit",
   " no tabs "},

  {"mid", 0, &mid, 0,
   "Cursor will be recentered on scrolls",
   "Cursor will not be recentered on scroll",
   "Center on scroll " },

  {"linums", 4, 0, (char *)&fdefault.linums,
   "Line numbers enabled",
   "Line numbers disabled",
   "N Line numbers "},

  {"marking", 0, &marking, 0,
   "Anchored block marking on",
   "Anchored block marking off",
   "Marking " },

  {"asis", 0, &dspasis, 0,
   "Characters above 127 shown as-is",
   "Characters above 127 shown in inverse",
   "Meta chars as-is "
    },

  {"force", 0, &force, 0,
   "Last line forced to have NL when file saved",
   "Last line not forces to have NL",
   "Force last NL " },

  {"nobackups", 0, &nobackups, 0,
   "Backup files will not be made",
   "Backup files will be made",
   " Disable backups " },

  {"lightoff", 0, &lightoff, 0,
   "Highlighting turned off after block operations",
   "Highlighting not turned off after block operations",
   "Auto unmark " },

  {"exask", 0, &exask, 0,
   "Prompt for filename in save & exit command",
   "Don't prompt for filename in save & exit command",
   "Exit ask "},

  {"beep", 0, &beep, 0,
   "Warning bell enabled", 
   "Warning bell disabled",
   "Beeps " },

  {"nosta", 0, &staen, 0,
   "Top-most status line disabled",
   "Top-most status line enabled",
   " Disable status line " },

  {"keepup", 0, &keepup, 0,
   "Status line updated constantly",
   "Status line updated once/sec",
   " Fast status line " },

  {"pg", 1, &pgamnt, 0,
   "Lines to keep for PgUp/PgDn or -1 for 1/2 window (%d): ",
   0,
   " No. PgUp/PgDn lines " },

  {"csmode", 0, &csmode, 0,
   "Start search after a search repeats previous search",
   "Start search always starts a new search",
   "Continued search " },

  {"rdonly", 4, 0, (char *)&fdefault.readonly,
   "Read only",
   "Full editing",
   "O Read only "},

  {"backpath", 2, (int *)&backpath, 0,
   "Backup files stored in (%s): ",
   0,
   "Path to backup files " },

  {"nonotice", 0, &nonotice, 0,
   0, 0, 0 },

  {"noxon", 0, &noxon, 0,
   0, 0, 0 },

  {"orphan", 0, &orphan, 0,
   0, 0, 0 },

  {"help", 0, &help, 0,
   0, 0, 0 },

  {"dopadding", 0, &dopadding, 0,
   0, 0, 0 },

  {"lines", 1, &lines, 0,
   0, 0, 0 },

  {"baud", 1, &Baud, 0,
   0, 0, 0 }, 

  {"columns", 1, &columns, 0,
   0, 0, 0 },

  {"skiptop", 1, &skiptop, 0,
   0, 0, 0 }, 

  { 0, 0, 0 }
 };

int isiz=0;

void izopts()
 {
 int x;
 for(x=0;glopts[x].name;++x)
  switch(glopts[x].type)
   {
   case 4: case 5: case 6: case 7:
   glopts[x].ofst=glopts[x].addr-(char *)&fdefault;
   }
 isiz=1;
 }

int glopt(s,arg,options,set)
char *s, *arg;
OPTIONS *options;
 {
 int ret=0;
 int st=1;
 int x;
 if(!isiz) izopts();
 if(s[0]=='-') st=0, ++s;
 for(x=0;glopts[x].name;++x)
  if(!zcmp(glopts[x].name,s))
   {
   switch(glopts[x].type)
    {
    case 0: if(set) *glopts[x].set=st;
    break;
    
    case 1: if(set) if(arg) sscanf(arg,"%d",glopts[x].set);
    break;

    case 2: if(set)
             if(arg) *(char **)glopts[x].set=zdup(arg);
             else *(char **)glopts[x].set=0;
    break;

    case 4: if(options) *(int *)((char *)options+glopts[x].ofst)=st;
            else if(set==2) *(int *)((char *)&fdefault+glopts[x].ofst)=st;
    break;
    
    case 5: if(arg)
             if(options) sscanf(arg,"%d",(int *)((char *)options+glopts[x].ofst));
             else if(set==2) sscanf(arg,"%d",(int *)((char *)&fdefault+glopts[x].ofst));
    break;

    case 7: if(arg)
             {
             int zz=0;
             sscanf(arg,"%d",&zz);
             if(zz) --zz;
             if(options) *(int *)((char *)options+glopts[x].ofst)=zz;
             else if(set==2) *(int *)((char *)&fdefault+glopts[x].ofst)=zz;
             }
    break;
    }
   if((glopts[x].type&3)==0 || !arg) return 1;
   else return 2;
   }
 if(!zcmp(s,"lmsg"))
  {
  if(arg)
   {
   if(options) options->lmsg=zdup(arg);
   else if(set==2) fdefault.lmsg=zdup(arg);
   ret=2;
   }
  else ret=1;
  }
 else if(!zcmp(s,"rmsg"))
  {
  if(arg)
   {
   if(options) options->rmsg=zdup(arg);
   else if(set==2) fdefault.rmsg=zdup(arg);
   ret=2;
   }
  else ret=1;
  }
 else if(!zcmp(s,"keymap"))
  {
  if(arg)
   {
   int y;
   for(y=0;!cwhitel(arg[y]);++y);
   if(!arg[y]) arg[y]=0;
   if(options && y) options->context=zdup(arg);
   ret=2;
   }
  else ret=1;
  }
 done: return ret;
 }

static int optx=0;

int doabrt1(bw,xx)
BW *bw;
int *xx;
 {
 free(xx);
 return -1;
 }

int doopt1(bw,s,xx,notify)
BW *bw;
char *s;
int *xx;
int *notify;
 {
 int ret=0;
 int x= *xx;
 int v;
 free(xx);
 switch(glopts[x].type)
  {
  case 1:
   v=calc(bw,s);
   if(merr) msgnw(bw,merr), ret= -1;
   else *glopts[x].set=v;
   break;
  case 2: if(s[0]) *(char **)glopts[x].set=zdup(s); break;
  case 5:
   v=calc(bw,s);
   if(merr) msgnw(bw,merr), ret= -1;
   else *(int *)((char *)&bw->o+glopts[x].ofst)=v;
   break;
  case 7:
   v=calc(bw,s)-1.0;
   if(merr) msgnw(bw,merr), ret= -1;
   else *(int *)((char *)&bw->o+glopts[x].ofst)=v;
   break;
  }
 vsrm(s);
 bw->b->o=bw->o;
 wfit(bw->parent->t);
 updall();
 if(notify) *notify=1;
 return ret;
 }

int doopt(m,x,object,flg)
MENU *m;
void *object;
 {
 BW *bw=m->parent->win->object;
 int *xx;
 char buf[80];
 int *notify=m->parent->notify;
 switch(glopts[x].type)
  {
  case 0:
  if(!flg) *glopts[x].set= !*glopts[x].set;
  else if(flg==1) *glopts[x].set= 1;
  else *glopts[x].set=0;
  uabort(m,MAXINT);
  msgnw(bw,*glopts[x].set?glopts[x].yes:glopts[x].no);
  break;

  case 4:
  if(!flg) *(int *)((char *)&bw->o+glopts[x].ofst)= !*(int *)((char *)&bw->o+glopts[x].ofst);
  else if(flg==1) *(int *)((char *)&bw->o+glopts[x].ofst)=1;
  else *(int *)((char *)&bw->o+glopts[x].ofst)=0;
  uabort(m,MAXINT);
  msgnw(bw,*(int *)((char *)&bw->o+glopts[x].ofst)?glopts[x].yes:glopts[x].no);
  if(glopts[x].ofst==(char *)&fdefault.readonly-(char *)&fdefault)
   bw->b->rdonly=bw->o.readonly;
  break;

  case 1:
  sprintf(buf,glopts[x].yes,*glopts[x].set);
  xx=(int *)malloc(sizeof(int)); *xx=x;
  m->parent->notify=0;
  uabort(m,MAXINT);
  if(wmkpw(bw,buf,NULL,doopt1,NULL,doabrt1,utypebw,xx,notify)) return 0;
  else return -1;

  case 2:
  if(*(char **)glopts[x].set) sprintf(buf,glopts[x].yes,*(char **)glopts[x].set);
  else sprintf(buf,glopts[x].yes,"");
  xx=(int *)malloc(sizeof(int)); *xx=x;
  m->parent->notify=0;
  uabort(m,MAXINT);
  if(wmkpw(bw,buf,NULL,doopt1,NULL,doabrt1,utypebw,xx,notify)) return 0;
  else return -1;

  case 5:
  sprintf(buf,glopts[x].yes,*(int *)((char *)&bw->o+glopts[x].ofst));
  goto in;
  
  case 7:
  sprintf(buf,glopts[x].yes,*(int *)((char *)&bw->o+glopts[x].ofst)+1);
  in: xx=(int *)malloc(sizeof(int)); *xx=x;
  m->parent->notify=0;
  uabort(m,MAXINT);
  if(wmkpw(bw,buf,NULL,doopt1,NULL,doabrt1,utypebw,xx,notify)) return 0;
  else return -1;
  }
 if(notify) *notify=1;
 bw->b->o=bw->o;
 wfit(bw->parent->t);
 updall();
 return 0;
 }

int doabrt(m,x,s)
MENU *m;
char **s;
 {
 optx=x;
 for(x=0;s[x];++x) free(s[x]);
 free(s);
 return -1;
 }

int umode(bw)
BW *bw;
 {
 int size;
 char **s;
 int x;
 bw->b->o.readonly=bw->o.readonly=bw->b->rdonly;
 for(size=0;glopts[size].menu;++size);
 s=(char **)malloc(sizeof(char *)*(size+1));
 for(x=0;x!=size;++x)
  {
  s[x]=(char *)malloc(40);
  switch(glopts[x].type)
   {
   case 0: sprintf(s[x],"%s%s",glopts[x].menu,*glopts[x].set?"ON":"OFF");
   break;
   
   case 1: sprintf(s[x],"%s%d",glopts[x].menu,*glopts[x].set);
   break;

   case 2: zcpy(s[x],glopts[x].menu);
   break;

   case 4: sprintf(s[x],"%s%s",glopts[x].menu,*(int *)((char *)&bw->o+glopts[x].ofst)?"ON":"OFF");
   break;
   
   case 5: sprintf(s[x],"%s%d",glopts[x].menu,*(int *)((char *)&bw->o+glopts[x].ofst));
   break;

   case 7: sprintf(s[x],"%s%d",glopts[x].menu,*(int *)((char *)&bw->o+glopts[x].ofst)+1);
   break;
   }
  }
 s[x]=0;
 if(mkmenu(bw,s,doopt,doabrt,NULL,optx,s,NULL)) return 0;
 else return -1;
 }


/* Process rc file
 * Returns 0 if the rc file was succefully processed
 *        -1 if the rc file couldn't be opened
 *         1 if there was a syntax error in the file
 */

int nhelp=0;			/* No. help screens so far */

int procrc(cap,name)
CAP *cap;
char *name;
 {
 OPTIONS *o=0;			/* Current options */
 KMAP *context=0;		/* Current context */
 unsigned char buf[1024];	/* Input buffer */
 FILE *fd;			/* rc file */
 int line=0;			/* Line number */
 int err=0;			/* Set to 1 if there was a syntax error */
 ossep(zcpy(buf,name));
#ifdef __MSDOS__
 fd=fopen(buf,"rt");
#else
 fd=fopen(buf,"r");
#endif
 
 if(!fd) return -1;		/* Return if we couldn't open the rc file */
 
 fprintf(stderr,"Processing '%s'...",name); fflush(stderr);
 
 while(++line, fgets(buf,1024,fd))
  switch(buf[0])
   {
   case ' ': case '\t': case '\n': case '\f': case 0:
   break;	/* Skip comment lines */

   case '*':	/* Select file types for file-type dependant options */
    {
    int x;
    o=(OPTIONS *)malloc(sizeof(OPTIONS));
    *o=fdefault;
    for(x=0;buf[x] && buf[x]!='\n' && buf[x]!=' ' && buf[x]!='\t';++x);
    buf[x]=0;
    o->next=options;
    options=o;
    o->name=zdup(buf);
    }
   break;
   
   case '-':	/* Set an option */
    {
    unsigned char *opt=buf+1;
    int x;
    unsigned char *arg=0;
    for(x=0;buf[x] && buf[x]!='\n' && buf[x]!=' ' && buf[x]!='\t';++x);
    if(buf[x] && buf[x]!='\n')
     {
     buf[x]=0;
     for(arg=buf+ ++x;buf[x] && buf[x]!='\n';++x);
     }
    buf[x]=0;
    if(!glopt(opt,arg,o,2))
     {
     err=1;
     fprintf(stderr,"\n%s %d: Unknown option %s",name,line,opt);
     }
    }
   break;
   
   case '{':	/* Enter help text */
    {
    int bfl;
    struct help *tmp=(struct help *)malloc(sizeof(struct help));
    nhelp++;
    tmp->next=first_help;
    first_help=tmp;
    tmp->name=vsncpy(NULL,0,sz(buf+1)-1);
    help_names=vaadd(help_names,tmp->name);
    tmp->hlptxt=0;
    tmp->hlpsiz=0;
    tmp->hlpbsz=0;
    tmp->hlplns=0;
    up:
    if(++line, !fgets(buf,256,fd))
     {
     err=1;
     fprintf(stderr,"\n%s %d: End of joerc file occured before end of help text",name,line);
     break;
     }
    if(buf[0]=='}')
     {
     if(!hlptxt)
      hlptxt=tmp->hlptxt,
      hlpsiz=tmp->hlpsiz,
      hlpbsz=tmp->hlpbsz,
      hlplns=tmp->hlplns;
     continue;
     }
    bfl=zlen(buf);
    if(tmp->hlpsiz+bfl>tmp->hlpbsz)
     {
     if(tmp->hlptxt) tmp->hlptxt=(char *)realloc(tmp->hlptxt,tmp->hlpbsz+bfl+1024);
     else tmp->hlptxt=(char *)malloc(bfl+1024), tmp->hlptxt[0]=0;
     tmp->hlpbsz+=bfl+1024;
     }
    zcpy(tmp->hlptxt+tmp->hlpsiz,buf);
    tmp->hlpsiz+=bfl;
    ++tmp->hlplns;
    goto up;
    }
   break;

   case ':':	/* Select context */
    {
    int x, c;
    for(x=1;!cwhitef(buf[x]);++x);
    c=buf[x]; buf[x]=0;
    if(x!=1)
     if(!zcmp(buf+1,"inherit"))
      if(context)
       {
       for(buf[x]=c;cwhite(buf[x]);++x);
       for(c=x;!cwhitef(buf[c]);++c);
       buf[c]=0;
       if(c!=x) kcpy(context,getcontext(buf+x));
       else
        {
        err=1;
        fprintf(stderr,"\n%s %d: context name missing from :inherit",name,line);
        }
       }
      else
       {
       err=1;
       fprintf(stderr,"\n%s %d: No context selected for :inherit",name,line);
       }
     else if(!zcmp(buf+1,"include"))
      {
      for(buf[x]=c;cwhite(buf[x]);++x);
      for(c=x;!cwhitef(buf[c]);++c);
      buf[c]=0;
      if(c!=x)
       {
       switch(procrc(buf+x))
        {
        case 1: err=1; break;
        case -1: fprintf(stderr,"\n%s %d: Couldn't open %s",name,line,buf+x);
                 err=1; break;
        }
       context=0;
       o=0;
       }
      else
       {
       err=1;
       fprintf(stderr,"\n%s %d: :include missing file name",name,line);
       }
      }
     else if(!zcmp(buf+1,"delete"))
      if(context)
       {
       int y;
       for(buf[x]=c;cwhite(buf[x]);++x);
       for(y=x;buf[y]!=0 && buf[y]!='\t' && buf[y]!='\n' &&
               (buf[y]!=' ' || buf[y+1]!=' ');++y);
       buf[y]=0;
       kdel(context,buf+x);
       }
      else
       {
       err=1;
       fprintf(stderr,"\n%s %d: No context selected for :delete",name,line);
       }
     else context=getcontext(buf+1);
    else
     {
     err=1;
     fprintf(stderr,"\n%s %d: Invalid context name",name,line);
     }
    }
   break;

   default:	/* Get key-sequence to macro binding */
    {
    int x, y, c;
    MACRO *m;
    if(!context)
     {
     err=1;
     fprintf(stderr,"\n%s %d: No context selected for macro to key-sequence binding",name,line);
     break;
     }

    /* Process macro */
    m=0; x=0;
    macroloop:
    if(buf[x]=='\"')
     {
     ++x;
     while(buf[x] && buf[x]!='\"')
      {
      if(buf[x]=='\\' && buf[x+1])
       {
       ++x;
       switch(buf[x])
        {
       case 'n': buf[x]=10; break;
       case 'r': buf[x]=13; break;
       case 'b': buf[x]=8; break;
       case 'f': buf[x]=12; break;
       case 'a': buf[x]=7; break;
       case 't': buf[x]=9; break;
       case 'x':
        c=0;
        if(buf[x+1]>='0' && buf[x+1]<='9') c=c*16+buf[++x]-'0';
        else if(buf[x+1]>='a' && buf[x+1]<='f' ||
                buf[x+1]>='A' && buf[x+1]<='F') c=c*16+(buf[++x]&0xF)+9;
        if(buf[x+1]>='0' && buf[x+1]<='9') c=c*16+buf[++x]-'0';
        else if(buf[x+1]>='a' && buf[x+1]<='f' ||
                buf[x+1]>='A' && buf[x+1]<='F') c=c*16+(buf[++x]&0xF)+9;
        buf[x]=c;
        break;
       case '0': case '1': case '2': case '3':
       case '4': case '5': case '6': case '7':
       case '8': case '9':
        c=buf[x]-'0';
        if(buf[x+1]>='0' && buf[x+1]<='7') c=c*8+buf[++x]-'0';
        if(buf[x+1]>='0' && buf[x+1]<='7') c=c*8+buf[++x]-'0';
        buf[x]=c;
        break;
        }
       }
      if(m)
       {
       if(!m->steps)
        {
        MACRO *macro=m;
        m=mkmacro(MAXINT,1,0);
        addmacro(m,macro);
        }
       }
      else m=mkmacro(MAXINT,1,0);
      addmacro(m,mkmacro(buf[x],1,findcmd("type")));
      ++x;
      }
     if(buf[x]=='\"') ++x;
     }
    else
     {
     for(y=x;
         buf[y] && buf[y]!=',' && buf[y]!=' ' && buf[y]!='\t' && buf[y]!='\n';
         ++y);
     if(y!=x)
      {
      int n;
      c=buf[y]; buf[y]=0;
      n=findcmd(buf+x);
      if(n== -1)
       {
       fprintf(stderr,"\n%s %d: Key function '%s' not found",name,line,buf);
       err=1;
       break;
       }
      else if(m)
       {
       if(!m->steps)
        {
        MACRO *macro=m;
        m=mkmacro(MAXINT,1,0);
        addmacro(m,macro);
        }
       addmacro(m,mkmacro(MAXINT,1,n));
       }
      else m=mkmacro(MAXINT,1,n);
      buf[x=y]=c;
      }
     }
    if(buf[x]==',')
     {
     ++x;
     if(!buf[x] || buf[x]==' ' || buf[x]=='\t' || buf[x]=='\n')
      {
      buf[x=0]=0;
      fgets(buf,1024,fd);
      }
     goto macroloop;
     }
    if(!m) break;

    /* Skip to start of key sequence */
    while(cwhite(buf[x])) ++x;

    /* Skip to end of key sequence */
    for(y=x;buf[y]!=0 && buf[y]!='\t' && buf[y]!='\n' && 
            (buf[y]!=' ' || buf[y+1]!=' ');++y);
    buf[y]=0;

    /* Add binding to context */
    if(kadd(cap,context,buf+x,m)== -1)
     {
     fprintf(stderr,"\n%s %d: Bad key sequence '%s'",name,line,buf+x);
     err=1;
     }
    }
   break;
   }
 fclose(fd);			/* Close rc file */

 /* Print proper ending string */
 if(err) fprintf(stderr,"\ndone\n");
 else fprintf(stderr,"done\n");

 return err;			/* 0 for success, 1 for syntax error */
 }

void izhelp()
 {
 struct help *tmp;
 /* Convert list of help screens into an array */
 if(nhelp)
  {
  help_structs=(struct help **) malloc(sizeof(struct help *)*(nhelp+1));
  help_structs[nhelp]=0;
  tmp=first_help;
  while(nhelp--)
   {
   help_structs[nhelp]=tmp;
   tmp=tmp->next;
   }
  }
 }
