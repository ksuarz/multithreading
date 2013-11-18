#ifndef LIST_H
#define LIST_H

#include "node.h"
#include <pthread.h>

/**
 * Structure definition for a synchronized list.
 */
typedef struct list {
    node_t *head;
    pthread_mutex_t *mutex;
} list_t;

/**
 * Adds the given data to the list. This call will block until the data
 * structure is safe to modify.
 */
int list_add(list_t *, void *);

/**
 * Creates a new list.
 */
list_t *list_create(void);

/**
 * Destroys a list, freeing all associated memory. Do not destroy a list if
 * threads are still waiting to modify it.
 */
void list_destroy(list_t *, void (*)(void *));

#endif
