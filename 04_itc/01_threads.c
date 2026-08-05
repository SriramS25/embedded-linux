#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

void *thread_func(void *arg){
	int thread_num = *((int *)arg);
        printf("Thread %d started - TID = %lu\n",thread_num,(unsigned long)pthread_self());
	printf("Message from : Thread %d\n",thread_num);
	sleep(1);
	printf("Thread %d finished\n",thread_num);
	return NULL;
}

int main(){

	pthread_t t[3];
	int nums[3];

	for(int i=0; i < 3; i++){
		nums[i] = i + 1;
		pthread_create(&t[i], NULL, thread_func, &nums[i]);
	}

	for(int i=0; i<3; i++){
		pthread_join(t[i], NULL);
	}

	printf("All thread completed, Successfully\n");

	return 0;
}
