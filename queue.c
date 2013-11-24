#include <stdlib.h>

#include "node.h"
#include <pthread.h>
#include "queue.h"

/**
 * Creates a new queue, initially empty.
 */
queue_t *queue_create(void) {
    queue_t *q = (queue_t *) malloc(sizeof(queue_t));
    if (q) {
        q->last = NULL;
        if (pthread_mutex_init(q->mutex, NULL) != 0) {
            free(q);
            q = NULL;
        }
    }
    return q;
}

/**
 * Enqueues the given data into the queue in a synchronized fashion. This call
 * will block until the data structure is safe to modify.
 */
void queue_enqueue(queue_t *queue, void *data) {
    node_t *node;
    if (!queue)
        return;

    node = create_node(data, NULL);
    pthread_mutex_lock(queue->mutex);
    if (queue->last == NULL) {
        queue->last = node;
    }
    else {
        node->next = queue->last->next;
        queue->last = node;
    }
    pthread_mutex_unlock(queue->mutex);
}

/**
 * Dequeues the data at the front of the queue, or NULL if there are no more
 * elements in the queue. This call will block until the data structure is safe
 * to modify.
 */
void *queue_dequeue(queue_t *queue) {
    void *data;
    if (!queue || !queue->last) {
        return NULL;
    }

    pthread_mutex_lock(queue->mutex);
    if (queue->last == queue->last->next) {
        // Only one item left in the queue
        data = queue->last->data;
        node_destroy(queue->last);
        queue->last = NULL;
    }
    else {
        // General case
        data = queue->last->next->data;
        queue->last->next = queue->last->next->next;
    }
    pthread_mutex_unlock(queue->mutex);
    return data;
}

/**
 * Destroys the queue, freeing all associated memory. Do not call this method if
 * threads are still waiting to perform enequeue or dequeue operations on the
 * specified queue.
 */
void queue_destroy(queue_t *queue) {
    node_t *node, *next;
    if (queue) {
        pthread_mutex_destroy(queue->mutex);
        node = queue->last;
        while(node) {
            next = node->next;
            node_destroy(node);
            node = next;
        }
        free(queue);
    }
}
