#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MAX_TASKS 32
#define MAX_PRIORITY 32
#define TICK_RATE_MS 1
#define TASK_STACK_SIZE 1024

// Task states
typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_SUSPENDED
} TaskState;

// Task control block
typedef struct {
    uint32_t id;
    void (*function)(void*);
    void* parameters;
    uint32_t priority;
    uint32_t deadline;
    uint32_t period;
    uintptr_t stack_pointer;  // Fixed: Using uintptr_t for pointer storage
    TaskState state;
    uint32_t blocked_tick;
    uint32_t timeout;
    char name[32];  // Added: Task name for better debugging
    uint32_t run_count;  // Added: Track number of times task has run
} TCB;

// RTOS kernel structure
typedef struct {
    TCB* tasks[MAX_TASKS];
    uint32_t num_tasks;
    uint32_t current_task;
    uint32_t tick_count;
    bool scheduler_running;
    uint32_t total_switches;  // Added: Track total context switches
} RTOS_Kernel;

// Global kernel instance
static RTOS_Kernel kernel;

// Priority queue for task scheduling
typedef struct {
    TCB* tasks[MAX_TASKS];
    uint32_t size;
} PriorityQueue;

// Initialize priority queue
void priority_queue_init(PriorityQueue* queue) {
    queue->size = 0;
}

// Insert task into priority queue
void priority_queue_insert(PriorityQueue* queue, TCB* task) {
    uint32_t i = queue->size++;
    
    while (i > 0) {
        uint32_t parent = (i - 1) / 2;
        if (queue->tasks[parent]->priority >= task->priority &&
            queue->tasks[parent]->deadline <= task->deadline)
            break;
            
        queue->tasks[i] = queue->tasks[parent];
        i = parent;
    }
    
    queue->tasks[i] = task;
}

// Extract highest priority task
TCB* priority_queue_extract(PriorityQueue* queue) {
    if (queue->size == 0) return NULL;
    
    TCB* result = queue->tasks[0];
    TCB* last = queue->tasks[--queue->size];
    
    uint32_t i = 0;
    while (i * 2 + 1 < queue->size) {
        uint32_t left = i * 2 + 1;
        uint32_t right = left + 1;
        uint32_t highest_priority = left;
        
        if (right < queue->size && 
            (queue->tasks[right]->priority > queue->tasks[left]->priority ||
             (queue->tasks[right]->priority == queue->tasks[left]->priority &&
              queue->tasks[right]->deadline < queue->tasks[left]->deadline)))
            highest_priority = right;
            
        if (last->priority > queue->tasks[highest_priority]->priority ||
            (last->priority == queue->tasks[highest_priority]->priority &&
             last->deadline <= queue->tasks[highest_priority]->deadline))
            break;
            
        queue->tasks[i] = queue->tasks[highest_priority];
        i = highest_priority;
    }
    
    if (queue->size > 0)
        queue->tasks[i] = last;
        
    return result;
}

// Initialize RTOS kernel
void rtos_init(void) {
    kernel.num_tasks = 0;
    kernel.current_task = 0;
    kernel.tick_count = 0;
    kernel.scheduler_running = false;
    kernel.total_switches = 0;
    
    for (int i = 0; i < MAX_TASKS; i++) {
        kernel.tasks[i] = NULL;
    }
    
    printf("RTOS Kernel initialized\n");
}

// Create new task with name
int rtos_create_task(const char* name, 
                     void (*function)(void*), 
                     void* parameters,
                     uint32_t priority,
                     uint32_t deadline,
                     uint32_t period) {
    if (kernel.num_tasks >= MAX_TASKS) {
        printf("Error: Maximum task limit reached\n");
        return -1;
    }
        
    TCB* new_task = (TCB*)malloc(sizeof(TCB));
    if (!new_task) {
        printf("Error: Failed to allocate TCB\n");
        return -1;
    }
        
    new_task->id = kernel.num_tasks;
    new_task->function = function;
    new_task->parameters = parameters;
    new_task->priority = priority;
    new_task->deadline = deadline;
    new_task->period = period;
    new_task->state = TASK_READY;
    new_task->blocked_tick = 0;
    new_task->timeout = 0;
    new_task->run_count = 0;
    strncpy(new_task->name, name, sizeof(new_task->name) - 1);
    new_task->name[sizeof(new_task->name) - 1] = '\0';
    
    // Allocate task stack
    void* stack = malloc(TASK_STACK_SIZE);
    if (!stack) {
        printf("Error: Failed to allocate stack for task %s\n", name);
        free(new_task);
        return -1;
    }
    new_task->stack_pointer = (uintptr_t)stack;
    
    kernel.tasks[kernel.num_tasks++] = new_task;
    printf("Task '%s' created with ID %d, Priority %d\n", 
           name, new_task->id, priority);
    return new_task->id;
}

// Schedule next task
TCB* schedule_next_task(void) {
    static PriorityQueue ready_queue;
    priority_queue_init(&ready_queue);
    
    // Add all ready tasks to priority queue
    for (uint32_t i = 0; i < kernel.num_tasks; i++) {
        TCB* task = kernel.tasks[i];
        if (task->state == TASK_READY) {
            priority_queue_insert(&ready_queue, task);
        }
    }
    
    // Get highest priority task
    TCB* next_task = priority_queue_extract(&ready_queue);
    if (next_task) {
        next_task->state = TASK_RUNNING;
        kernel.current_task = next_task->id;
    }
    
    return next_task;
}

