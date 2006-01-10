/* Line attribute database */

#include "types.h"
#include "syntax.h"


struct lattr_node *lattr_find(struct lattr_db *db, lattr_key_t key)
  {
  int y;
  struct lattr_node *t = db->top;
  for (y = t->nptrs; --y >= 0;)
    {
    while (t->ptrs[y]->key < key)
      t = t->ptrs[y];
    db->update[y] = t;
    }
  return t->ptrs[0];
  }

struct lattr_node *lattr_ins(struct lattr_db *db, lattr_key_t key, lattr_value_t value)
  {
  struct lattr_node *t;
  int level, x;

  /* Choose level for this node */
  for (level = 1; random() < 0x29000000; ++level);
  if (level > (lattr_SKIP_DEPTH-1))
    level = lattr_SKIP_DEPTH-1;

  /* Allocate new node */
  t = (struct lattr_node *)joe_malloc(sizeof(struct lattr_node) + level * sizeof(struct lattr_node *));

  /* Increase height of tree if necessary */
  while (level > db->top->nptrs)
    db->update[db->top->nptrs++] = db->top;

  /* Insert node in list */
  t->nptrs = level;

  t->key = key;

  t->value = value;

  for (x = 0; x != level; ++x)
    {
    t->ptrs[x] = db->update[x]->ptrs[x];
    db->update[x]->ptrs[x] = t;
    }

  return t;
  }

/* Delete a node */

void lattr_del(struct lattr_db *db, struct lattr_node *t)
  {
  int x;

  /* Remove from list */
  for (x = 0; x != db->top->nptrs && db->update[x]->ptrs[x] == t; ++x)
    db->update[x]->ptrs[x] = t->ptrs[x];

  /* Reduce height of tree if we can */
  while (db->top->nptrs && db->top->ptrs[db->top->nptrs-1] == &db->nil)
    --db->top->nptrs;

  /* Free node */
  joe_free(t);
  }

// Set key to value

void lattr_set(struct lattr_db *db, lattr_key_t key, lattr_value_t value)
  {
  struct lattr_node *t;
  t = lattr_find(db, key);
  if (t->key == key)
    {
    t->value = value;
    }
  else
    {
    lattr_ins(db, key, value);
    }
  }

// Find node with nearest key

int lattr_get(struct lattr_db *db, lattr_key_t *key, lattr_value_t *value)
  {
  struct lattr_node *t;
  t = lattr_find(db, *key);
  if (t->key == *key)
    {
    /* Exact match */
    *value = t->value;
    return 0;
    }
  else
    {
    /* Next lowest value */
    /* Point to previous node */
    t = db->update[0];
    *key = t->key;
    *value = t->value;
    return -1;
    }
  }

/* Delete all nodes after one with matching key */

void lattr_cut(struct lattr_db *db, lattr_key_t key)
  {
  struct lattr_node *u, *t=lattr_find(db, key);
  while (t != &db->nil)
    {
    u = t->ptrs[0];
    lattr_del(db, t);
    t=u;
    }
  }

struct lattr_db *mk_lattr_db()
  {
  int x;
  struct lattr_db *db;

  db = (struct lattr_db *)joe_malloc(sizeof(struct lattr_db));

  db->nil.key = 0x7FFFFFFF;
  db->nil.nptrs = 0;
  db->nil.ptrs[0] = &db->nil;

  db->top = (struct lattr_node *)joe_malloc(sizeof(struct lattr_node) + lattr_SKIP_DEPTH * sizeof(struct lattr_node *));
  db->top->key = 0;
  db->top->nptrs = 0;
  clear_state(&db->top->value);

  for(x = 0; x != lattr_SKIP_DEPTH; ++x)
    db->top->ptrs[x] = &db->nil;

  db->count = 0;

  lattr_ins(db, 0, db->top->value);

  return db;
  }

void rm_lattr_db(struct lattr_db *db)
  {
  struct lattr_node *t, *u;
  for (t=db->top; t != &db->nil; t = u)
    {
    u=t->ptrs[0];
    joe_free(t);
    }
  joe_free(db);
  }
