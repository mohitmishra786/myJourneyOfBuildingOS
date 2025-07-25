# Peterson's Solution and Hardware Support

Solving the critical section problem requires mechanisms that ensure mutual exclusion while avoiding deadlock and starvation. Peterson's algorithm represents an elegant software-only solution for two processes, demonstrating fundamental synchronization principles without relying on special hardware instructions. However, modern systems predominantly use hardware-assisted synchronization primitives for better performance and scalability.

## Peterson's Algorithm Foundation

Gary Peterson developed this algorithm in 1981 as a proof that software-only mutual exclusion is possible for two processes. The algorithm uses only ordinary read and write operations on shared variables, making no assumptions about atomic operations or special hardware support.

The algorithm employs two shared variables to coordinate access between processes:

```c
// Shared variables
bool flag[2] = {false, false};  // Intent to enter critical section
int turn = 0;                   // Whose turn to enter
```

Each process follows a specific protocol when attempting to enter its critical section:

```c
// Process i (where i is 0 or 1, j is 1-i)
void process_i() {
    while (true) {
        // Entry section
        flag[i] = true;        // Declare intent
        turn = j;              // Give turn to other process
        while (flag[j] && turn == j) {
            // Busy wait
        }
        
        // Critical section
        // ... perform critical work ...
        
        // Exit section
        flag[i] = false;       // No longer need access
        
        // Remainder section
        // ... other work ...
    }
}
```

The algorithm's correctness relies on a careful interplay between these variables. A process first declares its intent to enter the critical section by setting its flag to true. Then, it graciously offers the turn to the other process. If both processes want to enter simultaneously, the turn variable acts as a tiebreaker.

## Correctness Analysis

Peterson's algorithm satisfies all three requirements for a critical section solution:

**Mutual Exclusion**: Suppose both processes execute in their critical sections simultaneously. For this to happen, both processes must have passed their while loop conditions. This means:
- For process 0: `!(flag[1] && turn == 1)` must be true
- For process 1: `!(flag[0] && turn == 0)` must be true

However, if both processes are attempting entry, both flags are true. The turn variable can only have one value (either 0 or 1), so at least one process must find `turn` equal to its opponent's index, forcing it to wait.

**Progress**: If no process is in the critical section and some process wishes to enter, deadlock cannot occur. If only one process wants to enter, its opponent's flag is false, allowing immediate entry. If both want to enter, the turn variable ensures one proceeds.

**Bounded Waiting**: A process waits at most one turn before entering its critical section. Once a process exits, it sets its flag to false, allowing the waiting process to proceed regardless of the turn value.

```plantuml
participant "Process 0" as P0
participant "Shared Memory" as SM
participant "Process 1" as P1

P0 -> SM: flag[0] = true
P0 -> SM: turn = 1
P0 -> SM: read flag[1] (true)
P0 -> SM: read turn (1)
note over P0: Wait in loop

P1 -> SM: flag[1] = true  
P1 -> SM: turn = 0
P1 -> SM: read flag[0] (true)
P1 -> SM: read turn (0)
note over P1: Enter critical section

P1 -> SM: flag[1] = false
note over P1: Exit critical section

P0 -> SM: read flag[1] (false)
note over P0: Enter critical section
```

## Memory Ordering Considerations

Peterson's algorithm assumes sequential consistency, where all memory operations appear to execute in some sequential order that respects the program order of each individual process. Modern processors with relaxed memory models can reorder operations in ways that violate this assumption.

Consider what happens if the processor reorders the two writes in the entry section:

```c
// Original order
flag[i] = true;
turn = j;

// Reordered by processor
turn = j;
flag[i] = true;
```

This reordering can break mutual exclusion. If both processes experience this reordering, they might both see the other's flag as false when checking the while condition, allowing both to enter the critical section simultaneously.

Modern implementations require memory barriers or fence instructions to prevent such reorderings:

```c
// With memory barriers
flag[i] = true;
memory_barrier();
turn = j;
memory_barrier();
while (flag[j] && turn == j) {
    // busy wait
}
```

## Hardware Atomic Instructions

While Peterson's algorithm demonstrates theoretical possibility, practical systems rely on hardware-provided atomic instructions for better performance and simpler programming models. These instructions execute indivisibly, appearing to complete in a single step from other processors' perspectives.

### Test-and-Set Lock

The test-and-set instruction atomically reads a memory location and sets it to a new value, returning the old value. This primitive enables simple lock implementations:

```c
// Hardware instruction: atomically returns old value and sets new value
bool test_and_set(bool *target) {
    bool old_value = *target;
    *target = true;
    return old_value;
}

// Lock implementation
typedef struct {
    bool locked;
} spinlock_t;

void acquire_lock(spinlock_t *lock) {
    while (test_and_set(&lock->locked)) {
        // Spin until lock is acquired
    }
}

void release_lock(spinlock_t *lock) {
    lock->locked = false;
}
```

Test-and-set provides mutual exclusion but doesn't guarantee fairness. A process might acquire and release a lock repeatedly while other processes continue spinning, leading to potential starvation.

### Compare-and-Swap

Compare-and-swap (CAS) provides more flexible atomic operations by comparing a memory location with an expected value and updating it only if they match:

```c
// Returns true if swap occurred
bool compare_and_swap(int *ptr, int expected, int new_value) {
    if (*ptr == expected) {
        *ptr = new_value;
        return true;
    }
    return false;
}

// Lock-free counter increment
void atomic_increment(int *counter) {
    int current, new_value;
    do {
        current = *counter;
        new_value = current + 1;
    } while (!compare_and_swap(counter, current, new_value));
}
```

