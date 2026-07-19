#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

#include "protocol.h"

int main()
{
    int fd=shm_open(SHM_NAME,O_RDWR,0666);

    SharedBuffer *shared=
        mmap(NULL,
             sizeof(SharedBuffer),
             PROT_READ|PROT_WRITE,
             MAP_SHARED,
             fd,
             0);

    sem_t *empty=sem_open(SEM_EMPTY,0);
    sem_t *full=sem_open(SEM_FULL,0);
    sem_t *mutex=sem_open(SEM_MUTEX,0);

    sem_wait(full);
    sem_wait(mutex);

    printf("\nConsumer Critical Section\n");

    printf("Head = %d\n",shared->head);

    printf("Received = %d\n",
           shared->data[shared->head]);

    shared->head=
        (shared->head+1)%BUFFER_SIZE;

    printf("New Head = %d\n",shared->head);

    sem_post(mutex);
    sem_post(empty);

    munmap(shared,sizeof(SharedBuffer));

    close(fd);

    sem_close(empty);
    sem_close(full);
    sem_close(mutex);
}
