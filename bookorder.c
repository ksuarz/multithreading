#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"
#include "list.h"
#include "books.h"

static queue_t *queues[3];

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
    printf("./bookorder [arg1] [arg2] [arg3] \n[arg1] = the name of the database input file\n[arg2] = the name of the book order input file\n[arg3] = the list of category names\n");
    return;
}


/**
 * Code for the consumer threads. They take the orders and process them.
 */
void *consumer_thread(void *args) {
    // TODO
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
    int customer_id, i;
    float credit_limit;
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
        for (i=0; i<3; i++) {
	     
	    if (i == 0) {
		if((entry = strtok(lineptr, "|")) != NULL) {
		    name = (char *) malloc(strlen(entry) + 1);
		    strcpy(name, entry);
		}
	    }
	    else if (i== 1) {
		if ((entry = strtok(NULL, "|")) != NULL) {
		    customer_id = atoi(entry); 
		}
	    }
	    else if (i == 2) {
		if ((entry = strtok(NULL, "|")) != NULL) {
		    credit_limit = atof(entry);
		}
	    }
	    else {
		break;
	    }
        }
        
        newCustomer = customer_create(name, customer_id, credit_limit);
        database_add_customer(returnDatabase, newCustomer);

    }
    free(newCustomer);
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

    //Set up customer database from file
    database_t *customerDatabase = setup_database(argv[1]);

    //spawn producer thread
    
    //spawn consumer thread

}
