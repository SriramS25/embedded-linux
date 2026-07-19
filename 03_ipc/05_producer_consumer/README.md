# Producer Consumer Problem

## Overview

The Producer-Consumer problem demonstrates synchronization between processes using semaphores and a shared buffer.

---

## Components

- Producer
- Consumer
- Buffer
- Empty Semaphore
- Full Semaphore
- Mutex

---

## Synchronization Flow

Producer

wait(empty)

↓

wait(mutex)

↓

Insert Item

↓

post(mutex)

↓

post(full)

Consumer

wait(full)

↓

wait(mutex)

↓

Remove Item

↓

post(mutex)

↓

post(empty)

---

## Interview Questions

### Why three semaphores?

- empty
- full
- mutex

### What happens if mutex is removed?

Race conditions occur.

### What happens if full semaphore is removed?

Consumer may consume invalid data.
