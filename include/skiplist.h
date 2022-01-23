#ifndef MY_SKIPLIST_IMPL_H_
#define MY_SKIPLIST_IMPL_H_
#define SKPLIST_MAX_LEVELS 12 //this variable will be at conf file. It shows the max_levels of the skiplist
//it should be allocated according to L0 size
#include <pthread.h>
#include <stdint.h>
#define LOCK_TABLE_ENTRIES 2048

/* kv_category has the same format as Parallax
 * users can define the category of their keys
 * IMPORTANT: the *_INLOG choices should not be used for in-memory staff!
*/
enum kv_category {
	SKPLIST_SMALL_INPLACE = 0,
	SKPLIST_SMALL_INLOG,
	SKPLIST_MEDIUM_INPLACE,
	SKPLIST_MEDIUM_INLOG,
	SKPLIST_BIG_INLOG,
	SKPLIST_UNKNOWN_LOG_CATEGORY,
	SKPLIST_BIG_INPLACE
};

enum kv_type { SKPLIST_KV_FORMAT = 19, SKPLIST_KV_PREFIX = 20 };

struct skplist_lock_table {
	pthread_rwlock_t rx_lock;
	char pad[8];
};

struct node_data {
	void *key;
	void *value;
	uint64_t kv_dev_offt; /* used for ptr to log */
	uint32_t key_size;
	uint32_t value_size;
};

struct skiplist_node {
	struct skiplist_node *forward_pointer[SKPLIST_MAX_LEVELS];
	struct node_data *kv;
	uint32_t level;
	uint8_t is_NIL;
	uint8_t tombstone : 1;
};

struct skiplist_iterator {
	struct skiplist *iter_skplist;
	struct skiplist_node *iter_node;
	uint8_t is_valid;
};

struct skplist_insert_request {
	void *key;
	void *value;
	uint64_t kv_dev_offt;
	uint32_t key_size;
	uint32_t value_size;
	uint8_t tombstone : 1;
};

struct skplist_search_request {
	void *key;
	void *value;
	uint64_t kv_dev_offt;
	uint32_t key_size;
	uint32_t value_size;
	uint8_t found;
};

struct skiplist {
	struct skplist_lock_table ltable[LOCK_TABLE_ENTRIES];
	struct skiplist_node *header;
	struct skiplist_node *NIL_element; //last element of the skip list
	/* a generic key comparator, comparator should return:
	 * > 0 if key1 > key2
	 * < 0 key2 > key1
	 * 0 if key1 == key2 */
	int (*comparator)(void *key1, void *key2, char key1_format, char key2_format);

	/* generic node allocator */
	struct skiplist_node *(*make_node)(struct skplist_insert_request *ins_req);
	uint32_t level; //this variable will be used as the level hint
};

struct skiplist *init_skiplist(void);
void change_comparator_of_skiplist(struct skiplist *skplist,
				   int (*comparator)(void *key1, void *key2, char key1_format, char key2_format));

void change_node_allocator_of_skiplist(struct skiplist *skplist,
				       struct skiplist_node *make_node(struct skplist_insert_request *ins_req));
/*skiplist operations*/
void search_skiplist(struct skiplist *skplist, struct skplist_search_request *search_req);
void insert_skiplist(struct skiplist *skplist, struct skplist_insert_request *ins_req);
void delete_skiplist(struct skiplist *skplist, char *key); //TBI
void free_skiplist(struct skiplist *skplist);
/*iterators staff*/
void init_iterator(struct skiplist_iterator *iter, struct skiplist *skplist, uint32_t key_size, void *search_key);
void iter_seek_to_first(struct skiplist_iterator *iter, struct skiplist *skplist);
void get_next(struct skiplist_iterator *iter);
/*return 1 if valid 0 if not valid*/
uint8_t is_valid(struct skiplist_iterator *iter);
void skplist_close_iterator(struct skiplist_iterator *iter);
#endif // SKIPLIST_H_