CAS enables lock-free programming where threads coordinate without traditional locks. However, the ABA problem can occur when a value changes from A to B and back to A between a thread's read and CAS operation.

### Fetch-and-Add

Fetch-and-add atomically increments a memory location and returns the previous value. This instruction enables efficient implementation of atomic counters and ticket locks:

```c
// Ticket lock using fetch-and-add
typedef struct {
    int next_ticket;
    int now_serving;
} ticket_lock_t;

void acquire_ticket_lock(ticket_lock_t *lock) {
    int my_ticket = fetch_and_add(&lock->next_ticket, 1);
    while (lock->now_serving != my_ticket) {
        // Wait for our turn
        cpu_relax(); // Hint to processor for power efficiency
    }
}

void release_ticket_lock(ticket_lock_t *lock) {
    lock->now_serving++;
}
```

Ticket locks provide fairness guarantees that simple test-and-set locks lack. Processes acquire tickets in order and are served in the same sequence, preventing starvation.

## Memory Barriers and Fencing

Modern processors execute instructions out of order to maximize performance, but this reordering can break synchronization algorithms. Memory barriers (also called memory fences) provide ordering constraints that prevent certain reorderings.

Different types of barriers serve specific purposes:

**Load Barrier**: Ensures all load operations before the barrier complete before any load operations after the barrier.

**Store Barrier**: Ensures all store operations before the barrier complete before any store operations after the barrier.

**Full Barrier**: Combines load and store barriers, ensuring all memory operations before the barrier complete before any operations after.

```c
// x86-64 memory barrier implementations
static inline void load_barrier() {
    asm volatile("lfence" ::: "memory");
}

static inline void store_barrier() {
    asm volatile("sfence" ::: "memory");
}

static inline void memory_barrier() {
    asm volatile("mfence" ::: "memory");
}
```

## Cache Coherency Protocols

Hardware cache coherency protocols ensure that multiple processors maintain consistent views of shared memory. The MESI protocol (Modified, Exclusive, Shared, Invalid) represents one common approach:

```plantuml
state Modified
state Exclusive  
state Shared
state Invalid

Invalid --> Exclusive: Read miss\n(no other copies)
Invalid --> Shared: Read miss\n(other copies exist)
Exclusive --> Modified: Write hit
Exclusive --> Shared: Other processor\nreads
Shared --> Modified: Write hit
Shared --> Invalid: Other processor\nwrites
Modified --> Shared: Other processor\nreads
Modified --> Invalid: Other processor\nwrites
```

Each cache line exists in one of four states:
- **Modified**: The cache has the only valid copy and it has been modified
- **Exclusive**: The cache has the only valid copy and it matches memory
- **Shared**: Multiple caches have valid copies
- **Invalid**: The cache line is not valid

When a processor writes to a shared cache line, the coherency protocol invalidates copies in other caches, ensuring consistency but potentially causing performance penalties.

## Lock-Free Programming Principles

Lock-free programming eliminates traditional locks in favor of atomic operations and careful algorithm design. These approaches can provide better performance and avoid issues like deadlock and priority inversion:

```c
// Lock-free stack implementation
typedef struct node {
    int data;
    struct node *next;
} node_t;

typedef struct {
    node_t *head;
} lock_free_stack_t;

void push(lock_free_stack_t *stack, int value) {
    node_t *new_node = malloc(sizeof(node_t));
    new_node->data = value;
    
    do {
        new_node->next = stack->head;
    } while (!compare_and_swap(&stack->head, new_node->next, new_node));
}

int pop(lock_free_stack_t *stack) {
    node_t *head;
    do {
        head = stack->head;
        if (head == NULL) return -1; // Empty stack
    } while (!compare_and_swap(&stack->head, head, head->next));
    
    int value = head->data;
    free(head); // Note: This can cause ABA problems in practice
    return value;
}
```

Lock-free algorithms require careful consideration of memory management. The ABA problem becomes particularly relevant when nodes are freed and reallocated, potentially causing corruption if pointers are reused.

## Performance Characteristics

Different synchronization mechanisms exhibit varying performance characteristics depending on contention levels and system architecture:

**Low Contention**: Simple atomic operations often outperform locks due to reduced overhead. Lock-free algorithms excel when conflicts are rare.

**High Contention**: Locks with proper backoff strategies can outperform busy-waiting atomic operations. Ticket locks provide fairness but may suffer from cache line bouncing.

**NUMA Systems**: On Non-Uniform Memory Access systems, lock placement becomes crucial. Locks should reside in memory close to the processors most likely to access them.

## Practical Considerations

Real-world synchronization involves trade-offs between correctness, performance, and complexity. Peterson's algorithm, while theoretically elegant, suffers from busy-waiting that wastes CPU cycles. Modern systems prefer blocking synchronization primitives that allow the operating system to schedule other work.

Hardware support continues evolving with new atomic instructions and memory ordering models. Languages increasingly provide memory model specifications that define how concurrent operations behave, helping developers write portable synchronization code.

Understanding both classical algorithms like Peterson's solution and modern hardware primitives provides the foundation for implementing efficient synchronization mechanisms. The principles demonstrated in Peterson's algorithm - using flags for intent and turn variables for conflict resolution - appear in many sophisticated synchronization protocols.

The journey from software-only solutions to hardware-assisted primitives illustrates the continuous evolution of computer systems toward better performance while maintaining correctness guarantees. Modern synchronization builds upon these fundamental concepts while leveraging hardware capabilities to achieve efficiency at scale. 