#ifndef MY_SKIPLIST_IMPL_H_
#define MY_SKIPLIST_IMPL_H_
#define MAX_LEVELS 8 //this variable will be at conf file. It shows the max_levels of the skiplist
//it should be allocated according to L0 size
#include <pthread.h>
#include <stdint.h>

struct skiplist_node {
	pthread_rwlock_t rw_nodelock;
	struct skiplist_node *forward_pointer[MAX_LEVELS];
	uint32_t level;
	char *key;
	char *value;
	uint8_t is_NIL;
};

struct skiplist_iterator {
	pthread_rwlock_t rw_iterlock;
	uint8_t is_valid;
	struct skiplist_node *iter_node;
};

struct skiplist {
	uint32_t level; //this variable will be used as the level hint
	struct skiplist_node *header;
	struct skiplist_node *NIL_element; //last element of the skip list
};

void init_skiplist(struct skiplist *skplist);
char *search_skiplist(struct skiplist *skplist, char *search_key);
void insert_skiplist(struct skiplist *skplist, char *key, char *value);
void delete_skiplist(struct skiplist *skplist, char *key);
void free_skiplist(struct skiplist *skplist);
/*iterators staff*/
void init_iterator(struct skiplist_iterator *iter, struct skiplist *skplist, char *search_key);
void get_next(struct skiplist_iterator *iter);
uint32_t random_level(); // Required from simple_test.c

#endif // SKIPLIST_H_
