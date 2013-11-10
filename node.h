#ifndef NODE_H
#define NODE_H

/**
 * A standard singly-linked node that contains generic data.
 */
typedef struct node {
    void *data;
    struct node *next;
} node_t;

/**
 * Creates a new node with the given attributes.
 */
node_t *node_create(void *, node_t *);

/**
 * Destroys the given node.
 */
void node_destroy(node_t *);

#endif
