#include "node.h"
#include <stdlib.h>

/**
 * Creates a new node with the given data. Returns a pointer to a new node, or
 * NULL if memory allocation fails.
 */
node_t *node_create(void *data, node_t *next) {
    node_t *node = (node_t *) malloc(sizeof(node_t));
    if (node) {
        node->data = data;
        node->next = next;
        node->references = 0;
        return node;
    }
    else
        return NULL;
}

/**
 * Destroys the node, freeing all associated memory.
 */
void node_destroy(node_t *node) {
    free(node);
}
