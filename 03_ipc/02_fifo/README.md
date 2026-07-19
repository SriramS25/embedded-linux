# Named Pipe (FIFO)

## Overview

A FIFO (First In First Out), also called a Named Pipe, enables communication between unrelated processes using a special file created in the filesystem.

---

## Why FIFO?

Unlike anonymous pipes, FIFOs allow completely unrelated processes to communicate.

---

## Features

- Named in filesystem
- One-way communication
- Works between unrelated processes
- Persistent until removed

---

## System Calls Used

- mkfifo()
- open()
- read()
- write()
- close()
- unlink()

---

## Program Flow

Sender

open()

↓

write()

↓

close()

Receiver

open()

↓

read()

↓

close()

---

## Advantages

- Easy to use
- Persistent IPC endpoint
- Suitable for unrelated processes

---

## Limitations

- Half duplex
- Requires synchronization
- Lower flexibility than sockets

---

## Interview Questions

### Difference between Pipe and FIFO?

Anonymous Pipes require related processes. FIFOs can communicate between unrelated processes.

### Why use mkfifo()?

It creates the special FIFO file inside the filesystem.

### Can multiple writers use one FIFO?

Yes, but synchronization becomes the application's responsibility.

---

## References

- man fifo
- man mkfifo
