#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define ITERATIONS 1000000  // 1 million — shows race better

int counter = 0;

void *increment(void *arg) {
    for(int i = 0; i < ITERATIONS; i++) {
        counter++;  // NOT atomic — 3 CPU instructions
    }
    return NULL;
}

int main() {
    pthread_t t1, t2;

    pthread_create(&t1, NULL, increment, NULL);
    pthread_create(&t2, NULL, increment, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Expected : %d\n", ITERATIONS * 2);
    printf("Actual   : %d\n", counter);
    printf("Lost     : %d updates\n", (ITERATIONS * 2) - counter);
    return 0;
}
