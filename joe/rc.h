/*
 *	*rc file parser
 *	Copyright
 *		(C) 1992 Joseph H. Allen; 
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

/* int procrc(CAP *cap, char *name);  Process an rc file
   Returns 0 for success
          -1 for file not found
           1 for syntax error (errors written to stderr)
*/
int procrc(CAP *cap, char *name);

/* Validate rc file: return -1 if it's bad (call this after rc file has been loaded) */
int validate_rc();

/* Get a file path in the JOERC directory. If file is NULL get the directory 
   itself. First checks $XDG_CONFIG_HOME/joe, then ~/.config/joe, and 
   finally ~/.joe. If no user JOERC dir exists, it check JOEDATA.
   Returns NULL on failure. 
   Returns a dynamically allocated string (path to the file) on success.
*/
char *get_joerc_file_path(const char *file);

/* Get joe's main resource file path. 
   Run is argv[0].
   Returns a dynmaically allocated path to the joerc.
   Returns NULL on error.
*/
char *get_joerc_path(const char *run);

/* Get cache file of name NAME. Starts by checking XDG_CACHE_HOME, if no 
   dedicated cache dir is found, it will place the file into the joerc
   directory. If there is no JOERC directory, it will place the file
   as a dotfile in the user's home. 
   Returns a valid FILE* on success, returns NULL on error.
*/
FILE *get_cache_file(const char *name, const char *mode);
