/* Line attribute b-tree */

#define ltree_index_size 127
#define ltree_node_size 31
#define ltree_path_size 5

/* A node or index pointer: this is used for both indexes and paths */

struct ltree_ptr
  {
  struct ltree_node *node; /* Pointer to leaf or index */
  union
    {
    long count; /* No. values in this sub-tree */
    int idx; /* Entry index */
    };
  };

/* An index or leaf page */

struct ltree_node
  {
  union
    {
    struct ltree_ptr ptrs[ltree_index_size]; /* If this is an index page */
    HIGHLIGHT_STATE value[ltree_node_size]; /* If this is a leaf page */
    };
  int count; /* No. entries (values or pointers) in just this page */
  };

/* A whole database */

struct ltree_db
  {
  struct ltree_node *top;	/* Top level index */
  int levels;			/* No. levels (min is 2: one leaf and one index) */

  /* Current path */
  /* path[0] points to leaf page, path[levels-1] points to top level index page */
  struct ltree_ptr path[ltree_path_size];
  long line;			/* Line number of current path */

  };

/* Advance path */

struct ltree_node *ltree_next(struct ltree_db *db, int z)
  {
  if (++db->path[z].idx == db->path[z].node->count)
    {
    db->path[z].idx = 0;
    db->path[z].node = ltree_next(db, z + 1);
    }
  if (z)
    return db->path[z].node->ptrs[db->path[z].idx].node;
  else
    return 0;
  }

void ltree_advance(struct ltree_db *db)
  {
  ltree_next(db, 0);
  ++db->line;
  }

/* Retreat path */

struct ltree_node *ltree_prev(struct ltree_db *db, int z)
  {
  if (!db->path[z].idx--)
    {
    db->path[z].node = ltree_prev(db, z + 1);
    db->path[z].idx = db->path[z].node->count - 1;
    }
  if (z)
    return db->path[z].node->ptrs[db->path[z].idx].node;
  else
    return 0;
  }

void ltree_retreat(struct ltree_db *db)
  {
  ltree_prev(db, 0);
  --db->line;
  }

/* Find an entry, leave path to it */

void ltree_find(struct ltree_db *db, unsigned long find_line)
  {
  /* Are we already there? */
  if (db->line == find_line)
    {
    return;
    }
  /* Are we close? */
  else if (find_line > db->line && find_line - db->line < 100)
    {
    while (find_line != db->line)
      ltree_advance(db);
    }
  else if (find_line < db->line && db->line - find_line < 100)
    {
    while (find_line != db->line)
      ltree_retreat(db);
    }
  else
    {
    /* Full tree traversal */
    int z;
    struct ltree_node *p = db->top;
    db->line = fine_line;

    /* Index nodes */
    for (z = db->levels - 1; z != 0; --z)
      {
      int i;
      db->path[z].node = p;
      for (i = 0; i != ltree_index_size; ++i)
        {
        if (find_line > p->ptrs[i].count)
          find_line -= p->ptrs[i].count;
        else
          break;
        }
      db->path[z].idx = i;
      p = p->ptrs[i].node; /* Type cheating here on last one */
      }

    /* Leaf node */
    db->path[z].node = p;
    db->path[z].idx = find_line;
    db->idx = find_line;
    db->node = (struct ltree_node *)p;
    }
  }

/* Split an index node */

void ltree_split_index(struct ltree_db *db, int z)
  {
  /* Get current index node pointer and offset */
  struct ltree_node *node = db->path[z].node;
  int idx = db->path[z].idx;

  /* Create new index node: it gets all the values after idx */
  struct ltree_node *n = (struct ltree_node *)malloc(sizeof(struct ltree_node));

  /* Copy in pointers after idx */
  n->count = node->count - idx;
  mcpy(n->value, node->value + idx, n->count * sizeof(struct ltree_ptr));

  /* Adjust value counts in parent indexes */
  }

/* Insert a node pointer */

void ltree_insert_node(struct ltree_db *db, int z, struct ltree_node *n, long count)
  {
  /* Get index pointer */
  struct ltree_index *i = db->path[z];
  int idx = db->path[z].idx;

  /* Is there space for the new node pointer in this index? */
  if (i->count == ltree_index_size)
    {
    /* There isn't, so we need to split the index */
    ltree_split_index(db, z);
    /* ltree_split_index leaves path pointing to less-full index */

    /* Get index pointer again */
    i = db->path[z];
    idx = db->path[z].idx;
    }

  /* Make space for new node pointer in index */
  mmove(i->ptrs + idx + 1, i->ptrs + idx, i->count - idx);

  /* Write new node pointer into index */
  i->ptrs[idx].node = n;
  i->ptrs[idx].count = count;

  /* Update pointer count of index */
  ++i->count;

  /* Adjust value counts in parent indexes */
  for (++z; z != db->levels; ++z)
    db->path[z].node->ptrs[db->path[z].idx].count += count;
  }

/* Split leaf at current path */

void ltree_split_leaf(struct ltree_db *db)
  {
  int z;
  /* Get current leaf pointer and index */
  struct ltree_node *node = db->path[0].node;
  int idx = db->path[0].idx;

  /* Create new leaf node: it gets all the values after idx */
  struct ltree_node *n = (struct ltree_node *)malloc(sizeof(struct ltree_node));

  /* Copy in values after idx */
  n->count = node->count - idx;
  mcpy(n->value, node->value + idx, n->count * sizeof(HIGHLIGHT_STATE));

  /* Adjust value count in original node */
  node->count -= n->count;

  /* Adjust value counts in parent indexes */
  for (z = 1; z != db->levels; ++z)
    db->path[z].node->ptrs[db->path[z].idx].count -= count;

  /* Fix path: leaf pointer now goes to new node */
  db->path[0].node = n;
  db->path[0].idx = 0;

  /* Fix path: bump parent pointer so that insert goes to right place (it might be at end of index after this) */
  ++db->path[1].idx;

  /* Insert new leaf node pointer */
  ltree_insert_node(db, 1, n);
  }

