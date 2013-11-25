#Multithreaded Book Order System 

## Program overview
- Set up customer database
    - We read in the database.txt file and, for each line in the file, create a customer struct that corresponds to a new customer.  We put all the customer struct objects into a database, represented by a hash table.

- Set up queue
    - We create a queue that will hold all the orders that need to be processed.  We reasoned that this would be simpler for us to architect, rather than having a queue for each category.

- Spawn producer thread
    - We then spawn a single producer thread, as such:

	    pthread_create(&tid, NULL, producer_thread, (void *) argv[2]);




- Spawn consumer threads
- Let producer and consumer threads run

