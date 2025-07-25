#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdbool.h>

#define MAX_PROCESSES 20
#define MAX_QUEUES 4
#define MAX_NAME_LENGTH 32
#define AGING_THRESHOLD 10

// Process types for multilevel queue
typedef enum {
    PROCESS_SYSTEM,
    PROCESS_INTERACTIVE,
    PROCESS_BATCH,
    PROCESS_BACKGROUND
} process_type_t;

// Process structure for multilevel scheduling
typedef struct {
    int pid;
    char name[MAX_NAME_LENGTH];
    process_type_t type;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int priority;
    int queue_level;
    int waiting_time;
    int turnaround_time;
    int response_time;
    int start_time;
    int completion_time;
    int age;
    int io_frequency;
    bool first_execution;
    bool completed;
} mlq_process_t;

// Queue structure
typedef struct {
    mlq_process_t *processes[MAX_PROCESSES];
    int front;
    int rear;
    int count;
    int time_quantum;
    char name[MAX_NAME_LENGTH];
    int priority_level;
} process_queue_t;

// Multilevel scheduler context
typedef struct {
    process_queue_t queues[MAX_QUEUES];
    mlq_process_t processes[MAX_PROCESSES];
    int process_count;
    int current_time;
    bool feedback_enabled;
    int aging_threshold;
    int completed_processes;
} multilevel_scheduler_t;

// Performance metrics for multilevel scheduling
typedef struct {
    double avg_waiting_time[MAX_QUEUES];
    double avg_turnaround_time[MAX_QUEUES];
    double avg_response_time[MAX_QUEUES];
    double overall_avg_waiting;
    double overall_avg_turnaround;
    double overall_avg_response;
    int migrations;
    int aging_promotions;
} mlq_metrics_t;

// Function prototypes
void safe_printf(const char *format, ...);
void initialize_multilevel_scheduler(multilevel_scheduler_t *scheduler, bool feedback_enabled);
void initialize_queue(process_queue_t *queue, const char *name, int priority, int quantum);
void generate_multilevel_workload(multilevel_scheduler_t *scheduler);
void enqueue_process(process_queue_t *queue, mlq_process_t *process);
mlq_process_t *dequeue_process(process_queue_t *queue);
bool is_queue_empty(process_queue_t *queue);
void assign_process_to_queue(multilevel_scheduler_t *scheduler, mlq_process_t *process);
void schedule_multilevel_queue(multilevel_scheduler_t *scheduler);
void schedule_multilevel_feedback_queue(multilevel_scheduler_t *scheduler);
void apply_aging(multilevel_scheduler_t *scheduler);
void migrate_process(multilevel_scheduler_t *scheduler, mlq_process_t *process, int new_queue);
void analyze_process_behavior(mlq_process_t *process);
void calculate_mlq_metrics(multilevel_scheduler_t *scheduler, mlq_metrics_t *metrics);
void print_multilevel_results(multilevel_scheduler_t *scheduler, mlq_metrics_t *metrics);
void print_queue_status(multilevel_scheduler_t *scheduler);
void demonstrate_multilevel_queue(void);
void demonstrate_multilevel_feedback_queue(void);

// Thread-safe printf
pthread_mutex_t printf_mutex = PTHREAD_MUTEX_INITIALIZER;

void safe_printf(const char *format, ...) {
    va_list args;
    pthread_mutex_lock(&printf_mutex);
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fflush(stdout);
    pthread_mutex_unlock(&printf_mutex);
}

// Initialize multilevel scheduler
void initialize_multilevel_scheduler(multilevel_scheduler_t *scheduler, bool feedback_enabled) {
    memset(scheduler, 0, sizeof(multilevel_scheduler_t));
    scheduler->feedback_enabled = feedback_enabled;
    scheduler->aging_threshold = AGING_THRESHOLD;
    scheduler->current_time = 0;
    scheduler->completed_processes = 0;
    
    // Initialize queues with different characteristics
    initialize_queue(&scheduler->queues[0], "System", 0, 1);      // Highest priority, shortest quantum
    initialize_queue(&scheduler->queues[1], "Interactive", 1, 4); // High priority, short quantum
    initialize_queue(&scheduler->queues[2], "Batch", 2, 8);       // Normal priority, medium quantum
    initialize_queue(&scheduler->queues[3], "Background", 3, 16); // Low priority, long quantum
}

