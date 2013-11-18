#include "list.h"
#include "node.h"
#include <pthread.h>
#include <stdlib.h>

/**
 * Adds the given data to the list. This call will block until the data
 * structure is safe to modify.
 * On success, this returns 0. For any other error it returns 1.
 */
int list_add(list_t *list, void *data) {
    node_t *node;

    if (!list) {
        return 0;
    }

    node = node_create(data, NULL);
    if (!node) {
        return 0;
    }
    else {
        pthread_mutex_lock(list->mutex);
        node->next = list->head;
        list->head = node;
        pthread_mutex_unlock(list->mutex);
        return 1;
    }
}

/**
 * Creates a new list. Returns a pointer to the new list, or NULL if an error
 * occurs.
 */
list_t *list_create(void) {
    list_t *list = (list_t *) malloc(sizeof(list_t));
    if (list) {
        list->head = NULL;
        pthread_mutex_init(list->mutex);
    }
    return list;
}

/**
 * Destroys a list, freeing all associated memory. If the function pointer is
 * not NULL, it will be used to destroy the data contained in the list. Do not
 * destroy a list if threads are still waiting to modify it.
 */
void list_destroy(list_t *list, void (*destroy_func)(void *)) {
    node_t *node, *next;
    if (list) {
        node = list->head;
        while (node) {
            next = node->next;
            if (destroy_func) {
                destroy_func(node->data);
            }
            destroy_node(node);
            node = next;
        }
        pthread_mutex_destroy(list->mutex);
        free(list);
    }
}
