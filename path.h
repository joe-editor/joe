/*
	Directory and path functions
	Copyright (C) 1992 Joseph H. Allen

	This file is part of JOE (Joe's Own Editor)
*/

#ifndef _joePATH
#define _joePATH 1

#include "config.h"

char *joesep PARAMS((char *path));

/* char *namprt(char *path);
 * Return name part of a path.  There is no name if the last character
 * in the path is '/'.
 *
 * The name part of "/hello/there" is "there"
 * The name part of "/hello/" is ""
 * The name part if "/" is ""
 */
char *namprt PARAMS((char *path));
char *namepart PARAMS((char *tmp, char *path));

/* char *dirprt(char *path);
 * Return directory and drive part of a path.  I.E., everything to the
 * left of the name part.
 *
 * The directory part of "/hello/there" is "/hello/"
 * The directory part of "/hello/" is "/hello/"
 * The directory part of "/" is "/"
 */
char *dirprt PARAMS((char *path));

/* char *begprt(char *path);
 * Return the beginning part of a path.
 *
 * The beginning part of "/hello/there" is "/hello/"
 * The beginning part of "/hello/" is "/"
 * The beginning part of "/" is "/"
 */
char *begprt PARAMS((char *path));

/* char *endprt(char *path);
 * Return the ending part of a path.
 *
 * The ending part of "/hello/there" is "there"
 * The ending part of "/hello/" is "hello/"
 * The ending part of "/" is ""
 */
char *endprt PARAMS((char *path));

/* int mkpath(char *path);
 * Make sure path exists.  If it doesn't, try to create it
 *
 * Returns 1 for error or 0 for success.  The current directory
 * and drive will be at the given path if successful, otherwise
 * the drive and path will be elsewhere (not necessarily where they
 * were before mkpath was called).
 */
int mkpath PARAMS((char *path));

/* char *mktmp(char *);
 * Create an empty temporary file.  The file name created is the string passed
 * to this function postfixed with /joe.tmp.XXXXXX, where XXXXXX is some
 * string six chars long which makes this file unique.
*/
char *mktmp PARAMS((char *where));

/* Change drive and directory */
#define chddir chdir

/* int rmatch(char *pattern,char *string);
 * Return true if string matches pattern
 *
 * Pattern is:
 *     *                 matches 0 or more charcters
 *     ?                 matches any 1 character
 *     [...]             matches 1 character in set
 *     [^...]            matches 1 character not in set
 *     [[]               matches [
 *     [*]               matches *
 *     [?]               matches ?
 *     any other         matches self
 *
 *  Ranges of characters may be specified in sets with 'A-B'
 *  '-' may be specified in sets by placing it at the ends
 *  '[' may be specified in sets by placing it first
 */
int rmatch PARAMS((char *a, char *b));
int isreg PARAMS((char *s));

/* char **rexpnd(char *path,char *pattern);
 * Generate array (see va.h) of file names from directory in 'path'
 * which match the pattern 'pattern'
 */
char **rexpnd PARAMS((char *word));

int chpwd PARAMS((char *path));

#endif
