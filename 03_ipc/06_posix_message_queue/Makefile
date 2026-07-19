CC=gcc

CFLAGS=-Wall -Wextra -Iinclude

LDFLAGS=-lrt

all: sender receiver

sender: 
	$(CC) sender.c -o sender $(CFLAGS) $(LDFLAGS)

receiver:
	$(CC) receiver.c -o receiver $(CFLAGS) $(LDFLAGS)

clean:
	rm -f sender receiver
