# Real-Time Operating Systems: Predictable Performance

Real-Time Operating Systems (RTOS) are designed to handle time-critical tasks with guaranteed response times. Unlike general-purpose operating systems that optimize for average case performance, RTOS focuses on worst-case execution time and deterministic behavior.

## Core Requirements

Predictability: The system must guarantee response times for critical tasks.
Preemptability: Higher-priority tasks must be able to interrupt lower-priority ones.
Priority Inheritance: Mechanisms to prevent priority inversion.
Minimal Interrupt Latency: Fast and deterministic interrupt handling.

## Scheduling Paradigms

RTOS typically implements one or more of these scheduling approaches:

Rate Monotonic Scheduling (RMS):
• Fixed priority assignment based on period
• Shorter periods get higher priorities
• Optimal for periodic tasks
• Mathematically provable scheduling bounds

Earliest Deadline First (EDF):
• Dynamic priority based on absolute deadlines
• Optimal CPU utilization
• More complex implementation
• Higher runtime overhead

## Resource Management

RTOS resource management differs from traditional OS approaches:

Memory Management:
• Often uses static allocation
• Avoids paging and swapping
• Predictable memory access times
• Protected memory regions for critical tasks

I/O Management:
• Deterministic device drivers
• Bounded interrupt handling times
• Priority-based I/O queuing
• Direct memory access optimization

