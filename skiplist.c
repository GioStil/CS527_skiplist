#include "skiplist.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*FIXME this defines goes to parallax (already exists)*/
#define RWLOCK_INIT(L,attr) pthread_rwlock_init(L,attr)
#define RWLOCK_WRLOCK(L) pthread_rwlock_wrlock(L)
#define RWLOCK_RDLOCK(L) pthread_rwlock_rdlock(L)
#define RWLOCK_UNLOCK(L) pthread_rwlock_unlock(L)
/*FIXME this define goes to parallax(already exists)*/
#define MUTEX_INIT(L,attr) pthread_mutex_init(L, attr)
#define MUTEX_LOCK(L) pthread_mutex_lock(L)
#define MUTEX_UNLOCK(L) pthread_mutex_unlock(L)


uint32_t p = 1 / 4;
uint8_t is_list_level_lock_locked = 0;
pthread_mutex_t levels_lock_buf[MAX_LEVELS];
pthread_mutex_t skplist_level_hint_lock;


//FIXME this should be static and removed from the test file
uint32_t random_level() {
  uint32_t i;
  //MAX_LEVELS - 1 cause we want a number from range [0,MAX_LEVELS-1]
  for (i = 0; i < MAX_LEVELS - 1 && rand() % 2 == 0; i++)
    ;

  return i;
}

static struct skiplist_node *make_node(char *key, char *value, uint32_t level) {
  uint32_t key_size = strlen(key) + 1;
  uint32_t value_size = strlen(value) + 1;
  struct skiplist_node *new_node =
    (struct skiplist_node *)malloc(sizeof(struct skiplist_node));

  new_node->level = level;
  new_node->key = malloc(key_size);
  new_node->value = malloc(value_size);

  memcpy(new_node->key, key, key_size);
  memcpy(new_node->value,value, value_size);
  new_node->is_NIL = 0;

  RWLOCK_INIT(&new_node->rw_nodelock, NULL);
  return new_node;
}

// skplist is an object called by reference
void init_skiplist(struct skiplist *skplist) {
  int i;
  // allocate NIL (sentinel)
  skplist->NIL_element =
      (struct skiplist_node *)malloc(sizeof(struct skiplist_node));
  skplist->NIL_element->is_NIL = 1;
  skplist->NIL_element->level = 0;
  if(RWLOCK_INIT(&skplist->NIL_element->rw_nodelock, NULL) != 0) {
    exit(EXIT_FAILURE);
  }
  // level is 0
  skplist->level = 0; //FIXME reduntant in concurrent scheme

  skplist->header =
      (struct skiplist_node *)malloc(sizeof(struct skiplist_node));
  skplist->header->is_NIL = 0;
  skplist->header->level = 0;
  if(RWLOCK_INIT(&skplist->header->rw_nodelock, NULL) != 0){
    exit(EXIT_FAILURE);
  }

  // all forward pointers of header point to NIL
  for (i = 0; i < MAX_LEVELS; i++)
    skplist->header->forward_pointer[i] = skplist->NIL_element;

  MUTEX_INIT(&skplist_level_hint_lock, NULL);
}