/* Insert a node pointer with nnodes node pointers */

void ltree_insert_nodes(struct ltree_db *db, int z, struct ltree_node **ary, int nnodes, unsigned long count,
                        int node_for_path)
  {
  struct ltree_index *i = db->path[z].node;
  int idx = db->path[z].idx;
  if (nnodes - 1 <= ltree_index_size - i->count)
    { /* They fit in this index */
    int x;
    /* Make space */
    mmove(i->ptrs + idx + nnodes,
          i->ptrs + idx + 1,
          sizeof(struct ltree_ptr) * (i->count - (idx + 1)));
    /* Copy in node pointers */
    for (x = 0; x != nnodes; ++x)
      {
      i->ptrs[idx + x].node = ary[x];
      i->ptrs[idx + x].count = ary[x]->count;
      }
    /* Update path */
    db->path[z].idx += node_for_path;
    /* Update item counts */
    for (; z != db->levels; ++z)
      {
      struct ltree_index *i = db->path[z].node;
      int idx = db->path[z].idx;
      i->ptrs[idx].count += count;
      }
    /* Update path */
    }
  else
    { /* Split the index */
    unsigned long total = nnodes + i->count; /* Total number of items */
    unsigned long rem = total;
    unsigned long nnodes = (total / ltree_index_size); /* Number of new nodes */
    unsigned long each = total / nnodes; /* Number of items in each node (except last can be different) */
    unsigned long x;
    int y, z;
    int node_for_path;
    struct ltree_node **ary;
    ary = (struct ltree_node **)malloc(sizeof(struct ltree_node *)*nnodes);
    /* Create nodes */
    for (x = 0; x != nnodes; ++x)
      {
      struct ltree_node *n = (struct ltree_node *)malloc(sizeof(struct ltree_node));
      ary[x] = n;
      if (rem < each)
        {
        n->count = rem;
        }
      else
        {
        n->count = each;
        rem -= each;
        }
      }
    /* Copy in data, initialize new values */
    y = 0;
    z = 0;
    for (x = 0; x != total; ++x)
      {
      if (x < idx)
        {
        ary[y]->value[z] = node->ptrs[x];
        }
      else if (x >= total - idx)
        {
        ary[y]->value[z] = node->ptrs[x - (total - idx)];
        }
      else if (x == idx)
        {
        /* Update path pointer */
        db->path[0].node = ary[y];
        db->path[0].idx = z;
        node_for_path = y;
        invalidate_state(ary[y]->value[z]);
        }
      else
        {
        ary[y]->value[z] = ary[x - idx]
        invalidate_state(ary[y]->value[z]);
        }
      ++z;
      if (z == ary[y]->count)
        {
        ++y;
        z = 0;
        }
      }
    /* Delete original node */
    free(node);
    /* Replace pointer in index page with nnodes pointers */
    ltree_insert_nodes(db, 1, ary, nnodes, total, node_for_path);
    free(ary);
    }
  }

/* Insert N lines at current position */

void ltree_insert(struct ltree_db *db, unsigned long count)
  {
  struct ltree_node *node = db->path[0].node;
  int idx = db->path[0].idx;
  if (count <= ltree_node_size - node->count)
    { /* They fit in current node */
    int z;
    /* Make space */
    mmove(node->value + idx + count,
          node->value + idx,
          (node->count - idx)*sizeof(HIGHLIGHT_STATE));
    /* Invalidate the new entries */
    for (z = idx; z != idx + count; ++z)
      invalidate_state(node->value[z]);
    /* Update counts */
    for (z = 0; z != db->levels; ++z)
      db->path[z].node->count += count;
    }
  else
    { /* The node has to be split into n nodes */
    unsigned long total = count + node->count; /* Total number of items */
    unsigned long rem = total;
    unsigned long nnodes = (total / ltree_node_size); /* Number of new nodes */
    unsigned long each = total / nnodes; /* Number of items in each node (except last can be different) */
    unsigned long x;
    int y, z;
    int node_for_path;
    struct ltree_node **ary;
    ary = (struct ltree_node **)malloc(sizeof(struct ltree_node *)*nnodes);
    /* Create nodes */
    for (x = 0; x != nnodes; ++x)
      {
      struct ltree_node *n = (struct ltree_node *)malloc(sizeof(struct ltree_node));
      ary[x] = n;
      if (rem < each)
        {
        n->count = rem;
        }
      else
        {
        n->count = each;
        rem -= each;
        }
      }
    /* Copy in data, initialize new values */
    y = 0;
    z = 0;
    for (x = 0; x != total; ++x)
      {
      if (x < idx)
        {
        ary[y]->value[z] = node->value[x];
        }
      else if (x >= total - idx)
        {
        ary[y]->value[z] = node->value[x - (total - idx)];
        }
      else if (x == idx)
        {
        /* Update path pointer */
        db->path[0].node = ary[y];
        db->path[0].idx = z;
        node_for_path = y;
        invalidate_state(ary[y]->value[z]);
        }
      else
        {
        invalidate_state(ary[y]->value[z]);
        }
      ++z;
      if (z == ary[y]->count)
        {
        ++y;
        z = 0;
        }
      }
    /* Delete original node */
    free(node);
    /* Replace pointer in index page with nnodes pointers */
    ltree_insert_nodes(db, 1, ary, nnodes, total, node_for_path);
    free(ary);
    }
  }

/* Delete N lines at current position */

void ltree_delete(struct ltree_db *db, unsigned long count)
  {
  }
