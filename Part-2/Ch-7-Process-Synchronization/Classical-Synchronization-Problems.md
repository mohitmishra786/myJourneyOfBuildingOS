# Classical Synchronization Problems

Classical synchronization problems serve as benchmarks for evaluating the effectiveness and correctness of synchronization mechanisms. These problems capture common patterns that appear repeatedly in concurrent systems, from operating system kernels to distributed applications. Understanding their solutions provides insight into the fundamental principles of concurrent programming and helps developers recognize similar patterns in real-world scenarios.

## The Producer Consumer Problem

The producer-consumer problem models situations where one or more processes generate data while other processes consume that data. This pattern appears in numerous contexts: compiler phases passing data through pipelines, web servers handling requests, and multimedia applications processing audio or video streams.

The basic setup involves producers that create items and place them in a shared buffer, consumers that remove items from the buffer for processing, and the need to coordinate access to prevent buffer overflow or underflow while maintaining efficiency.

```plantuml
class Buffer {
  +items: array[BUFFER_SIZE]
  +count: int
  +in: int
  +out: int
  +produce(item): void
  +consume(): item
}

class Producer {
  +run(): void
}

class Consumer {
  +run(): void
}

Producer --> Buffer: writes to
Consumer --> Buffer: reads from
Buffer o-- "1" Semaphore: empty_slots
Buffer o-- "1" Semaphore: full_slots  
Buffer o-- "1" Mutex: buffer_lock
```

The challenge lies in ensuring that producers don't add items to a full buffer and consumers don't remove items from an empty buffer, while allowing multiple producers and consumers to operate concurrently when possible.

### Semaphore-Based Solution

The semaphore solution uses three synchronization primitives: two counting semaphores to track available slots and filled slots, plus a mutex for atomic buffer access.

```c
#define BUFFER_SIZE 10

typedef struct {
    int buffer[BUFFER_SIZE];
    int in;
    int out;
} shared_buffer_t;

shared_buffer_t buffer = {.in = 0, .out = 0};
semaphore_t empty_slots, full_slots, mutex;

void init_producer_consumer() {
    semaphore_init(&empty_slots, BUFFER_SIZE);
    semaphore_init(&full_slots, 0);
    semaphore_init(&mutex, 1);
}

void producer_process() {
    while (true) {
        int item = produce_item();
        
        semaphore_wait(&empty_slots);
        semaphore_wait(&mutex);
        
        buffer.buffer[buffer.in] = item;
        buffer.in = (buffer.in + 1) % BUFFER_SIZE;
        
        semaphore_signal(&mutex);
        semaphore_signal(&full_slots);
    }
}

void consumer_process() {
    while (true) {
        semaphore_wait(&full_slots);
        semaphore_wait(&mutex);
        
        int item = buffer.buffer[buffer.out];
        buffer.out = (buffer.out + 1) % BUFFER_SIZE;
        
        semaphore_signal(&mutex);
        semaphore_signal(&empty_slots);
        
        consume_item(item);
    }
}
```

The critical insight is the ordering of semaphore operations. Producers must acquire empty_slots before mutex to avoid deadlock when the buffer is full. Similarly, consumers must acquire full_slots before mutex when the buffer is empty.

### Multiple Producers and Consumers

The solution scales naturally to multiple producers and consumers. The semaphores automatically coordinate between multiple instances, ensuring that the total number of items produced equals the number consumed, regardless of the interleaving of operations.

```c
void create_producer_consumer_system() {
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    
    init_producer_consumer();
    
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_create(&producers[i], NULL, producer_process, NULL);
    }
    
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_create(&consumers[i], NULL, consumer_process, NULL);
    }
}
```

## The Readers Writers Problem

The readers-writers problem addresses scenarios where data can be safely read by multiple processes simultaneously but requires exclusive access for writing. This pattern is fundamental in database systems, file systems, and cache implementations.

The problem has several variants depending on priority policies. The first readers-writers problem gives priority to readers, potentially starving writers. The second variant prioritizes writers, and the third attempts to provide fairness to both.

### First Readers Writers Problem

