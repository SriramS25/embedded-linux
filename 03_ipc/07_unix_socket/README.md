# UNIX Domain Socket (SOCK_STREAM)

## Overview

UNIX Domain Sockets provide Inter-Process Communication (IPC) between processes running on the same Linux system. Unlike TCP/IP sockets, they do not use network protocols and instead communicate through a file system path.

This example demonstrates a connection-oriented client-server communication using `AF_UNIX` and `SOCK_STREAM`.

---

## Why UNIX Domain Sockets?

UNIX Domain Sockets are used when:

- Communication is only between local processes.
- High performance is required.
- Bidirectional communication is needed.
- Multiple clients must communicate with one server.

---

## Features

- Full duplex communication
- Reliable data transfer
- Connection-oriented
- Local machine communication
- Uses filesystem pathname

Example socket path:

/tmp/server.sock

---

## System Calls Used

### Server

- socket()
- bind()
- listen()
- accept()
- recv()
- send()
- close()
- unlink()

### Client

- socket()
- connect()
- send()
- recv()
- close()

---

## Program Flow

Server

socket()

↓

bind()

↓

listen()

↓

accept()

↓

recv()

↓

send()

↓

close()

Client

socket()

↓

connect()

↓

send()

↓

recv()

↓

close()

---

## Build

```bash
make
```

---

## Run

Terminal 1

```bash
./server
```

Terminal 2

```bash
./client
```

---

## Expected Output

Server

```text
Socket created successfully
Socket bind successfully
Listening for incoming connections...
Client FD = 4
accept: Client connected successfully
Received from client: Hello Server
Sent 12 bytes to client
```

Client

```text
Socket created successfully
Connected to server successfully
Sent 12 bytes
Received from server : Hello Client
```

---

## Advantages

- Faster than TCP sockets on the same machine.
- Full duplex communication.
- Reliable byte stream.
- Supports multiple clients.
- Same programming model as TCP sockets.

---

## Limitations

- Cannot communicate across different machines.
- Requires server socket cleanup using `unlink()`.
- Stream sockets do not preserve message boundaries.

---

## Real-world Use Cases

- Communication between local services.
- Database servers.
- Printing systems.
- Desktop applications.
- Embedded Linux daemons.
- System services using client-server architecture.

---

## Interview Questions

### Why is `bind()` required only on the server?

The server binds to a well-known socket path so that clients know where to connect. The client usually receives an automatically assigned local endpoint.

---

### Why does `accept()` return a new file descriptor?

The listening socket continues accepting new clients while each connected client gets its own socket descriptor for communication.

---

### Why does `recv()` not append `'\0'`?

Sockets transfer raw bytes, not C strings. The application must append the null terminator if the received data is to be treated as a string.

---

### Why do we use `sizeof(buffer)-1` in `recv()`?

To reserve one byte for the null terminator before printing the received data as a C string.

---

### What happens if `recv()` returns 0?

The peer has closed the connection gracefully.

---

## References

- Linux Manual Pages (`man socket`, `man bind`, `man listen`, `man accept`)
- Advanced Programming in the UNIX Environment (APUE)
- The Linux Programming Interface (TLPI)
