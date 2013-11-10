#ifndef QUEUE_H
#define QUEUE_H

#include "node.h"
#include <pthread.h>

/**
 * Type for a synchronized queue. This is implemented as a linked list and
 * contains mutexes to implement synchronized enqueue and dequeue methods.
 */
typedef struct queue {
    node_t *head;
    pthread_mutex_t *mutex;
} queue_t;

/**
 * Creates a new queue, initially empty.
 */
queue_t *queue_create(void);

/**
 * Enqueues the given data into the queue in a synchronized fashion. This call
 * will block until the data structure is safe to modify.
 */
void queue_enqueue(queue_t *, void *);

/**
 * Dequeues the data at the front of the queue, or NULL if there are no more
 * elements in the queue. This call will block until the data structure is safe
 * to modify.
 */
void *queue_dequeue(queue_t *);

/**
 * Destroys the queue, freeing all associated memory. Do not call this method if
 * threads are still waiting to perform enequeue or dequeue operations on the
 * specified queue.
 */
void queue_destroy(queue_t *);

#endif