In this variant, readers have priority. As long as at least one reader is active, new readers can join immediately while writers must wait. This can lead to writer starvation if readers arrive frequently.

```c
typedef struct {
    int readers_count;
    semaphore_t read_count_mutex;
    semaphore_t write_mutex;
} rw_lock_t;

void rw_init(rw_lock_t *rw) {
    rw->readers_count = 0;
    semaphore_init(&rw->read_count_mutex, 1);
    semaphore_init(&rw->write_mutex, 1);
}

void start_read(rw_lock_t *rw) {
    semaphore_wait(&rw->read_count_mutex);
    rw->readers_count++;
    if (rw->readers_count == 1) {
        semaphore_wait(&rw->write_mutex);
    }
    semaphore_signal(&rw->read_count_mutex);
}

void end_read(rw_lock_t *rw) {
    semaphore_wait(&rw->read_count_mutex);
    rw->readers_count--;
    if (rw->readers_count == 0) {
        semaphore_signal(&rw->write_mutex);
    }
    semaphore_signal(&rw->read_count_mutex);
}

void start_write(rw_lock_t *rw) {
    semaphore_wait(&rw->write_mutex);
}

void end_write(rw_lock_t *rw) {
    semaphore_signal(&rw->write_mutex);
}
```

### Second Readers Writers Problem

The second variant prioritizes writers by preventing new readers from starting once a writer is waiting. This solution prevents writer starvation but can starve readers.

```c
typedef struct {
    int readers_count;
    int writers_count;
    int waiting_writers;
    semaphore_t read_try;
    semaphore_t write_mutex;
    semaphore_t read_count_mutex;
    semaphore_t write_count_mutex;
} fair_rw_lock_t;

void fair_rw_init(fair_rw_lock_t *rw) {
    rw->readers_count = 0;
    rw->writers_count = 0;
    rw->waiting_writers = 0;
    semaphore_init(&rw->read_try, 1);
    semaphore_init(&rw->write_mutex, 1);
    semaphore_init(&rw->read_count_mutex, 1);
    semaphore_init(&rw->write_count_mutex, 1);
}

void fair_start_read(fair_rw_lock_t *rw) {
    semaphore_wait(&rw->read_try);
    semaphore_wait(&rw->read_count_mutex);
    
    rw->readers_count++;
    if (rw->readers_count == 1) {
        semaphore_wait(&rw->write_mutex);
    }
    
    semaphore_signal(&rw->read_count_mutex);
    semaphore_signal(&rw->read_try);
}

void fair_start_write(fair_rw_lock_t *rw) {
    semaphore_wait(&rw->write_count_mutex);
    rw->waiting_writers++;
    if (rw->waiting_writers == 1) {
        semaphore_wait(&rw->read_try);
    }
    semaphore_signal(&rw->write_count_mutex);
    
    semaphore_wait(&rw->write_mutex);
    
    semaphore_wait(&rw->write_count_mutex);
    rw->waiting_writers--;
    rw->writers_count++;
    semaphore_signal(&rw->write_count_mutex);
}

void fair_end_write(fair_rw_lock_t *rw) {
    semaphore_wait(&rw->write_count_mutex);
    rw->writers_count--;
    if (rw->writers_count == 0 && rw->waiting_writers == 0) {
        semaphore_signal(&rw->read_try);
    }
    semaphore_signal(&rw->write_count_mutex);
    
    semaphore_signal(&rw->write_mutex);
}
```

## The Bounded Buffer Problem

The bounded buffer problem generalizes the producer-consumer pattern to handle finite buffer capacity efficiently. This problem appears in operating system I/O subsystems, network protocol stacks, and parallel processing pipelines.

```plantuml
participant "Producer 1" as P1
participant "Producer 2" as P2
participant "Bounded Buffer" as BB
participant "Consumer 1" as C1
participant "Consumer 2" as C2

P1 -> BB: wait(empty)
P1 -> BB: wait(mutex)
P1 -> BB: add item
P1 -> BB: signal(mutex)
P1 -> BB: signal(full)

C1 -> BB: wait(full)
C1 -> BB: wait(mutex)
C1 -> BB: remove item
C1 -> BB: signal(mutex)
C1 -> BB: signal(empty)

P2 -> BB: wait(empty)
note right: May block if buffer full

C2 -> BB: wait(full)
note right: May block if buffer empty
```

