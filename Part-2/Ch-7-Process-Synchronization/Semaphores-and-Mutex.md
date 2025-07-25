# Semaphores and Mutex

High-level synchronization primitives provide abstractions that simplify concurrent programming while maintaining efficiency and correctness. Semaphores and mutexes represent two fundamental building blocks that enable developers to coordinate access to shared resources without dealing with low-level atomic operations directly. These primitives form the foundation for more complex synchronization mechanisms and appear throughout modern operating systems.

## Semaphore Fundamentals

Edsger Dijkstra introduced semaphores in 1965 as a generalization of binary locks. A semaphore maintains an integer value that represents the number of resources available or the number of processes that can proceed simultaneously. The semaphore provides two atomic operations that manipulate this value safely.

The wait operation (originally called P from the Dutch "proberen" meaning "to try") decrements the semaphore value. If the result becomes negative, the calling process blocks until another process signals the semaphore. The signal operation (originally called V from "verhogen" meaning "to increase") increments the semaphore value and potentially wakes up a waiting process.

```c
typedef struct {
    int value;
    queue_t *waiting_queue;
    spinlock_t lock;
} semaphore_t;

void semaphore_wait(semaphore_t *sem) {
    acquire_lock(&sem->lock);
    sem->value--;
    if (sem->value < 0) {
        enqueue(sem->waiting_queue, current_process);
        release_lock(&sem->lock);
        block_process();
    } else {
        release_lock(&sem->lock);
    }
}

void semaphore_signal(semaphore_t *sem) {
    acquire_lock(&sem->lock);
    sem->value++;
    if (sem->value <= 0) {
        process_t *waiting = dequeue(sem->waiting_queue);
        if (waiting != NULL) {
            wakeup_process(waiting);
        }
    }
    release_lock(&sem->lock);
}
```

The semaphore's power lies in its flexibility. When initialized to 1, it functions as a binary semaphore or mutex, allowing mutual exclusion. When initialized to a larger value, it controls access to multiple instances of a resource, such as connections in a connection pool or buffers in a bounded buffer.

## Binary Semaphores vs Counting Semaphores

Binary semaphores, also known as mutexes, can only hold values 0 or 1. They provide mutual exclusion for critical sections where only one process should execute at a time. The binary nature simplifies reasoning about correctness and enables certain optimizations.

```c
// Binary semaphore initialization
semaphore_t mutex;
semaphore_init(&mutex, 1);

// Critical section protection
void critical_section() {
    semaphore_wait(&mutex);
    // Critical section code
    shared_resource++;
    semaphore_signal(&mutex);
}
```

Counting semaphores can hold any non-negative integer value, representing the number of available resources. They naturally model scenarios where multiple processes can access different instances of the same resource type simultaneously.

```c
// Counting semaphore for resource pool
semaphore_t resource_pool;
semaphore_init(&resource_pool, 5); // 5 available resources

void use_resource() {
    semaphore_wait(&resource_pool);  // Acquire resource
    // Use one of the 5 available resources
    perform_work_with_resource();
    semaphore_signal(&resource_pool); // Release resource
}
```

## Producer Consumer Problem Solution

The producer-consumer problem demonstrates semaphore utility in coordinating between processes that generate and consume data. Multiple producers create items and place them in a shared buffer, while consumers remove and process items. The solution requires three semaphores to handle different aspects of the synchronization.

```plantuml
participant Producer
participant Buffer
participant Consumer

Producer -> Buffer: Wait for empty slots
Producer -> Buffer: Wait for mutex
Producer -> Buffer: Add item
Producer -> Buffer: Signal mutex
Producer -> Buffer: Signal full slots

Consumer -> Buffer: Wait for full slots  
Consumer -> Buffer: Wait for mutex
Consumer -> Buffer: Remove item
Consumer -> Buffer: Signal mutex
Consumer -> Buffer: Signal empty slots
```

The complete solution uses semaphores to track empty slots, full slots, and provide mutual exclusion for buffer access:

```c
#define BUFFER_SIZE 10

typedef struct {
    int buffer[BUFFER_SIZE];
    int in, out;
} circular_buffer_t;

circular_buffer_t shared_buffer = {.in = 0, .out = 0};
semaphore_t empty_slots, full_slots, mutex;

void initialize_semaphores() {
    semaphore_init(&empty_slots, BUFFER_SIZE);
    semaphore_init(&full_slots, 0);
    semaphore_init(&mutex, 1);
}

void producer() {
    while (true) {
        int item = produce_item();
        
        semaphore_wait(&empty_slots);
        semaphore_wait(&mutex);
        
        shared_buffer.buffer[shared_buffer.in] = item;
        shared_buffer.in = (shared_buffer.in + 1) % BUFFER_SIZE;
        
        semaphore_signal(&mutex);
        semaphore_signal(&full_slots);
    }
}

void consumer() {
    while (true) {
        semaphore_wait(&full_slots);
        semaphore_wait(&mutex);
        
        int item = shared_buffer.buffer[shared_buffer.out];
        shared_buffer.out = (shared_buffer.out + 1) % BUFFER_SIZE;
        
        semaphore_signal(&mutex);
        semaphore_signal(&empty_slots);
        
        consume_item(item);
    }
}
```