char *search_skiplist(struct skiplist *skplist, char *search_key) {
  int i, ret;
  uint32_t key_size, node_size;
  char* ret_val;
  struct skiplist_node* next_curr;

  RWLOCK_RDLOCK(&skplist->header->rw_nodelock);
  struct skiplist_node* curr = skplist->header;

  for (i = skplist->header->level; i >= 0; i--) {

    next_curr = curr->forward_pointer[i];

    while (1){

      if(curr->forward_pointer[i]->is_NIL){
        break;
      }


      ret = memcmp(curr->forward_pointer[i]->key, search_key, key_size);

      if(ret < 0){
        RWLOCK_UNLOCK(&curr->rw_nodelock);
        curr = next_curr;
        RWLOCK_RDLOCK(&curr->forward_pointer[i]->rw_nodelock);
        next_curr = curr->forward_pointer[i];
      } else{
        RWLOCK_UNLOCK(&curr->forward_pointer[i]->rw_nodelock);
        break;
      }
    }
  }

  //we are infront of the node at level 0, node is locked, next node is not locked!
  //lock the next node, no data race for inserts here cause the forward ptr is locked
  RWLOCK_RDLOCK(&curr->forward_pointer[0]->rw_nodelock);
  next_curr = curr->forward_pointer[0]; //FIXME reduntant?
  RWLOCK_UNLOCK(&curr->rw_nodelock);
  curr = next_curr;

  //corner case
  //next element for level 0 is sentinel, key not found
  if (curr->is_NIL){
    RWLOCK_UNLOCK(&curr->rw_nodelock);
    return NULL;
  }

  ret = memcmp(curr->key, search_key, key_size);
  if (ret == 0){
    ret_val = curr->value;
    RWLOCK_UNLOCK(&curr->rw_nodelock);
    return ret_val;
  } else {
    RWLOCK_UNLOCK(&curr->rw_nodelock);
    return NULL;
  }
}

/*(write)lock the node in front of node *key* at level lvl*/
static struct skiplist_node* getLock(struct skiplist_node* curr, char* key, int lvl)
{
  //see if we can advance further due to parallel modifications
  //first proceed with read locks, then acquire write locks
  /*Weak serach will be implemented later(performance issues)*/

  //...
  //
  int ret, node_key_size;
  int key_size = strlen(key);
  struct skiplist_node* next_curr;

  if(lvl == 0 ) //if lvl is 0 we have locked the curr due to the search accross the levels
    RWLOCK_UNLOCK(&curr->rw_nodelock);

  //acquire the write locks from now on
  RWLOCK_WRLOCK(&curr->rw_nodelock);
  next_curr = curr->forward_pointer[lvl];

  while(1){
    if(curr->forward_pointer[lvl]->is_NIL){
      break;
    }

    node_key_size = strlen(curr->forward_pointer[lvl]->key);
    if(node_key_size > key_size)
      ret = memcmp(curr->forward_pointer[lvl]->key, key, node_key_size);
    else
       ret = memcmp(curr->forward_pointer[lvl]->key, key, key_size);

    if(ret < 0){
      RWLOCK_UNLOCK(&curr->rw_nodelock);
      curr = next_curr;
      RWLOCK_WRLOCK(&curr->rw_nodelock);
      next_curr = curr->forward_pointer[lvl];
    }else{
      break;
    }
  }
  return curr;
}

static uint32_t calculate_level(struct skiplist* skplist)
{
  uint32_t i,lvl = 0;
  for(i = 0; i < MAX_LEVELS; i++){
    if(skplist->header->forward_pointer[i] != skplist->NIL_element)
      lvl = i;
    else
      break;
  }

  return lvl;
}

