#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutexA;
pthread_mutex_t mutexB;

void *thread1_func(void *arg){
	printf("Thread 1: Attempting to lock mutex A\n");
	pthread_mutex_lock(&mutexA);
	printf("Thread 1: MutexA Locked\n");
	sleep(1);

	printf("Thread 1: Attempting to Lock MutexB \n");
	pthread_mutex_lock(&mutexB);
	printf("Thread 1: MutexB Locked\n");

	//Critical section
	printf("Thread 1: Doing work ...\n");

	pthread_mutex_unlock(&mutexB);
	pthread_mutex_unlock(&mutexA);
	return NULL;
}

void *thread2_func(void *arg){
        printf("Thread 2: Attempting to lock mutexB\n");
        pthread_mutex_lock(&mutexB);
        printf("Thread 2: MutexB Locked\n");
        sleep(1);

        printf("Thread 2: Attempting to Lock MutexA \n");
        pthread_mutex_lock(&mutexA);
        printf("Thread 2: MutexA Locked\n");

        //Critical section
        printf("Thread 2: Doing work ...\n");

        pthread_mutex_unlock(&mutexA);
        pthread_mutex_unlock(&mutexB);
        return NULL;
}

int main(){
	pthread_t t1,t2;

	//Initialize Mutex
	pthread_mutex_init(&mutexA, NULL);
	pthread_mutex_init(&mutexB, NULL);

	printf("Starting Deadlock program....\n");
	printf("Thread 1: A then B\n");
	printf("Thread 2: B then A\n");
	printf("DEADLOCK Imminent!\n\n");

	//create threads
	pthread_create(&t1, NULL, thread1_func, NULL);
	pthread_create(&t2, NULL, thread2_func, NULL);

	//wait for the threads to finish
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	printf("Program Completed\n");


	//Clean Up
	pthread_mutex_destroy(&mutexA);
	pthread_mutex_destroy(&mutexB);
	return 0;
}

