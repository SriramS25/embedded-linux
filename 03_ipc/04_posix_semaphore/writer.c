#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <string.h>
#include <stdlib.h>
#include "protocol.h"

int main(){

	int fd;
	fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);

	if(fd == -1){
		perror("sh_open");
		exit(EXIT_FAILURE);
	}

	if(ftruncate(fd,sizeof(SensorData_t)) == -1){
		perror("ftruncate");
		exit(EXIT_FAILURE);
	}

	SensorData_t *shared = mmap(NULL,
			sizeof(SensorData_t),
			PROT_READ|PROT_WRITE,
			MAP_SHARED,
			fd,
			0);

	if(shared == MAP_FAILED){
		perror("mmap");
		exit(EXIT_FAILURE);
	}

	sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);

	if(sem == SEM_FAILED){
		perror("sem_open");
		exit(EXIT_FAILURE);
	}

	sem_wait(sem);
	
	printf("Writing sensor Data ....\n");
	
	shared->magic = MAGIC_NUMBER;
	shared->version = VERSION;

	printf("Temperature : ");
	scanf("%d", &shared->temperature);

	printf("Pressure : ");
	scanf("%d", &shared->pressure);
	
	printf("Humidity : ");
	scanf("%f", &shared->humidity);

	sem_post(sem);
	munmap(shared, sizeof(SensorData_t));
	
			close(fd);
			sem_close(sem);
			return 0;
			}
		
