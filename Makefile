CC = gcc
"CFLAGS = -Wall -g"
CFLAGS = -g

all: 

indraneel: bookorder.c books.c list.c node.c queue.c
	$(CC) $(CFLAGS) -o bookorder bookorder.c books.c list.c node.c queue.c

clean:
	rm -f *.o

