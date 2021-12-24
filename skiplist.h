#ifndef MY_SKIPLIST_IMPL_H_
#define MY_SKIPLIST_IMPL_H_
#define MAX_LEVELS 8 //this variable will be at conf file. It shows the max_levels of the skiplist
                     //it should be allocated according to L0 size
#include <inttypes.h>

struct skiplist_node{
    struct skiplist_node* forward_pointer[MAX_LEVELS];
    char* key;
    char* value;
    uint8_t is_NIL; //(1) can we determine the maximum key?? if not we use this variable
};

struct skiplist{
    uint32_t level; //currently the highest level of the list
    struct skiplist_node* header;
    struct skiplist_node* NIL_element; //last element of the skip list
};


void init_skiplist(struct skiplist* skplist);
char* search_skiplist(struct skiplist* skplist, char* search_key);
void insert_skiplist(struct skiplist* skplist, char* key, char* value);
void delete_skiplist(struct skiplist* skplist, char* key);

#endif // SKIPLIST_H_