```c
/* Real-Time Operating System Core Implementation */

#include <stdint.h>
#include <stdbool.h>

#define MAX_TASKS 32
#define MAX_PRIORITY 32

/* Task Control Block */
typedef struct {
    uint32_t id;
    uint32_t priority;
    uint32_t period;
    uint32_t deadline;
    uint32_t execution_time;
    void (*task_function)(void);
    enum {
        TASK_READY,
        TASK_RUNNING,
        TASK_BLOCKED,
        TASK_SUSPENDED
    } state;
    uint32_t stack_pointer;
    uint32_t stack_base;
    uint32_t stack_size;
} task_t;

/* Scheduler State */
typedef struct {
    task_t* tasks[MAX_TASKS];
    uint32_t task_count;
    task_t* current_task;
    uint32_t system_time;
} scheduler_t;

/* Priority Queue Implementation */
typedef struct {
    task_t* tasks[MAX_TASKS];
    uint32_t size;
} priority_queue_t;

/* Global RTOS State */
static scheduler_t scheduler;
static priority_queue_t ready_queue;

/* Initialize Task */
task_t* create_task(void (*function)(void), uint32_t priority, 
                    uint32_t period, uint32_t deadline) {
    task_t* task = malloc(sizeof(task_t));
    task->id = get_next_task_id();
    task->priority = priority;
    task->period = period;
    task->deadline = deadline;
    task->task_function = function;
    task->state = TASK_READY;
    
    // Allocate stack
    task->stack_size = TASK_STACK_SIZE;
    task->stack_base = allocate_stack(task->stack_size);
    task->stack_pointer = initialize_stack(task->stack_base, task->stack_size);
    
    return task;
}

/* Priority Queue Operations */
void push_task(priority_queue_t* queue, task_t* task) {
    uint32_t i = queue->size++;
    while (i > 0) {
        uint32_t parent = (i - 1) / 2;
        if (queue->tasks[parent]->priority >= task->priority) {
            break;
        }
        queue->tasks[i] = queue->tasks[parent];
        i = parent;
    }
    queue->tasks[i] = task;
}

task_t* pop_task(priority_queue_t* queue) {
    if (queue->size == 0) {
        return NULL;
    }
    
    task_t* task = queue->tasks[0];
    queue->size--;
    
    task_t* last = queue->tasks[queue->size];
    uint32_t i = 0;
    while (true) {
        uint32_t left = 2 * i + 1;
        uint32_t right = 2 * i + 2;
        uint32_t highest = i;
        
        if (left < queue->size && 
            queue->tasks[left]->priority > queue->tasks[highest]->priority) {
            highest = left;
        }
        if (right < queue->size && 
            queue->tasks[right]->priority > queue->tasks[highest]->priority) {
            highest = right;
        }
        
        if (highest == i) {
            break;
        }
        
        queue->tasks[i] = queue->tasks[highest];
        i = highest;
    }
    queue->tasks[i] = last;
    
    return task;
}

/* Scheduler Implementation */
void schedule_next_task() {
    // Save context of current task if any
    if (scheduler.current_task) {
        save_context();
        if (scheduler.current_task->state == TASK_RUNNING) {
            scheduler.current_task->state = TASK_READY;
            push_task(&ready_queue, scheduler.current_task);
        }
    }
    
    // Get highest priority task
    scheduler.current_task = pop_task(&ready_queue);
    if (scheduler.current_task) {
        scheduler.current_task->state = TASK_RUNNING;
        restore_context(scheduler.current_task);
    }
}

/* Deadline Checking */
void check_deadlines() {
    uint32_t current_time = get_system_time();
    
    for (uint32_t i = 0; i < scheduler.task_count; i++) {
        task_t* task = scheduler.tasks[i];
        if (task->state != TASK_SUSPENDED) {
            if (current_time >= task->deadline) {
                handle_deadline_miss(task);
            }
        }
    }
}

/* Timer Interrupt Handler */
void timer_interrupt_handler() {
    scheduler.system_time++;
    
    // Check for tasks that need to be released
    for (uint32_t i = 0; i < scheduler.task_count; i++) {
        task_t* task = scheduler.tasks[i];
        if (task->state == TASK_SUSPENDED && 
            scheduler.system_time % task->period == 0) {
            task->state = TASK_READY;
            task->deadline = scheduler.system_time + task->period;
            push_task(&ready_queue, task);
        }
    }
    
    check_deadlines();
    schedule_next_task();
}

/* Mutex Implementation with Priority Inheritance */
typedef struct {
    bool locked;
    task_t* owner;
    uint32_t original_priority;
    priority_queue_t waiting_tasks;
} mutex_t;

int mutex_lock(mutex_t* mutex) {
    disable_interrupts();
    
    if (!mutex->locked) {
        mutex->locked = true;
        mutex->owner = scheduler.current_task;
        mutex->original_priority = mutex->owner->priority;
        enable_interrupts();
        return 0;
    }
    
    // Priority inheritance
    if (scheduler.current_task->priority > mutex->owner->priority) {
        mutex->owner->priority = scheduler.current_task->priority;
    }
    
    scheduler.current_task->state = TASK_BLOCKED;
    push_task(&mutex->waiting_tasks, scheduler.current_task);
    
    enable_interrupts();
    schedule_next_task();
    return 0;
}

void mutex_unlock(mutex_t* mutex) {
    disable_interrupts();
    
    if (mutex->owner == scheduler.current_task) {
        mutex->owner->priority = mutex->original_priority;
        mutex->locked = false;
        mutex->owner = NULL;
        
        // Wake up the highest priority task waiting for this mutex
        task_t* waiting_task = pop_task(&mutex->waiting_tasks);
        if (waiting_task) {
            waiting_task->state = TASK_READY;
            push_task(&ready_queue, waiting_task);
        }
        
        // If a task was unblocked, we might need to reschedule if its priority is higher
        if (waiting_task && waiting_task->priority > scheduler.current_task->priority) {
            enable_interrupts();
            schedule_next_task();  // This will switch to the higher priority task if necessary
            return;  // Ensure no further execution in this function after context switch
        }
    }
    
    enable_interrupts();
}

/* Context Management */
void save_context() {
    // Placeholder for context saving logic. This would typically involve
    // saving registers, program counter, etc., to the stack or a context structure.
    // The exact implementation depends on the architecture.
}

void restore_context(task_t* task) {
    // Restore the saved context of the task. This includes setting the stack pointer
    // and loading registers from where save_context stored them.
    // Implementation would be architecture-specific.
    scheduler.current_task = task;
    // Set stack pointer etc...
}

/* Helper Functions */
uint32_t get_next_task_id() {
    static uint32_t task_id_counter = 0;
    return task_id_counter++;
}

void* allocate_stack(size_t size) {
    // This function should allocate memory for the task's stack
    return malloc(size);
}

uint32_t initialize_stack(uint32_t stack_base, size_t stack_size) {
    // Initialize the stack with initial context, return the initial stack pointer
    // Here you would set up the initial stack frame for the task including return address, etc.
    return stack_base + stack_size;  // Simplified, real implementation would set initial context
}

uint32_t get_system_time() {
    // Return the current system time. This could be from a hardware timer or a software counter.
    return scheduler.system_time;
}

void handle_deadline_miss(task_t* task) {
    // Handle what happens when a task misses its deadline
    // This could involve logging, task suspension, or invoking a recovery mechanism
    printf("Deadline missed for task ID %u!\n", task->id);
    // Additional logic like task suspension or error handling would go here
}

void disable_interrupts() {
    // Implementation depends on the hardware; this is a placeholder
}

void enable_interrupts() {
    // Implementation depends on the hardware; this is a placeholder
}

/* Main RTOS initialization */
void rtos_init() {
    scheduler.task_count = 0;
    scheduler.current_task = NULL;
    scheduler.system_time = 0;
    ready_queue.size = 0;
    // Initialize any hardware timers or other necessary components
}

/* Starting the RTOS */
void rtos_start() {
    // This would typically enable interrupts, start the timer, and call schedule_next_task
    // for the first time to get the scheduler running.
    enable_interrupts();
    schedule_next_task();
    // If we return from here, something has gone very wrong; usually, you'd enter an infinite loop or halt
}
```
