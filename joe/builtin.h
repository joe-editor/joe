/* Support for built-in config files */

#ifndef JOEWIN

struct jfile {
	FILE *f;	/* Regular file, or NULL for built-in */
	const char *p;	/* Built-in file pointer */
};

JFILE *jfopen(const char *name, const char *mode);
char *jfgets(char **buf,JFILE *f);
int jfclose(JFILE *f);
char **jgetbuiltins(const char *suffix);

extern const char *builtins[];

#else

#include "jwbuiltin.h"

JFILE *jfopen(char *name, const char *mode);
char *jfgets(char **buf, JFILE *f);
char **jgetbuiltins(char *suffix);

#define jfclose jwfclose

#endif