The ordering of semaphore operations is crucial. Producers must check for empty slots before acquiring the mutex to avoid deadlock situations where all buffer slots are full but the producer holds the mutex, preventing consumers from making progress.

## Mutex Implementation and Variants

While binary semaphores can provide mutual exclusion, dedicated mutex implementations often include additional features like ownership tracking and priority inheritance. These features address specific problems that arise in real-world systems.

### Basic Mutex Implementation

A simple mutex tracks whether it's currently locked and maintains a queue of waiting processes:

```c
typedef struct {
    bool locked;
    process_t *owner;
    queue_t *waiting_queue;
    spinlock_t internal_lock;
} mutex_t;

void mutex_lock(mutex_t *mtx) {
    acquire_lock(&mtx->internal_lock);
    
    if (!mtx->locked) {
        mtx->locked = true;
        mtx->owner = current_process;
        release_lock(&mtx->internal_lock);
    } else {
        enqueue(mtx->waiting_queue, current_process);
        release_lock(&mtx->internal_lock);
        block_process();
    }
}

void mutex_unlock(mutex_t *mtx) {
    acquire_lock(&mtx->internal_lock);
    
    if (mtx->owner != current_process) {
        // Error: trying to unlock mutex not owned by current process
        release_lock(&mtx->internal_lock);
        return;
    }
    
    process_t *next = dequeue(mtx->waiting_queue);
    if (next != NULL) {
        mtx->owner = next;
        wakeup_process(next);
    } else {
        mtx->locked = false;
        mtx->owner = NULL;
    }
    
    release_lock(&mtx->internal_lock);
}
```

### Priority Inheritance

Priority inversion occurs when a high-priority process waits for a mutex held by a low-priority process, while a medium-priority process prevents the low-priority process from running. Priority inheritance temporarily raises the mutex owner's priority to that of the highest-priority waiting process.

```c
typedef struct {
    bool locked;
    process_t *owner;
    int original_priority;
    priority_queue_t *waiting_queue; // Ordered by priority
    spinlock_t internal_lock;
} priority_mutex_t;

void priority_mutex_lock(priority_mutex_t *mtx) {
    acquire_lock(&mtx->internal_lock);
    
    if (!mtx->locked) {
        mtx->locked = true;
        mtx->owner = current_process;
        mtx->original_priority = current_process->priority;
    } else {
        priority_enqueue(mtx->waiting_queue, current_process);
        
        // Priority inheritance
        if (current_process->priority > mtx->owner->priority) {
            set_process_priority(mtx->owner, current_process->priority);
        }
        
        release_lock(&mtx->internal_lock);
        block_process();
        return;
    }
    
    release_lock(&mtx->internal_lock);
}

void priority_mutex_unlock(priority_mutex_t *mtx) {
    acquire_lock(&mtx->internal_lock);
    
    process_t *next = priority_dequeue(mtx->waiting_queue);
    if (next != NULL) {
        // Transfer ownership
        mtx->owner = next;
        mtx->original_priority = next->priority;
        wakeup_process(next);
    } else {
        // Restore original priority
        set_process_priority(mtx->owner, mtx->original_priority);
        mtx->locked = false;
        mtx->owner = NULL;
    }
    
    release_lock(&mtx->internal_lock);
}
```

## Read-Write Locks

Read-write locks optimize scenarios where data is read frequently but modified infrequently. Multiple readers can access shared data simultaneously, but writers require exclusive access. This approach significantly improves performance for read-heavy workloads.

```plantuml
state "No Access" as NA
state "Readers Active" as RA  
state "Writer Active" as WA

NA --> RA: First Reader
NA --> WA: Writer Request
RA --> RA: Additional Reader
RA --> NA: Last Reader Exits
RA --> WA: Writer Waits,\nLast Reader Exits
WA --> NA: Writer Exits
WA --> RA: Writer Exits,\nReaders Waiting
WA --> WA: Writer Exits,\nWriter Waiting
```

A typical read-write lock implementation uses semaphores and counters to track active readers and coordinate with writers:

