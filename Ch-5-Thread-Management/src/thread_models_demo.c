/**
 * Thread Models Demonstration
 * 
 * This program demonstrates different thread models including:
 * - User-level threads simulation
 * - Kernel-level threads using pthreads
 * - Thread pool implementation
 * - Performance comparison between models
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <semaphore.h>
#include <errno.h>
#include <stdarg.h>

#define MAX_THREADS 10
#define MAX_TASKS 1000
#define STACK_SIZE 8192

// Performance measurement structure
typedef struct {
    struct timeval start_time;
    struct timeval end_time;
    long operations_completed;
    long context_switches;
} performance_metrics_t;

// User-level thread simulation structure
typedef struct user_thread {
    int thread_id;
    void *stack;
    void *(*start_routine)(void *);
    void *arg;
    int state; // 0 = ready, 1 = running, 2 = blocked, 3 = terminated
    struct user_thread *next;
} user_thread_t;

// User-level thread scheduler
typedef struct {
    user_thread_t *ready_queue;
    user_thread_t *current_thread;
    int thread_count;
    performance_metrics_t metrics;
} user_scheduler_t;

// Thread pool structures
typedef struct task {
    void *(*function)(void *);
    void *argument;
    struct task *next;
} task_t;

typedef struct thread_pool {
    pthread_t *threads;
    task_t *task_queue;
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_cond;
    pthread_cond_t shutdown_cond;
    int thread_count;
    int active_threads;
    int shutdown;
    performance_metrics_t metrics;
} thread_pool_t;

// Global variables
user_scheduler_t user_scheduler = {0};
thread_pool_t *global_pool = NULL;

// Function prototypes
void demonstrate_user_level_threads(void);
void demonstrate_kernel_level_threads(void);
void demonstrate_thread_pool(void);
void compare_performance(void);

// User-level thread functions
user_thread_t* create_user_thread(void *(*start_routine)(void *), void *arg);
void schedule_user_threads(void);
void user_thread_yield(void);
void *user_thread_example_work(void *arg);

// Kernel-level thread functions
void *kernel_thread_example_work(void *arg);

// Thread pool functions
thread_pool_t* create_thread_pool(int num_threads);
void thread_pool_add_task(thread_pool_t *pool, void *(*function)(void *), void *argument);
void* thread_pool_worker(void *arg);
void destroy_thread_pool(thread_pool_t *pool);
void *thread_pool_example_work(void *arg);

// Utility functions
void start_timer(performance_metrics_t *metrics);
void end_timer(performance_metrics_t *metrics);
long get_elapsed_microseconds(performance_metrics_t *metrics);
void print_performance_report(const char *model_name, performance_metrics_t *metrics);

int main() {
    printf("=== Thread Models Demonstration ===\n\n");
    
    printf("1. User-level Threads Simulation\n");
    demonstrate_user_level_threads();
    
    printf("\n2. Kernel-level Threads (Pthreads)\n");
    demonstrate_kernel_level_threads();
    
    printf("\n3. Thread Pool Implementation\n");
    demonstrate_thread_pool();
    
    printf("\n4. Performance Comparison\n");
    compare_performance();
    
    return 0;
}

void demonstrate_user_level_threads(void) {
    printf("Creating user-level threads with cooperative scheduling...\n");
    
    start_timer(&user_scheduler.metrics);
    
    // Initialize user-level scheduler
    user_scheduler.ready_queue = NULL;
    user_scheduler.current_thread = NULL;
    user_scheduler.thread_count = 0;
    
    // Create multiple user threads
    for (int i = 0; i < 5; i++) {
        user_thread_t *thread = create_user_thread(user_thread_example_work, (void *)(long)i);
        if (thread) {
            user_scheduler.thread_count++;
            printf("Created user thread %d\n", i);
        }
    }
    
    // Run user-level scheduler
    printf("Starting user-level scheduler...\n");
    schedule_user_threads();
    
    end_timer(&user_scheduler.metrics);
    print_performance_report("User-level Threads", &user_scheduler.metrics);
}

user_thread_t* create_user_thread(void *(*start_routine)(void *), void *arg) {
    user_thread_t *thread = malloc(sizeof(user_thread_t));
    if (!thread) {
        perror("Failed to allocate user thread");
        return NULL;
    }
    
    thread->stack = malloc(STACK_SIZE);
    if (!thread->stack) {
        perror("Failed to allocate thread stack");
        free(thread);
        return NULL;
    }
    
    static int thread_id_counter = 0;
    thread->thread_id = thread_id_counter++;
    thread->start_routine = start_routine;
    thread->arg = arg;
    thread->state = 0; // ready
    thread->next = user_scheduler.ready_queue;
    user_scheduler.ready_queue = thread;
    
    return thread;
}

void schedule_user_threads(void) {
    int rounds = 0;
    const int max_rounds = 10;
    
    while (user_scheduler.ready_queue && rounds < max_rounds) {
        user_thread_t *current = user_scheduler.ready_queue;
        user_scheduler.ready_queue = current->next;
        user_scheduler.current_thread = current;
        
        current->state = 1; // running
        user_scheduler.metrics.context_switches++;
        
        printf("Scheduling user thread %d (round %d)\n", current->thread_id, rounds);
        
        // Simulate thread execution
        current->start_routine(current->arg);
        
        current->state = 3; // terminated
        free(current->stack);
        free(current);
        
        rounds++;
        user_scheduler.metrics.operations_completed++;
        
        // Simulate cooperative yield
        usleep(1000); // 1ms
    }
}

void *user_thread_example_work(void *arg) {
    int thread_id = (int)(long)arg;
    printf("User thread %d executing work...\n", thread_id);
    
    // Simulate some computational work
    volatile int sum = 0;
    for (int i = 0; i < 10000; i++) {
        sum += i;
    }
    
    printf("User thread %d completed work (sum = %d)\n", thread_id, sum);
    return NULL;
}

void demonstrate_kernel_level_threads(void) {
    printf("Creating kernel-level threads using pthreads...\n");
    
    performance_metrics_t metrics = {0};
    start_timer(&metrics);
    
    pthread_t threads[5];
    int thread_args[5];
    
    // Create kernel threads
    for (int i = 0; i < 5; i++) {
        thread_args[i] = i;
        int result = pthread_create(&threads[i], NULL, kernel_thread_example_work, &thread_args[i]);
        if (result != 0) {
            fprintf(stderr, "Error creating thread %d: %s\n", i, strerror(result));
            continue;
        }
        printf("Created kernel thread %d\n", i);
        metrics.operations_completed++;
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < 5; i++) {
        void *retval;
        int result = pthread_join(threads[i], &retval);
        if (result != 0) {
            fprintf(stderr, "Error joining thread %d: %s\n", i, strerror(result));
        } else {
            printf("Kernel thread %d completed\n", i);
        }
    }
    
    end_timer(&metrics);
    print_performance_report("Kernel-level Threads", &metrics);
}

void *kernel_thread_example_work(void *arg) {
    int thread_id = *(int *)arg;
    printf("Kernel thread %d executing work...\n", thread_id);
    
    // Simulate some computational work
    volatile int sum = 0;
    for (int i = 0; i < 10000; i++) {
        sum += i;
    }
    
    // Simulate I/O operation (demonstrates blocking behavior)
    usleep(1000 * (thread_id + 1)); // Variable sleep time
    
    printf("Kernel thread %d completed work (sum = %d)\n", thread_id, sum);
    return NULL;
}

void demonstrate_thread_pool(void) {
    printf("Creating thread pool with worker threads...\n");
    
    global_pool = create_thread_pool(3);
    if (!global_pool) {
        printf("Failed to create thread pool\n");
        return;
    }
    
    start_timer(&global_pool->metrics);
    
    // Submit tasks to thread pool
    for (int i = 0; i < 10; i++) {
        int *task_id = malloc(sizeof(int));
        *task_id = i;
        thread_pool_add_task(global_pool, thread_pool_example_work, task_id);
        printf("Submitted task %d to thread pool\n", i);
    }
    
    // Wait for all tasks to complete
    sleep(3);
    
    end_timer(&global_pool->metrics);
    print_performance_report("Thread Pool", &global_pool->metrics);
    
    destroy_thread_pool(global_pool);
}

thread_pool_t* create_thread_pool(int num_threads) {
    thread_pool_t *pool = malloc(sizeof(thread_pool_t));
    if (!pool) {
        perror("Failed to allocate thread pool");
        return NULL;
    }
    
    pool->threads = malloc(sizeof(pthread_t) * num_threads);
    if (!pool->threads) {
        perror("Failed to allocate thread array");
        free(pool);
        return NULL;
    }
    
    pool->task_queue = NULL;
    pool->thread_count = num_threads;
    pool->active_threads = 0;
    pool->shutdown = 0;
    
    // Initialize synchronization primitives
    if (pthread_mutex_init(&pool->queue_mutex, NULL) != 0 ||
        pthread_cond_init(&pool->queue_cond, NULL) != 0 ||
        pthread_cond_init(&pool->shutdown_cond, NULL) != 0) {
        perror("Failed to initialize synchronization primitives");
        free(pool->threads);
        free(pool);
        return NULL;
    }
    
    // Create worker threads
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&pool->threads[i], NULL, thread_pool_worker, pool) != 0) {
            perror("Failed to create worker thread");
            pool->shutdown = 1;
            destroy_thread_pool(pool);
            return NULL;
        }
        pool->active_threads++;
    }
    
    printf("Thread pool created with %d worker threads\n", num_threads);
    return pool;
}

void thread_pool_add_task(thread_pool_t *pool, void *(*function)(void *), void *argument) {
    if (!pool || pool->shutdown) {
        return;
    }
    
    task_t *task = malloc(sizeof(task_t));
    if (!task) {
        perror("Failed to allocate task");
        return;
    }
    
    task->function = function;
    task->argument = argument;
    task->next = NULL;
    
    pthread_mutex_lock(&pool->queue_mutex);
    
    // Add task to end of queue
    if (pool->task_queue == NULL) {
        pool->task_queue = task;
    } else {
        task_t *current = pool->task_queue;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = task;
    }
    
    pool->metrics.operations_completed++;
    pthread_cond_signal(&pool->queue_cond);
    pthread_mutex_unlock(&pool->queue_mutex);
}

void* thread_pool_worker(void *arg) {
    thread_pool_t *pool = (thread_pool_t *)arg;
    
    while (1) {
        pthread_mutex_lock(&pool->queue_mutex);
        
        // Wait for task or shutdown signal
        while (pool->task_queue == NULL && !pool->shutdown) {
            pthread_cond_wait(&pool->queue_cond, &pool->queue_mutex);
        }
        
        if (pool->shutdown) {
            pthread_mutex_unlock(&pool->queue_mutex);
            break;
        }
        
        // Get task from queue
        task_t *task = pool->task_queue;
        pool->task_queue = task->next;
        
        pthread_mutex_unlock(&pool->queue_mutex);
        
        // Execute task
        if (task->function) {
            task->function(task->argument);
        }
        
        free(task);
    }
    
    return NULL;
}

void *thread_pool_example_work(void *arg) {
    int task_id = *(int *)arg;
    printf("Thread pool executing task %d\n", task_id);
    
    // Simulate computational work
    volatile int sum = 0;
    for (int i = 0; i < 5000; i++) {
        sum += i * task_id;
    }
    
    // Simulate variable execution time
    usleep(500000 + (task_id % 3) * 100000); // 0.5-0.8 seconds
    
    printf("Thread pool completed task %d (sum = %d)\n", task_id, sum);
    
    free(arg); // Clean up task argument
    return NULL;
}

void destroy_thread_pool(thread_pool_t *pool) {
    if (!pool) {
        return;
    }
    
    pthread_mutex_lock(&pool->queue_mutex);
    pool->shutdown = 1;
    pthread_cond_broadcast(&pool->queue_cond);
    pthread_mutex_unlock(&pool->queue_mutex);
    
    // Wait for all worker threads to finish
    for (int i = 0; i < pool->active_threads; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    
    // Clean up remaining tasks
    while (pool->task_queue) {
        task_t *task = pool->task_queue;
        pool->task_queue = task->next;
        free(task);
    }
    
    // Destroy synchronization primitives
    pthread_mutex_destroy(&pool->queue_mutex);
    pthread_cond_destroy(&pool->queue_cond);
    pthread_cond_destroy(&pool->shutdown_cond);
    
    free(pool->threads);
    free(pool);
    
    printf("Thread pool destroyed\n");
}

void compare_performance(void) {
    printf("Performance comparison summary:\n");
    printf("Model                   | Operations | Time (μs) | Context Switches\n");
    printf("------------------------|------------|-----------|------------------\n");
    
    // Note: In a real implementation, you would collect these metrics
    // during actual execution. This is a simplified demonstration.
    
    printf("User-level Threads      |         5  |      2500 |               5\n");
    printf("Kernel-level Threads    |         5  |      8000 |              10\n");
    printf("Thread Pool             |        10  |     12000 |               3\n");
    
    printf("\nObservations:\n");
    printf("- User-level threads have lowest overhead but limited by blocking calls\n");
    printf("- Kernel-level threads provide true parallelism but higher overhead\n");
    printf("- Thread pools amortize creation costs across multiple tasks\n");
}

// Utility functions
void start_timer(performance_metrics_t *metrics) {
    gettimeofday(&metrics->start_time, NULL);
    metrics->operations_completed = 0;
    metrics->context_switches = 0;
}

void end_timer(performance_metrics_t *metrics) {
    gettimeofday(&metrics->end_time, NULL);
}

long get_elapsed_microseconds(performance_metrics_t *metrics) {
    return (metrics->end_time.tv_sec - metrics->start_time.tv_sec) * 1000000 +
           (metrics->end_time.tv_usec - metrics->start_time.tv_usec);
}

void print_performance_report(const char *model_name, performance_metrics_t *metrics) {
    long elapsed = get_elapsed_microseconds(metrics);
    printf("%s Performance:\n", model_name);
    printf("  Operations completed: %ld\n", metrics->operations_completed);
    printf("  Execution time: %ld μs\n", elapsed);
    printf("  Context switches: %ld\n", metrics->context_switches);
    if (metrics->operations_completed > 0) {
        printf("  Average time per operation: %ld μs\n", 
               elapsed / metrics->operations_completed);
    }
    printf("\n");
} 