// Initialize a single queue
void initialize_queue(process_queue_t *queue, const char *name, int priority, int quantum) {
    memset(queue, 0, sizeof(process_queue_t));
    strcpy(queue->name, name);
    queue->priority_level = priority;
    queue->time_quantum = quantum;
    queue->front = 0;
    queue->rear = 0;
    queue->count = 0;
}

// Generate workload for multilevel scheduling
void generate_multilevel_workload(multilevel_scheduler_t *scheduler) {
    scheduler->process_count = 12;
    
    // System processes
    for (int i = 0; i < 2; i++) {
        mlq_process_t *p = &scheduler->processes[i];
        p->pid = i + 1;
        snprintf(p->name, MAX_NAME_LENGTH, "SYS%d", i + 1);
        p->type = PROCESS_SYSTEM;
        p->arrival_time = i;
        p->burst_time = 3 + (i % 3);
        p->remaining_time = p->burst_time;
        p->priority = 0;
        p->queue_level = 0;
        p->io_frequency = 1;
        p->first_execution = true;
        p->completed = false;
        p->age = 0;
    }
    
    // Interactive processes
    for (int i = 2; i < 6; i++) {
        mlq_process_t *p = &scheduler->processes[i];
        p->pid = i + 1;
        snprintf(p->name, MAX_NAME_LENGTH, "INT%d", i - 1);
        p->type = PROCESS_INTERACTIVE;
        p->arrival_time = i - 1;
        p->burst_time = 5 + (i % 4);
        p->remaining_time = p->burst_time;
        p->priority = 1;
        p->queue_level = 1;
        p->io_frequency = 3 + (i % 3);
        p->first_execution = true;
        p->completed = false;
        p->age = 0;
    }
    
    // Batch processes
    for (int i = 6; i < 9; i++) {
        mlq_process_t *p = &scheduler->processes[i];
        p->pid = i + 1;
        snprintf(p->name, MAX_NAME_LENGTH, "BAT%d", i - 5);
        p->type = PROCESS_BATCH;
        p->arrival_time = i - 2;
        p->burst_time = 10 + (i % 5);
        p->remaining_time = p->burst_time;
        p->priority = 2;
        p->queue_level = 2;
        p->io_frequency = 1;
        p->first_execution = true;
        p->completed = false;
        p->age = 0;
    }
    
    // Background processes
    for (int i = 9; i < 12; i++) {
        mlq_process_t *p = &scheduler->processes[i];
        p->pid = i + 1;
        snprintf(p->name, MAX_NAME_LENGTH, "BG%d", i - 8);
        p->type = PROCESS_BACKGROUND;
        p->arrival_time = i - 3;
        p->burst_time = 8 + (i % 6);
        p->remaining_time = p->burst_time;
        p->priority = 3;
        p->queue_level = 3;
        p->io_frequency = 1;
        p->first_execution = true;
        p->completed = false;
        p->age = 0;
    }
}

// Enqueue process to a specific queue
void enqueue_process(process_queue_t *queue, mlq_process_t *process) {
    if (queue->count < MAX_PROCESSES) {
        queue->processes[queue->rear] = process;
        queue->rear = (queue->rear + 1) % MAX_PROCESSES;
        queue->count++;
    }
}

// Dequeue process from a specific queue
mlq_process_t *dequeue_process(process_queue_t *queue) {
    if (queue->count > 0) {
        mlq_process_t *process = queue->processes[queue->front];
        queue->front = (queue->front + 1) % MAX_PROCESSES;
        queue->count--;
        return process;
    }
    return NULL;
}

// Check if queue is empty
bool is_queue_empty(process_queue_t *queue) {
    return queue->count == 0;
}

// Assign process to appropriate queue based on type
void assign_process_to_queue(multilevel_scheduler_t *scheduler, mlq_process_t *process) {
    int queue_index = process->queue_level;
    if (queue_index >= 0 && queue_index < MAX_QUEUES) {
        enqueue_process(&scheduler->queues[queue_index], process);
    }
}

// Migrate process between queues
void migrate_process(multilevel_scheduler_t *scheduler, mlq_process_t *process, int new_queue) {
    if (new_queue >= 0 && new_queue < MAX_QUEUES && new_queue != process->queue_level) {
        process->queue_level = new_queue;
        assign_process_to_queue(scheduler, process);
    }
}

// Analyze process behavior for feedback decisions
void analyze_process_behavior(mlq_process_t *process) {
    // Simple heuristic: if process frequently blocks for I/O, it's interactive
    if (process->io_frequency > 2) {
        process->type = PROCESS_INTERACTIVE;
    } else if (process->burst_time > 10) {
        process->type = PROCESS_BATCH;
    }
}

