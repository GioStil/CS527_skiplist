#ifndef MY_SKIPLIST_IMPL_H_
#define MY_SKIPLIST_IMPL_H_
#define SKPLIST_MAX_LEVELS 12 //this variable will be at conf file. It shows the max_levels of the skiplist
//it should be allocated according to L0 size
#include <pthread.h>
#include <stdint.h>
#define LOCK_TABLE_ENTRIES 2048

struct skplist_lock_table {
	pthread_rwlock_t rx_lock;
	char pad[8];
};

struct node_data {
	void *key;
	void *value;
	uint32_t key_size;
	uint32_t value_size;
};

struct skiplist_node {
	struct skiplist_node *forward_pointer[SKPLIST_MAX_LEVELS];
	struct node_data *kv;
	uint32_t level;
	uint8_t is_NIL;
};

struct skiplist_iterator {
	struct skiplist *iter_skplist;
	struct skiplist_node *iter_node;
	uint8_t is_valid;
};

struct skplist_insert_request {
	void *key;
	void *value;
	uint32_t key_size;
	uint32_t value_size;
};

struct skplist_search_request {
	void *key;
	void *value;
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
	int (*comparator)(void *key1, void *key2);

	/* generic node allocator */
	struct skiplist_node *(*make_node)(struct skiplist *skplist, struct skplist_insert_request *ins_req);
	/* This functions stores the value to its corresponding node.
	 * users may want to store metadata along with the value
	 * and they can achieve this by specifying their own function implementation
	 */
	void (*store_value)(struct skiplist_node *node, struct skplist_insert_request *ins_req);

	/* if a user have modified the store_value functionality, the retrieval functionality
	 * have to be accordinlgy changed if needed*/
	void (*retrieve_value)(struct skiplist_node *node, struct skplist_search_request *search_req);

	uint32_t level; //this variable will be used as the level hint
};

struct skiplist *init_skiplist(void);
void change_comparator_of_skiplist(struct skiplist *skplist, int (*comparator)(void *key1, void *key2));

void change_node_allocator_of_skiplist(struct skiplist *skplist,
				       struct skiplist_node *make_node(struct skiplist *skplist,
								       struct skplist_insert_request *ins_req));
void change_store_value(struct skiplist *skplist,
			void (*store_value)(struct skiplist_node *node, struct skplist_insert_request *ins_req));

void change_retrieve_value(struct skiplist *skplist, void (*retrieve_value)(struct skiplist_node *node,
									    struct skplist_search_request *search_req));

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
void *get_key(struct skiplist_iterator *iter);
uint32_t get_key_size(struct skiplist_iterator *iter);
void *get_value(struct skiplist_iterator *iter);
uint32_t get_value_size(struct skiplist_iterator *iter);
void skplist_close_iterator(struct skiplist_iterator *iter);
#endif // SKIPLIST_H_
