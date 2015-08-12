/*
 *	Character maps
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

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


/* An interval map item */

struct interval_map {
	struct interval interval;
	void *map;
};

/* A character map */

struct cmap {
	void *direct_map[128];		/* Direct mapping for ASCII range */
	ptrdiff_t size;			/* No. items in range_map */
	struct interval_map *range_map;	/* Sorted range map */
	void *dflt_map;
};

/* Build character map from interval list */
void cmap_build(struct cmap *cmap, struct interval_list *list, void *dflt_map);

/* Clear a cmap */
void clr_cmap(struct cmap *cmap);

/* Look up single character in a character map, return what it's mapped to */
void *cmap_lookup(struct cmap *cmap, int ch);

/* A radix tree map: (unicode character) -> (void *) */

struct Rtree {
	struct First top;
	struct Level second;
	struct Mid mid;
	struct Level third;
	struct Level leaf;
};

void rtree_init(struct Rtree *r);
void rtree_clr(struct Rtree *r);
void *rtree_lookup(struct Rtree *r, int ch);
void *rtree_lookup_unopt(struct Rtree *r, int ch);
void rtree_add(struct Rtree *r, int ch, void *map);
void rtree_opt(struct Rtree *r);

void rtree_set(struct Rtree *r, struct interval *array, ptrdiff_t len, void *map);
