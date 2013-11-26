#ifndef BOOKS_H
#define BOOKS_H

#define MAXCUSTOMERS 512

#include "queue.h"

/**
 * A structure holding the information for book orders.
 */
typedef struct order {
    char *title;
    float price;
    int customer_id;
    char *category;
} order_t;

/**
 * Creates a new book order structure.
 */
order_t *order_create(char *, float, int, char *);

/**
 * Destroys a book order structure, freeing all associated memory.
 */
void order_destroy(order_t *);

/**
 * A structure that carries data for the customer after every purchace.
 * successful orders use all three structure fields; unsuccessful book orders
 * only use the first two.
 */
typedef struct receipt {
    char *title;
    float price;
    float remaining_credit;
} receipt_t;

/**
 * Creates a new order receipt.
 */
receipt_t *receipt_create(char *, float, float);

/**
 * Destroys a receipt structure.
 */
void receipt_destroy(void *);

/**
 * Structure holding all customer information.
 */
typedef struct customer {
    char *name;
    int customer_id;
    float credit_limit;
    queue_t *successful_orders;
    queue_t *failed_orders;
} customer_t;

/**
 * Creates a new customer for the database.
 */
customer_t *customer_create(char *, int, float);

/**
 * Destroys the customer, freeing all data.
 */
void customer_destroy(customer_t *);

/**
 * A hash map containing all customers and their data.
 */
typedef struct database {
    customer_t *customer[MAXCUSTOMERS];
} database_t;

/**
 * Creates a new empty database.
 */
database_t *database_create(void);

/**
 * Destroys the given database, freeing all associated memory.
 */
void database_destroy(database_t *);

/**
 * Adds a new customer to the database.
 */
void database_add_customer(database_t *, customer_t *);

/**
 * Retrieves a customer from the database.
 */
customer_t *database_retrieve_customer(database_t *, int);

#endif
