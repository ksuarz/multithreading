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
    node_t *node, *ptr;
    if (!queue)
        return;

    node = node_create(data, NULL);
    pthread_mutex_lock(&queue->mutex);
    if (queue->last == NULL) {
        // Queue is empty
        queue->last = node;
        queue->last->next = queue->last;
    }
    else {
        //indraneel version
        for (ptr = queue->last->next; ptr != queue->last; ptr = ptr->next);
        node->next = queue->last;
        ptr->next = node;
        queue->last = node;

        // General case (a linear time algorithm)
        // `node` will point to the node whose next is last
        /*
        for (ptr = queue->last; ptr->next != queue->last; ptr = ptr->next);
        node->next = queue->last->next;
        queue->last = node;
        ptr->next = queue->last;
        */
    }
    pthread_mutex_unlock(&queue->mutex);
}

/**
 * Dequeues the data at the front of the queue, or NULL if there are no more
 * elements in the queue. This is a non-blocking function.
 */
void *queue_dequeue(queue_t *queue) {
    void *data;
    node_t *to_destroy;

    if (queue == NULL || queue->last == NULL) {
        return NULL;
    }

    if (!queue->last->next || queue->last == queue->last->next) {
        // Only one item left in the queue
        data = queue->last->data;
        node_destroy(queue->last);
        queue->last = NULL;
    }
    else {
        // General case
        // TODO this leaks the memory in the order
        to_destroy = queue->last->next;
        data = queue->last->next->data;
        queue->last->next = queue->last->next->next;
        node_destroy(to_destroy);
    }
    return data;
}

/**
 * Destroys the queue, freeing all associated memory. Do not call this method if
 * threads are still waiting to perform enequeue or dequeue operations on the
 * specified queue.
 */
void queue_destroy(queue_t *queue, void (*destroy_func)(void *)) {
    node_t *node, *next;
    if (queue) {
        pthread_mutex_destroy(&queue->mutex);
        pthread_cond_destroy(&queue->nonempty);

        if (queue->last == queue->last->next) {
            // Special case: size one queue
            node_destroy(queue->last);
        }
        else {
            // Walk the linked list
            node = queue->last;
            while(node) {
                next = node->next;
                if (destroy_func) {
                    destroy_func(node->data);
                }
                node_destroy(node);
                node = next;
            }
        }
        free(queue);
    }
}

/**
 * Returns true if the queue is NULL or is empty and false otherwise.
 */
int queue_isempty(queue_t *queue) {
    return queue == NULL || queue->last == NULL;
}

/**
 * Peeks at the top of the queue without dequeueing it. If there is nothing in
 * the queue, this returns NULL. This is a non-blocking function.
 */
const void *queue_peek(queue_t *queue) {
    if (queue && queue->last && queue->last->next) {
        return queue->last->next->data;
    }
    else {
        return NULL;
    }
}
