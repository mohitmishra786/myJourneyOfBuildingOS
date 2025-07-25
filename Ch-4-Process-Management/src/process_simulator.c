#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>

/* Process States */
typedef enum {
    PROCESS_NEW = 0,
    PROCESS_READY = 1,
    PROCESS_RUNNING = 2,
    PROCESS_WAITING = 3,
    PROCESS_TERMINATED = 4
} process_state_t;

/* Process Control Block */
typedef struct process_control_block {
    uint32_t pid;                    /* Process ID */
    uint32_t parent_pid;             /* Parent Process ID */
    process_state_t state;           /* Current process state */
    uint32_t priority;               /* Process priority */
    uint32_t cpu_time_used;          /* CPU time consumed */
    uint32_t burst_time;             /* CPU burst time */
    uint32_t arrival_time;           /* Process arrival time */
    uint32_t waiting_time;           /* Time spent waiting */
    uint32_t turnaround_time;        /* Total time in system */
    uint32_t memory_size;            /* Memory requirement */
    char process_name[32];           /* Process name */
} pcb_t;

/* Process Queue Node */
typedef struct process_node {
    pcb_t* process;
    struct process_node* next;
} process_node_t;

/* Process Queue */
typedef struct process_queue {
    process_node_t* head;
    process_node_t* tail;
    uint32_t count;
} process_queue_t;

/* Process Scheduler */
typedef struct process_scheduler {
    process_queue_t ready_queue;
    process_queue_t waiting_queue;
    pcb_t* running_process;
    uint32_t time_quantum;
    uint32_t current_time;
    uint32_t total_processes;
    uint32_t completed_processes;
} scheduler_t;

/* Function prototypes */
void init_scheduler(scheduler_t* sched, uint32_t quantum);
pcb_t* create_process(uint32_t pid, const char* name, uint32_t burst_time, 
                      uint32_t priority, uint32_t arrival_time);
void add_to_queue(process_queue_t* queue, pcb_t* process);
pcb_t* remove_from_queue(process_queue_t* queue);
void schedule_next_process(scheduler_t* sched);
void run_simulation(scheduler_t* sched, uint32_t simulation_time);
void print_process_state(pcb_t* process);
void print_scheduler_statistics(scheduler_t* sched);

/* Initialize scheduler */
void init_scheduler(scheduler_t* sched, uint32_t quantum) {
    sched->ready_queue.head = NULL;
    sched->ready_queue.tail = NULL;
    sched->ready_queue.count = 0;
    
    sched->waiting_queue.head = NULL;
    sched->waiting_queue.tail = NULL;
    sched->waiting_queue.count = 0;
    
    sched->running_process = NULL;
    sched->time_quantum = quantum;
    sched->current_time = 0;
    sched->total_processes = 0;
    sched->completed_processes = 0;
}

/* Create new process */
pcb_t* create_process(uint32_t pid, const char* name, uint32_t burst_time, 
                      uint32_t priority, uint32_t arrival_time) {
    pcb_t* process = malloc(sizeof(pcb_t));
    if (!process) return NULL;
    
    process->pid = pid;
    process->parent_pid = 1; /* Init process */
    process->state = PROCESS_NEW;
    process->priority = priority;
    process->cpu_time_used = 0;
    process->burst_time = burst_time;
    process->arrival_time = arrival_time;
    process->waiting_time = 0;
    process->turnaround_time = 0;
    process->memory_size = 1024 + (rand() % 2048); /* Random memory size */
    strncpy(process->process_name, name, sizeof(process->process_name) - 1);
    process->process_name[sizeof(process->process_name) - 1] = '\0';
    
    return process;
}

/* Add process to queue */
void add_to_queue(process_queue_t* queue, pcb_t* process) {
    process_node_t* node = malloc(sizeof(process_node_t));
    if (!node) return;
    
    node->process = process;
    node->next = NULL;
    
    if (queue->tail) {
        queue->tail->next = node;
    } else {
        queue->head = node;
    }
    queue->tail = node;
    queue->count++;
}

/* Remove process from queue */
pcb_t* remove_from_queue(process_queue_t* queue) {
    if (!queue->head) return NULL;
    
    process_node_t* node = queue->head;
    pcb_t* process = node->process;
    
    queue->head = node->next;
    if (!queue->head) {
        queue->tail = NULL;
    }
    queue->count--;
    
    free(node);
    return process;
}

/* Schedule next process */
void schedule_next_process(scheduler_t* sched) {
    /* If current process is running, handle context switch */
    if (sched->running_process) {
        if (sched->running_process->state == PROCESS_RUNNING) {
            /* Process time slice expired, move to ready queue */
            sched->running_process->state = PROCESS_READY;
            add_to_queue(&sched->ready_queue, sched->running_process);
            printf("Time %u: Process %s preempted (time slice expired)\n", 
                   sched->current_time, sched->running_process->process_name);
        }
    }
    
    /* Get next process from ready queue */
    sched->running_process = remove_from_queue(&sched->ready_queue);
    
    if (sched->running_process) {
        sched->running_process->state = PROCESS_RUNNING;
        printf("Time %u: Process %s scheduled (PID %u)\n", 
               sched->current_time, sched->running_process->process_name,
               sched->running_process->pid);
    }
}

