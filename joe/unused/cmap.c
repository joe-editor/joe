/*
 *	Characters maps
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

#include "types.h"

/* Build a cmap from an interval list */

void cmap_build(struct cmap *cmap, struct interval_list *list, void *dflt_map)
{
	struct interval_list *l;
	int x;
	/* Record default */
	cmap->dflt_map = dflt_map;
	for (x = 0; x != 128; ++x)
		cmap->direct_map[x] = dflt_map;
	/* Fill in direct map */
	for (l = list; l; l = l->next)
		if (l->interval.first >= 128)
			break;
		else {
			int ch;
			for (ch = l->interval.first; ch < 128 && ch <= l->interval.last; ++ch)
				cmap->direct_map[ch] = l->map;
		}
	/* Skip over list items which are wholly in the direct map */
	while (list && list->interval.last < 128)
		 list = list -> next;
	/* Calculate size of array */
	cmap->size = 0;
	for (l = list; l; l = l->next)
		++cmap->size;
	if (cmap->size) {
		/* Allocate and populate array */
		cmap->range_map = (struct interval_map *)joe_malloc(SIZEOF(struct interval_map) * cmap->size);
		x = 0;
		for (l = list; l; l = l->next) {
			cmap->range_map[x].interval.first = l->interval.first;
			cmap->range_map[x].interval.last = l->interval.last;
			cmap->range_map[x].map = l->map;
			if (cmap->range_map[x].interval.first < 128)
				cmap->range_map[x].interval.first = 128;
			++x;
		}
	} else
		cmap->range_map = 0;
}

/* Clear cmap, free space */

void clr_cmap(struct cmap *cmap)
{
	if (cmap->range_map)
		joe_free(cmap->range_map);
	cmap->range_map = 0;
	cmap->size = 0;
}

/* Lookup a character in a cmap, return its assigned mapping */

void *cmap_lookup(struct cmap *cmap, int ch)
{
	if (ch < 128)
		return cmap->direct_map[ch];
	if (cmap->size) {
		ptrdiff_t min = 0;
		ptrdiff_t mid;
		ptrdiff_t max = cmap->size - 1;
		if (ch < cmap->range_map[min].interval.first || ch > cmap->range_map[max].interval.last)
			goto no_match;
		while (max >= min) {
			mid = (min + max) / 2;
			if (ch > cmap->range_map[mid].interval.last)
				min = mid + 1;
			else if (ch < cmap->range_map[mid].interval.first)
				max = mid - 1;
			else
				return cmap->range_map[mid].map;
		}
	}
	no_match:
	return cmap->dflt_map;
}

#ifdef junk
struct slist {
	struct slist *next;
	char *s;
} *slist;

char *find(char *s)
{
	struct slist *l;
	for (l = slist; l; l = l->next)
		if (!zcmp(l->s, s))
			return l->s;
	l = (struct slist *)joe_malloc(SIZEOF(struct slist));
	l->next = slist;
	slist = l;
	l->s = zdup(s);
	return l->s;
}

int main(int argc, char *argv)
{
	char buf[100];
	struct interval_list *list = 0;
	while (gets(buf)) {
		if (buf[0] == 'a') {
			int first, last;
			int map;
			sscanf(buf + 1," %d %d %d",&first,&last,&map);
			printf("a %d %d %d\n", first, last, map);
			list = interval_add(list, first, last, map);
		} else if (buf[0] == 's') {
			struct interval_list *l;
			for (l = list; l; l = l->next)
				printf("%d %d %d\n",l->interval.first,l->interval.last,l->map);
		}
	}
}
#endif