// Simulate task execution (for demonstration)
void simulate_task_execution(TCB* task) {
    task->run_count++;
    task->function(task->parameters);
}

// System tick handler
void rtos_tick_handler(void) {
    kernel.tick_count++;
    
    // Print tick information every 100 ticks
    if (kernel.tick_count % 100 == 0) {
        printf("\n[Tick %d] System Status:\n", kernel.tick_count);
        printf("Total context switches: %d\n", kernel.total_switches);
    }
    
    // Check blocked tasks
    for (uint32_t i = 0; i < kernel.num_tasks; i++) {
        TCB* task = kernel.tasks[i];
        if (task->state == TASK_BLOCKED) {
            if (kernel.tick_count - task->blocked_tick >= task->timeout) {
                printf("[Tick %d] Task '%s' unblocked\n", 
                       kernel.tick_count, task->name);
                task->state = TASK_READY;
            }
        }
    }
    
    // Check if we need to preempt current task
    TCB* current = kernel.tasks[kernel.current_task];
    if (current && current->state == TASK_RUNNING) {
        bool should_preempt = false;
        
        // Check if any higher priority task is ready
        for (uint32_t i = 0; i < kernel.num_tasks; i++) {
            TCB* task = kernel.tasks[i];
            if (task->state == TASK_READY && task->priority > current->priority) {
                should_preempt = true;
                break;
            }
        }
        
        if (should_preempt) {
            printf("[Tick %d] Preempting task '%s'\n", 
                   kernel.tick_count, current->name);
            current->state = TASK_READY;
            TCB* next = schedule_next_task();
            if (next) {
                kernel.total_switches++;
                printf("[Tick %d] Switching from '%s' to '%s'\n", 
                       kernel.tick_count, current->name, next->name);
                simulate_task_execution(next);
            }
        } else {
            // Continue executing current task
            simulate_task_execution(current);
        }
    } else {
        // No task running, schedule next task
        TCB* next = schedule_next_task();
        if (next) {
            kernel.total_switches++;
            printf("[Tick %d] Starting task '%s'\n", 
                   kernel.tick_count, next->name);
            simulate_task_execution(next);
        }
    }
}

// Block current task
void rtos_block_task(uint32_t timeout) {
    TCB* current = kernel.tasks[kernel.current_task];
    if (current) {
        printf("[Tick %d] Task '%s' blocking for %d ticks\n", 
               kernel.tick_count, current->name, timeout);
        current->state = TASK_BLOCKED;
        current->blocked_tick = kernel.tick_count;
        current->timeout = timeout;
        
        // Schedule next task
        TCB* next = schedule_next_task();
        if (next) {
            kernel.total_switches++;
            printf("[Tick %d] Switching from '%s' (blocked) to '%s'\n", 
                   kernel.tick_count, current->name, next->name);
            simulate_task_execution(next);
        }
    }
}

// Start RTOS scheduler
void rtos_start(void) {
    printf("\nStarting RTOS Scheduler\n");
    printf("----------------------\n");
    kernel.scheduler_running = true;
    
    // Start first task
    TCB* first_task = schedule_next_task();
    if (first_task) {
        printf("[Tick %d] Starting first task '%s'\n", 
               kernel.tick_count, first_task->name);
        simulate_task_execution(first_task);
    }
}

// Example task functions
void task1(void* params) {
    TCB* task = kernel.tasks[kernel.current_task];
    printf("[Tick %d] Task '%s' running (execution #%d)\n", 
           kernel.tick_count, task->name, task->run_count);
    rtos_block_task(100);
}

void task2(void* params) {
    TCB* task = kernel.tasks[kernel.current_task];
    printf("[Tick %d] Task '%s' running (execution #%d)\n", 
           kernel.tick_count, task->name, task->run_count);
    rtos_block_task(200);
}

void task3(void* params) {
    TCB* task = kernel.tasks[kernel.current_task];
    printf("[Tick %d] Task '%s' running (execution #%d)\n", 
           kernel.tick_count, task->name, task->run_count);
    rtos_block_task(150);
}

// Example usage
int main() {
    rtos_init();
    
    // Create tasks with names and different priorities
    rtos_create_task("LowPriorityTask", task1, NULL, 1, 1000, 1000);
    rtos_create_task("HighPriorityTask", task2, NULL, 3, 2000, 2000);
    rtos_create_task("MediumPriorityTask", task3, NULL, 2, 1500, 1500);
    
    // Start scheduler
    rtos_start();
    
    // Simulate tick interrupts
    printf("\nSimulating system ticks...\n");
    for (int i = 0; i < 500; i++) {
        rtos_tick_handler();
    }
    
    // Print final statistics
    printf("\nFinal System Statistics:\n");
    printf("----------------------\n");
    printf("Total ticks: %d\n", kernel.tick_count);
    printf("Total context switches: %d\n", kernel.total_switches);
    
    for (uint32_t i = 0; i < kernel.num_tasks; i++) {
        TCB* task = kernel.tasks[i];
        printf("Task '%s': Ran %d times\n", task->name, task->run_count);
        
        // Cleanup
        free((void*)task->stack_pointer);
        free(task);
    }
    
    return 0;
}
