CC=gcc

CFLAGS=-Wall -Wextra -Iinclude

LDFLAGS=-pthread

TARGETS=init producer consumer cleanup

all: $(TARGETS)

init:
	$(CC) $(CFLAGS) init.c -o init $(LDFLAGS)

producer:
	$(CC) $(CFLAGS) producer.c -o producer $(LDFLAGS)

consumer:
	$(CC) $(CFLAGS) consumer.c -o consumer $(LDFLAGS)

cleanup:
	$(CC) $(CFLAGS) cleanup.c -o cleanup $(LDFLAGS)

clean:
	rm -f $(TARGETS)
