/*
 *	Character maps
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

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


