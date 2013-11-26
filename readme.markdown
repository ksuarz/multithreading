#Multithreaded Book Order System 

## Program overview
- Set up customer database
    - We read in the database.txt file and, for each line in the file, create a customer struct that corresponds to a new customer. We put all the customer struct objects into a database, represented by a hash table.

- Set up queue
    - We create a queue that will hold all the orders that need to be processed.  We reasoned that this would be simpler for us to architect, rather than having a queue for each category.

- Spawn producer thread
    - We then spawn a single producer thread, with the following function:
	    pthread_create(&tid, NULL, producer_thread, (void *) argv[2]);
    - The producer_thread function takes in the order text file and adds each item to the master queue.  The producer thread alerts all the consumer threads on enqueue.  Once the producer thread runs out of orders to add, it broadcasts this fact to the consumer threads.

- Spawn consumer threads
    - For each category inputted at call time, we create a new consumer thread using this function:

	    pthread_create(&tid[i + 1], NULL, consumer_thread, all_categories[i]);

    Each thread gets its corresponding category, acquires the mutex lock, and checks to see if the next item in the queue is of the same category.  If not, it gives up the lock and waits. If so, it processes the order and then relinquishes the lock.
    
The producer thread will wait for all of these threads to finish before continuing.

Once the threads are complete, we will print the final report by iterating over our global data structure containing all our customers and print out all pertintent information.

We clean up and exit.





