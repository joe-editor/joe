/*
 *	Character classes and character maps
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
ptrdiff_t interval_test(struct interval *array, ptrdiff_t size, int ch);

/* An interval list item (for lists of interval to bind mappings) */

struct interval_list {
	struct interval_list *next; /* Next item in list */
	struct interval interval; /* Range of characters */
	void *map;
};

struct interval_list *mkinterval(struct interval_list *next, int first, int last, void *map);

void rminterval(struct interval_list *item);

/* Add a single interval to an interval list */
struct interval_list *interval_add(struct interval_list *interval_list, int first, int last, void *map);

/* Add set of intervals (a character class) to an interval list */
struct interval_list *interval_set(struct interval_list *list, struct interval *array, int size, void *map);

/* Look up single character in an interval list, return what it's mapped to */
void *interval_lookup(struct interval_list *list, void *dflt, int ch);

/* A character classes and maps implemented as a radix trees */

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
	int entry[32];
};

struct Leaf {
	void *entry[16];
};

struct Ileaf {
	int entry[16];
};

struct Level {
	int alloc;
	int size;
	union {
		struct Mid *b;
		struct Mid *c;
		struct Leaf *d;
		struct Ileaf *e;
	} table;
};

/* A radix tree bit-map for character classes */

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
void rset_add(struct Rset *r, int ch, int che);
void rset_opt(struct Rset *r);
void rset_set(struct Rset *r, struct interval *array, ptrdiff_t size);
void rset_show(struct Rset *r);

/* A radix tree map: (unicode character) -> (void *) or (unicode character) -> (int) */

struct Rtree {
	struct First top;
	struct Level second;
	struct Mid mid;
	struct Level third;
	struct Level leaf;
};

/* Functions for character -> pointer maps */

void rtree_init(struct Rtree *r);
void rtree_clr(struct Rtree *r);
void *rtree_lookup(struct Rtree *r, int ch);
void *rtree_lookup_unopt(struct Rtree *r, int ch);
void rtree_add(struct Rtree *r, int ch, int che, void *map);
void rtree_opt(struct Rtree *r);
void rtree_set(struct Rtree *r, struct interval *array, ptrdiff_t len, void *map);
void rtree_build(struct Rtree *r, struct interval_list *l);
void rtree_show(struct Rtree *r);

/* Functions for character -> int maps */

void rmap_init(struct Rtree *r);
void rmap_clr(struct Rtree *r);
int rmap_lookup(struct Rtree *r, int ch, int dflt);
int rmap_lookup_unopt(struct Rtree *r, int ch, int dflt);
void rmap_add(struct Rtree *r, int ch, int che, int map, int dflt);
void rmap_opt(struct Rtree *r);
void rmap_set(struct Rtree *r, struct interval *array, ptrdiff_t len, int map, int dflt);
void rmap_show(struct Rtree *r);

/* A character class */

struct Cclass {
	ptrdiff_t size; /* Malloc size of intervals */
	ptrdiff_t len; /* Number of entries in intervals */
	struct interval *intervals; /* sorted, non-overlapping, non-adjacent */
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
void cclass_union(struct Cclass *cclass, struct Cclass *n);
void cclass_merge(struct Cclass *cclass, struct interval *intervals, int len);

/* Subtract one character class from another */
void cclass_diff(struct Cclass *cclass, struct Cclass *n);

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

void cclass_show(struct Cclass *cclass);
