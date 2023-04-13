#include "linked_list.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Node *new_node(void *value) {
  Node *node = malloc(sizeof(Node));
  if (node == NULL) {
    printf("ERROR: could not allocate new node: %s.\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  node->value = value;
  node->next = NULL;

  return node;
}

LinkedList *new_list(void) {
  LinkedList *list = malloc(sizeof(LinkedList));
  if (list == NULL) {
    printf("ERROR: could not allocate new linked list: %s.\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  list->head = NULL;
  list->tail = NULL;
  list->size = 0;

  return list;
}

void append(LinkedList *list, void *value) {
  Node *new_value = new_node(value);

  if (list->tail == NULL) { // First element inserted
    list->tail = new_value;
    list->head = new_value;
  } else {
    list->head->next = new_value;
    list->head = new_value;
  }

  size_t cur_size = list->size;
  list->size = cur_size + 1;
}

void free_list(LinkedList *list) {
  if (list->tail == NULL) {
    free(list);
    return;
  }

  Node *cur_node = list->tail;
  while (cur_node != NULL) {
    Node *tmp = cur_node->next;
    cur_node->next = NULL;
    // This probably leaks memory, if cur_node->value itself has pointers,
    // they would not be freed
    // TODO: receive free_value function pointer and execute for each node value
    free(cur_node);

    cur_node = tmp;
  }
}
