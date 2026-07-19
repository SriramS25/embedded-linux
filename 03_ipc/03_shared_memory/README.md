# POSIX Shared Memory

## Overview

Shared Memory is the fastest IPC mechanism because processes directly access the same memory region without copying data through the kernel after setup.

---

## Why Shared Memory?

Used when:

- Large data must be shared
- High performance is required
- Frequent communication occurs

---

## Features

- Fastest IPC
- Shared address space
- Minimal kernel overhead
- Requires synchronization

---

## System Calls Used

- shm_open()
- ftruncate()
- mmap()
- munmap()
- shm_unlink()

---

## Program Flow

Writer

shm_open()

↓

ftruncate()

↓

mmap()

↓

write()

Reader

shm_open()

↓

mmap()

↓

read()

---

## Advantages

- Extremely fast
- No data copying after mapping
- Efficient for large data

---

## Limitations

- Requires synchronization
- More complex than pipes
- Shared memory lifetime must be managed

---

## Interview Questions

### Why is Shared Memory the fastest IPC?

Processes access the same physical memory without copying data between processes after the initial mapping.

### Why is mmap() required?

It maps the shared memory object into the process's virtual address space.

### Can two processes write simultaneously?

Yes, which is why semaphores or mutexes are usually required.

---

## References

- man shm_open
- man mmap
