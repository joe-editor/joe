/*
 *	Character sets
 *	Copyright
 *		(C) 2004 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

/* For sorted from_map entries */

struct pair {
	int first;			/* Unicode */
	int last;			/* Byte */
};

/* A character set */

struct charmap {
	struct charmap *next;		/* Linked list of loaded character maps */
	const char *name;		/* Name of this one */

	int type;			/* 0=byte, 1=UTF-8 */

	/* Character predicate functions */

	int (*is_punct)(struct charmap *map,int c);
	int (*is_print)(struct charmap *map,int c);
	int (*is_space)(struct charmap *map,int c);
	int (*is_alpha_)(struct charmap *map,int c);
	int (*is_alnum_)(struct charmap *map,int c);

	/* Character conversion functions */

	int (*to_lower)(struct charmap *map,int c);
	int (*to_upper)(struct charmap *map,int c);

	/* Information for byte-oriented character sets */

	int *to_map;			/* Convert byte to unicode */

	unsigned char lower_map[256];	/* Convert to lower case */
	unsigned char upper_map[256];

	struct pair from_map[256];	/* Convert from unicode to byte */

	ptrdiff_t from_size;			/* No. paris in from_map */

	char print_map[32];	/* Bit map of printable characters */
	char alpha__map[32];	/* Bit map of alphabetic characters and _ */
	char alnum__map[32];	/* Bit map of alphanumeric characters and _ */
};

/* Predicates */

#define joe_ispunct(map,c) ((map)->is_punct((map),(c)))
#define joe_isprint(map,c) ((map)->is_print((map),(c)))
#define joe_isspace(map,c) ((map)->is_space((map),(c)))
#define joe_isalpha_(map,c) ((map)->is_alpha_((map),(c)))
#define joe_isalnum_(map,c) ((map)->is_alnum_((map),(c)))

int joe_isblank(struct charmap *map,int c); /* Space or Tab only */
int joe_isspace_eos(struct charmap *map,int c); /* Space, Tab, CR, LF, FF or NUL */

/* Conversion functions */

#define joe_tolower(map,c) ((map)->to_lower((map),(c)))
#define joe_toupper(map,c) ((map)->to_upper((map),(c)))

/* Find (load if necessary) a character set */
struct charmap *find_charmap(const char *name);

/* Get available encodings */
char **get_encodings(void);

/* Convert from unicode to byte */
int from_uni(struct charmap *cset, int c);
int from_utf8(struct charmap *map,const char *s);

/* Convert from byte to unicode */
int to_uni(struct charmap *cset, int c);
void to_utf8(struct charmap *map,char *s,int c);

void joe_locale();
extern struct charmap *locale_map;	/* Character map of terminal */
extern struct charmap *utf8_map;	/* UTF-8 character map */
extern struct charmap *utf16_map;	/* UTF-16 character map */
extern struct charmap *utf16r_map;	/* UTF-16 reversed  character map */
extern struct charmap *ascii_map;	/* Plain ASCII map */
extern const char *locale_lang;	/* Locale language (like de_DE) */
extern const char *locale_msgs;	/* Locale language for editor messages (like de_DE) */

/* Guess map */
struct charmap *guess_map(const char *buf, ptrdiff_t len);

extern int guess_non_utf8;
extern int guess_utf8;

void my_iconv(char *dest, ptrdiff_t destsiz, struct charmap *dest_map,
              const char *src,struct charmap *src_map);

void my_iconv1(char *dest, ptrdiff_t destsiz, struct charmap *dest_map,
              const int *src);
