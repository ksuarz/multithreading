#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"
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
 * All of the input categories, but formatted as one string.
 */
char *input_categories;

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
 * Prints appropriate usage of this application to standard out.
 */
void print_usage() {
    printf("./bookorder <db> <orders> <cats> \n"
           "\t<db> = the name of the database input file\n"
           "\t<orders> = the name of the book order input file\n"
           "\t<cats> = a quoted list of category names, separated by spaces\n");
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

        if (!is_done && queue_isempty(queue)) {
            pthread_cond_wait(&queue->nonempty, &queue->mutex);
        }

        if (is_done && queue_isempty(queue)) {
            // No more orders to process. Exit this thread.
            pthread_mutex_unlock(&queue->mutex);
            return NULL;
        }
        else if (queue_isempty(queue)) {
            // The queue is empty again.
            pthread_mutex_unlock(&queue->mutex);
            sched_yield();
            continue;
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
            if (!customer) {
                // Invalid customer ID
                fprintf(stderr, "There is no customer in the database with"
                        "customer ID %d.\n", order->customer_id);
            }
            else {
                receipt = receipt_create(order->title,
                                         order->price,
                                         customer->credit_limit - order->price);
                if (customer->credit_limit < order->price) {
                    // Insufficient funds.
                    printf("%s has insufficient funds for a purchase.\n"
                           "\tBook: %s\n\tRemaining credit: $%.2f\n\n",
                           customer->name,
                           order->title,
                           customer->credit_limit);
                    queue_enqueue(customer->failed_orders, receipt);
                }
                else {
                    // Subtract price from remaining credit
                    customer->credit_limit -= order->price;
                    printf("Customer %s has made a successful purchase!\n"
                           "\tBook: %s\n\tPrice: $%.2f\n"
                           "\tRemaining credit: $%.2f\n\n",
                           customer->name,
                           order->title,
                           order->price,
                           customer->credit_limit);
                    queue_enqueue(customer->successful_orders, receipt);
                }
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
        // Parse this line, getting relevant information
        title = strtok(lineptr, delims);
        price = atof(strtok(NULL, delims));
        customer_id = atoi(strtok(NULL, delims));
        category = strtok(NULL, delims);

        if (strstr(input_categories, category) == NULL) {
            fprintf(stderr, "The category %s is not a valid category as "
                    "specified in the input. This order will be skipped.\n",
                    category);
            continue;
        }

        // Enqueue this order in a thread-safe manner
        order = order_create(title, price, customer_id, category);
        pthread_mutex_lock(&queue->mutex);
        queue_enqueue(queue, (void *) order);
        pthread_mutex_unlock(&queue->mutex);

        // Alert the consumers
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


/**
 * Runs the program.
 */
int main(int argc, char **argv) {
    char *category, **all_categories;
    customer_t *customer;
    float revenue;
    int i, num_categories;
    receipt_t *receipt;
    void *ignore;

    // Check for the proper amount of arguments
    if (argc != 4) {
        fprintf(stderr, "Error: wrong number of arguments\n");
        print_usage();
        return 1;
    }

    // Figure out how many categories there are
    input_categories = malloc(strlen(argv[3]) + 1);
    strcpy(input_categories, argv[3]);
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

    // Set up customer database from file and our queue
    customerDatabase = setup_database(argv[1]);
    queue = queue_create();

    // Spawn producer thread
    pthread_create(&tid[0], NULL, producer_thread, (void *) argv[2]);

    // Spawn all the consumer threads
    for (i = 0; i < num_categories; i++) {
        pthread_create(&tid[i + 1], NULL, consumer_thread, all_categories[i]);
    }

    // Wait for all the other threads to finish before continuing
    for (i = 0; i < num_categories + 1; i++) {
        pthread_join(tid[i], &ignore);
    }

    // Now we can print our final report
    printf("\n\n");
    revenue = 0.0f;
    for (i = 0; i < MAXCUSTOMERS; i++) {
        customer = customerDatabase->customer[i];
        if (customer == NULL) {
            continue;
        }

        // Print out this customer's data
        printf("=== Customer Info ===\n");
        printf("--- Balance ---\n");
        printf("Customer name: %s\n", customer->name);
        printf("Customer ID number: %d\n", customer->customer_id);
        printf("Remaining credit: %.2f\n", customer->credit_limit);

        // Successful book orders
        printf("\n--- Successful orders ---\n");
        if (queue_isempty(customer->successful_orders)) {
                printf("\tNone.\n");
        }
        else {
            while ((receipt = (receipt_t *)
                        queue_dequeue(customer->successful_orders)) != NULL) {
                printf("%s|%.2f|%.2f\n", receipt->title, receipt->price,
                                           receipt->remaining_credit);
                revenue += receipt->price;
                receipt_destroy(receipt);
            }
        }

        // Failed book orders
        printf("\n--- Failed orders ---\n");
        if (queue_isempty(customer->failed_orders)) {
            printf("None.\n");
        }
        else {
            while ((receipt = (receipt_t *)
                        queue_dequeue(customer->failed_orders)) != NULL) {
                printf("%s|%.2f\n", receipt->title, receipt->price);
                receipt_destroy(receipt);
            }
        }
        printf("=== End Customer Info ===\n\n");
    }

    printf("Total Revenue: $%.2f\n", revenue);

    // Free all the memory we allocated
    database_destroy(customerDatabase);
    queue_destroy(queue, NULL);
    return 0;
}
