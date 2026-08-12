#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 5
#define TOTAL_ITEMS 20

//-------- RING BUFFER STRUCTURE-----------
typedef struct{

	int buffer[BUFFER_SIZE];
	int head;   		//where producer writes
	int tail;		//where consumer reads
	int count;		//Number of items in buffer
	
	pthread_mutex_t mutex;
	pthread_cond_t	not_full; 	//signaled when buffer not full
	pthread_cond_t 	not_empty;	//signaled when buffer not empty
	
} ring_buffer_t;

//=======RING BUFFER FUNCTIONS=========
void ring_buffer_init(ring_buffer_t *rb){
	rb->head = 0;
	rb->tail = 0;
	rb->count = 0;
	pthread_mutex_init(&rb->mutex, NULL);
	pthread_cond_init(&rb->not_full, NULL);
	pthread_cond_init(&rb->not_empty, NULL);
}

void ring_buffer_destroy(ring_buffer_t *rb){
	pthread_mutex_destroy(&rb->mutex);
	pthread_cond_destroy(&rb->not_full);
	pthread_cond_destroy(&rb->not_empty);
}

int ring_buffer_is_full(ring_buffer_t *rb){
	return rb->count == BUFFER_SIZE;
}

int ring_buffer_is_empty(ring_buffer_t *rb){
	return rb->count == 0;
}

//Producer adds data to buffer
void ring_buffer_put(ring_buffer_t *rb, int data){
	pthread_mutex_lock(&rb->mutex);

	//wait while buffer is full
	while(ring_buffer_is_full(rb)){
		printf("[PRODUCER] ⚠️ Buffer FULL! Waiting...\n");
		pthread_cond_wait(&rb->not_full, &rb->mutex);
	}

	//Add data to buffer
	rb->buffer[rb->head] = data;
	rb->head = (rb->head + 1) % BUFFER_SIZE;
	rb->count++;

	printf("[PRODUCER] + Added: %2d | Count: %d | Head: %d | Tail: %d\n", data,rb->count, rb->head,rb->tail);

	//Signal Consumer that buffer is not empty
	pthread_cond_signal(&rb->not_empty);
	pthread_mutex_unlock(&rb->mutex);
}

//consumer removes data from buffer
int ring_buffer_get(ring_buffer_t *rb){
	pthread_mutex_lock(&rb->mutex);

	//wait while buffer is empty
	while(ring_buffer_is_empty(rb)){
                printf("[CONSUMER] ⚠️ Buffer EMPTY! Waiting...\n");
		pthread_cond_wait(&rb->not_empty, &rb->mutex);
	}

	//remove data from buffer
	int data = rb->buffer[rb->tail];
	rb->tail = (rb->tail + 1)% BUFFER_SIZE;
	rb->count--;

	printf("[CONSUMER] ➖ Removed: %2d | Count: %d | Head: %d | Tail: %d\n", data, rb->count, rb->head, rb->tail);

	//Signal producer that buffer is not full
	pthread_cond_signal(&rb->not_full);
	pthread_mutex_unlock(&rb->mutex);
	return data;
}


//=======THREAD FUNCTIONS=====
ring_buffer_t shared_buffer;
int items_produced = 0;
int items_consumed = 0;

void *producer_thread(void *arg){
	int item = 0;
	while(items_produced < TOTAL_ITEMS){
		//Generate sensor reading [random data]
		item = rand() % 1000 + 1;        //1 - 1000
		ring_buffer_put(&shared_buffer, item);
		items_produced++;

		usleep(rand() % 200000 + 100000); //100-300ms
		}

	printf("[PRODUCER] :✅ All %d items produced !\n", TOTAL_ITEMS);
	return NULL;
}

void *consumer_thread(void *arg){
	while(items_consumed < TOTAL_ITEMS){
		int data = ring_buffer_get(&shared_buffer);
		items_consumed++;

		//Process the data (simulate work)
		printf("[CONSUMER]  🔄 Processing: %d | Consumed : %d/%d\n", data,items_consumed, TOTAL_ITEMS);
		usleep(rand()%300000 + 200000); //200-500ms
	}

 	printf("[CONSUMER] ✅ All %d items consumed!\n", TOTAL_ITEMS);
    	return NULL;
}

// =============== BUFFER STATE VISUALIZATION ================
// Add this improved visualization
void print_buffer_state(ring_buffer_t *rb) {
    pthread_mutex_lock(&rb->mutex);
    
    printf("\n╔═══════════════════════════════════════════════╗\n");
    printf("║              BUFFER STATE                     ║\n");
    printf("╠═══════════════════════════════════════════════╣\n");
    printf("║ Count: %2d  |  Head: %d  |  Tail: %d          ║\n", 
           rb->count, rb->head, rb->tail);
    
    // Show contents
    printf("║ Data:   ");
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (i == rb->head && i == rb->tail && rb->count == 0) {
            printf("[ H/T ]");
        } else if (i == rb->head) {
            printf("[ HEAD]");
        } else if (i == rb->tail) {
            printf("[ TAIL]");
        } else if (rb->buffer[i] != 0) {
            printf(" %4d  ", rb->buffer[i]);
        } else {
            printf(" [    ]");
        }
    }
    printf("\n");
    
    printf("║ Index:  ");
    for (int i = 0; i < BUFFER_SIZE; i++) {
        printf("  %2d   ", i);
    }
    printf("\n");
    
    // Show the circular buffer visually
    printf("║ ┌─────────────────────────────────────────┐\n");
    printf("║ │ ");
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (rb->buffer[i] != 0) {
            printf("%4d ", rb->buffer[i]);
        } else {
            printf("     ");
        }
    }
    printf("│\n");
    printf("║ └─────────────────────────────────────────┘\n");
    
    printf("╚═══════════════════════════════════════════════╝\n\n");
    
    pthread_mutex_unlock(&rb->mutex);
}


//===== MAIN =====
int main(){
	pthread_t producer,consumer;
	srand(time(NULL));

	printf("\n");
        printf("====================================================\n");
	printf("\t Producer - Consumer with Ring Buffer\n");
 	printf("=====================================================\n");
	printf("Buffer Size : %d\n", BUFFER_SIZE);
	printf("Total Items : %d\n", TOTAL_ITEMS);
	printf("======================================================\n\n");
	

	//Initialize ring buffer
	ring_buffer_init(&shared_buffer);	

	//create threads
	pthread_create(&producer, NULL, producer_thread, NULL);
	pthread_create(&consumer, NULL, consumer_thread, NULL);
	
	//Show buffer state peridically
	/*for(int i = 0; i<30; i++){
		print_buffer_state(&shared_buffer);
		sleep(1);
	}*/

	//Show buffer state while threads are running
	while (items_produced < TOTAL_ITEMS || items_consumed < TOTAL_ITEMS) {
        	print_buffer_state(&shared_buffer);
        	usleep(500000);  // Check every 500ms
    	}

	//wait for thread to finish
	pthread_join(producer, NULL);
	pthread_join(consumer, NULL);

	//Final state
	printf("\n=========================================================\n");
	printf("FINAL BUFFER STATE: \n");
	print_buffer_state(&shared_buffer);

	//Cleanup
	ring_buffer_destroy(&shared_buffer);

	printf("✅ All done! Producer- consumer completed successfully!\n");
	printf("==============================================\n\n");

	return 0;
}
	



