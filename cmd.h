#ifndef _Icmd
#define _Icmd 1

typedef struct cmd CMD;
typedef struct macro MACRO;

/* Command entry */

struct cmd
 {
 char *name;		/* Command name */
 int flag;		/* Execution flags */
 int (*func)();		/* Function bound to name */
 MACRO *m;		/* Macro bound to name */
 int arg;		/* 0= arg is meaningless, 1= ok */
 char *negarg;		/* Command to use if arg was negative */
 };

extern CMD cmds[];	/* Built-in commands */

/* Command execution flags */

#define EMID 1		/* Recenter screen */
#define ECHKXCOL 2	/* Don't execute command if cursor column is wrong */
#define EFIXXCOL 4	/* Fix column position after command has executed */
#define EMINOR 8	/* Full screen update not needed */
#define EPOS 16		/* A position history command */
#define EMOVE 32	/* A movement for position history purposes */
#define EKILL 64	/* Function is a kill */
#define EMOD 128	/* Not allowed on readonly files */

/* CMD *findcmd(char *s);
 * Return command address for given name
 */
CMD *findcmd();
void addcmd();

/* Execute a command.  Returns return value of command */
int execmd();

extern int beep;

#endif