// Apply aging mechanism to prevent starvation
void apply_aging(multilevel_scheduler_t *scheduler) {
    for (int i = 0; i < scheduler->process_count; i++) {
        mlq_process_t *process = &scheduler->processes[i];
        
        if (!process->completed && process->queue_level > 0) {
            process->age++;
            
            if (process->age >= scheduler->aging_threshold) {
                // Promote to higher priority queue
                int new_queue = process->queue_level - 1;
                process->queue_level = new_queue;
                process->age = 0;
                
                safe_printf("  [AGING] Process %s promoted to queue %d\n", 
                           process->name, new_queue);
            }
        }
    }
}

// Multilevel Queue Scheduling (fixed queues)
void schedule_multilevel_queue(multilevel_scheduler_t *scheduler) {
    // Add all arrived processes to their designated queues
    for (int i = 0; i < scheduler->process_count; i++) {
        if (scheduler->processes[i].arrival_time <= scheduler->current_time) {
            assign_process_to_queue(scheduler, &scheduler->processes[i]);
        }
    }
    
    while (scheduler->completed_processes < scheduler->process_count) {
        bool found_process = false;
        
        // Check queues in priority order
        for (int queue_idx = 0; queue_idx < MAX_QUEUES; queue_idx++) {
            process_queue_t *queue = &scheduler->queues[queue_idx];
            
            if (!is_queue_empty(queue)) {
                mlq_process_t *process = dequeue_process(queue);
                found_process = true;
                
                // Record response time on first execution
                if (process->first_execution) {
                    process->start_time = scheduler->current_time;
                    process->response_time = process->start_time - process->arrival_time;
                    process->first_execution = false;
                }
                
                // Execute for time quantum or remaining time
                int execution_time = (process->remaining_time < queue->time_quantum) ?
                                   process->remaining_time : queue->time_quantum;
                
                safe_printf("Time %d: Executing %s from %s queue for %d units\n",
                           scheduler->current_time, process->name, queue->name, execution_time);
                
                scheduler->current_time += execution_time;
                process->remaining_time -= execution_time;
                
                if (process->remaining_time == 0) {
                    // Process completed
                    process->completion_time = scheduler->current_time;
                    process->turnaround_time = process->completion_time - process->arrival_time;
                    process->waiting_time = process->turnaround_time - process->burst_time;
                    process->completed = true;
                    scheduler->completed_processes++;
                    
                    safe_printf("  Process %s completed at time %d\n", 
                               process->name, scheduler->current_time);
                } else {
                    // Put back in same queue (no migration in basic multilevel queue)
                    enqueue_process(queue, process);
                }
                break;
            }
        }
        
        if (!found_process) {
            // No process ready, advance time
            scheduler->current_time++;
            
            // Check for new arrivals
            for (int i = 0; i < scheduler->process_count; i++) {
                if (scheduler->processes[i].arrival_time == scheduler->current_time) {
                    assign_process_to_queue(scheduler, &scheduler->processes[i]);
                }
            }
        }
    }
}

