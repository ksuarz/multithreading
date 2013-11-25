#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "../node.h"
#include "../queue.h"

queue_t *queue;


void single_thread_test() {
    char *itemOne = "single-thread-item-1";
    char *itemTwo = "single-thread-item-2";
    char *dequeued_item = NULL;
    queue_t *queue = queue_create();
    queue_enqueue(queue, itemOne);
    queue_enqueue(queue, itemTwo);
    dequeued_item = queue_dequeue(queue);
    printf("dequeued_item = %s\n", dequeued_item);
    /*
    dequeued_item = queue_dequeue(queue);
    printf("dequeued_item = %s\n", dequeued_item);
    */
    printf("pointer to queue before destroy = %p\n",queue);
    queue_destroy(queue);
    printf("pointer to queue after destroy = %p\n",queue);
}

void *thread_code(void *args) {
   
    queue_enqueue(queue, args);

}

void multi_thread_test() {
    queue = queue_create();
    int i;
    pthread_t my_thread;
    for (i=0; i<10; i++) {
	int *j = (int *) malloc(sizeof(int));
	*j = i;
	pthread_create(&my_thread, NULL, &thread_code, j);
    }
    pthread_exit(0);
}



int main (int argc, char **argv) {
    single_thread_test();
}
