#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

#define BUFFER_SIZE 10
#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 3
#define ITEMS_PER_PRODUCER 20

typedef struct {
    int buffer[BUFFER_SIZE];
    int in;
    int out;
    sem_t *empty_slots;
    sem_t *full_slots;
    pthread_mutex_t mutex;
} bounded_buffer_t;

bounded_buffer_t shared_buffer;
int total_produced = 0;
int total_consumed = 0;
pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;

void buffer_init(bounded_buffer_t *buf) {
    buf->in = 0;
    buf->out = 0;
    
    // Use named semaphores instead of deprecated sem_init
    sem_unlink("/empty_slots");
    sem_unlink("/full_slots");
    buf->empty_slots = sem_open("/empty_slots", O_CREAT, 0644, BUFFER_SIZE);
    buf->full_slots = sem_open("/full_slots", O_CREAT, 0644, 0);
    
    if (buf->empty_slots == SEM_FAILED || buf->full_slots == SEM_FAILED) {
        perror("sem_open failed");
        exit(1);
    }
    
    pthread_mutex_init(&buf->mutex, NULL);
}

void buffer_destroy(bounded_buffer_t *buf) {
    sem_close(buf->empty_slots);
    sem_close(buf->full_slots);
    sem_unlink("/empty_slots");
    sem_unlink("/full_slots");
    pthread_mutex_destroy(&buf->mutex);
}

void produce_item(bounded_buffer_t *buf, int item) {
    sem_wait(buf->empty_slots);
    pthread_mutex_lock(&buf->mutex);
    
    buf->buffer[buf->in] = item;
    printf("Producer %p produced item %d at position %d\n", 
           (void*)pthread_self(), item, buf->in);
    buf->in = (buf->in + 1) % BUFFER_SIZE;
    
    pthread_mutex_unlock(&buf->mutex);
    sem_post(buf->full_slots);
}

int consume_item(bounded_buffer_t *buf) {
    sem_wait(buf->full_slots);
    pthread_mutex_lock(&buf->mutex);
    
    int item = buf->buffer[buf->out];
    printf("Consumer %p consumed item %d from position %d\n", 
           (void*)pthread_self(), item, buf->out);
    buf->out = (buf->out + 1) % BUFFER_SIZE;
    
    pthread_mutex_unlock(&buf->mutex);
    sem_post(buf->empty_slots);
    
    return item;
}

void* producer(void* arg) {
    int producer_id = *(int*)arg;
    
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int item = producer_id * 1000 + i;
        produce_item(&shared_buffer, item);
        
        pthread_mutex_lock(&stats_mutex);
        total_produced++;
        pthread_mutex_unlock(&stats_mutex);
        
        usleep(rand() % 100000);
    }
    
    printf("Producer %d finished\n", producer_id);
    return NULL;
}

void* consumer(void* arg) {
    int consumer_id = *(int*)arg;
    int items_consumed = 0;
    
    while (1) {
        pthread_mutex_lock(&stats_mutex);
        if (total_consumed >= NUM_PRODUCERS * ITEMS_PER_PRODUCER) {
            pthread_mutex_unlock(&stats_mutex);
            break;
        }
        pthread_mutex_unlock(&stats_mutex);
        
        consume_item(&shared_buffer);
        items_consumed++;
        
        pthread_mutex_lock(&stats_mutex);
        total_consumed++;
        pthread_mutex_unlock(&stats_mutex);
        
        usleep(rand() % 150000);
    }
    
    printf("Consumer %d finished, consumed %d items\n", consumer_id, items_consumed);
    return NULL;
}

int main() {
    srand(time(NULL));
    buffer_init(&shared_buffer);
    
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    int producer_ids[NUM_PRODUCERS];
    int consumer_ids[NUM_CONSUMERS];
    
    printf("Starting Producer-Consumer Demonstration\n");
    printf("Buffer size: %d, Producers: %d, Consumers: %d\n\n", 
           BUFFER_SIZE, NUM_PRODUCERS, NUM_CONSUMERS);
    
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        producer_ids[i] = i + 1;
        pthread_create(&producers[i], NULL, producer, &producer_ids[i]);
    }
    
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumer_ids[i] = i + 1;
        pthread_create(&consumers[i], NULL, consumer, &consumer_ids[i]);
    }
    
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }
    
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }
    
    printf("\nFinal Statistics:\n");
    printf("Total produced: %d\n", total_produced);
    printf("Total consumed: %d\n", total_consumed);
    
    buffer_destroy(&shared_buffer);
    pthread_mutex_destroy(&stats_mutex);
    
    return 0;
} 