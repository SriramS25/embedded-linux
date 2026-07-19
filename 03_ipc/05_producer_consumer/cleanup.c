#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <semaphore.h>

#include "protocol.h"

int main(void)
{
    printf("Cleaning IPC Resources...\n");

    if(shm_unlink(SHM_NAME) == -1)
        perror("shm_unlink");
    else
        printf("Shared Memory Removed\n");

    if(sem_unlink(SEM_EMPTY) == -1)
        perror("sem_unlink EMPTY");
    else
        printf("SEM_EMPTY Removed\n");

    if(sem_unlink(SEM_FULL) == -1)
        perror("sem_unlink FULL");
    else
        printf("SEM_FULL Removed\n");

    if(sem_unlink(SEM_MUTEX) == -1)
        perror("sem_unlink MUTEX");
    else
        printf("SEM_MUTEX Removed\n");

    return 0;
}
