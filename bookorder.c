#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"
#include "list.h"
#include "books.h"

/**
 * The database containing all customer information.
 */
database_t *customerDatabase;

/**
 * The megaqueue holding all the book orders to be processed.
 */
queue_t *queue;

/**
 * This value is zero if the consumer thread is still working and set to one
 * once it completes execution.
 */
int is_done;

/**
 * Tells us how many threads are active.
 */
int counter;

/**
 * Indicates whether or not all the threads are done or not.
 */
pthread_cond_t complete;

/**
 * Returns a positive number if the filename points to a readable File
 * Returns 0 otherwise
 */
int is_file(char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    else {
        return 0;
    }
}


/**
 * Prints appropriate usage of this application to stdout
 */
void print_usage() {
    printf("./bookorder <db> <orders> <catlist ...> \n"
           "\t[db] = the name of the database input file\n"
           "\t[orders] = the name of the book order input file\n"
           "\t[catlist] = a list of category names, separated by spaces\n");
}


/**
 * Code for the consumer threads. They take the orders and process them. The
 * argument to this function should be a null-terminated string representing the
 * category name for this thread.
 */
void *consumer_thread(void *args) {
    char *category, *input;
    customer_t *customer;
    order_t *order;
    receipt_t *receipt;

    // Get the category for this thread.
    input = (char *) args;
    category = (char *) malloc(strlen(input) + 1);
    strcpy(category, input);

    while (!is_done || !queue_isempty(queue)) {
        // We wait until there is something in the queue
        pthread_mutex_lock(&queue->mutex);
        pthread_cond_wait(&queue->nonempty, &queue->mutex);

        if (is_done && queue_isempty(queue)) {
            // No more orders to process. Exit this thread.
            pthread_mutex_unlock(&queue->mutex);
            return NULL;
        }
        else if (queue->last == NULL) {
            // The queue is empty again.
            pthread_mutex_unlock(&queue->mutex);
            sched_yield();
        }

        order = (order_t *) queue_peek(queue);
        if (strcmp(order->category, category) != 0) {
            // This book is not in our category.
            pthread_mutex_unlock(&queue->mutex);
            sched_yield();
        }
        else {
            // Process the order.
            order = (order_t *) queue_dequeue(queue);
            customer = database_retrieve_customer(customerDatabase,
                                                  order->customer_id);
            receipt = receipt_create(order->title, order->price,
                                     customer->credit_limit - order->price);
            if (customer->credit_limit < order->price) {
                // Insufficient funds.
                printf("Customer %s has insufficient funds to purchase '%s'."
                       "Remaining credit limit is %g.\n", customer->name,
                       order->title, customer->credit_limit);
                list_add(customer->failed_orders, receipt);
            }
            else {
                // Subtract price from remaining credit
                customer->credit_limit -= order->price;
                printf("Customer %s has successfully purchased '%s' for %g."
                       "Remaining credit limit is %g.\n", customer->name,
                       order->title, order->price, customer->credit_limit);
                list_add(customer->successful_orders, receipt);
            }
            order_destroy(order);
            pthread_mutex_unlock(&queue->mutex);
        }
    }
    return NULL;
}


/**
 * Code for the producer threads. They open an input file and place orders into
 * the queue for processing.
 */
void *producer_thread(void *args) {
    FILE *file;
    char *category, *lineptr, *title;
    float price;
    int customer_id;
    order_t *order;
    size_t len;
    ssize_t read;
    pthread_t tid;

    // Open the text file containing orders
    len = 0;
    lineptr = NULL;
    file = fopen((char *) args, "r");
    if (file == NULL) {
        fprintf(stderr, "An error occurred while opening the file.\n");
        exit(1);
    }

    // Begin parsing the order text file line by line
    while ((read = getline(&lineptr, &len, file)) != -1) {
        // Parse this line, getting relevant information
        title = strtok(lineptr, "|");
        price = atof(strtok(NULL, "|"));
        customer_id = atoi(strtok(NULL, "|"));
        category = strtok(NULL, "|");

        // Enqueue this order and alert the consumer threads
        order = order_create(title, price, customer_id, category);
        queue_enqueue(queue, order);
        pthread_cond_signal(&queue->nonempty);
    }

    // Finally, tell the consumers that we're done producing orders.
    is_done = 1;
    pthread_cond_broadcast(&queue->nonempty);
    free(lineptr);
    fclose(file);
    return NULL;
}


/**
 * Sets up the customer database
 */
database_t *setup_database(char *filepath) {
    FILE *database;
    char *entry, *lineptr, *name;
    float credit_limit;
    int customer_id;
    size_t len;
    ssize_t read;
    database_t *returnDatabase = database_create();
    customer_t *newCustomer = NULL;

    //make sure filepath is referencing a valid file
    if (is_file(filepath) == 0) {
        fprintf(stderr, "Error: %s is not a valid filepath\n", filepath);
        exit(1);
    }

    database = fopen(filepath, "r");

    lineptr = NULL;
    len = 0;

    while ((read = getline(&lineptr, &len, database)) != -1) {
        newCustomer = NULL;
        if((entry = strtok(lineptr, "|")) != NULL) {
            name = (char *) malloc(strlen(entry) + 1);
            strcpy(name, entry);
        }
        if ((entry = strtok(NULL, "|")) != NULL) {
            customer_id = atoi(entry);
        }
        if ((entry = strtok(NULL, "|")) != NULL) {
            credit_limit = atof(entry);
        }

        newCustomer = customer_create(name, customer_id, credit_limit);
        database_add_customer(returnDatabase, newCustomer);

    }
    free(lineptr);
    fclose(database);
    return returnDatabase;
}


int main(int argc, char **argv) {
    pthread_t tid;
    int i;

    // Check for the proper amount of arguments
    if (argc < 3) {
        fprintf(stderr, "Error: wrong number of arguments\n");
        print_usage();
        return 1;
    }
    else if (argc < 4) {
        fprintf(stderr, "Error: there must be at least one category.\n");
        return 1;
    }

    // Set up customer database from file
    customerDatabase = setup_database(argv[1]);

    // Next, set up the queue
    queue = queue_create();

    // This is the number of threads we'll be spawning
    counter = argc - 2;
    pthread_cond_init(&complete, NULL);

    // Spawn producer thread
    pthread_create(&tid, NULL, producer_thread, (void *) argv[2]);

    // Spawn all the consumer threads
    for (i = 3; i < argc; i++) {
        pthread_create(&tid, NULL, consumer_thread, (void *) argv[i]);
    }

    // Let the producers and consumers run
    pthread_cond_wait()
}
