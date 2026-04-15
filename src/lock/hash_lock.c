#include "hash_lock.h"

#include <stdio.h>
#include <stdlib.h>

// #define HASHNUM 13
// #define HASH(key) key % HASHNUM

// typedef struct HashNode {
//   int value;
//   int key;
//   struct HashNode* next;
// } Hnode, *Hlist;

// struct HashTable {
//   pthread_mutex_t mutex;
//   Hlist head;
// };

// typedef struct {
//   struct HashTable table[HASHNUM];
// } hash_lock_t;

void hashInit(hash_lock_t* bucket) {
  for (int i = 0; i < HASHNUM; i++) {
    pthread_mutex_init(&bucket->table[i].mutex, NULL);
    // bucket->table[i].head = malloc(sizeof(Hnode));
    // bucket->table[i].head->value = -1;
    // bucket->table[i].head->key = -1;
    bucket->table[i].head = NULL;
  }
}

int getValue(hash_lock_t* bucket, int key) {
  int result = -1;
  pthread_mutex_lock(&bucket->table[hash(key)].mutex);
  Hlist now = bucket->table[hash(key)].head;
  while (now != NULL) {
    if (now->key == key) {
      result = now->value;
      break;
    }
    now = now->next;
  }
  pthread_mutex_unlock(&bucket->table[hash(key)].mutex);
  return result;
}

void insert(hash_lock_t* bucket, int key, int value) {
  pthread_mutex_lock(&bucket->table[hash(key)].mutex);
  Hlist now = bucket->table[hash(key)].head;
  while (now != NULL) {
    if (now->key == key) {
      now->value = value;
      pthread_mutex_unlock(&bucket->table[hash(key)].mutex);
      return;
    }
    now = now->next;
  }
  Hlist temp = malloc(sizeof(Hnode));
  temp->key = key;
  temp->value = value;
  temp->next = bucket->table[hash(key)].head;
  bucket->table[hash(key)].head = temp;
  pthread_mutex_unlock(&bucket->table[hash(key)].mutex);
}

int setKey(hash_lock_t* bucket, int key, int new_key) {
  if (key == new_key) {
    return 0;
  }
  int h1 = hash(key);
  int h2 = hash(new_key);
  if (h1 < h2) {
    pthread_mutex_lock(&bucket->table[h1].mutex);
    pthread_mutex_lock(&bucket->table[h2].mutex);
  } else if (h1 > h2) {
    pthread_mutex_lock(&bucket->table[h2].mutex);
    pthread_mutex_lock(&bucket->table[h1].mutex);
  } else {
    pthread_mutex_lock(&bucket->table[h1].mutex);
  }
  Hlist h1_prev = NULL;
  Hlist h1_curr = bucket->table[h1].head;
  Hlist target = NULL;
  while (h1_curr != NULL) {
    if (h1_curr->key == key) {
      target = h1_curr;
      break;
    }
    h1_prev = h1_curr;
    h1_curr = h1_curr->next;
  }
  if (target == NULL) {
    if (h1 != h2) {
      pthread_mutex_unlock(&bucket->table[h2].mutex);
    }
    pthread_mutex_unlock(&bucket->table[h1].mutex);
    return -1;
  }
  Hlist h2_prev = NULL;
  Hlist h2_curr = bucket->table[h2].head;
  while (h2_curr != NULL) {
    if (h2_curr->key == new_key) {
      break;
    }
    h2_prev = h2_curr;
    h2_curr = h2_curr->next;
  }
  if (h2_curr != NULL) {
    h2_curr->value = target->value;
    if (h1_prev == NULL) {
      bucket->table[h1].head = target->next;
    } else {
      h1_prev->next = target->next;
    }
    free(target);
    if (h1 != h2) {
      pthread_mutex_unlock(&bucket->table[h2].mutex);
    }
    pthread_mutex_unlock(&bucket->table[h1].mutex);
    return 0;
  }
  if (h1_prev == NULL) {
    bucket->table[h1].head = target->next;
  } else {
    h1_prev->next = target->next;
  }
  if (h1 == h2) {
    target->next = bucket->table[h1].head;
    bucket->table[h1].head = target;
  } else {
    target->next = bucket->table[h2].head;
    bucket->table[h2].head = target;
  }
  target->key = new_key;
  if (h1 != h2) {
    pthread_mutex_unlock(&bucket->table[h2].mutex);
  }
  pthread_mutex_unlock(&bucket->table[h1].mutex);
  return 0;
}