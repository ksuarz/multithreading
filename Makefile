# Makefile
# Compiles the book order program.

CC = gcc
CFLAGS = -Wall -g

all: order

order: bookorder.c books.c books.h list.c list.h node.c node.h queue.c queue.h
	$(CC) $(CFLAGS) -lpthread -o bookorder *.c

backend: books.c list.c node.c queue.c
	$(CC) $(CFLAGS) -c books.c list.c node.c queue.c

indraneel: bookorder.c books.c list.c node.c queue.c
	$(CC) $(CFLAGS) -o bookorder bookorder.c books.c list.c node.c queue.c

test-queue: queue.c tests/test-queue.c
	$(CC) $(CFLAGS) -o test-queue tests/test-queue.c queue.c node.c
clean:
	rm -f *.o

