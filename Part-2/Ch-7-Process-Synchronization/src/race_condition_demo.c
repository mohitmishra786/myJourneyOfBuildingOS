#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdatomic.h>

#define NUM_THREADS 4
#define INCREMENTS_PER_THREAD 1000000

int unsafe_counter = 0;
int safe_counter = 0;
atomic_int atomic_counter = 0;
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

void* unsafe_increment(void* arg) {
    (void)arg; // Suppress unused parameter warning
    for (int i = 0; i < INCREMENTS_PER_THREAD; i++) {
        unsafe_counter++;
    }
    return NULL;
}

void* safe_increment(void* arg) {
    (void)arg; // Suppress unused parameter warning
    for (int i = 0; i < INCREMENTS_PER_THREAD; i++) {
        pthread_mutex_lock(&counter_mutex);
        safe_counter++;
        pthread_mutex_unlock(&counter_mutex);
    }
    return NULL;
}

void* atomic_increment(void* arg) {
    (void)arg; // Suppress unused parameter warning
    for (int i = 0; i < INCREMENTS_PER_THREAD; i++) {
        atomic_fetch_add(&atomic_counter, 1);
    }
    return NULL;
}

void run_race_condition_test(void* (*func)(void*), const char* test_name) {
    pthread_t threads[NUM_THREADS];
    
    printf("Running %s test...\n", test_name);
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, func, NULL);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
}

int main() {
    printf("Race Condition Demonstration\n");
    printf("Expected final value: %d\n\n", NUM_THREADS * INCREMENTS_PER_THREAD);
    
    run_race_condition_test(unsafe_increment, "Unsafe");
    printf("Unsafe counter result: %d\n\n", unsafe_counter);
    
    run_race_condition_test(safe_increment, "Mutex-protected");
    printf("Safe counter result: %d\n\n", safe_counter);
    
    run_race_condition_test(atomic_increment, "Atomic operations");
    printf("Atomic counter result: %d\n\n", atomic_load(&atomic_counter));
    
    pthread_mutex_destroy(&counter_mutex);
    return 0;
} 