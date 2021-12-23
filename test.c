#include "skiplist.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#define KVS_NUM 2000000

void populate_the_skiplist(struct skiplist* skplist)
{
    int i;

    for(i = 0; i < KVS_NUM; i++){
        insert_skiplist(skplist, "lala", "lalala");
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

    printf("Testing inserts\n");
    populate_the_skiplist(&my_skiplist);
    printf("Inserts test passeed\n");
}
