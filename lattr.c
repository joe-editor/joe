/*
 *	Line attribute cache
 *	Copyright
 *		(C) 2006 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 *
 * This is a gap buffer which stores the syntax highlighter parser state for
 * the beginning of each line.  The computation is lazy, so not all lines in
 * the file will have their state computed (for example, if you never visit
 * the end of the file).
 *
 * When a change occurs (an insert or a delete), two things happen:
 *   First, if whole lines are inserted or deleted, the corresponding states
 *   are inserted or deleted as well, so that the cached states still match
 *   up with their lines following the insert or delete.
 *
 *   Second, a window of states which must be recomputed is generated.  This
 *   covers all the space for the change with the lowest line number to the
 *   change with the highest line number.
 *
 * Now, when the screen update algorithm requests the state for a line in or
 * after this region, the states within the region are recomputed.  The
 * cached states after the region may be valid: if a state is computed which
 * matches whatever is in the cache, then all of the remaining states are
 * trusted.  The idea is that we should not always have to recompute the
 * syntax states for the entire file for small changes.
 *
 * Anyway, prior to this cache, JOE would use lots of CPU time whenever you
 * hit PgUp, because it would have to recompute all of the states from the
 * first line of the file to the top of the screen.  Now this information is
 * saved, which uses more space, but allows JOE to use much less CPU time.
 */

#include "types.h"
#include "syntax.h"
#include "b.h"

/* Create a line attribute database */

struct lattr_db *mk_lattr_db(B *new_b, struct high_syntax *new_syn) {
	struct lattr_db *db = (struct lattr_db *)joe_malloc(sizeof(struct lattr_db));
	db->next = 0;
	db->syn = new_syn;
	db->b = new_b;
	db->end = 512;
	db->hole = 1;
	db->ehole = db->end;
	db->buffer = (HIGHLIGHT_STATE *)calloc(db->end, sizeof(HIGHLIGHT_STATE));
	db->first_invalid = 1;
	db->invalid_window = -1;
	/* State of first line is idle */
	clear_state(db->buffer+0);
	return db;
}

/* Delete a database */

void rm_lattr_db(struct lattr_db *db) {
	free(db->buffer);
	free(db);
}

/* Delete linked list of databases */

void rm_all_lattr_db(struct lattr_db *db) {
	struct lattr_db *n;
	while (db) {
		n = db->next;
		db->next = 0;
		rm_lattr_db(db);
		db = n;
	}
}

/* Set gap position */

void lattr_hole(struct lattr_db *db, long pos) {
	if (pos > db->hole) {
		long amnt = pos - db->hole;
		mmove(db->buffer + db->hole, db->buffer + db->ehole, amnt * sizeof(HIGHLIGHT_STATE));
		db->hole = pos;
		db->ehole += amnt;
	} else if (pos < db->hole) {
		long amnt = db->hole - pos;
		mmove(db->buffer + db->ehole - amnt, db->buffer + pos, amnt * sizeof(HIGHLIGHT_STATE));
		db->hole = pos;
		db->ehole -= amnt;
	}
}

/* Make sure there is enough space for an insert */

void lattr_check(struct lattr_db *db, long amnt) {
	if (amnt > db->ehole - db->hole) {
		/* Not enough space */
		/* Amount of additional space needed */
		amnt = amnt - (db->ehole - db->hole) + 16;
		db->buffer = (HIGHLIGHT_STATE *)realloc(db->buffer, (db->end + amnt) * sizeof(HIGHLIGHT_STATE));
		mmove(db->buffer + db->ehole, db->buffer + db->ehole + amnt, db->end - db->ehole);
		db->ehole += amnt;
		db->end += amnt;
	}
}

/* Find a database for a particular syntax in a buffer.  If none exists, create one. */

struct lattr_db *find_lattr_db(B *b, struct high_syntax *y) {
	struct lattr_db *db;
	for (db = b->db; db && db->syn != y; db = db->next);
	if (!db) {
		db = mk_lattr_db(b, y);
		db->next = b->db;
		b->db = db;
	}
	return db;
}

/* Drop a database, but only if no BWs refer to it */

void drop_lattr_db(B *b, struct lattr_db *db) {
#if junk
	if (!lattr_db_in_use(db)) {
		if (b->db == db) {
			b->db = db->next;
		} else {
			struct lattr_db *x;
			for (x = b->db; x->next != db; x = x->next);
			x->next = db->next;
		}
		rm_lattr_db(db);
	}
#endif
}

/* An insert occurred */

void lattr_ins(struct lattr_db *db,long line,long size) {
	++line; /* First invalid line is the one following the insert */

	/* Are we before the end? */
	if (line < lattr_size(db)) {
		/* Insert */
		if (size) {
			lattr_hole(db, line);
			lattr_check(db, size);
			db->ehole -= size;
		}
		/* Update beginning of invalid window */
		if (line < db->first_invalid) {
			if (db->invalid_window != -1)
				db->invalid_window += db->first_invalid - line;
			else
				db->invalid_window = 0;
			db->first_invalid = line;
		}
		/* invalid_window can not be set to -1 at this point */
		/* Update end of invalid window */
		if (line + size > db->first_invalid + db->invalid_window) {
			db->invalid_window = line + size - db->first_invalid;
		}
	}
}

