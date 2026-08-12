#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>

#define PARKING_SPACE 3
#define TOTAL_CARS    10

sem_t parking_lot;
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

void safe_print(const char* msg) {
    pthread_mutex_lock(&print_mutex);
    printf("%s", msg);
    fflush(stdout);
    pthread_mutex_unlock(&print_mutex);
}

void *car(void *arg) {
    int car_id = *(int *)arg;
    int spaces_left;
    char msg[200];
    
    sprintf(msg, "🚗 Car %-2d: Looking for parking .....\n", car_id);
    safe_print(msg);
    
    // Try to park (decrement semaphore)
    sem_wait(&parking_lot);
    
    // ✅ Get actual count safely
    sem_getvalue(&parking_lot, &spaces_left);
    
    sprintf(msg, "🚗 Car %-2d: ✅ PARKED! [Spaces left: %d]\n", 
            car_id, spaces_left);
    safe_print(msg);
    
    // Simulate parking time (random between 1-3 seconds)
    sleep(rand() % 3 + 1);
    
    sprintf(msg, "🚗 Car %-2d: Leaving..\n", car_id);
    safe_print(msg);
    
    // Leave (increment semaphore)
    sem_post(&parking_lot);
    
    return NULL;
}

int main() {
    pthread_t cars[TOTAL_CARS];
    int car_ids[TOTAL_CARS];
    int final_count;
    
    srand(time(NULL));  // Seed random number generator
    
    // Initialize semaphore with 3 available spaces
    sem_init(&parking_lot, 0, PARKING_SPACE);
    
    printf("==========================================\n");
    printf("     🅿️  PARKING LOT SIMULATION\n");
    printf("==========================================\n");
    printf("Total Spaces: %d\n", PARKING_SPACE);
    printf("Total Cars:   %d\n", TOTAL_CARS);
    printf("==========================================\n\n");
    
    // Create all car threads
    for(int i = 0; i < TOTAL_CARS; i++) {
        car_ids[i] = i + 1;
        pthread_create(&cars[i], NULL, car, &car_ids[i]);
        usleep(50000);  // 50ms delay between arrivals
    }
    
    // Wait for all cars
    for(int i = 0; i < TOTAL_CARS; i++) {
        pthread_join(cars[i], NULL);
    }
    
    // Verify final count
    sem_getvalue(&parking_lot, &final_count);
    printf("\n==========================================\n");
    printf("Final spaces available: %d\n", final_count);
    printf("Expected: %d\n", PARKING_SPACE);
    printf("Status: %s\n", 
           (final_count == PARKING_SPACE) ? "✅ SUCCESS" : "❌ ERROR");
    
    sem_destroy(&parking_lot);
    pthread_mutex_destroy(&print_mutex);
    
    printf("==========================================\n");
    printf("🏁 All cars have parked and left!\n");
    
    return 0;
}
