# POSIX Message Queue

## Overview

POSIX Message Queues provide message-oriented communication between processes.

Unlike pipes, message boundaries are preserved.

---

## Features

- Message based
- Priority support
- Kernel-managed queue
- Multiple writers/readers

---

## System Calls Used

- mq_open()
- mq_send()
- mq_receive()
- mq_close()
- mq_unlink()

---

## Advantages

- Preserves message boundaries
- Supports message priorities
- Easier synchronization than shared memory

---

## Limitations

- Limited queue size
- Slower than shared memory

---

## Interview Questions

### Difference between Pipe and Message Queue?

Pipes transfer byte streams. Message Queues preserve message boundaries.

### Why message priority?

Higher priority messages are received before lower priority ones.

### Can multiple processes use the same queue?

Yes.

