#include <stdlib.h>

#include "node.h"
#include "queue.h"
#include <pthread.h>

/**
 * Creates a new queue, initially empty.
 */
queue_t *queue_create(void) {
    queue_t *q = (queue_t *) malloc(sizeof(queue_t));
    if (q) {
        q->last = NULL;
        if (pthread_mutex_init(&q->mutex, NULL) != 0) {
            free(q);
            q = NULL;
        }
        else if (pthread_cond_init(&q->nonempty, NULL) != 0) {
            pthread_mutex_destroy(&q->mutex);
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
    pthread_mutex_lock(&queue->mutex);
    if (queue->last == NULL) {
        queue->last = node;
        queue->last->next = queue->last;
    }
    else {
        node->next = queue->last->next;
        queue->last = node;
    }
    pthread_mutex_unlock(&queue->mutex);
}

/**
 * Dequeues the data at the front of the queue, or NULL if there are no more
 * elements in the queue. This call will block until the data structure is safe
 * to modify.
 */
void *queue_dequeue(queue_t *queue) {
    void *data;
    if (queue == NULL || queue->last == NULL) {
        return NULL;
    }

    pthread_mutex_lock(&queue->mutex);
    if (queue->last == queue->last->next) {
        // Only one item left in the queue
        data = queue->last->data;
        destroy_node(queue->last);
        queue->last = NULL;
    }
    else {
        // General case
        data = queue->last->next->data;
        queue->last->next = queue->last->next->next;
    }
    pthread_mutex_unlock(&queue->mutex);
    return data;
}

/**
 * Destroys the queue, freeing all associated memory. Do not call this method if
 * threads are still waiting to perform enequeue or dequeue operations on the
 * specified queue.
 */
void queue_destroy(queue_t *queue) {
    // TODO must destroy order structs inside the queue
    node_t *node, *next;
    if (queue) {
        pthread_mutex_destroy(&queue->mutex);
        pthread_cond_destroy(&queue->nonempty);

        if (queue->last = queue->last->next) {
            // Special case: size one queue
            destroy_node(queue->last);
        }
        else {
            // Walk the linked list
            node = queue->last;
            while(node) {
                next = node->next;
                destroy_node(node);
                node = next;
            }
        }
        free(queue);
    }
}

/**
 * Peeks at the top of the queue without dequeueing it. If there is nothing in
 * the queue, this returns NULL.
 */
const void *queue_peek(queue_t *queue) {
    if (queue && queue->last && queue->last->next) {
        return queue->last->next->data;
    }
    else {
        return NULL;
    }
}
