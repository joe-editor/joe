/* Line attribute database */

typedef long lattr_key_t;
typedef HIGHLIGHT_STATE lattr_value_t;
#define lattr_SKIP_DEPTH 10	/* Max pointers in a node */
#define lattr_hist 1024		/* Max no. of nodes */

struct lattr_node
  {
  LINK(struct lattr_node) link;	/* Doubly-linked list of nodes in age order */
  lattr_key_t key;		/* Key for this node */
  int nptrs;			/* No. pointers */
  lattr_value_t value;		/* Value */
  struct lattr_node *ptrs[1];	/* Node pointers */
  };

struct lattr_db
  {
  struct lattr_node *top;	/* Skip list root node */
  struct lattr_node nil;	/* Ending skiplist node and doubly-linked list base */
  struct lattr_node *update[lattr_SKIP_DEPTH];
                                /* Search path record */
  int count;			/* No. nodes */
  };

struct lattr_db *mk_lattr_db();
                                /* Initialize database structure */

void rm_lattr_db(struct lattr_db *db);
                                /* Delete database structure */

void lattr_set(struct lattr_db *db,lattr_key_t key,lattr_value_t value);
                                /* Set key to value */

int lattr_get(struct lattr_db *db,lattr_key_t *key,lattr_value_t *value);
                                /* Get nearest key/value */

void lattr_cut(struct lattr_db *db,lattr_key_t key);
                                /* Delete all nodes which follow key */