### Advanced Buffer Management

Real systems often require more sophisticated buffer management with features like priority queues, batch processing, or overflow handling.

```c
typedef struct {
    int *buffer;
    int capacity;
    int count;
    int head, tail;
    semaphore_t empty_slots;
    semaphore_t full_slots;
    mutex_t buffer_mutex;
    bool overflow_policy_drop;
} advanced_buffer_t;

bool buffer_produce_with_policy(advanced_buffer_t *buf, int item) {
    if (buf->overflow_policy_drop) {
        if (semaphore_try_wait(&buf->empty_slots) != 0) {
            return false; // Drop item if buffer full
        }
    } else {
        semaphore_wait(&buf->empty_slots);
    }
    
    mutex_lock(&buf->buffer_mutex);
    
    buf->buffer[buf->tail] = item;
    buf->tail = (buf->tail + 1) % buf->capacity;
    buf->count++;
    
    mutex_unlock(&buf->buffer_mutex);
    semaphore_signal(&buf->full_slots);
    
    return true;
}

int buffer_consume_batch(advanced_buffer_t *buf, int *items, int max_items) {
    int consumed = 0;
    
    while (consumed < max_items) {
        if (semaphore_try_wait(&buf->full_slots) != 0) {
            break; // No more items available
        }
        
        mutex_lock(&buf->buffer_mutex);
        
        items[consumed] = buf->buffer[buf->head];
        buf->head = (buf->head + 1) % buf->capacity;
        buf->count--;
        consumed++;
        
        mutex_unlock(&buf->buffer_mutex);
        semaphore_signal(&buf->empty_slots);
    }
    
    return consumed;
}
```

## The Sleeping Barber Problem

The sleeping barber problem illustrates resource allocation and queuing scenarios. A barber shop has one barber, one barber chair, and a waiting room with a limited number of chairs. Customers arriving to a full waiting room leave immediately.

```c
#define WAITING_CHAIRS 5

typedef struct {
    semaphore_t customers;        // Number of customers waiting
    semaphore_t barber;          // 0 or 1: is barber available?
    semaphore_t waiting_mutex;   // Protects waiting room access
    int waiting_customers;
} barbershop_t;

void barbershop_init(barbershop_t *shop) {
    semaphore_init(&shop->customers, 0);
    semaphore_init(&shop->barber, 0);
    semaphore_init(&shop->waiting_mutex, 1);
    shop->waiting_customers = 0;
}

void barber_process(barbershop_t *shop) {
    while (true) {
        semaphore_wait(&shop->customers);
        semaphore_wait(&shop->waiting_mutex);
        
        shop->waiting_customers--;
        semaphore_signal(&shop->barber);
        semaphore_signal(&shop->waiting_mutex);
        
        cut_hair();
    }
}

bool customer_process(barbershop_t *shop) {
    semaphore_wait(&shop->waiting_mutex);
    
    if (shop->waiting_customers < WAITING_CHAIRS) {
        shop->waiting_customers++;
        semaphore_signal(&shop->customers);
        semaphore_signal(&shop->waiting_mutex);
        
        semaphore_wait(&shop->barber);
        get_haircut();
        return true;
    } else {
        semaphore_signal(&shop->waiting_mutex);
        return false; // Leave if no chairs available
    }
}
```

## The Cigarette Smokers Problem

This problem demonstrates synchronization between processes with different resource requirements. Three smokers each have unlimited supplies of one ingredient (tobacco, paper, or matches), and a dealer alternately provides two of the three ingredients.