void insert_skiplist(struct skiplist* skplist, char* key, char* value)
{
  int i, ret, lvl, header_rdlock_flag;
  uint32_t key_size, node_key_size;
  struct skiplist_node* update_vector[MAX_LEVELS];
  struct skiplist_node* next_curr;
  header_rdlock_flag = 0;
  key_size = strlen(key);
  RWLOCK_RDLOCK(&skplist->header->rw_nodelock);
  struct skiplist_node* curr = skplist->header;
  //we have the lock of the header, determine the lvl of the list
  lvl = calculate_level(skplist);
  /*traverse the levels till 0 */
  for(i = lvl; i >= 0; i--){
    next_curr = curr->forward_pointer[i];
    while(1){
      if(curr->forward_pointer[i]->is_NIL){
        break;
      }

      node_key_size = strlen(curr->forward_pointer[i]->key);
      if(node_key_size > key_size)
        ret = memcmp(curr->forward_pointer[i]->key, key, node_key_size);
      else
        ret = memcmp(curr->forward_pointer[i]->key, key, key_size);

      if(ret < 0){
        RWLOCK_UNLOCK(&curr->rw_nodelock);
        curr = next_curr;
        RWLOCK_RDLOCK(&curr->rw_nodelock);
        next_curr = curr->forward_pointer[i];
      } else{
        break;
      }
    }
    update_vector[i] = curr; //store the work done until now, this may NOT be the final nodes
                             //think that the concurrent inserts can update the list in the meanwhile
  }

  curr = getLock(curr, key, 0); //header rd lock flag is 0 here always
  //compare forward's key with the key
  //take as key_size the bigger key else we could have conflicts with e.g. 5 and 50 key
  if(!curr->forward_pointer[0]->is_NIL){
    node_key_size = strlen(curr->forward_pointer[0]->key);
    key_size = key_size > node_key_size ? key_size : node_key_size;
    ret = memcmp(curr->forward_pointer[0]->key, key, key_size);
  }else
    ret = 1;

  //updates are done only with the curr node write locked, so we dont have race using the
  //forward pointer?
  if(ret == 0 ){ //update logic
    curr->forward_pointer[0]->value = strdup(value); //FIXME change strdup
    RWLOCK_UNLOCK(&curr->rw_nodelock);
    return;
  } else{ //insert logic
    int new_node_lvl = random_level();
    struct skiplist_node* new_node = make_node(key, value, new_node_lvl);
    MUTEX_LOCK(&levels_lock_buf[new_node->level]);

    //we need to update the header correcly cause new_node_lvl > lvl
    for(i = lvl + 1; i <= new_node_lvl; i++)
      update_vector[i] = skplist->header;

    for(i = 0; i <= new_node->level; i++){
      //update_vector might be altered, find the correct rightmost node if it has changed
      if(i != 0){
        curr = getLock(update_vector[i], key, i); //we can change curr now cause level i-1 has
      }                                 //effectivly the new node and our job is done
      //linking logic
      new_node->forward_pointer[i] = curr->forward_pointer[i];
      curr->forward_pointer[i] = new_node;
      RWLOCK_UNLOCK(&curr->rw_nodelock);
    }

    MUTEX_UNLOCK(&levels_lock_buf[new_node->level]);
  }
}

//update_vector is an array of size MAX_LEVELS
static void delete_key(struct skiplist* skplist, struct skiplist_node** update_vector, struct skiplist_node* curr)
{
  int i;
  for(i = 0; i <= skplist->level; i++){
    if(update_vector[i]->forward_pointer[i] != curr)
      break; //modifications for upper levels don't apply (passed the level of the curr node)

    update_vector[i]->forward_pointer[i] = curr->forward_pointer[i];
  }
  free(curr); //FIXME we will use tombstones?

  //previous skplist->level dont have nodes anymore. header points to NIL, so reduce the level of the list
  while(skplist->level > 0 && skplist->header->forward_pointer[skplist->level]->is_NIL)
    --skplist->level;
}

void delete_skiplist(struct skiplist* skplist, char* key)
{
  int32_t i, key_size;
  int ret;
  struct skiplist_node* update_vector[MAX_LEVELS];
  struct skiplist_node* curr = skplist->header;

  key_size = strlen(key);

  for(i = skplist->level; i >= 0; i--){
    while(1){

      if(curr->forward_pointer[i]->is_NIL == 1)
        break; //reached sentinel

      ret = memcmp(curr->forward_pointer[i]->key, key, key_size);
      if(ret < 0 )
        curr = curr->forward_pointer[i];
      else
        break;
    }

    update_vector[i] = curr;
  }
  //retrieve it and check for existence
  curr = curr->forward_pointer[0];
  if(!curr->is_NIL)
    ret = memcmp(curr->key, key, key_size);
  else
    return; //Did not found the key to delete (reached sentinel)

  if(ret == 0) //found the key
    delete_key(skplist, update_vector, curr);

  /*FIXME else key doesn't exist in list so terminate. (should we warn the user?)*/
}
