#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

#include "protocol.h"

int main()
{
    int value;

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

    printf("Enter value : ");
    scanf("%d",&value);

    sem_wait(empty);
    sem_wait(mutex);

    printf("\nProducer Critical Section\n");

    printf("Old Tail = %d\n",shared->tail);

    shared->data[shared->tail]=value;

    shared->tail=
        (shared->tail+1)%BUFFER_SIZE;

    printf("New Tail = %d\n",shared->tail);

    sem_post(mutex);
    sem_post(full);

    munmap(shared,sizeof(SharedBuffer));

    close(fd);

    sem_close(empty);
    sem_close(full);
    sem_close(mutex);
}