```c
typedef enum {
    TOBACCO_PAPER,
    TOBACCO_MATCHES,
    PAPER_MATCHES
} ingredient_pair_t;

typedef struct {
    semaphore_t tobacco_smoker;
    semaphore_t paper_smoker;
    semaphore_t matches_smoker;
    semaphore_t dealer;
} smokers_sync_t;

void smokers_init(smokers_sync_t *sync) {
    semaphore_init(&sync->tobacco_smoker, 0);
    semaphore_init(&sync->paper_smoker, 0);
    semaphore_init(&sync->matches_smoker, 0);
    semaphore_init(&sync->dealer, 1);
}

void dealer_process(smokers_sync_t *sync) {
    while (true) {
        semaphore_wait(&sync->dealer);
        
        ingredient_pair_t pair = random_ingredient_pair();
        
        switch (pair) {
            case TOBACCO_PAPER:
                semaphore_signal(&sync->matches_smoker);
                break;
            case TOBACCO_MATCHES:
                semaphore_signal(&sync->paper_smoker);
                break;
            case PAPER_MATCHES:
                semaphore_signal(&sync->tobacco_smoker);
                break;
        }
    }
}

void tobacco_smoker_process(smokers_sync_t *sync) {
    while (true) {
        semaphore_wait(&sync->tobacco_smoker);
        smoke_cigarette();
        semaphore_signal(&sync->dealer);
    }
}
```

## Resource Allocation Patterns

These classical problems reveal common patterns in resource allocation and coordination:

**Mutual Exclusion**: Ensuring exclusive access to shared resources while allowing concurrent access when safe.

**Resource Pooling**: Managing a finite set of identical resources among multiple consumers.

**Producer Consumer Coordination**: Synchronizing data flow between different processing stages.

**Priority and Fairness**: Balancing throughput with equitable resource access.

### Performance Analysis

Each solution has different performance characteristics under varying load conditions:

```c
typedef struct {
    unsigned long operations_completed;
    unsigned long total_wait_time;
    unsigned long max_queue_length;
    unsigned long deadlock_count;
} performance_metrics_t;

void analyze_synchronization_performance(performance_metrics_t *metrics) {
    double average_wait_time = (double)metrics->total_wait_time / 
                              metrics->operations_completed;
    double throughput = metrics->operations_completed / MEASUREMENT_PERIOD;
    
    printf("Average wait time: %.2f ms\n", average_wait_time);
    printf("Throughput: %.2f operations/second\n", throughput);
    printf("Max queue length: %lu\n", metrics->max_queue_length);
    printf("Deadlocks detected: %lu\n", metrics->deadlock_count);
}
```

## Modern Variations and Extensions

Contemporary systems extend these classical problems with additional requirements:

**Real-time Constraints**: Adding deadline requirements and priority scheduling.

**Fault Tolerance**: Handling process failures and recovery scenarios.

**Distributed Systems**: Extending synchronization across network boundaries.

**Load Balancing**: Dynamically adjusting resource allocation based on demand.

```c
typedef struct {
    int priority;
    timestamp_t deadline;
    void *data;
} priority_item_t;

typedef struct {
    priority_item_t *items;
    int capacity;
    int count;
    mutex_t lock;
    condition_t not_empty;
    condition_t not_full;
} priority_buffer_t;

void priority_produce(priority_buffer_t *buf, priority_item_t item) {
    mutex_lock(&buf->lock);
    
    while (buf->count == buf->capacity) {
        condition_wait(&buf->not_full, &buf->lock);
    }
    
    // Insert in priority order
    int pos = buf->count;
    while (pos > 0 && buf->items[pos-1].priority < item.priority) {
        buf->items[pos] = buf->items[pos-1];
        pos--;
    }
    buf->items[pos] = item;
    buf->count++;
    
    condition_signal(&buf->not_empty);
    mutex_unlock(&buf->lock);
}
```

These classical synchronization problems provide a foundation for understanding more complex coordination scenarios. They demonstrate fundamental principles that apply across different domains and scales, from single-machine concurrency to distributed system coordination. Mastering these patterns enables developers to recognize similar structures in new problems and apply proven solution techniques.

The evolution of these problems from theoretical exercises to practical implementation patterns reflects the maturation of concurrent programming. Modern systems build upon these foundations while adding features like timeout handling, priority support, and fault tolerance to meet contemporary application requirements. 