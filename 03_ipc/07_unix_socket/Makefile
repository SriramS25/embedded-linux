# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g

# Executables
TARGETS = server client

# Default target
all: $(TARGETS)

# Build server
server: server.c
	$(CC) $(CFLAGS) -o server server.c

# Build client
client: client.c
	$(CC) $(CFLAGS) -o client client.c

# Remove executables and socket file
clean:
	rm -f server client /tmp/server.sock

# Rebuild everything
rebuild: clean all
