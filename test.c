#include "skiplist.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <string.h>

#define KVS_NUM 1000000
#define KV_PREFIX "ts"
#define NUM_OF_THREADS 8

struct thread_info{
    pthread_t th;
    uint32_t* tid;
};

struct skiplist my_skiplist;

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

static void* populate_the_skiplist(void* args)
{
    int i, from, to;
    char* key = malloc(strlen(KV_PREFIX) + sizeof(long long unsigned));
    int* tid = (int*) args;

    from =  (int) ( ( (*tid) / (double) NUM_OF_THREADS ) * KVS_NUM );
    to = (int) ( ( (*tid + 1) / (double) NUM_OF_THREADS) * KVS_NUM );
    printf("inserting from %d to %d\n", from, to);
    for(i = from; i < to; i++){
        memcpy(key, KV_PREFIX, strlen(KV_PREFIX));
        sprintf(key + strlen(KV_PREFIX), "%llu", (long long unsigned)i);
        insert_skiplist(&my_skiplist, key, key);
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
        printf("level's %llu size is %llu\n", i , count);
    }
}

static void delete_half_keys(struct skiplist* skplist)
{
    int i;
    char* key = malloc(strlen(KV_PREFIX) + sizeof(long long unsigned));

    for(i = 0; i < KVS_NUM / 2; i++){
        memcpy(key, KV_PREFIX, strlen(KV_PREFIX));
        sprintf(key + strlen(KV_PREFIX), "%llu" , (unsigned long long)i);
        printf("Deleting key%s\n", key);
        delete_skiplist(skplist, key);
        print_skplist(skplist);
    }
}

//this function also validates the return results
static void* search_the_skiplist(void* args){
    int i, from, to;
    char* key = malloc(strlen(KV_PREFIX) + sizeof(long long unsigned));
    int* tid = (int*) args;
    char* ret_val;

    from = (int) ( ( (*tid) / (double) NUM_OF_THREADS ) * KVS_NUM );
    to = (int) ( ( (*tid + 1) / (double) NUM_OF_THREADS) * KVS_NUM );
    printf("Searching from %d to %d\n", from, to);
    for(i = from; i < to; i++){
       memcpy(key, KV_PREFIX, strlen(KV_PREFIX));
       sprintf(key + strlen(KV_PREFIX), "%llu" , (unsigned long long)i);
       ret_val = search_skiplist(&my_skiplist, key);
       assert(ret_val != NULL);
       assert(memcmp(ret_val, key, strlen(key)) == 0); //keys and value are same in this test
    }
}

static void validate_number_of_kvs()
{
    int count = 0;
    struct skiplist_node* curr = my_skiplist.header->forward_pointer[0]; //skip the header

    while(!curr->is_NIL){
        ++count;
        curr = curr->forward_pointer[0];
    }
    assert(count == KVS_NUM);
}

static void validate_number_of_kvs_with_iterators()
{
    int count = 0;
    struct skiplist_iterator iter;
    init_iterator(&iter, &my_skiplist, "ts0"); //ts0 is the first key in this test
    while(iter.is_valid){
        ++count;
        get_next(&iter);
    }
    assert(count = KVS_NUM);
}

int main(){
    srand(time(0));
    int i;
    struct thread_info thread_buf[NUM_OF_THREADS];

    init_skiplist(&my_skiplist);
    assert(my_skiplist.level == 0);
    for(i = 0; i < MAX_LEVELS; i++)
        assert(my_skiplist.header->forward_pointer[i] == my_skiplist.NIL_element);

    test_random_level_generator();

    for(i = 0; i < NUM_OF_THREADS; i++){
        thread_buf[i].tid = (uint32_t*) malloc(sizeof(int));
        *thread_buf[i].tid = i;
        pthread_create(&thread_buf[i].th, NULL, populate_the_skiplist, thread_buf[i].tid);
    }

    for(i = 0; i < NUM_OF_THREADS; i++)
        pthread_join(thread_buf[i].th, NULL);

    validate_number_of_kvs();
    printf("Validation of number of KVs passed\n");

    for(i = 0; i < NUM_OF_THREADS; i++){
        thread_buf[i].tid = (uint32_t*) malloc(sizeof(int));
        *thread_buf[i].tid = i;
        pthread_create(&thread_buf[i].th, NULL, search_the_skiplist, thread_buf[i].tid);
    }

    for(i = 0; i < NUM_OF_THREADS; i++)
        pthread_join(thread_buf[i].th, NULL);

    validate_number_of_kvs_with_iterators();

    free_skiplist(&my_skiplist);
}
