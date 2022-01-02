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
struct skiplist concurrent_skiplist;

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

static void populate_skiplist_with_single_writer(struct skiplist* skplist)
{
    int i;
    char* key = malloc(strlen(KV_PREFIX) + sizeof(long long unsigned));

    for(i = 0; i < KVS_NUM; i++){
        memcpy(key, KV_PREFIX, strlen(KV_PREFIX));
        sprintf(key + strlen(KV_PREFIX), "%llu", (long long unsigned)i);
        insert_skiplist(skplist, key, key);
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
        //printf("Inserting key%s\n",key);
        insert_skiplist(&concurrent_skiplist, key, key);
        //print_skplist(&my_skiplist);
    }
}

static void  validate_number_of_KVS(struct skiplist skplist){
    int count;
    struct skiplist_node* curr = skplist.header;

    while(curr->forward_pointer[0] != skplist.NIL_element){
        curr = curr->forward_pointer[0];
        count++;
    }

    assert(count - 1 == KVS_NUM); //-1 for the header node
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

/*compare the lists and check if all the nodes are present in the concurrent list
 *according to the single writer list (which is correct)
 *the function checks only the level0 of the skiplists where all keys reside*/
static void compare_the_lists(struct skiplist clist, struct skiplist swlist)
{
    int ret;
    struct skiplist_node* ccurr, *swcurr;
    ccurr = clist.header->forward_pointer[0]; //skip the header, start from the first key
    swcurr = swlist.header->forward_pointer[0]; //skip the header, start from the first key

    while(swcurr->forward_pointer[0] != swlist.NIL_element){
        int key_size = strlen(swcurr->key) > strlen(ccurr->key) ? strlen(swcurr->key) : strlen(ccurr->key);
        ret = memcmp(swcurr->key, ccurr->key, key_size);
        if(ret == 0){ //all good step
            swcurr = swcurr->forward_pointer[0];
            ccurr = ccurr->forward_pointer[0];
        }else{
            printf("Found key %s over %s that doesnt exist at the concurrent skiplist\n", swcurr->key, ccurr->key);
            break;
        }
    }
}

int main(){
    srand(time(0));
    int i;
    struct thread_info thread_buf[NUM_OF_THREADS];
    struct skiplist skiplist_single_writer;

    init_skiplist(&skiplist_single_writer);
    init_skiplist(&concurrent_skiplist);

    populate_skiplist_with_single_writer(&skiplist_single_writer);
    validate_number_of_KVS(skiplist_single_writer);

    for(i = 0; i < NUM_OF_THREADS; i++){
        thread_buf[i].tid = (uint32_t*) malloc(sizeof(int));
        *thread_buf[i].tid = i;
        pthread_create(&thread_buf[i].th, NULL, populate_the_skiplist, thread_buf[i].tid);
    }

    for(i = 0; i < NUM_OF_THREADS; i++)
        pthread_join(thread_buf[i].th, NULL);

    compare_the_lists(concurrent_skiplist,skiplist_single_writer);
    validate_number_of_KVS(concurrent_skiplist);

    free_skiplist(&concurrent_skiplist);
    free_skiplist(&skiplist_single_writer);
}
