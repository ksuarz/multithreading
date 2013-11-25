#ifndef LIST_H
#define LIST_H

#include "node.h"

/**
 * Structure definition for a generic linked list.
 */
typedef struct list {
    node_t *head;
} list_t;

/**
 * Adds the given data to the list. 
 */
int list_add(list_t *, void *);

/**
 * Creates a new list.
 */
list_t *list_create(void);

/**
 * Destroys a list, freeing all associated memory. 
 */
void list_destroy(list_t *, void (*)(void *));

#endif