// Multilevel Feedback Queue Scheduling (with migration and aging)
void schedule_multilevel_feedback_queue(multilevel_scheduler_t *scheduler) {
    // Add all arrived processes to highest priority queue initially
    for (int i = 0; i < scheduler->process_count; i++) {
        if (scheduler->processes[i].arrival_time <= scheduler->current_time) {
            scheduler->processes[i].queue_level = 0; // Start in highest priority queue
            assign_process_to_queue(scheduler, &scheduler->processes[i]);
        }
    }
    
    while (scheduler->completed_processes < scheduler->process_count) {
        bool found_process = false;
        
        // Apply aging periodically
        if (scheduler->current_time % 5 == 0) {
            apply_aging(scheduler);
        }
        
        // Check queues in priority order
        for (int queue_idx = 0; queue_idx < MAX_QUEUES; queue_idx++) {
            process_queue_t *queue = &scheduler->queues[queue_idx];
            
            if (!is_queue_empty(queue)) {
                mlq_process_t *process = dequeue_process(queue);
                found_process = true;
                
                // Record response time on first execution
                if (process->first_execution) {
                    process->start_time = scheduler->current_time;
                    process->response_time = process->start_time - process->arrival_time;
                    process->first_execution = false;
                }
                
                // Execute for time quantum or remaining time
                int execution_time = (process->remaining_time < queue->time_quantum) ?
                                   process->remaining_time : queue->time_quantum;
                
                safe_printf("Time %d: Executing %s from %s queue for %d units\n",
                           scheduler->current_time, process->name, queue->name, execution_time);
                
                scheduler->current_time += execution_time;
                process->remaining_time -= execution_time;
                
                if (process->remaining_time == 0) {
                    // Process completed
                    process->completion_time = scheduler->current_time;
                    process->turnaround_time = process->completion_time - process->arrival_time;
                    process->waiting_time = process->turnaround_time - process->burst_time;
                    process->completed = true;
                    scheduler->completed_processes++;
                    
                    safe_printf("  Process %s completed at time %d\n", 
                               process->name, scheduler->current_time);
                } else {
                    // Process used full quantum - demote to lower priority queue
                    if (execution_time == queue->time_quantum && queue_idx < MAX_QUEUES - 1) {
                        process->queue_level = queue_idx + 1;
                        safe_printf("  Process %s demoted to queue %d\n", 
                                   process->name, process->queue_level);
                    }
                    
                    // Analyze behavior for potential promotion
                    analyze_process_behavior(process);
                    
                    // Re-queue the process
                    assign_process_to_queue(scheduler, process);
                }
                break;
            }
        }
        
        if (!found_process) {
            // No process ready, advance time
            scheduler->current_time++;
            
            // Check for new arrivals
            for (int i = 0; i < scheduler->process_count; i++) {
                if (scheduler->processes[i].arrival_time == scheduler->current_time) {
                    scheduler->processes[i].queue_level = 0; // New processes start in top queue
                    assign_process_to_queue(scheduler, &scheduler->processes[i]);
                }
            }
        }
    }
}

// Calculate performance metrics for multilevel scheduling
void calculate_mlq_metrics(multilevel_scheduler_t *scheduler, mlq_metrics_t *metrics) {
    memset(metrics, 0, sizeof(mlq_metrics_t));
    
    int queue_counts[MAX_QUEUES] = {0};
    double total_waiting = 0, total_turnaround = 0, total_response = 0;
    
    for (int i = 0; i < scheduler->process_count; i++) {
        mlq_process_t *p = &scheduler->processes[i];
        int queue_type = (int)p->type;
        
        if (queue_type < MAX_QUEUES) {
            metrics->avg_waiting_time[queue_type] += p->waiting_time;
            metrics->avg_turnaround_time[queue_type] += p->turnaround_time;
            metrics->avg_response_time[queue_type] += p->response_time;
            queue_counts[queue_type]++;
        }
        
        total_waiting += p->waiting_time;
        total_turnaround += p->turnaround_time;
        total_response += p->response_time;
    }
    
    // Calculate averages per queue
    for (int i = 0; i < MAX_QUEUES; i++) {
        if (queue_counts[i] > 0) {
            metrics->avg_waiting_time[i] /= queue_counts[i];
            metrics->avg_turnaround_time[i] /= queue_counts[i];
            metrics->avg_response_time[i] /= queue_counts[i];
        }
    }
    
    // Calculate overall averages
    metrics->overall_avg_waiting = total_waiting / scheduler->process_count;
    metrics->overall_avg_turnaround = total_turnaround / scheduler->process_count;
    metrics->overall_avg_response = total_response / scheduler->process_count;
}

// Print queue status
void print_queue_status(multilevel_scheduler_t *scheduler) {
    safe_printf("\n=== Queue Status ===\n");
    for (int i = 0; i < MAX_QUEUES; i++) {
        process_queue_t *queue = &scheduler->queues[i];
        safe_printf("Queue %d (%s): %d processes, quantum=%d\n", 
                   i, queue->name, queue->count, queue->time_quantum);
    }
}

