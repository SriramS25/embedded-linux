#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <semaphore.h>

#include "protocol.h"

int main()
{
    int fd = shm_open(SHM_NAME,O_CREAT|O_RDWR,0666);

    if(fd==-1)
    {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if(ftruncate(fd,sizeof(SharedBuffer))==-1)
    {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    SharedBuffer *shared =
        mmap(NULL,
             sizeof(SharedBuffer),
             PROT_READ|PROT_WRITE,
             MAP_SHARED,
             fd,
             0);

    if(shared==MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    memset(shared,0,sizeof(SharedBuffer));

    shared->head=0;
    shared->tail=0;

    sem_open(SEM_EMPTY,O_CREAT,0666,BUFFER_SIZE);
    sem_open(SEM_FULL,O_CREAT,0666,0);
    sem_open(SEM_MUTEX,O_CREAT,0666,1);

    printf("IPC Initialized\n");

    munmap(shared,sizeof(SharedBuffer));
    close(fd);

    return 0;
}
