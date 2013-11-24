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
database_t *customerDatabase

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
    printf("./bookorder [arg1] [arg2] [arg3] \n"
           "[arg1] = the name of the database input file\n"
           "[arg2] = the name of the book order input file\n"
           "[arg3] = the list of category names\n");
}


/**
 * Code for the consumer threads. They take the orders and process them. The
 * argument to this function should be a null-terminated string representing the
 * category name for this thread.
 */
void *consumer_thread(void *args) {
    char *category;
    order_t *order;
    customer_t *customer;

    // Get the category for this thread.
    strcpy(category, (char *) args);

    while (is_done != 0) {
        // We wait until there is something in the queue
        pthread_mutex_lock(&queue->mutexqueue->mutex);
        pthread_cond_wait(&queue->nonempty, &queue->mutexqueue->mutex);

        if (is_done == 0) {
            // No more orders to process. Exit this thread.
            pthread_mutex_unlock(&queue->mutexqueue->mutex);
            return NULL;
        }
        else if (queue->last == NULL) {
            // The queue is empty again.
            pthread_mutex_unlock(&queue->mutexqueue->mutex);
            sched_yield();
        }
        else if (strcmp(queue_peek(queue)->category, category) != 0) {
            // This book is not in our category.
            pthread_mutex_unlock(&queue->mutexqueue->mutex);
            sched_yield();
        }
        else {
            // Process the order.
            // TODO
            order = (order_t *) queue_dequeue(queue);
            // TODO may have to be placed in book order struct to absolutely
            // guarantee proper order of execution
            customer = database_retrieve_customer(
                    customerDatabase,
                    order->customer_id);
        }
    }
    return NULL;
}

/**
 * Code for the producer threads. They open an input file and place orders into
 * the queue for processing.
 */
void *producer_thread(void *args) {
    // TODO
    return NULL;
}

/**
 * Sets up the customer database
 */
database_t *setup_database(char *filepath) {
    FILE *database; 
    char *entry, *lineptr, *name;
    float credit_limit;
    int customer_id, i;
    size_t len;
    ssize_t read;
    database_t *returnDatabase = database_create();
    customer_t *newCustomer = NULL;

    //make sure filepath is referencing a valid file
    if (is_file(filepath) == 0) {
        fprintf(stderr, "Error: %s is not a valid filepath\n");
        return returnDatabase;
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
    //There should be three arguments
    if (argc != 4) {
        fprintf(stderr, "Error: wrong number of arguments\n");
        print_usage();
        return;
    }

    // Set up customer database from file
    customerDatabase = setup_database(argv[1]);

    // Next, set up the queue and the condition variable
    queue = queue_create();

    // Spawn producer thread
    // TODO
}
