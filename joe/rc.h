/*
 *	*rc file parser
 *	Copyright
 *		(C) 1992 Joseph H. Allen;
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

/* int procrc(CAP *cap, char *name);  Process an rc file
   fd is file descriptor of open file
   name is path to file to use for error messages
   Returns 0 for success
          -1 for file not found
           1 for syntax error (errors written to stderr)
*/
int procrc(CAP *cap, JFILE *f, char *name);

/* Validate rc file: return -1 if it's bad (call this after rc file has been loaded) */
int validate_rc(void);
