/**
 * POSIX Threads (Pthread) Programming Demonstration
 * 
 * This program demonstrates advanced pthread programming concepts:
 * - Thread attributes and customization
 * - Thread-specific data (TSD)
 * - Signal handling in multithreaded programs
 * - Thread cancellation and cleanup
 * - Real-time scheduling
 * - Thread performance optimization
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sched.h>
#include <stdarg.h>

#define NUM_THREADS 5
#define CUSTOM_STACK_SIZE (2 * 1024 * 1024) // 2MB
#define TSD_ITERATIONS 3

// Thread-specific data key
pthread_key_t tsd_key;

// Global synchronization objects
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_once_t init_once = PTHREAD_ONCE_INIT;

// Thread data structure
typedef struct {
    int thread_id;
    char *thread_name;
    int priority;
    size_t stack_size;
    int detached;
    void *specific_data;
} thread_info_t;

// TSD data structure
typedef struct {
    int counter;
    char buffer[256];
    pthread_t thread_id;
} tsd_data_t;

// Function prototypes
void demonstrate_thread_attributes(void);
void demonstrate_thread_specific_data(void);
void demonstrate_signal_handling(void);
void demonstrate_thread_cancellation(void);
void demonstrate_realtime_scheduling(void);

// Thread function prototypes
void *attribute_demo_thread(void *arg);
void *tsd_demo_thread(void *arg);
void *signal_demo_thread(void *arg);
void *cancellation_demo_thread(void *arg);
void *realtime_demo_thread(void *arg);

// Utility functions
void safe_printf(const char *format, ...);
void init_tsd_key(void);
void tsd_destructor(void *data);
void cleanup_handler(void *arg);
void signal_handler(int sig);

int main() {
    printf("=== POSIX Threads Programming Demonstration ===\n\n");
    
    printf("1. Thread Attributes Demonstration\n");
    demonstrate_thread_attributes();
    
    printf("\n2. Thread-Specific Data Demonstration\n");
    demonstrate_thread_specific_data();
    
    printf("\n3. Signal Handling Demonstration\n");
    demonstrate_signal_handling();
    
    printf("\n4. Thread Cancellation Demonstration\n");
    demonstrate_thread_cancellation();
    
    printf("\n5. Real-time Scheduling Demonstration\n");
    demonstrate_realtime_scheduling();
    
    // Cleanup global resources
    pthread_key_delete(tsd_key);
    pthread_mutex_destroy(&print_mutex);
    
    return 0;
}

void demonstrate_thread_attributes(void) {
    printf("Creating threads with custom attributes...\n");
    
    pthread_t threads[3];
    pthread_attr_t attr[3];
    thread_info_t thread_data[3];
    
    // Initialize thread data
    for (int i = 0; i < 3; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].thread_name = malloc(32);
        snprintf(thread_data[i].thread_name, 32, "CustomThread-%d", i);
        thread_data[i].priority = 0;
        thread_data[i].detached = (i == 2) ? 1 : 0; // Last thread is detached
    }
    
    // Create threads with different attributes
    for (int i = 0; i < 3; i++) {
        int result = pthread_attr_init(&attr[i]);
        if (result != 0) {
            fprintf(stderr, "Failed to initialize attributes: %s\n", strerror(result));
            continue;
        }
        
        // Set stack size for first thread
        if (i == 0) {
            result = pthread_attr_setstacksize(&attr[i], CUSTOM_STACK_SIZE);
            if (result != 0) {
                safe_printf("Failed to set stack size: %s\n", strerror(result));
            } else {
                thread_data[i].stack_size = CUSTOM_STACK_SIZE;
                safe_printf("Thread %d: Custom stack size set to %zu bytes\n", 
                           i, CUSTOM_STACK_SIZE);
            }
        }
        
        // Set detached state for last thread
        if (i == 2) {
            result = pthread_attr_setdetachstate(&attr[i], PTHREAD_CREATE_DETACHED);
            if (result != 0) {
                safe_printf("Failed to set detached state: %s\n", strerror(result));
            } else {
                safe_printf("Thread %d: Set to detached state\n", i);
            }
        }
        
        // Create thread
        result = pthread_create(&threads[i], &attr[i], attribute_demo_thread, &thread_data[i]);
        if (result != 0) {
            fprintf(stderr, "Failed to create thread %d: %s\n", i, strerror(result));
        } else {
            safe_printf("Created thread %d with custom attributes\n", i);
        }
        
        // Clean up attributes
        pthread_attr_destroy(&attr[i]);
    }
    
    // Wait for joinable threads (not the detached one)
    for (int i = 0; i < 2; i++) {
        void *retval;
        int result = pthread_join(threads[i], &retval);
        if (result != 0) {
            fprintf(stderr, "Failed to join thread %d: %s\n", i, strerror(result));
        } else {
            safe_printf("Thread %d completed and joined\n", i);
        }
    }
    
    // Wait a bit for detached thread to complete
    sleep(2);
    
    // Cleanup thread data
    for (int i = 0; i < 3; i++) {
        free(thread_data[i].thread_name);
    }
    
    printf("Thread attributes demonstration completed\n");
}

void *attribute_demo_thread(void *arg) {
    thread_info_t *info = (thread_info_t *)arg;
    
    safe_printf("Thread %s starting execution\n", info->thread_name);
    
    // Get and display thread attributes (Linux-specific)
#ifdef __linux__
    pthread_attr_t attr;
    size_t stack_size;
    int detach_state;
    
    pthread_getattr_np(pthread_self(), &attr);
    pthread_attr_getstacksize(&attr, &stack_size);
    pthread_attr_getdetachstate(&attr, &detach_state);
    
    safe_printf("Thread %s: Stack size = %zu, Detached = %s\n",
               info->thread_name,
               stack_size,
               (detach_state == PTHREAD_CREATE_DETACHED) ? "Yes" : "No");
    
    pthread_attr_destroy(&attr);
#else
    safe_printf("Thread %s: Running (attribute introspection not available on this platform)\n",
               info->thread_name);
#endif
    
    // Simulate some work
    for (int i = 0; i < 5; i++) {
        safe_printf("Thread %s: Working... iteration %d\n", info->thread_name, i + 1);
        usleep(200000); // 0.2 seconds
    }
    
    safe_printf("Thread %s completing\n", info->thread_name);
    
    return NULL;
}

void demonstrate_thread_specific_data(void) {
    printf("Demonstrating thread-specific data (TSD)...\n");
    
    // Initialize TSD key once
    pthread_once(&init_once, init_tsd_key);
    
    pthread_t threads[3];
    int thread_ids[3];
    
    // Create threads that use TSD
    for (int i = 0; i < 3; i++) {
        thread_ids[i] = i;
        int result = pthread_create(&threads[i], NULL, tsd_demo_thread, &thread_ids[i]);
        if (result != 0) {
            fprintf(stderr, "Failed to create TSD thread %d: %s\n", i, strerror(result));
        }
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Thread-specific data demonstration completed\n");
}

void init_tsd_key(void) {
    int result = pthread_key_create(&tsd_key, tsd_destructor);
    if (result != 0) {
        fprintf(stderr, "Failed to create TSD key: %s\n", strerror(result));
        exit(1);
    }
    safe_printf("TSD key created successfully\n");
}

void tsd_destructor(void *data) {
    if (data) {
        tsd_data_t *tsd = (tsd_data_t *)data;
        safe_printf("TSD destructor called for thread %lu (counter = %d)\n",
                   (unsigned long)tsd->thread_id, tsd->counter);
        free(data);
    }
}

void *tsd_demo_thread(void *arg) {
    int thread_id = *(int *)arg;
    
    // Allocate and initialize thread-specific data
    tsd_data_t *tsd = malloc(sizeof(tsd_data_t));
    if (!tsd) {
        safe_printf("Thread %d: Failed to allocate TSD\n", thread_id);
        return NULL;
    }
    
    tsd->counter = 0;
    tsd->thread_id = pthread_self();
    snprintf(tsd->buffer, sizeof(tsd->buffer), "Thread-%d-Data", thread_id);
    
    // Associate data with key
    int result = pthread_setspecific(tsd_key, tsd);
    if (result != 0) {
        safe_printf("Thread %d: Failed to set specific data: %s\n", 
                   thread_id, strerror(result));
        free(tsd);
        return NULL;
    }
    
    safe_printf("Thread %d: TSD initialized\n", thread_id);
    
    // Use thread-specific data
    for (int i = 0; i < TSD_ITERATIONS; i++) {
        tsd_data_t *my_tsd = (tsd_data_t *)pthread_getspecific(tsd_key);
        if (my_tsd) {
            my_tsd->counter++;
            safe_printf("Thread %d: TSD counter = %d, buffer = %s\n",
                       thread_id, my_tsd->counter, my_tsd->buffer);
        }
        usleep(300000); // 0.3 seconds
    }
    
    safe_printf("Thread %d: TSD demonstration completed\n", thread_id);
    return NULL;
}

void demonstrate_signal_handling(void) {
    printf("Demonstrating signal handling in multithreaded programs...\n");
    
    // Set up signal handler
    signal(SIGUSR1, signal_handler);
    
    pthread_t threads[2];
    int thread_ids[2];
    
    // Block SIGUSR1 for all threads initially
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
    
    // Create threads
    for (int i = 0; i < 2; i++) {
        thread_ids[i] = i;
        int result = pthread_create(&threads[i], NULL, signal_demo_thread, &thread_ids[i]);
        if (result != 0) {
            fprintf(stderr, "Failed to create signal demo thread %d: %s\n", 
                   i, strerror(result));
        }
    }
    
    // Wait a bit, then send signals
    sleep(1);
    safe_printf("Main thread sending SIGUSR1 to process...\n");
    
    // Unblock SIGUSR1 for thread 0 only
    pthread_kill(threads[0], SIGUSR1);
    
    // Wait for threads to complete
    for (int i = 0; i < 2; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Signal handling demonstration completed\n");
}

void *signal_demo_thread(void *arg) {
    int thread_id = *(int *)arg;
    
    if (thread_id == 0) {
        // This thread will handle signals
        sigset_t set;
        int sig;
        
        sigemptyset(&set);
        sigaddset(&set, SIGUSR1);
        
        safe_printf("Thread %d: Waiting for SIGUSR1...\n", thread_id);
        
        int result = sigwait(&set, &sig);
        if (result == 0) {
            safe_printf("Thread %d: Received signal %d (SIGUSR1)\n", thread_id, sig);
        } else {
            safe_printf("Thread %d: sigwait failed: %s\n", thread_id, strerror(result));
        }
    } else {
        // This thread keeps SIGUSR1 blocked
        safe_printf("Thread %d: Running with SIGUSR1 blocked\n", thread_id);
        
        for (int i = 0; i < 5; i++) {
            safe_printf("Thread %d: Working... iteration %d\n", thread_id, i + 1);
            usleep(400000); // 0.4 seconds
        }
    }
    
    return NULL;
}

void signal_handler(int sig) {
    // Note: This handler won't be called in our demo since we use sigwait
    safe_printf("Signal handler called with signal %d\n", sig);
}

void demonstrate_thread_cancellation(void) {
    printf("Demonstrating thread cancellation and cleanup...\n");
    
    pthread_t cancellable_thread;
    int thread_data = 42;
    
    // Create a cancellable thread
    int result = pthread_create(&cancellable_thread, NULL, 
                               cancellation_demo_thread, &thread_data);
    if (result != 0) {
        fprintf(stderr, "Failed to create cancellable thread: %s\n", strerror(result));
        return;
    }
    
    // Let the thread run for a while
    sleep(2);
    
    // Cancel the thread
    safe_printf("Main thread: Cancelling thread...\n");
    result = pthread_cancel(cancellable_thread);
    if (result != 0) {
        fprintf(stderr, "Failed to cancel thread: %s\n", strerror(result));
    }
    
    // Wait for the thread to complete
    void *retval;
    result = pthread_join(cancellable_thread, &retval);
    if (result != 0) {
        fprintf(stderr, "Failed to join cancelled thread: %s\n", strerror(result));
    } else {
        if (retval == PTHREAD_CANCELED) {
            safe_printf("Thread was successfully cancelled\n");
        } else {
            safe_printf("Thread completed normally with return value: %p\n", retval);
        }
    }
    
    printf("Thread cancellation demonstration completed\n");
}

void *cancellation_demo_thread(void *arg) {
    int *data = (int *)arg;
    
    safe_printf("Cancellable thread starting (data = %d)\n", *data);
    
    // Set cancellation type to asynchronous for demonstration
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    
    // Register cleanup handler
    char *resource = malloc(256);
    if (resource) {
        strcpy(resource, "Important resource that needs cleanup");
        pthread_cleanup_push(cleanup_handler, resource);
        
        safe_printf("Cancellable thread: Cleanup handler registered\n");
        
        // Simulate long-running work
        for (int i = 0; i < 100; i++) {
            safe_printf("Cancellable thread: Working... iteration %d\n", i + 1);
            usleep(100000); // 0.1 seconds
            
            // Test cancellation point
            pthread_testcancel();
        }
        
        // This cleanup pop won't execute if thread is cancelled
        pthread_cleanup_pop(1); // 1 = execute cleanup
    }
    
    safe_printf("Cancellable thread completed normally\n");
    return NULL;
}

void cleanup_handler(void *arg) {
    char *resource = (char *)arg;
    safe_printf("Cleanup handler: Freeing resource: %s\n", resource);
    free(resource);
}

void demonstrate_realtime_scheduling(void) {
    printf("Demonstrating real-time scheduling...\n");
    
    // Check if we have permission for real-time scheduling
    int max_priority = sched_get_priority_max(SCHED_FIFO);
    int min_priority = sched_get_priority_min(SCHED_FIFO);
    
    if (max_priority == -1 || min_priority == -1) {
        printf("Real-time scheduling not available or insufficient permissions\n");
        printf("Run as root or with appropriate capabilities for real-time scheduling\n");
        return;
    }
    
    safe_printf("SCHED_FIFO priority range: %d - %d\n", min_priority, max_priority);
    
    pthread_t rt_thread;
    pthread_attr_t attr;
    struct sched_param param;
    int thread_data = 1;
    
    // Initialize attributes
    int result = pthread_attr_init(&attr);
    if (result != 0) {
        fprintf(stderr, "Failed to initialize RT attributes: %s\n", strerror(result));
        return;
    }
    
    // Set scheduling policy to FIFO
    result = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    if (result != 0) {
        safe_printf("Failed to set SCHED_FIFO: %s\n", strerror(result));
        pthread_attr_destroy(&attr);
        return;
    }
    
    // Set priority
    param.sched_priority = min_priority + 1;
    result = pthread_attr_setschedparam(&attr, &param);
    if (result != 0) {
        safe_printf("Failed to set RT priority: %s\n", strerror(result));
        pthread_attr_destroy(&attr);
        return;
    }
    
    // Set explicit scheduling (don't inherit from creating thread)
    result = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (result != 0) {
        safe_printf("Failed to set explicit scheduling: %s\n", strerror(result));
        pthread_attr_destroy(&attr);
        return;
    }
    
    // Create real-time thread
    result = pthread_create(&rt_thread, &attr, realtime_demo_thread, &thread_data);
    if (result != 0) {
        if (result == EPERM) {
            safe_printf("Permission denied for real-time scheduling\n");
            safe_printf("Try running as root or with CAP_SYS_NICE capability\n");
        } else {
            fprintf(stderr, "Failed to create RT thread: %s\n", strerror(result));
        }
        pthread_attr_destroy(&attr);
        return;
    }
    
    safe_printf("Real-time thread created successfully\n");
    
    // Wait for thread to complete
    pthread_join(rt_thread, NULL);
    
    // Cleanup
    pthread_attr_destroy(&attr);
    
    printf("Real-time scheduling demonstration completed\n");
}

void *realtime_demo_thread(void *arg) {
    int *data = (int *)arg;
    
    // Get and display scheduling information
    int policy;
    struct sched_param param;
    
    int result = pthread_getschedparam(pthread_self(), &policy, &param);
    if (result == 0) {
        const char *policy_name;
        switch (policy) {
            case SCHED_FIFO: policy_name = "SCHED_FIFO"; break;
            case SCHED_RR: policy_name = "SCHED_RR"; break;
            case SCHED_OTHER: policy_name = "SCHED_OTHER"; break;
            default: policy_name = "UNKNOWN"; break;
        }
        
        safe_printf("RT Thread: Policy = %s, Priority = %d\n",
                   policy_name, param.sched_priority);
    }
    
    // Simulate real-time work with precise timing
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for (int i = 0; i < 10; i++) {
        safe_printf("RT Thread: Time-critical operation %d\n", i + 1);
        
        // Simulate deterministic work
        volatile int sum = 0;
        for (int j = 0; j < 100000; j++) {
            sum += j;
        }
        
        usleep(50000); // 50ms
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    long elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000L + 
                     (end.tv_nsec - start.tv_nsec);
    
    safe_printf("RT Thread: Completed in %ld nanoseconds\n", elapsed_ns);
    safe_printf("RT Thread: Data value = %d\n", *data);
    
    return NULL;
}

// Utility function for thread-safe printing
void safe_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    pthread_mutex_lock(&print_mutex);
    vprintf(format, args);
    fflush(stdout);
    pthread_mutex_unlock(&print_mutex);
    
    va_end(args);
} 