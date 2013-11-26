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
    printf("./bookorder <db> <orders> <catlist> \n"
           "\t[db] = the name of the database input file\n"
           "\t[orders] = the name of the book order input file\n"
           "\t[catlist] = a quoted list of category names, separated by spaces\n");
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

    printf("Consumer thread for category %s created!\n", category);

    while (!is_done || !queue_isempty(queue)) {
        // We wait until there is something in the queue
	printf("Consumer for %s about to lock mutex\n", category);
        pthread_mutex_lock(&queue->mutex);

        if (!is_done && queue_isempty(queue)) {
            printf("Consumer for %s waits for queue to be populated.\n", category);
            pthread_cond_wait(&queue->nonempty, &queue->mutex);
            printf("Consumer for %s has awoken from waiting!\n", category);
        }

        printf("Consumer for %s is examining the queue.\n", category);
        if (is_done && queue_isempty(queue)) {
            printf("Consumer for %s detects completion and is exiting.\n", category);
            // No more orders to process. Exit this thread.
            pthread_mutex_unlock(&queue->mutex);
            return NULL;
        }
        else if (queue_isempty(queue)) {
            printf("Consumer for %s detects queue is empty again. Unlocking mutex and yielding.\n", category);
            // The queue is empty again.
            pthread_mutex_unlock(&queue->mutex);
            sched_yield();
            continue;
        }

        order = (order_t *) queue_peek(queue);
        if (strcmp(order->category, category) != 0) {
            // This book is not in our category.
            printf("Consumer for %s detects next book %s of category %s is not our category. Unlocking mutex and yielding.\n", category, order->title, order->category);
            pthread_mutex_unlock(&queue->mutex);
            sched_yield();
        }
        else {
            printf("Consumer for %s is now processing an order!\n", category);
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
            printf("Consumer for %s is done processing. Unlocking mutex.\n", category);
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
    const char *delims = "|\r\n";
    float price;
    int customer_id;
    order_t *order;
    size_t len;
    ssize_t read;

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
        printf("Producer is parsing the next line...\n");
        // Parse this line, getting relevant information
        title = strtok(lineptr, delims);
        price = atof(strtok(NULL, delims));
        customer_id = atoi(strtok(NULL, delims));
        category = strtok(NULL, delims);

        // Enqueue this order and alert the consumer threads
        order = order_create(title, price, customer_id, category);
        if (order == NULL) {
            fprintf(stderr, "Consumer attempted to make an order but the call failed.\n");
        }
	//printf("order->title = %s\n", order->title);	
        printf("Producer is calling to enqueue %s, trying to obtain the mutex.\n", title);
        queue_enqueue(queue, (void *) order);
        printf("Producer enqueue complete. Unlocking the mutex.\n");
        printf("Producer is signalling the other threads now.\n");
        pthread_cond_signal(&queue->nonempty);
    }

    printf("Producer is complete. Dying and broadcasting.\n");
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
    char *category, **all_categories;
    customer_t *customer;
    float revenue;
    int i, num_categories;
    list_t *receipt_list;
    node_t *receipt_node;
    receipt_t *receipt;
    void *ignore;

    // Check for the proper amount of arguments
    if (argc != 4) {
        fprintf(stderr, "Error: wrong number of arguments\n");
        print_usage();
        return 1;
    }

    // Figure out how many categories there are
    // TODO can be cleaned up
    all_categories = (char **) calloc(1024, sizeof(char *));
    num_categories = 0;
    category = strtok(argv[3], " ");
    if (category == NULL) {
        fprintf(stderr, "Error: Must specify at least one category.\n");
        exit(1);
    }
    all_categories[0] = malloc(strlen(category) + 1);
    strcpy(all_categories[0], category);
    num_categories++;

    while ((category = strtok(NULL, " ")) != NULL) {
        all_categories[num_categories] = malloc(strlen(category) + 1);
        strcpy(all_categories[num_categories], category);
        num_categories++;
    }

    // Holds all the thread ids spawned later
    pthread_t tid[num_categories + 1];
    printf("created pthread arr\n");

    // Set up customer database from file
    customerDatabase = setup_database(argv[1]);
    printf("set up database\n");

    // Next, set up the queue
    queue = queue_create();
    printf("queue create\n");

    // Spawn producer thread
    pthread_create(&tid[0], NULL, producer_thread, (void *) argv[2]);
    printf("spawn \n");

    // Spawn all the consumer threads
    for (i = 0; i < num_categories; i++) {
        pthread_create(&tid[i + 1], NULL, consumer_thread, all_categories[i]);
    }
    printf("spawn threads\n");

    // Wait for all the other threads to finish before continuing
    printf("begin wait\n");
    for (i = 0; i < num_categories + 1; i++) {
        pthread_join(tid[i], &ignore);
	printf("continue wait\n");
    }
    printf("end wait\n");

    // Now we can print our final report
    revenue = 0.0f;
    for (i = 0; i < MAXCUSTOMERS; i++) {
        customer = customerDatabase->customer[i];
        if (customer == NULL) {
            continue;
        }

        // Print out this customer's data
        printf("%s [ID: %d]\n", customer->name, customer->customer_id);
        printf("Remaining credit: %g\n", customer->credit_limit);

        // Successful book orders
        printf("Successful orders:\n");
        if ((receipt_list = customer->successful_orders) == NULL ||
             receipt_list->head == NULL) {
            printf("\tNone.\n");
        }
        else {
            receipt_node = receipt_list->head;
            while (receipt_node != NULL) {
                receipt = (receipt_t *) receipt_node->data;
                printf("\tBook: %s\n", receipt->title);
                printf("\tPrice: %g\n", receipt->price);
                printf("\tCredit remaining: %g\n\n",
                        receipt->remaining_credit);
                revenue += receipt->price;
                receipt_node = receipt_node->next;
            }
        }

        // Failed book orders
        printf("\nFailed orders:\n");
        if ((receipt_list = customer->failed_orders) == NULL ||
             receipt_list->head == NULL) {
            printf("\tNone.\n");
        }
        else {
            receipt_node = receipt_list->head;
            while (receipt_node != NULL) {
                receipt = (receipt_t *) receipt_node->data;
                printf("\tBook: %s\n", receipt->title);
                printf("\tPrice: %g\n\n", receipt->price);
            }
        }
        printf("\n");
    }

    printf("Total Revenue: %g\n", revenue);

    // Free all the memory we allocated
    database_destroy(customerDatabase);
    queue_destroy(queue, NULL);
    return 0;
}
