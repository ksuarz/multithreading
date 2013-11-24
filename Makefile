CC = gcc
"CFLAGS = -Wall -g"
CFLAGS = -g

all: 

indraneel: bookorder.c books.c list.c node.c queue.c
	$(CC) $(CFLAGS) -o bookorder bookorder.c books.c list.c node.c queue.c

test-queue: queue.c tests/test-queue.c
	$(CC) $(CFLAGS) -o test-queue tests/test-queue.c queue.c node.c
clean:
	rm -f *.o

