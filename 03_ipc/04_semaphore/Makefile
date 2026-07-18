CC=gcc

CFLAGS=-Wall -Wextra -Iinclude

LDFLAGS=-pthread

all: writer reader

writer:
	$(CC) writer.c -o writer $(CFLAGS) $(LDFLAGS)

reader:
	$(CC) reader.c -o reader $(CFLAGS) $(LDFLAGS)

clean:
	rm -f writer reader
