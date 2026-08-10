#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define PARKING_SPACE 3
#define TOTAL_CARS    10

sem_t parking_lot; 	//counting semaphore

void *car(void *arg){
	int car_id = *(int *)arg;
	int spaces_left;
	printf("Car %d : Looking for parking .....\n", car_id);
	
	//Try to park [decrement semaphore]
	sem_wait(&parking_lot); //<- acquire a space
	//if no space, car waits here!
	sem_getvalue(&parking_lot, &spaces_left);
	printf("Car %d : PARKED ! [Spaces left: %d]\n",car_id,spaces_left);

	//simulate parking time
	sleep(2);

	printf("Car %d : Leaving..\n",car_id);

	//Leave (increment semaphore)
	sem_post(&parking_lot);
	return NULL;
}

int main(){
	pthread_t cars[TOTAL_CARS];
	int car_ids[TOTAL_CARS];

	//Initialize semaphore with 3 available spaces;
	sem_init(&parking_lot, 0, PARKING_SPACE);

	printf("=====PARKING LOT SIMULATION ========\n");
	printf("Total Spaces: %d\n", PARKING_SPACE);
	printf("Total Cars: %d\n", TOTAL_CARS);

	//create all car threads
	for(int i=0 ; i < TOTAL_CARS; i++){
		car_ids[i] = i + 1;
		pthread_create(&cars[i], NULL, car, &car_ids[i]);
		usleep(1000);
	}

	for(int i=0; i< TOTAL_CARS; i++){
		pthread_join(cars[i], NULL);
	}

	sem_destroy(&parking_lot);
	printf("\n\n\n All cars have parked and left!\n");
	return 0;
}

