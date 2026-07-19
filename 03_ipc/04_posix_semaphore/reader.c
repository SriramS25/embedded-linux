#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>

#include "protocol.h"

int main(){
	int fd;

	fd = shm_open(SHM_NAME, O_RDWR, 0666);

	if(fd == -1){
		perror("fd : sh_open");
		exit(EXIT_FAILURE);
	}
	
	SensorData_t *shared = mmap(NULL, 
				sizeof(SensorData_t),
				PROT_READ | PROT_WRITE,
				MAP_SHARED,
				fd,
				0);
	
	if(shared == MAP_FAILED){
		perror("mmap");
		exit(EXIT_FAILURE);
	}

	sem_t *sem = sem_open(SEM_NAME, 0);

	if(sem == SEM_FAILED){
		perror("sem_open");
		exit(EXIT_FAILURE);
	}

	sem_wait(sem);

	if(shared->magic != MAGIC_NUMBER){
		printf("invalid data");
	}
	else{
		printf("\nSensor Data\n");
		printf("-----------------------\n");

		printf("Temperature: %d\n",shared->temperature);
		printf("Pressure: %d\n",shared->pressure);
		printf("Humidity: %f\n",shared->humidity);

	}

	sem_post(sem);
	munmap(shared, sizeof(SensorData_t));
	close(fd);
	sem_close(sem);

	return 0;
}
