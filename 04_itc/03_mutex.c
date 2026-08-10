#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define ITERATIONS 1000000  // 1 million — shows race better
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //STATIC INITIALIZER

int counter = 0;

void *increment_with_mutex(void *arg) {
    for(int i = 0; i < ITERATIONS; i++) {
	pthread_mutex_lock(&mutex);
        counter++;  // NOT atomic — 3 CPU instructions
	pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t t1, t2;
    clock_t start,end;
    double cpu_time_used;

    printf("=== MUTEX PROTECTED COUNTER ===\n");
    printf("Iterations per thread: %d\n", ITERATIONS);
    printf("Expected result: %d\n\n", ITERATIONS * 2);
	
    //start clock
    start = clock();

    pthread_create(&t1, NULL, increment_with_mutex, NULL);
    pthread_create(&t2, NULL, increment_with_mutex, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    end = clock();
    cpu_time_used = ((double)(end - start))/CLOCKS_PER_SEC;

    printf("Actual   : %d\n", counter);
    printf("Time taken: %.6f seconds\n", cpu_time_used);
    printf("Status   : %s\n", 
           (counter == ITERATIONS * 2) ? "✅ CORRECT!" : "❌ RACE CONDITION!");
    

    //destroy mutex
    pthread_mutex_destroy(&mutex);
    return 0;
}
