# Anonymous Pipe

## Overview

An Anonymous Pipe is the simplest IPC mechanism in Linux. It provides a unidirectional communication channel between related processes (typically parent and child). Pipes exist only while the processes are running and cannot be accessed using a filesystem pathname.

---

## Why Anonymous Pipe?

Anonymous Pipes are useful when:

- Parent and child need to exchange data.
- Temporary communication is required.
- Simple one-way communication is sufficient.

---

## Features

- Unidirectional communication
- Parent-child relationship required
- Kernel-managed buffer
- Automatically destroyed when processes exit

---

## System Calls Used

- pipe()
- fork()
- read()
- write()
- close()

---

## Program Flow

Parent

pipe()

↓

fork()

↓

write()

↓

close()

Child

read()

↓

close()

---

## Advantages

- Simple to implement
- Fast communication
- Kernel-managed synchronization

---

## Limitations

- Only between related processes
- One-way communication
- No message boundaries

---

## Interview Questions

### Why are two file descriptors returned by pipe()?

One descriptor is used for reading and the other for writing.

### Why should unused pipe ends be closed?

Closing unused ends prevents resource leaks and ensures EOF is detected correctly.

### Can unrelated processes use an anonymous pipe?

No. Anonymous pipes are designed for related processes created using fork().

---

## References

- man pipe
- TLPI
- APUE
