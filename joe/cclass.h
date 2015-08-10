/*
 *	Character classes
 *	Copyright
 *		(C) 2015 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

/* An interval.  A character matches if it's in the range.
   An array of these can be used to define a character class. */

struct interval {
	int first;
	int last;
};

/* Sort an struct interval array */
void interval_sort(struct interval *array, ptrdiff_t size);

/* Test if character is in a sorted interval array using binary search.
   Returns -1 if not found, or index to matching struct interval */
int interval_test(struct interval *array, ptrdiff_t size, int ch);

/* A character class implemented as a radix tree
   These structures are also used for character maps based
   on radix trees: see cmap.h */

#define LEAFSIZE 16
#define LEAFMASK 0xF
#define LEAFSHIFT 0

#define THIRDSIZE 32
#define THIRDMASK 0x1f
#define THIRDSHIFT 4

#define SECONDSIZE 32
#define SECONDMASK 0x1f
#define SECONDSHIFT 9

#define TOPSIZE 68
#define TOPMASK 0x7f
#define TOPSHIFT 14

struct First {
	short entry[68];
};

struct Mid {
	short entry[32];
};

struct Leaf {
	void *entry[16];
};

struct Level {
	int alloc;
	int size;
	union {
		struct Mid *b;
		struct Mid *c;
		struct Leaf *d;
	} table;
};

struct Rset {
	int flag;
	struct First top;
	struct Level second;
	struct Mid mid;
	struct Level third;
};

void rset_init(struct Rset *r);
void rset_clr(struct Rset *r);
int rset_lookup(struct Rset *r, int ch);
int rset_lookup_unopt(struct Rset *r, int ch);
void rset_add(struct Rset *r, int ch);
void rset_opt(struct Rset *r);
void rset_set(struct Rset *r, struct interval *array, int size);
void rset_show(struct Rset *r);

/* A character class */

struct Cclass {
	ptrdiff_t size; /* Malloc size of array */
	ptrdiff_t len; /* Number of entries in array */
	struct interval *array; /* sorted, non-overlapping, non-adjacent */
	struct Rset rset[1]; /* Radix tree version of character class */
};

/* Initialize a character class */
void cclass_init(struct Cclass *cclass);

/* Free memory used by a Cclass (does not free cclass itself) */
void cclass_clr(struct Cclass *cclass);

/* Add a range to a character class */
void cclass_add(struct Cclass *cclass, int first, int last);

/* Remove a range to a character class */
void cclass_sub(struct Cclass *cclass, int first, int last);

/* Merge one character class into another */
void cclass_union(struct Cclass *cclass, struct interval *array, int len);

/* Subtract one character class from another */
void cclass_diff(struct Cclass *cclass, struct interval *array, int len);

/* Compute inverse of a character class */
void cclass_inv(struct Cclass *cclass);

/* Lookup a character in a character class using binary search.
   Return true if character is in the class. */
int cclass_lookup_unopt(struct Cclass *m, int ch);

/* Optimize a character class for fast lookup with cclass_lookup.  Generates a radix tree
   version of the character class. */
void cclass_opt(struct Cclass *cclass);

/* Return true if character is in the class */
int cclass_lookup(struct Cclass *cclass, int ch);
