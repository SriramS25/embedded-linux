CC=gcc

CFLAGS=-Wall -Wextra -Iinclude

all: writer reader

writer:
	$(CC) writer.c -o writer $(CFLAGS)

reader:
	$(CC) reader.c -o reader $(CFLAGS)

clean:
	rm -f writer reader
	rm -f /tmp/ipc_fifo