/* A deletion occurred */

void lattr_del(struct lattr_db *db, long line, long size) {
	++line; /* First invalid line is the one following the delete */

	/* Are we before the end? */
	if (line < lattr_size(db)) {
		/* Delete */
		if (size) {
			lattr_hole(db, line);
			if (size > db->end - db->ehole)
				size = db->end - db->ehole;
			db->ehole += size;
			/* Fix window position */
			if (db->invalid_window == -1) {
				/* There is no window */
				db->first_invalid -= size;
			} else {
				/* There is a window: check all of the boundary cases */
				if (line < db->first_invalid) {
					if (line + size < db->first_invalid) {
						db->first_invalid -= size;
					} else if (line + size < db->first_invalid + db->invalid_window) {
						db->invalid_window -= line + size - db->first_invalid;
						db->first_invalid = line;
					} else {
						db->first_invalid = line;
						db->invalid_window = 0;
					}
				} else if (line < db->first_invalid + db->invalid_window) {
					if (line + size < db->first_invalid + db->invalid_window)
						db->invalid_window -= size;
					else
						db->invalid_window = line - db->first_invalid;
				}
				/* If we deleted the window */
				if (db->first_invalid + db->invalid_window == lattr_size(db)) {
					db->first_invalid = lattr_size(db);
					db->invalid_window = -1;
				}
			}
		}
		/* Update beginning of invalid window */
		if (line < db->first_invalid) {
			if (db->invalid_window != -1)
				db->invalid_window += db->first_invalid - line;
			else
				db->invalid_window = 0;
			db->first_invalid = line;
		}
		/* Update end of invalid window */
		if (line > db->first_invalid && line > db->first_invalid + db->invalid_window)
			db->invalid_window = line - db->first_invalid;
	}
}

/* Get attribute for a specific line */

HIGHLIGHT_STATE lattr_get(struct lattr_db *db, struct high_syntax *y, P *p, long line) {

	/* Past end of file? */
	if (line > p->b->eof->line) {
		HIGHLIGHT_STATE x;
		clear_state(&x);
		return x;
	}

	/* Check if we need to expand */
	if (line >= lattr_size(db)) {
		long x;
		/* Expand by this amount */
		long amnt = line - lattr_size(db) + 1;
		/* Set position to end */
		lattr_hole(db, lattr_size(db));
		lattr_check(db, amnt);
		db->ehole -= amnt;
		/* Set invalid window to cover new space */
		if (db->invalid_window == -1) {
			db->first_invalid = lattr_size(db) - amnt;
			db->invalid_window = amnt;
		} else {
			db->invalid_window = lattr_size(db) - db->first_invalid;
		}
	}

	/* Check if we need to shrink */
	if (p->b->eof->line + 1 < lattr_size(db)) {
		lattr_hole(db, p->b->eof->line + 1);
		db->ehole = db->end;
		if (db->first_invalid > lattr_size(db)) {
			db->first_invalid = lattr_size(db);
			db->invalid_window = -1;
		}
		if (db->invalid_window != -1 && db->first_invalid + db->invalid_window > lattr_size(db)) {
			db->invalid_window = lattr_size(db) - db->first_invalid;
		}
	}

	/* Check if we are pointing to a valid record */
	if (line >= db->first_invalid) {
		long ln;
		P *tmp = 0;
		HIGHLIGHT_STATE state;
		tmp = pdup(p, US "lattr_get");
		ln = db->first_invalid; /* First line with known good state */
		state = lattr_lvalue(db, ln - 1); /* Known good state */
		/* Compute up to requested line */
		pline(tmp, ln - 1);

		/* Recompute everything in invalid window */
		while (ln < db->first_invalid + db->invalid_window /* && ln < line */) {
			state = parse(y, tmp, state);
			lattr_lvalue(db, ln) = state;
			++ln;
		}

		/* Update invalid window: hopefully we did the whole window */
		db->invalid_window -= ln - db->first_invalid;
		db->first_invalid = ln;

		/* Recompute until match found.  If match is found, we can assume rest is valid */
		while (ln < lattr_size(db) /* && ln < line */) {
			HIGHLIGHT_STATE *prev;
			state = parse(y, tmp, state);
			prev = &lattr_lvalue(db, ln);
			if (!eq_state(prev, &state))
				lattr_lvalue(db, ln) = state;
			else {
				db->first_invalid = lattr_size(db);
				db->invalid_window = -1;
				++ln;
				break;
			}
			++ln;
		}

		/* Update invalid pointer */
		if (ln > db->first_invalid) {
			db->first_invalid = ln;
			db->invalid_window = 0;
		}
		if (ln == lattr_size(db)) {
			db->first_invalid = ln;
			db->invalid_window = -1;
		}
		prm(tmp);
	}

	/* Return with attribute */
	return lattr_lvalue(db, line);
}
