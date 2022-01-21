#ifndef MY_SKIPLIST_IMPL_H_
#define MY_SKIPLIST_IMPL_H_
#define SKPLIST_MAX_LEVELS 12 //this variable will be at conf file. It shows the max_levels of the skiplist
//it should be allocated according to L0 size
#include <pthread.h>
#include <stdint.h>

/* kv_category has the same format as Parallax
 * users can define the category of their keys
 * IMPORTANT: the *_INLOG choices should not be used for in-memory staff!
 * BIG_INPLACE is an extra field for the skiplist
*/
enum kv_category {
	SKPLIST_SMALL_INPLACE = 0,
	SKPLIST_SMALL_INLOG,
	SKPLIST_MEDIUM_INPLACE,
	SKPLIST_MEDIUM_INLOG,
	SKPLIST_BIG_INLOG,
	SKPLIST_UNKNOWN_LOG_CATEGORY,
	SKLIST_BIG_INPLACE
};

enum kv_type { SKPLIST_KV_INPLACE, SKPLIST_KV_FORMAT };

struct skiplist_node {
	/*for parallax use*/
	uint64_t kv_dev_offt;
	pthread_rwlock_t rw_nodelock;
	struct skiplist_node *forward_pointer[SKPLIST_MAX_LEVELS];
	uint32_t level;
	uint32_t key_size;
	void *key;
	uint64_t value_size;
	void *value;
	enum kv_category cat;
	uint8_t tombstone : 1;
	uint8_t is_NIL;
};

struct skiplist_iterator {
	pthread_rwlock_t rw_iterlock;
	uint8_t is_valid;
	struct skiplist_node *iter_node;
};

struct skplist_insert_request {
	uint64_t kv_dev_offt;
	uint32_t key_size;
	void *key;
	uint32_t value_size;
	void *value;
	enum kv_category cat;
	uint8_t tombstone : 1;
};

struct skiplist {
	uint32_t level; //this variable will be used as the level hint
	struct skiplist_node *header;
	struct skiplist_node *NIL_element; //last element of the skip list
	/* a generic key comparator, comparator should return:
	 * > 0 if key1 > key2
	 * < 0 key2 > key1
	 * 0 if key1 == key2 */
	int (*comparator)(void *key1, void *key2, char key1_format, char key2_format);

	/* generic node allocator */
	struct skiplist_node *(*make_node)(struct skplist_insert_request *ins_req);
};

struct value_descriptor {
	void *value;
	uint32_t value_size;
	uint8_t found;
};

struct skiplist *init_skiplist(void);
void change_comparator_of_skiplist(struct skiplist *skplist,
				   int (*comparator)(void *key1, void *key2, char key1_format, char key2_format));
/*skiplist operations*/
struct value_descriptor search_skiplist(struct skiplist *skplist, uint32_t key_size, void *search_key);
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
