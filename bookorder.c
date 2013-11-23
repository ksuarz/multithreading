#include <stdio.h>
#include <stdlib.h>

/**
 * The queues where book orders are placed for processing. Each thread should be
 * given an ID, either 0, 1, or 2, corresponding to the three queues in this
 * array.
 */
queue_t *queues[3];

/**
 * The integers in this array correspond to the queues in the other array. The
 * value busy[i] is 1 if producer thread i is still not done parsing book
 * orders. Once all book orders are processed, busy[i] should be set to zero.
 * Then, consumer thread i finishes if queues[i] is empty and busy[i] is zero.
 */
int busy[3];

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

int main(int argc, char **argv) {

}
