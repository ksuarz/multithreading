#include "node.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Creates a new node with the given attributes. Returns a pointer to the new
 * node, or NULL if memory allocation fails.
 */
node_t *node_create(void *data, node_t *next) {
    node_t *node = (node_t *) malloc(sizeof(node_t));
    if (node) {
        node->data = data;
        node->next = next;
    }
    return node;
}

/**
 * Destroys the given node. Note that this method does not free the data it
 * contains.
 */
void node_destroy(node_t *node) {
    if (node) {
        free(node);
    }
}
