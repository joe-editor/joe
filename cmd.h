/*
 *	Command execution
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_CMD_H
#define _JOE_CMD_H 1

#include "config.h"
#include "types.h"

extern CMD cmds[];		/* Built-in commands */
extern int beep;

/* Command execution flags */

#define EMID		  1	/* Recenter screen */
#define ECHKXCOL	  2	/* Don't execute command if cursor column is wrong */
#define EFIXXCOL	  4	/* Fix column position after command has executed */
#define EMINOR		  8	/* Full screen update not needed */
#define EPOS		 16	/* A position history command */
#define EMOVE		 32	/* A movement for position history purposes */
#define EKILL		 64	/* Function is a kill */
#define EMOD		128	/* Not allowed on readonly files */
/* These use same bits as TYPE* in types.h */
#define EBLOCK		0x4000	/* Finish block selection (call udropon) */

/* CMD *findcmd(char *s);
 * Return command address for given name
 */
CMD *findcmd PARAMS((unsigned char *s));
void addcmd PARAMS((unsigned char *s, MACRO *m));

/* Execute a command.  Returns return value of command */
int execmd PARAMS((CMD *cmd, int k));

extern B *cmdhist;

#endif