// Print multilevel scheduling results
void print_multilevel_results(multilevel_scheduler_t *scheduler, mlq_metrics_t *metrics) {
    safe_printf("\n=== Process Execution Results ===\n");
    safe_printf("PID  Name   Type         Arrival  Burst  Queue  Start  Complete  Wait  Turn  Response\n");
    safe_printf("----------------------------------------------------------------------------------\n");
    
    const char *type_names[] = {"System", "Interactive", "Batch", "Background"};
    
    for (int i = 0; i < scheduler->process_count; i++) {
        mlq_process_t *p = &scheduler->processes[i];
        safe_printf("%-4d %-6s %-12s %-7d %-6d %-6d %-6d %-8d %-5d %-5d %-8d\n",
                   p->pid, p->name, type_names[p->type], p->arrival_time, p->burst_time,
                   p->queue_level, p->start_time, p->completion_time,
                   p->waiting_time, p->turnaround_time, p->response_time);
    }
    
    safe_printf("\n=== Performance Metrics by Queue Type ===\n");
    safe_printf("Queue Type   | Avg Wait | Avg Turn | Avg Response\n");
    safe_printf("---------------------------------------------\n");
    
    for (int i = 0; i < MAX_QUEUES; i++) {
        safe_printf("%-12s | %8.2f | %8.2f | %8.2f\n",
                   type_names[i],
                   metrics->avg_waiting_time[i],
                   metrics->avg_turnaround_time[i],
                   metrics->avg_response_time[i]);
    }
    
    safe_printf("\n=== Overall Performance Metrics ===\n");
    safe_printf("Overall Average Waiting Time:    %.2f\n", metrics->overall_avg_waiting);
    safe_printf("Overall Average Turnaround Time: %.2f\n", metrics->overall_avg_turnaround);
    safe_printf("Overall Average Response Time:   %.2f\n", metrics->overall_avg_response);
}

// Demonstrate Multilevel Queue Scheduling
void demonstrate_multilevel_queue(void) {
    safe_printf("\n" "========================================\n");
    safe_printf("Multilevel Queue Scheduling Demonstration\n");
    safe_printf("========================================\n");
    
    multilevel_scheduler_t scheduler;
    mlq_metrics_t metrics;
    
    initialize_multilevel_scheduler(&scheduler, false);
    generate_multilevel_workload(&scheduler);
    
    safe_printf("Fixed queue assignment based on process type:\n");
    safe_printf("- System processes → Queue 0 (quantum=1)\n");
    safe_printf("- Interactive processes → Queue 1 (quantum=4)\n");
    safe_printf("- Batch processes → Queue 2 (quantum=8)\n");
    safe_printf("- Background processes → Queue 3 (quantum=16)\n\n");
    
    schedule_multilevel_queue(&scheduler);
    calculate_mlq_metrics(&scheduler, &metrics);
    print_multilevel_results(&scheduler, &metrics);
}

// Demonstrate Multilevel Feedback Queue Scheduling
void demonstrate_multilevel_feedback_queue(void) {
    safe_printf("\n" "========================================\n");
    safe_printf("Multilevel Feedback Queue Scheduling Demonstration\n");
    safe_printf("========================================\n");
    
    multilevel_scheduler_t scheduler;
    mlq_metrics_t metrics;
    
    initialize_multilevel_scheduler(&scheduler, true);
    generate_multilevel_workload(&scheduler);
    
    safe_printf("Dynamic queue assignment with feedback:\n");
    safe_printf("- All processes start in Queue 0\n");
    safe_printf("- Demotion on quantum expiry\n");
    safe_printf("- Promotion through aging mechanism\n");
    safe_printf("- Aging threshold: %d time units\n\n", AGING_THRESHOLD);
    
    schedule_multilevel_feedback_queue(&scheduler);
    calculate_mlq_metrics(&scheduler, &metrics);
    print_multilevel_results(&scheduler, &metrics);
}

int main(void) {
    safe_printf("Advanced CPU Scheduling: Multilevel Queue Demonstration\n");
    safe_printf("=======================================================\n");
    safe_printf("This program demonstrates multilevel queue and multilevel feedback\n");
    safe_printf("queue scheduling algorithms with different process types.\n");
    
    // Demonstrate Multilevel Queue Scheduling
    demonstrate_multilevel_queue();
    
    // Demonstrate Multilevel Feedback Queue Scheduling
    demonstrate_multilevel_feedback_queue();
    
    safe_printf("\n=== Key Differences Summary ===\n");
    safe_printf("Multilevel Queue:\n");
    safe_printf("- Fixed queue assignment based on process type\n");
    safe_printf("- No process migration between queues\n");
    safe_printf("- Simpler implementation\n");
    safe_printf("- Potential for starvation in lower priority queues\n\n");
    
    safe_printf("Multilevel Feedback Queue:\n");
    safe_printf("- Dynamic queue assignment based on behavior\n");
    safe_printf("- Process migration and aging mechanisms\n");
    safe_printf("- Adaptive to changing process characteristics\n");
    safe_printf("- Better fairness through aging\n");
    
    safe_printf("\nDemonstration completed successfully!\n");
    return 0;
} 