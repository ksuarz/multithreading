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
node_t *create_node(void *, node_t *);

/**
 * Destroys the node, freeing all associated memory.
 */
void destroy_node(node_t *);

#endif
