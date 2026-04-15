#include "list_lock.h"

#include <stdio.h>
#include <stdlib.h>

// typedef int DataType;

// typedef struct node {
//   DataType value;
//   struct node* next;
// } LNode, *LinkList;

// typedef struct {
//   LinkList head;
//   pthread_mutex_t mutex;
//   pthread_cond_t cond;
// } list_lock_t;

void listInit(list_lock_t* list) {
  //list->head = malloc(sizeof(LNode));
  //list->head->value = 0;
  list->head = NULL;
  pthread_mutex_init(&list->mutex, NULL);
  pthread_cond_init(&list->cond, NULL);
}

void producer(list_lock_t* list, DataType value) {
  pthread_mutex_lock(&list->mutex);
  LinkList temp = malloc(sizeof(LNode));
  temp->value = value;
  temp->next = list->head;
  list->head = temp;
  pthread_cond_signal(&list->cond);
  pthread_mutex_unlock(&list->mutex);
}

void consumer(list_lock_t* list) {
  pthread_mutex_lock(&list->mutex);
  while (list->head == NULL) {
    pthread_cond_wait(&list->cond, &list->mutex);
  }
  LinkList temp = list->head;
  list->head = temp->next;
  free(temp);
  pthread_mutex_unlock(&list->mutex);
}

int getListSize(list_lock_t* list) {
  pthread_mutex_lock(&list->mutex);
  int count = 0;
  LinkList p = list->head;
  while (p != NULL) {
    p = p->next;
    count++;
  }
  pthread_mutex_unlock(&list->mutex);
  return count;
}