# POSIX Semaphore

## Overview

Semaphores are synchronization primitives used to coordinate access to shared resources among multiple processes or threads.

---

## Why Semaphores?

To prevent race conditions while accessing shared resources.

---

## Features

- Synchronization
- Mutual exclusion
- Counting resource availability

---

## System Calls Used

- sem_open()
- sem_wait()
- sem_post()
- sem_close()
- sem_unlink()

---

## Program Flow

Process A

sem_wait()

↓

Critical Section

↓

sem_post()

---

## Advantages

- Prevents race conditions
- Supports process synchronization
- Counting semaphore support

---

## Limitations

- Incorrect usage may cause deadlocks
- More difficult debugging

---

## Interview Questions

### Difference between Mutex and Semaphore?

Mutex provides ownership. Semaphores simply maintain a counter.

### Why sem_wait()?

It decreases the semaphore count and blocks if unavailable.

### Why sem_post()?

It increments the semaphore and wakes waiting processes.

