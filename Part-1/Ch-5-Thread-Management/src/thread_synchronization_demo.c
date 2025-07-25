/**
 * Thread Synchronization Demonstration
 * 
 * This program demonstrates various thread synchronization mechanisms:
 * - Mutexes and critical sections
 * - Condition variables
 * - Semaphores
 * - Reader-writer locks
 * - Producer-consumer problem
 * - Deadlock prevention techniques
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <stdarg.h>
#include <fcntl.h>

#define BUFFER_SIZE 10
#define NUM_PRODUCERS 3
#define NUM_CONSUMERS 2
#define NUM_READERS 5
#define NUM_WRITERS 2
#define ITEMS_PER_PRODUCER 5

// Global shared data
volatile int shared_counter = 0;
volatile int race_condition_counter = 0;

// Buffer for producer-consumer problem
typedef struct {
    int items[BUFFER_SIZE];
    int in;
    int out;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} bounded_buffer_t;

// Shared data for reader-writer problem
typedef struct {
    int data;
    int reader_count;
    pthread_rwlock_t rwlock;
    pthread_mutex_t reader_count_mutex;
} shared_data_t;

// Synchronization primitives
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t *binary_semaphore = NULL;
sem_t *counting_semaphore = NULL;

// Global data structures
bounded_buffer_t buffer;
shared_data_t shared_data;

// Function prototypes
void demonstrate_race_condition(void);
void demonstrate_mutex_protection(void);
void demonstrate_condition_variables(void);
void demonstrate_semaphores(void);
void demonstrate_reader_writer_locks(void);
void demonstrate_deadlock_prevention(void);

// Worker thread functions
void *race_condition_worker(void *arg);
void *mutex_protected_worker(void *arg);
void *producer_thread(void *arg);
void *consumer_thread(void *arg);
void *reader_thread(void *arg);
void *writer_thread(void *arg);
void *semaphore_worker(void *arg);
void *deadlock_thread_a(void *arg);
void *deadlock_thread_b(void *arg);

// Utility functions
void safe_printf(const char *format, ...);
void initialize_synchronization_objects(void);
void cleanup_synchronization_objects(void);
double get_time_diff(struct timeval start, struct timeval end);

int main() {
    printf("=== Thread Synchronization Demonstration ===\n\n");
    
    initialize_synchronization_objects();
    
    printf("1. Race Condition Demonstration\n");
    demonstrate_race_condition();
    
    printf("\n2. Mutex Protection\n");
    demonstrate_mutex_protection();
    
    printf("\n3. Producer-Consumer with Condition Variables\n");
    demonstrate_condition_variables();
    
    printf("\n4. Semaphore Synchronization\n");
    demonstrate_semaphores();
    
    printf("\n5. Reader-Writer Locks\n");
    demonstrate_reader_writer_locks();
    
    printf("\n6. Deadlock Prevention\n");
    demonstrate_deadlock_prevention();
    
    cleanup_synchronization_objects();
    return 0;
}

void demonstrate_race_condition(void) {
    printf("Creating threads that increment a shared counter without synchronization...\n");
    
    pthread_t threads[5];
    int thread_ids[5];
    race_condition_counter = 0;
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    // Create threads that will race on the shared counter
    for (int i = 0; i < 5; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, race_condition_worker, &thread_ids[i]) != 0) {
            perror("Failed to create race condition thread");
            return;
        }
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }
    
    gettimeofday(&end, NULL);
    
    printf("Expected result: %d\n", 5 * 1000);
    printf("Actual result: %d\n", race_condition_counter);
    printf("Race condition detected: %s\n", 
           (race_condition_counter != 5000) ? "YES" : "NO");
    printf("Execution time: %.2f ms\n", get_time_diff(start, end));
}

void *race_condition_worker(void *arg) {
    int thread_id = *(int *)arg;
    
    safe_printf("Thread %d starting race condition work\n", thread_id);
    
    // Increment counter 1000 times without synchronization
    for (int i = 0; i < 1000; i++) {
        int temp = race_condition_counter;
        // Simulate some processing time to increase race condition likelihood
        for (volatile int j = 0; j < 10; j++);
        race_condition_counter = temp + 1;
    }
    
    safe_printf("Thread %d completed race condition work\n", thread_id);
    return NULL;
}

void demonstrate_mutex_protection(void) {
    printf("Creating threads that increment a shared counter with mutex protection...\n");
    
    pthread_t threads[5];
    int thread_ids[5];
    shared_counter = 0;
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    // Create threads that use mutex protection
    for (int i = 0; i < 5; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, mutex_protected_worker, &thread_ids[i]) != 0) {
            perror("Failed to create mutex protected thread");
            return;
        }
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }
    
    gettimeofday(&end, NULL);
    
    printf("Expected result: %d\n", 5 * 1000);
    printf("Actual result: %d\n", shared_counter);
    printf("Mutex protection successful: %s\n", 
           (shared_counter == 5000) ? "YES" : "NO");
    printf("Execution time: %.2f ms\n", get_time_diff(start, end));
}

void *mutex_protected_worker(void *arg) {
    int thread_id = *(int *)arg;
    
    safe_printf("Thread %d starting mutex protected work\n", thread_id);
    
    // Increment counter 1000 times with mutex protection
    for (int i = 0; i < 1000; i++) {
        pthread_mutex_lock(&counter_mutex);
        shared_counter++;
        pthread_mutex_unlock(&counter_mutex);
    }
    
    safe_printf("Thread %d completed mutex protected work\n", thread_id);
    return NULL;
}

void demonstrate_condition_variables(void) {
    printf("Implementing producer-consumer pattern with condition variables...\n");
    
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    int producer_ids[NUM_PRODUCERS];
    int consumer_ids[NUM_CONSUMERS];
    
    // Initialize buffer
    buffer.in = 0;
    buffer.out = 0;
    buffer.count = 0;
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    // Create producer threads
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        producer_ids[i] = i;
        if (pthread_create(&producers[i], NULL, producer_thread, &producer_ids[i]) != 0) {
            perror("Failed to create producer thread");
            return;
        }
    }
    
    // Create consumer threads
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumer_ids[i] = i;
        if (pthread_create(&consumers[i], NULL, consumer_thread, &consumer_ids[i]) != 0) {
            perror("Failed to create consumer thread");
            return;
        }
    }
    
    // Wait for all producers to complete
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }
    
    // Wait a bit for consumers to finish processing
    sleep(2);
    
    // Cancel consumer threads (they run indefinitely)
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_cancel(consumers[i]);
        pthread_join(consumers[i], NULL);
    }
    
    gettimeofday(&end, NULL);
    printf("Producer-consumer demonstration completed\n");
    printf("Final buffer count: %d\n", buffer.count);
    printf("Execution time: %.2f ms\n", get_time_diff(start, end));
}

void *producer_thread(void *arg) {
    int producer_id = *(int *)arg;
    
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        pthread_mutex_lock(&buffer.mutex);
        
        // Wait while buffer is full
        while (buffer.count == BUFFER_SIZE) {
            safe_printf("Producer %d waiting - buffer full\n", producer_id);
            pthread_cond_wait(&buffer.not_full, &buffer.mutex);
        }
        
        // Produce item
        int item = producer_id * 100 + i;
        buffer.items[buffer.in] = item;
        buffer.in = (buffer.in + 1) % BUFFER_SIZE;
        buffer.count++;
        
        safe_printf("Producer %d produced item %d (buffer count: %d)\n", 
                   producer_id, item, buffer.count);
        
        // Signal that buffer is not empty
        pthread_cond_signal(&buffer.not_empty);
        pthread_mutex_unlock(&buffer.mutex);
        
        // Simulate production time
        usleep(100000 + (rand() % 200000)); // 0.1-0.3 seconds
    }
    
    safe_printf("Producer %d finished\n", producer_id);
    return NULL;
}

void *consumer_thread(void *arg) {
    int consumer_id = *(int *)arg;
    
    while (1) {
        pthread_mutex_lock(&buffer.mutex);
        
        // Wait while buffer is empty
        while (buffer.count == 0) {
            safe_printf("Consumer %d waiting - buffer empty\n", consumer_id);
            pthread_cond_wait(&buffer.not_empty, &buffer.mutex);
        }
        
        // Consume item
        int item = buffer.items[buffer.out];
        buffer.out = (buffer.out + 1) % BUFFER_SIZE;
        buffer.count--;
        
        safe_printf("Consumer %d consumed item %d (buffer count: %d)\n", 
                   consumer_id, item, buffer.count);
        
        // Signal that buffer is not full
        pthread_cond_signal(&buffer.not_full);
        pthread_mutex_unlock(&buffer.mutex);
        
        // Simulate consumption time
        usleep(150000 + (rand() % 250000)); // 0.15-0.4 seconds
    }
    
    return NULL;
}

void demonstrate_semaphores(void) {
    printf("Demonstrating semaphore synchronization...\n");
    
    pthread_t threads[6];
    int thread_ids[6];
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    // Create threads that use semaphores
    for (int i = 0; i < 6; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, semaphore_worker, &thread_ids[i]) != 0) {
            perror("Failed to create semaphore worker thread");
            return;
        }
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < 6; i++) {
        pthread_join(threads[i], NULL);
    }
    
    gettimeofday(&end, NULL);
    printf("Semaphore demonstration completed\n");
    printf("Execution time: %.2f ms\n", get_time_diff(start, end));
}

void *semaphore_worker(void *arg) {
    int thread_id = *(int *)arg;
    
    if (thread_id < 3) {
        // Use binary semaphore (acts like mutex)
        safe_printf("Thread %d waiting for binary semaphore\n", thread_id);
        sem_wait(binary_semaphore);
        
        safe_printf("Thread %d acquired binary semaphore\n", thread_id);
        sleep(1); // Simulate critical section work
        safe_printf("Thread %d releasing binary semaphore\n", thread_id);
        
        sem_post(binary_semaphore);
    } else {
        // Use counting semaphore (max 2 concurrent access)
        safe_printf("Thread %d waiting for counting semaphore\n", thread_id);
        sem_wait(counting_semaphore);
        
        safe_printf("Thread %d acquired counting semaphore\n", thread_id);
        sleep(2); // Simulate resource usage
        safe_printf("Thread %d releasing counting semaphore\n", thread_id);
        
        sem_post(counting_semaphore);
    }
    
    return NULL;
}

void demonstrate_reader_writer_locks(void) {
    printf("Demonstrating reader-writer locks...\n");
    
    pthread_t readers[NUM_READERS];
    pthread_t writers[NUM_WRITERS];
    int reader_ids[NUM_READERS];
    int writer_ids[NUM_WRITERS];
    
    // Initialize shared data
    shared_data.data = 0;
    shared_data.reader_count = 0;
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    // Create reader threads
    for (int i = 0; i < NUM_READERS; i++) {
        reader_ids[i] = i;
        if (pthread_create(&readers[i], NULL, reader_thread, &reader_ids[i]) != 0) {
            perror("Failed to create reader thread");
            return;
        }
    }
    
    // Create writer threads
    for (int i = 0; i < NUM_WRITERS; i++) {
        writer_ids[i] = i;
        if (pthread_create(&writers[i], NULL, writer_thread, &writer_ids[i]) != 0) {
            perror("Failed to create writer thread");
            return;
        }
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
    
    for (int i = 0; i < NUM_WRITERS; i++) {
        pthread_join(writers[i], NULL);
    }
    
    gettimeofday(&end, NULL);
    printf("Reader-writer demonstration completed\n");
    printf("Final shared data value: %d\n", shared_data.data);
    printf("Execution time: %.2f ms\n", get_time_diff(start, end));
}

void *reader_thread(void *arg) {
    int reader_id = *(int *)arg;
    
    for (int i = 0; i < 3; i++) {
        // Acquire read lock
        pthread_rwlock_rdlock(&shared_data.rwlock);
        
        // Update reader count (protected by separate mutex)
        pthread_mutex_lock(&shared_data.reader_count_mutex);
        shared_data.reader_count++;
        int current_readers = shared_data.reader_count;
        pthread_mutex_unlock(&shared_data.reader_count_mutex);
        
        // Read shared data
        int data_value = shared_data.data;
        safe_printf("Reader %d reading data: %d (concurrent readers: %d)\n", 
                   reader_id, data_value, current_readers);
        
        // Simulate reading time
        usleep(500000); // 0.5 seconds
        
        // Update reader count
        pthread_mutex_lock(&shared_data.reader_count_mutex);
        shared_data.reader_count--;
        pthread_mutex_unlock(&shared_data.reader_count_mutex);
        
        // Release read lock
        pthread_rwlock_unlock(&shared_data.rwlock);
        
        // Wait before next read
        usleep(200000); // 0.2 seconds
    }
    
    safe_printf("Reader %d finished\n", reader_id);
    return NULL;
}

void *writer_thread(void *arg) {
    int writer_id = *(int *)arg;
    
    for (int i = 0; i < 2; i++) {
        // Acquire write lock (exclusive)
        safe_printf("Writer %d waiting for write lock\n", writer_id);
        pthread_rwlock_wrlock(&shared_data.rwlock);
        
        // Write to shared data
        shared_data.data += (writer_id + 1) * 10;
        safe_printf("Writer %d writing data: %d\n", writer_id, shared_data.data);
        
        // Simulate writing time
        sleep(1);
        
        // Release write lock
        pthread_rwlock_unlock(&shared_data.rwlock);
        safe_printf("Writer %d released write lock\n", writer_id);
        
        // Wait before next write
        sleep(1);
    }
    
    safe_printf("Writer %d finished\n", writer_id);
    return NULL;
}

// Global mutexes for deadlock demonstration
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;

void demonstrate_deadlock_prevention(void) {
    printf("Demonstrating deadlock prevention with ordered locking...\n");
    
    pthread_t thread_a, thread_b;
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    // Create threads that acquire locks in different orders
    if (pthread_create(&thread_a, NULL, deadlock_thread_a, NULL) != 0) {
        perror("Failed to create deadlock thread A");
        return;
    }
    
    if (pthread_create(&thread_b, NULL, deadlock_thread_b, NULL) != 0) {
        perror("Failed to create deadlock thread B");
        return;
    }
    
    // Wait for threads to complete
    pthread_join(thread_a, NULL);
    pthread_join(thread_b, NULL);
    
    gettimeofday(&end, NULL);
    printf("Deadlock prevention demonstration completed\n");
    printf("Execution time: %.2f ms\n", get_time_diff(start, end));
}

void *deadlock_thread_a(void *arg) {
    (void)arg;  // Suppress unused parameter warning
    safe_printf("Thread A acquiring locks in order: mutex_1, mutex_2\n");
    
    // Always acquire mutexes in the same order to prevent deadlock
    pthread_mutex_lock(&mutex_1);
    safe_printf("Thread A acquired mutex_1\n");
    
    sleep(1); // Simulate some work
    
    pthread_mutex_lock(&mutex_2);
    safe_printf("Thread A acquired mutex_2\n");
    
    // Critical section
    safe_printf("Thread A in critical section\n");
    sleep(1);
    
    // Release in reverse order
    pthread_mutex_unlock(&mutex_2);
    safe_printf("Thread A released mutex_2\n");
    
    pthread_mutex_unlock(&mutex_1);
    safe_printf("Thread A released mutex_1\n");
    
    return NULL;
}

void *deadlock_thread_b(void *arg) {
    (void)arg;  // Suppress unused parameter warning
    safe_printf("Thread B acquiring locks in order: mutex_1, mutex_2\n");
    
    // Acquire mutexes in the same order as thread A to prevent deadlock
    pthread_mutex_lock(&mutex_1);
    safe_printf("Thread B acquired mutex_1\n");
    
    sleep(1); // Simulate some work
    
    pthread_mutex_lock(&mutex_2);
    safe_printf("Thread B acquired mutex_2\n");
    
    // Critical section
    safe_printf("Thread B in critical section\n");
    sleep(1);
    
    // Release in reverse order
    pthread_mutex_unlock(&mutex_2);
    safe_printf("Thread B released mutex_2\n");
    
    pthread_mutex_unlock(&mutex_1);
    safe_printf("Thread B released mutex_1\n");
    
    return NULL;
}

// Utility functions
void safe_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    pthread_mutex_lock(&print_mutex);
    vprintf(format, args);
    fflush(stdout);
    pthread_mutex_unlock(&print_mutex);
    
    va_end(args);
}

void initialize_synchronization_objects(void) {
    // Initialize mutex and condition variables for buffer
    if (pthread_mutex_init(&buffer.mutex, NULL) != 0 ||
        pthread_cond_init(&buffer.not_full, NULL) != 0 ||
        pthread_cond_init(&buffer.not_empty, NULL) != 0) {
        perror("Failed to initialize buffer synchronization objects");
        exit(1);
    }
    
    // Initialize reader-writer lock
    if (pthread_rwlock_init(&shared_data.rwlock, NULL) != 0 ||
        pthread_mutex_init(&shared_data.reader_count_mutex, NULL) != 0) {
        perror("Failed to initialize reader-writer synchronization objects");
        exit(1);
    }
    
    // Initialize semaphores (using named semaphores for macOS compatibility)
    binary_semaphore = sem_open("/binary_sem", O_CREAT | O_EXCL, 0644, 1);
    if (binary_semaphore == SEM_FAILED) {
        sem_unlink("/binary_sem");  // Remove if already exists
        binary_semaphore = sem_open("/binary_sem", O_CREAT, 0644, 1);
    }
    
    counting_semaphore = sem_open("/counting_sem", O_CREAT | O_EXCL, 0644, 2);
    if (counting_semaphore == SEM_FAILED) {
        sem_unlink("/counting_sem");  // Remove if already exists
        counting_semaphore = sem_open("/counting_sem", O_CREAT, 0644, 2);
    }
    
    if (binary_semaphore == SEM_FAILED || counting_semaphore == SEM_FAILED) {
        perror("Failed to initialize semaphores");
        exit(1);
    }
}

void cleanup_synchronization_objects(void) {
    // Cleanup buffer synchronization objects
    pthread_mutex_destroy(&buffer.mutex);
    pthread_cond_destroy(&buffer.not_full);
    pthread_cond_destroy(&buffer.not_empty);
    
    // Cleanup reader-writer synchronization objects
    pthread_rwlock_destroy(&shared_data.rwlock);
    pthread_mutex_destroy(&shared_data.reader_count_mutex);
    
    // Cleanup semaphores
    if (binary_semaphore != NULL) {
        sem_close(binary_semaphore);
        sem_unlink("/binary_sem");
    }
    if (counting_semaphore != NULL) {
        sem_close(counting_semaphore);
        sem_unlink("/counting_sem");
    }
    
    // Cleanup other mutexes
    pthread_mutex_destroy(&counter_mutex);
    pthread_mutex_destroy(&print_mutex);
    pthread_mutex_destroy(&mutex_1);
    pthread_mutex_destroy(&mutex_2);
}

double get_time_diff(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) * 1000.0 + 
           (end.tv_usec - start.tv_usec) / 1000.0;
} 