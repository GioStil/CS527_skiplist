#include "skiplist.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <string.h>

#define KVS_NUM 100000
#define KV_PREFIX "ts"


static void print_skplist(struct skiplist* skplist)
{
    struct skiplist_node* curr;
    for(int i = 0; i < MAX_LEVELS; i++){
        curr = skplist->header;
        printf("keys at level %d -> ", i);
        while(!curr->is_NIL){
            printf("[%s], ", curr->key);
            curr = curr->forward_pointer[i];
        }
        printf("\n");
    }
}

static void populate_the_skiplist(struct skiplist* skplist)
{
    int i;
    char* key = malloc(3 + sizeof(long long unsigned));

    for(i = 0; i < KVS_NUM; i++){
        memcpy(key, KV_PREFIX, strlen(KV_PREFIX));
        sprintf(key + strlen(KV_PREFIX), "%llu", (long long unsigned)i);
        insert_skiplist(skplist, key, "lala");
    }
}

static void test_random_level_generator()
{
    uint32_t i, rand_level;
    for(i = 0; i < KVS_NUM; i++){
        rand_level = random_level();
        assert(rand_level < MAX_LEVELS && rand_level >= 0);
    }

}

static void print_each_level_size(struct skiplist skplist)
{
    uint64_t count, i;
    struct skiplist_node* curr;
    for(i = 0; i < MAX_LEVELS; i++){
        count = 0;
        curr = skplist.header;

        while(curr->is_NIL == 0){
            count++;
            curr = curr->forward_pointer[i];
        }
        printf("level's %lu size is %lu\n", i , count);
    }
}

int main(){
    srand(time(0));
    int i;
    struct skiplist my_skiplist;

    printf("Testing initialization\n");
    init_skiplist(&my_skiplist);
    assert(my_skiplist.level == 0);
    for(i = 0; i < MAX_LEVELS; i++)
        assert(my_skiplist.header->forward_pointer[i] == my_skiplist.NIL_element);
    printf("Initialization passed\n");

    printf("Testing random_level generator\n");
    test_random_level_generator();
    printf("Random level generator passed\n");

    printf("Testing inserts\n");
    populate_the_skiplist(&my_skiplist);
    printf("Inserts test passeed\n");

    print_skplist(&my_skiplist);
    print_each_level_size(my_skiplist);
}
