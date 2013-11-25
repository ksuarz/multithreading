#ifndef NODE_H
#define NODE_H

/*
 * Type for a basic linked list node.
 */
struct node {
    void *data;
    struct node *next;
    int references;
};

typedef struct node node_t;

/**
 * Creates a new node with the given data.
 */
node_t *node_create(void *, node_t *);

/**
 * Destroys the node, freeing all associated memory.
 */
void node_destroy(node_t *);

#endif