```c
typedef struct {
    int readers_count;
    semaphore_t read_mutex;    // Protects readers_count
    semaphore_t write_mutex;   // Provides writer mutual exclusion
    semaphore_t reader_queue;  // Prevents reader starvation
} rwlock_t;

void rwlock_init(rwlock_t *rw) {
    rw->readers_count = 0;
    semaphore_init(&rw->read_mutex, 1);
    semaphore_init(&rw->write_mutex, 1);
    semaphore_init(&rw->reader_queue, 1);
}

void read_lock(rwlock_t *rw) {
    semaphore_wait(&rw->reader_queue);
    semaphore_wait(&rw->read_mutex);
    
    rw->readers_count++;
    if (rw->readers_count == 1) {
        semaphore_wait(&rw->write_mutex); // First reader blocks writers
    }
    
    semaphore_signal(&rw->read_mutex);
    semaphore_signal(&rw->reader_queue);
}

void read_unlock(rwlock_t *rw) {
    semaphore_wait(&rw->read_mutex);
    
    rw->readers_count--;
    if (rw->readers_count == 0) {
        semaphore_signal(&rw->write_mutex); // Last reader unblocks writers
    }
    
    semaphore_signal(&rw->read_mutex);
}

void write_lock(rwlock_t *rw) {
    semaphore_wait(&rw->write_mutex);
}

void write_unlock(rwlock_t *rw) {
    semaphore_signal(&rw->write_mutex);
}
```

## Semaphore Implementation Strategies

Operating systems implement semaphores using different strategies depending on performance requirements and system characteristics. The choice affects both correctness and performance under various workload patterns.

### Blocking Implementation

The traditional blocking implementation puts waiting processes to sleep, allowing the operating system to schedule other work. This approach works well for long critical sections but incurs overhead from context switches.

### Spinning Implementation

Spinning implementations busy-wait instead of blocking, consuming CPU cycles but avoiding context switch overhead. This approach suits short critical sections and low-latency requirements but wastes resources when contention is high.

```c
typedef struct {
    volatile int value;
    volatile int waiting_count;
} spin_semaphore_t;

void spin_semaphore_wait(spin_semaphore_t *sem) {
    while (true) {
        if (atomic_decrement(&sem->value) >= 0) {
            return; // Successfully acquired
        }
        
        atomic_increment(&sem->waiting_count);
        while (sem->value <= 0) {
            cpu_pause(); // Hint to processor for power efficiency
        }
        atomic_decrement(&sem->waiting_count);
    }
}

void spin_semaphore_signal(spin_semaphore_t *sem) {
    atomic_increment(&sem->value);
}
```

### Hybrid Implementation

Adaptive implementations combine blocking and spinning strategies, spinning briefly before blocking. This approach provides low latency for short waits while avoiding resource waste for longer waits.

## Performance Considerations

Semaphore and mutex performance depends heavily on contention patterns and system characteristics. Understanding these factors helps choose appropriate synchronization strategies for specific applications.

**Cache Line Bouncing**: When multiple processors frequently access the same semaphore, the cache line containing the semaphore bounces between caches, creating performance bottlenecks. Techniques like exponential backoff and padding can mitigate this issue.

**False Sharing**: Multiple semaphores in the same cache line can cause false sharing, where updates to one semaphore invalidate the cache line for other semaphores. Proper alignment and padding prevent this problem.

**Thundering Herd**: When many processes wait on the same semaphore, signaling can wake all waiters simultaneously, causing excessive contention. Fair queuing and batch processing can address this issue.

## POSIX Synchronization Primitives

POSIX defines standard APIs for semaphores and mutexes that provide portability across different operating systems. These APIs include features like timeouts, error handling, and inter-process synchronization.

```c
#include <pthread.h>
#include <semaphore.h>

// POSIX mutex usage
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void critical_section() {
    pthread_mutex_lock(&mutex);
    // Critical section
    pthread_mutex_unlock(&mutex);
}

// POSIX semaphore usage
sem_t semaphore;

void initialize() {
    sem_init(&semaphore, 0, 1); // Initial value 1
}

void use_semaphore() {
    sem_wait(&semaphore);
    // Protected section
    sem_post(&semaphore);
}
```

Modern implementations provide additional features like robust mutexes that handle process termination gracefully and process-shared synchronization primitives for inter-process coordination.

Semaphores and mutexes represent essential tools in the concurrent programming toolkit. Their combination of simplicity and power makes them suitable for a wide range of synchronization problems, from simple mutual exclusion to complex coordination patterns. Understanding their implementation details and performance characteristics enables developers to make informed decisions about when and how to use these primitives effectively.

The evolution from basic binary semaphores to sophisticated priority-inheriting mutexes illustrates how operating systems continue to refine synchronization mechanisms to meet the demands of modern concurrent applications. These primitives serve as building blocks for higher-level synchronization constructs while remaining fundamental to system programming and kernel development. 