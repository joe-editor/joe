#ifndef _Irc
#define _Irc 1

#include "kbd.h"
#include "macro.h"

typedef struct options OPTIONS;
struct options {
	OPTIONS *next;
	char *name;
	int overtype;
	int lmargin;
	int rmargin;
	int autoindent;
	int wordwrap;
	int tab;
	int indentc;
	int istep;
	char *context;
	char *lmsg;
	char *rmsg;
	int linums;
	int readonly;
	int french;
	int spaces;
	int crlf;
	MACRO *mnew;		/* Macro to execute for new files */
	MACRO *mold;		/* Macro to execute for existing files */
	MACRO *msnew;		/* Macro to execute before saving new files */
	MACRO *msold;		/* Macro to execute before saving existing files */
};

extern OPTIONS pdefault;
void setopt PARAMS((OPTIONS *n, char *name));

/* KMAP *getcontext(char *name);
 * Find and return the KMAP for a given context name.  If none is found, an
 * empty kmap is created, bound to the context name, and returned.
 */
KMAP *getcontext PARAMS((char *name));

/* int procrc(CAP *cap, char *name);  Process an rc file
   Returns 0 for success
          -1 for file not found
           1 for syntax error (errors written to stderr)
*/
int procrc PARAMS((CAP *cap, char *name));

int glopt PARAMS((char *s, char *arg, OPTIONS *options, int set));

/* FIXME: cyclic dependency of header files (here on BW struct)
int umode PARAMS((BW *bw));
*/
int umode();

#endif
