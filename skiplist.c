#include "skiplist.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

uint32_t p = 1 / 4;

static uint32_t random_level() {
  uint32_t i;
  for (i = 0; i < MAX_LEVELS && rand() % 2 == 0; i++)
    ;

  return i;
}

static struct skiplist_node *make_node(char *key, char *value) {
  struct skiplist_node *new_node =
      (struct skiplist_node *)malloc(sizeof(struct skiplist_node));
  new_node->key = strdup(key);
  new_node->value = strdup(value);
  new_node->is_NIL = 0;

  return new_node;
}

// skplist is an object called by reference
void init_skiplist(struct skiplist *skplist) {
  int i;
  // allocate NIL (sentinel)
  skplist->NIL_element =
      (struct skiplist_node *)calloc(1, sizeof(struct skiplist_node));
  skplist->NIL_element->is_NIL = 1;

  // level is 0
  skplist->level = 0;

  skplist->header =
      (struct skiplist_node *)malloc(sizeof(struct skiplist_node));
  skplist->header->is_NIL = 0;
  // all forward pointers of header point to NIL
  for (i = 0; i < MAX_LEVELS; i++)
    skplist->header->forward_pointer[i] = skplist->NIL_element;

  // srand(time(NULL));
}

char *search_skiplist(struct skiplist *skplist, char *search_key) {
  int i, ret;
  struct skiplist_node *curr = skplist->header;

  for (i = skplist->level; i >= 0; i--) {
    while (!curr->forward_pointer[i]->is_NIL &&
           memcmp(curr->forward_pointer[i]->key, search_key,
                  strlen(search_key)) < 0)
      curr = curr->forward_pointer[i];
  }

  if (curr->forward_pointer[0]
          ->is_NIL) // next element for level 0 is sentinel, key not found
    return NULL;

  // we are infront of the key if exists
  // get next node and check for existence
  curr = curr->forward_pointer[0];
  ret = memcmp(curr->key, search_key, strlen(search_key));
  if (ret == 0)
    return curr->value;
  else
    return NULL;
}

void insert_skiplist(struct skiplist *skplist, char *key, char *value) {
  int i, ret, lvl;
  struct skiplist_node *update_vector[MAX_LEVELS];
  struct skiplist_node *curr = skplist->header;

  // init update_vector
  // keeps the leftmost node of each level that has to be modified after the
  // insertion
  for (i = 0; i < MAX_LEVELS; i++)
    update_vector[i] = skplist->header;

  for (i = skplist->level; i >= 0; i--) {
    while (1) {
      // reached sentinel
      if (curr->forward_pointer[i]->is_NIL)
        break;

      ret = memcmp(curr->forward_pointer[i]->key, key, strlen(key));
      if (ret < 0)
          curr = curr->forward_pointer[i];
      else
          break;
    }
    update_vector[i] = curr;
  }

  // we are infront of the wanted key if exist
  // retrieve if and check for existence
  curr = curr->forward_pointer[0];
  if (!curr->is_NIL)
    ret = memcmp(curr->key, key, strlen(key));
  else
    ret = 1; // curr node is the sentinel node, key always wins

  if (ret == 0) {
    curr->value = strdup(value);
    return;
  } else {
    lvl = 0;//random_level();
    if (lvl > skplist->level) {
      for (i = skplist->level + 1; i < lvl; i++)
        update_vector[i] =
            skplist->header
                ->forward_pointer[i]; // header should point to sentinel?
      skplist->level = lvl;
    }

    struct skiplist_node *new_node = make_node(key, value);

    for (i = 0; i <= lvl; i++) {
      new_node->forward_pointer[i] = update_vector[i]->forward_pointer[i];
      update_vector[i]->forward_pointer[i] = new_node;
    }
  }
}