/* Print process state */
void print_process_state(pcb_t* process) {
    const char* state_names[] = {"NEW", "READY", "RUNNING", "WAITING", "TERMINATED"};
    
    printf("Process %s (PID %u):\n", process->process_name, process->pid);
    printf("  State: %s\n", state_names[process->state]);
    printf("  Priority: %u\n", process->priority);
    printf("  CPU Time Used: %u\n", process->cpu_time_used);
    printf("  Burst Time: %u\n", process->burst_time);
    printf("  Waiting Time: %u\n", process->waiting_time);
    printf("  Memory Size: %u KB\n", process->memory_size);
    printf("\n");
}

/* Run simulation */
void run_simulation(scheduler_t* sched, uint32_t simulation_time) {
    printf("Starting Process Management Simulation\n");
    printf("Time Quantum: %u\n", sched->time_quantum);
    printf("========================================\n\n");
    
    while (sched->current_time < simulation_time) {
        printf("--- Time: %u ---\n", sched->current_time);
        
        /* Check for process arrivals */
        /* This would typically read from input or generate random arrivals */
        
        /* Execute current process */
        if (sched->running_process) {
            sched->running_process->cpu_time_used++;
            
            /* Check if process completed */
            if (sched->running_process->cpu_time_used >= sched->running_process->burst_time) {
                sched->running_process->state = PROCESS_TERMINATED;
                sched->running_process->turnaround_time = 
                    sched->current_time - sched->running_process->arrival_time + 1;
                
                printf("Time %u: Process %s completed\n", 
                       sched->current_time, sched->running_process->process_name);
                
                sched->completed_processes++;
                free(sched->running_process);
                sched->running_process = NULL;
            }
        }
        
        /* Update waiting times for processes in ready queue */
        process_node_t* node = sched->ready_queue.head;
        while (node) {
            node->process->waiting_time++;
            node = node->next;
        }
        
        /* Schedule next process if needed */
        if (!sched->running_process && sched->ready_queue.count > 0) {
            schedule_next_process(sched);
        } else if (sched->running_process && 
                  (sched->current_time % sched->time_quantum == 0) && 
                  sched->ready_queue.count > 0) {
            /* Time slice expired */
            schedule_next_process(sched);
        }
        
        /* Display current state */
        if (sched->running_process) {
            printf("Running: %s (CPU time: %u/%u)\n", 
                   sched->running_process->process_name,
                   sched->running_process->cpu_time_used,
                   sched->running_process->burst_time);
        } else {
            printf("CPU Idle\n");
        }
        
        printf("Ready queue size: %u\n", sched->ready_queue.count);
        printf("Waiting queue size: %u\n", sched->waiting_queue.count);
        printf("\n");
        
        sched->current_time++;
        
        /* Small delay for visualization */
        usleep(500000); /* 0.5 second */
    }
}

/* Print scheduler statistics */
void print_scheduler_statistics(scheduler_t* sched) {
    printf("\n========== Simulation Statistics ==========\n");
    printf("Total simulation time: %u\n", sched->current_time);
    printf("Total processes created: %u\n", sched->total_processes);
    printf("Processes completed: %u\n", sched->completed_processes);
    
    if (sched->completed_processes > 0) {
        printf("CPU Utilization: %.2f%%\n", 
               (float)(sched->current_time - sched->completed_processes) / 
               sched->current_time * 100);
    }
    
    printf("Time quantum: %u\n", sched->time_quantum);
    printf("==========================================\n");
}

/* Demonstration function */
void demonstrate_process_management(void) {
    scheduler_t scheduler;
    init_scheduler(&scheduler, 3); /* Time quantum of 3 */
    
    /* Create some sample processes */
    pcb_t* p1 = create_process(101, "Browser", 8, 1, 0);
    pcb_t* p2 = create_process(102, "TextEditor", 4, 2, 1);
    pcb_t* p3 = create_process(103, "Compiler", 12, 3, 2);
    pcb_t* p4 = create_process(104, "MediaPlayer", 6, 1, 3);
    
    /* Add processes to scheduler */
    p1->state = PROCESS_READY;
    add_to_queue(&scheduler.ready_queue, p1);
    scheduler.total_processes++;
    
    p2->state = PROCESS_READY;
    add_to_queue(&scheduler.ready_queue, p2);
    scheduler.total_processes++;
    
    p3->state = PROCESS_READY;
    add_to_queue(&scheduler.ready_queue, p3);
    scheduler.total_processes++;
    
    p4->state = PROCESS_READY;
    add_to_queue(&scheduler.ready_queue, p4);
    scheduler.total_processes++;
    
    printf("Initial Process Information:\n");
    print_process_state(p1);
    print_process_state(p2);
    print_process_state(p3);
    print_process_state(p4);
    
    /* Run simulation */
    run_simulation(&scheduler, 25);
    
    /* Print final statistics */
    print_scheduler_statistics(&scheduler);
}

int main(void) {
    srand(time(NULL));
    
    printf("Process Management Simulator\n");
    printf("===========================\n\n");
    
    demonstrate_process_management();
    
    return 0;
} 