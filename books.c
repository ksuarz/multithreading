#include "books.h"
#include "queue.h"
#include "node.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Creates a new book order structure.
 */
order_t *order_create(char *title, float price, int cust_id, char *category) {
    order_t *order = (order_t *) malloc(sizeof(order_t));
    if (order) {
        order->customer_id = cust_id;
        order->price = price;
        order->title = (char *) malloc(strlen(title) + 1);
        order->category = (char *) malloc(strlen(category) + 1);
        strcpy(order->title, title);
        strcpy(order->category, category);
    }
    return order;
}

/**
 * Destroys a book order structure, freeing all associated memory.
 */
void order_destroy(order_t *order) {
    if (order) {
        free(order->title);
        free(order->category);
        free(order);
    }
}

/**
 * Creates a new order receipt. Returns a pointer to the new structure, or NULL
 * if allocation fails.
 */
receipt_t *receipt_create(char *title, float price, float remaining_credit) {
    receipt_t *receipt = (receipt_t *) malloc(sizeof(receipt_t));
    if (receipt) {
        receipt->price = price;
        receipt->remaining_credit = remaining_credit;
        receipt->title = (char *) malloc(strlen(title) + 1);
        strcpy(receipt->title, title);
    }
    return receipt;
}

/**
 * Destroys a receipt structure.
 */
void receipt_destroy(void *data) {
    receipt_t *receipt;
    if (data) {
        receipt = (receipt_t *) data;
        free(receipt->title);
        free(receipt);
    }
}

/**
 * Creates a new customer for the database.
 */
customer_t *customer_create(char *name, int customer_id, float credit_limit) {
    customer_t *customer = (customer_t *) malloc(sizeof(customer_t));
    if (customer) {
        customer->customer_id = customer_id;
        customer->credit_limit = credit_limit;
        customer->name = (char *) malloc(strlen(name) + 1);
        strcpy(customer->name, name);
        customer->successful_orders = queue_create();
        customer->failed_orders = queue_create();
    }
    return customer;
}

/**
 * Destroys the customer, freeing all data.
 */
void customer_destroy(customer_t *customer) {
    if (customer) {
        free(customer->name);
        queue_destroy(customer->successful_orders, &receipt_destroy);
        queue_destroy(customer->failed_orders, &receipt_destroy);
        free(customer);
    }
}

/**
 * Creates a new empty database.
 */
database_t *database_create(void) {
    return (database_t *) malloc(sizeof(database_t));
}

/**
 * Destroys the given database, freeing all associated memory.
 */
void database_destroy(database_t *database) {
    int i;
    if (database) {
        for (i = 0; i < MAXCUSTOMERS; i++) {
            customer_destroy(database->customer[i]);
        }
        free(database);
    }
}

/**
 * Adds a new customer to the database.
 */
void database_add_customer(database_t *database, customer_t *customer) {
    database->customer[customer->customer_id] = customer;
}

/**
 * Retrieves a customer from the database.
 */
customer_t *database_retrieve_customer(database_t *database, int customer_id) {
    if (customer_id >= 0 || customer_id < MAXCUSTOMERS) {
        return database->customer[customer_id];
    }
    else {
        fprintf(stderr, "Customer id %d out of bounds of the database.\n", customer_id);
        return NULL;
    }
}
