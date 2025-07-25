#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdbool.h>
#include <limits.h>

// Maximum number of processes and algorithms
#define MAX_PROCESSES 20
#define MAX_ALGORITHMS 4
#define MAX_NAME_LENGTH 32

// Process states
typedef enum {
    PROCESS_NEW,
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_WAITING,
    PROCESS_TERMINATED
} process_state_t;

// Scheduling algorithms
typedef enum {
    ALGORITHM_FCFS,
    ALGORITHM_SJF,
    ALGORITHM_ROUND_ROBIN,
    ALGORITHM_PRIORITY
} scheduling_algorithm_t;

// Process structure
typedef struct {
    int pid;
    char name[MAX_NAME_LENGTH];
    int arrival_time;
    int burst_time;
    int priority;
    int remaining_time;
    int start_time;
    int completion_time;
    int waiting_time;
    int turnaround_time;
    int response_time;
    process_state_t state;
    bool first_execution;
} process_t;

// Scheduling context
typedef struct {
    process_t processes[MAX_PROCESSES];
    int process_count;
    int current_time;
    int time_quantum;
    scheduling_algorithm_t algorithm;
    char algorithm_name[MAX_NAME_LENGTH];
} scheduler_context_t;

// Performance metrics
typedef struct {
    double avg_waiting_time;
    double avg_turnaround_time;
    double avg_response_time;
    double cpu_utilization;
    double throughput;
    int total_context_switches;
} performance_metrics_t;

// Function prototypes
void safe_printf(const char *format, ...);
void initialize_scheduler_context(scheduler_context_t *ctx, scheduling_algorithm_t algorithm, int time_quantum);
void generate_process_workload(scheduler_context_t *ctx, int count, bool random_workload);
void reset_process_state(scheduler_context_t *ctx);
void schedule_fcfs(scheduler_context_t *ctx);
void schedule_sjf(scheduler_context_t *ctx);
void schedule_round_robin(scheduler_context_t *ctx);
void schedule_priority(scheduler_context_t *ctx);
void calculate_performance_metrics(scheduler_context_t *ctx, performance_metrics_t *metrics);
void print_process_table(scheduler_context_t *ctx);
void print_performance_metrics(const char *algorithm_name, performance_metrics_t *metrics);
void print_gantt_chart(scheduler_context_t *ctx);
void compare_algorithms(void);
void demonstrate_algorithm(scheduling_algorithm_t algorithm, const char *name, int time_quantum);
int compare_arrival_time(const void *a, const void *b);
int compare_burst_time(const void *a, const void *b);
int compare_priority(const void *a, const void *b);

// Thread-safe printf implementation
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

// Initialize scheduler context
void initialize_scheduler_context(scheduler_context_t *ctx, scheduling_algorithm_t algorithm, int time_quantum) {
    memset(ctx, 0, sizeof(scheduler_context_t));
    ctx->algorithm = algorithm;
    ctx->time_quantum = time_quantum;
    ctx->current_time = 0;
    
    switch (algorithm) {
        case ALGORITHM_FCFS:
            strcpy(ctx->algorithm_name, "First-Come-First-Serve");
            break;
        case ALGORITHM_SJF:
            strcpy(ctx->algorithm_name, "Shortest Job First");
            break;
        case ALGORITHM_ROUND_ROBIN:
            strcpy(ctx->algorithm_name, "Round Robin");
            break;
        case ALGORITHM_PRIORITY:
            strcpy(ctx->algorithm_name, "Priority Scheduling");
            break;
    }
}

// Generate process workload
void generate_process_workload(scheduler_context_t *ctx, int count, bool random_workload) {
    ctx->process_count = count;
    
    if (random_workload) {
        srand(time(NULL));
        for (int i = 0; i < count; i++) {
            ctx->processes[i].pid = i + 1;
            snprintf(ctx->processes[i].name, MAX_NAME_LENGTH, "P%d", i + 1);
            ctx->processes[i].arrival_time = rand() % 10;
            ctx->processes[i].burst_time = (rand() % 20) + 1;
            ctx->processes[i].priority = rand() % 5;
            ctx->processes[i].remaining_time = ctx->processes[i].burst_time;
            ctx->processes[i].state = PROCESS_NEW;
            ctx->processes[i].first_execution = true;
        }
    } else {
        // Predefined workload for consistent demonstration
        int arrival_times[] = {0, 1, 2, 3, 4};
        int burst_times[] = {10, 5, 8, 3, 6};
        int priorities[] = {3, 1, 4, 2, 5};
        
        for (int i = 0; i < count && i < 5; i++) {
            ctx->processes[i].pid = i + 1;
            snprintf(ctx->processes[i].name, MAX_NAME_LENGTH, "P%d", i + 1);
            ctx->processes[i].arrival_time = arrival_times[i];
            ctx->processes[i].burst_time = burst_times[i];
            ctx->processes[i].priority = priorities[i];
            ctx->processes[i].remaining_time = ctx->processes[i].burst_time;
            ctx->processes[i].state = PROCESS_NEW;
            ctx->processes[i].first_execution = true;
        }
    }
}

// Reset process state for new scheduling run
void reset_process_state(scheduler_context_t *ctx) {
    ctx->current_time = 0;
    for (int i = 0; i < ctx->process_count; i++) {
        ctx->processes[i].remaining_time = ctx->processes[i].burst_time;
        ctx->processes[i].state = PROCESS_NEW;
        ctx->processes[i].start_time = 0;
        ctx->processes[i].completion_time = 0;
        ctx->processes[i].waiting_time = 0;
        ctx->processes[i].turnaround_time = 0;
        ctx->processes[i].response_time = 0;
        ctx->processes[i].first_execution = true;
    }
}

// Comparison functions for sorting
int compare_arrival_time(const void *a, const void *b) {
    process_t *p1 = (process_t *)a;
    process_t *p2 = (process_t *)b;
    return p1->arrival_time - p2->arrival_time;
}

int compare_burst_time(const void *a, const void *b) {
    process_t *p1 = (process_t *)a;
    process_t *p2 = (process_t *)b;
    if (p1->arrival_time <= p2->arrival_time) {
        return p1->burst_time - p2->burst_time;
    }
    return p1->arrival_time - p2->arrival_time;
}

int compare_priority(const void *a, const void *b) {
    process_t *p1 = (process_t *)a;
    process_t *p2 = (process_t *)b;
    return p1->priority - p2->priority;
}

// First-Come-First-Serve scheduling
void schedule_fcfs(scheduler_context_t *ctx) {
    // Sort processes by arrival time
    qsort(ctx->processes, ctx->process_count, sizeof(process_t), compare_arrival_time);
    
    ctx->current_time = 0;
    for (int i = 0; i < ctx->process_count; i++) {
        process_t *process = &ctx->processes[i];
        
        // Wait for process arrival
        if (ctx->current_time < process->arrival_time) {
            ctx->current_time = process->arrival_time;
        }
        
        // Record start time and response time
        process->start_time = ctx->current_time;
        process->response_time = process->start_time - process->arrival_time;
        
        // Execute process to completion
        process->state = PROCESS_RUNNING;
        ctx->current_time += process->burst_time;
        process->completion_time = ctx->current_time;
        process->state = PROCESS_TERMINATED;
        
        // Calculate performance metrics
        process->turnaround_time = process->completion_time - process->arrival_time;
        process->waiting_time = process->turnaround_time - process->burst_time;
    }
}

// Shortest Job First scheduling
void schedule_sjf(scheduler_context_t *ctx) {
    ctx->current_time = 0;
    int completed = 0;
    
    while (completed < ctx->process_count) {
        int shortest_job = -1;
        int shortest_time = INT_MAX;
        
        // Find the shortest job among arrived processes
        for (int i = 0; i < ctx->process_count; i++) {
            if (ctx->processes[i].arrival_time <= ctx->current_time &&
                ctx->processes[i].state != PROCESS_TERMINATED &&
                ctx->processes[i].burst_time < shortest_time) {
                shortest_time = ctx->processes[i].burst_time;
                shortest_job = i;
            }
        }
        
        if (shortest_job == -1) {
            // No process available, advance time to next arrival
            int next_arrival = INT_MAX;
            for (int i = 0; i < ctx->process_count; i++) {
                if (ctx->processes[i].state != PROCESS_TERMINATED &&
                    ctx->processes[i].arrival_time > ctx->current_time &&
                    ctx->processes[i].arrival_time < next_arrival) {
                    next_arrival = ctx->processes[i].arrival_time;
                }
            }
            ctx->current_time = next_arrival;
            continue;
        }
        
        process_t *process = &ctx->processes[shortest_job];
        
        // Record start time and response time
        process->start_time = ctx->current_time;
        process->response_time = process->start_time - process->arrival_time;
        
        // Execute process to completion
        process->state = PROCESS_RUNNING;
        ctx->current_time += process->burst_time;
        process->completion_time = ctx->current_time;
        process->state = PROCESS_TERMINATED;
        
        // Calculate performance metrics
        process->turnaround_time = process->completion_time - process->arrival_time;
        process->waiting_time = process->turnaround_time - process->burst_time;
        
        completed++;
    }
}

// Round Robin scheduling
void schedule_round_robin(scheduler_context_t *ctx) {
    // Sort processes by arrival time
    qsort(ctx->processes, ctx->process_count, sizeof(process_t), compare_arrival_time);
    
    ctx->current_time = 0;
    int completed = 0;
    int current_process = 0;
    
    while (completed < ctx->process_count) {
        bool found_process = false;
        
        // Find next ready process
        for (int i = 0; i < ctx->process_count; i++) {
            int idx = (current_process + i) % ctx->process_count;
            process_t *process = &ctx->processes[idx];
            
            if (process->arrival_time <= ctx->current_time &&
                process->state != PROCESS_TERMINATED) {
                
                found_process = true;
                current_process = idx;
                
                // Record response time on first execution
                if (process->first_execution) {
                    process->start_time = ctx->current_time;
                    process->response_time = process->start_time - process->arrival_time;
                    process->first_execution = false;
                }
                
                // Execute for time quantum or remaining time
                int execution_time = (process->remaining_time < ctx->time_quantum) ?
                                   process->remaining_time : ctx->time_quantum;
                
                process->state = PROCESS_RUNNING;
                ctx->current_time += execution_time;
                process->remaining_time -= execution_time;
                
                if (process->remaining_time == 0) {
                    // Process completed
                    process->completion_time = ctx->current_time;
                    process->state = PROCESS_TERMINATED;
                    process->turnaround_time = process->completion_time - process->arrival_time;
                    process->waiting_time = process->turnaround_time - process->burst_time;
                    completed++;
                } else {
                    process->state = PROCESS_READY;
                }
                
                current_process = (current_process + 1) % ctx->process_count;
                break;
            }
        }
        
        if (!found_process) {
            // No process ready, advance time to next arrival
            int next_arrival = INT_MAX;
            for (int i = 0; i < ctx->process_count; i++) {
                if (ctx->processes[i].state != PROCESS_TERMINATED &&
                    ctx->processes[i].arrival_time > ctx->current_time &&
                    ctx->processes[i].arrival_time < next_arrival) {
                    next_arrival = ctx->processes[i].arrival_time;
                }
            }
            if (next_arrival != INT_MAX) {
                ctx->current_time = next_arrival;
            }
        }
    }
}

// Priority scheduling (preemptive)
void schedule_priority(scheduler_context_t *ctx) {
    ctx->current_time = 0;
    int completed = 0;
    
    while (completed < ctx->process_count) {
        int highest_priority_job = -1;
        int highest_priority = INT_MAX;
        
        // Find highest priority job among arrived processes
        for (int i = 0; i < ctx->process_count; i++) {
            if (ctx->processes[i].arrival_time <= ctx->current_time &&
                ctx->processes[i].state != PROCESS_TERMINATED &&
                ctx->processes[i].priority < highest_priority) {
                highest_priority = ctx->processes[i].priority;
                highest_priority_job = i;
            }
        }
        
        if (highest_priority_job == -1) {
            // No process available, advance time to next arrival
            int next_arrival = INT_MAX;
            for (int i = 0; i < ctx->process_count; i++) {
                if (ctx->processes[i].state != PROCESS_TERMINATED &&
                    ctx->processes[i].arrival_time > ctx->current_time &&
                    ctx->processes[i].arrival_time < next_arrival) {
                    next_arrival = ctx->processes[i].arrival_time;
                }
            }
            ctx->current_time = next_arrival;
            continue;
        }
        
        process_t *process = &ctx->processes[highest_priority_job];
        
        // Record response time on first execution
        if (process->first_execution) {
            process->start_time = ctx->current_time;
            process->response_time = process->start_time - process->arrival_time;
            process->first_execution = false;
        }
        
        // Execute for 1 time unit (to allow preemption)
        process->state = PROCESS_RUNNING;
        ctx->current_time++;
        process->remaining_time--;
        
        if (process->remaining_time == 0) {
            // Process completed
            process->completion_time = ctx->current_time;
            process->state = PROCESS_TERMINATED;
            process->turnaround_time = process->completion_time - process->arrival_time;
            process->waiting_time = process->turnaround_time - process->burst_time;
            completed++;
        } else {
            process->state = PROCESS_READY;
        }
    }
}

// Calculate performance metrics
void calculate_performance_metrics(scheduler_context_t *ctx, performance_metrics_t *metrics) {
    memset(metrics, 0, sizeof(performance_metrics_t));
    
    int total_waiting = 0, total_turnaround = 0, total_response = 0;
    int total_burst_time = 0;
    
    for (int i = 0; i < ctx->process_count; i++) {
        total_waiting += ctx->processes[i].waiting_time;
        total_turnaround += ctx->processes[i].turnaround_time;
        total_response += ctx->processes[i].response_time;
        total_burst_time += ctx->processes[i].burst_time;
    }
    
    metrics->avg_waiting_time = (double)total_waiting / ctx->process_count;
    metrics->avg_turnaround_time = (double)total_turnaround / ctx->process_count;
    metrics->avg_response_time = (double)total_response / ctx->process_count;
    metrics->cpu_utilization = (double)total_burst_time / ctx->current_time * 100.0;
    metrics->throughput = (double)ctx->process_count / ctx->current_time;
    
    // Context switches calculation (simplified)
    if (ctx->algorithm == ALGORITHM_ROUND_ROBIN || ctx->algorithm == ALGORITHM_PRIORITY) {
        metrics->total_context_switches = ctx->process_count * 2; // Approximate
    } else {
        metrics->total_context_switches = ctx->process_count;
    }
}

// Print process table
void print_process_table(scheduler_context_t *ctx) {
    safe_printf("\n=== Process Execution Details ===\n");
    safe_printf("PID  Name  Arrival  Burst  Priority  Start  Complete  Waiting  Turnaround  Response\n");
    safe_printf("--------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < ctx->process_count; i++) {
        process_t *p = &ctx->processes[i];
        safe_printf("%-4d %-5s %-7d %-6d %-8d %-6d %-8d %-8d %-10d %-8d\n",
                   p->pid, p->name, p->arrival_time, p->burst_time, p->priority,
                   p->start_time, p->completion_time, p->waiting_time,
                   p->turnaround_time, p->response_time);
    }
}

// Print performance metrics
void print_performance_metrics(const char *algorithm_name, performance_metrics_t *metrics) {
    safe_printf("\n=== %s Performance Metrics ===\n", algorithm_name);
    safe_printf("Average Waiting Time:    %.2f\n", metrics->avg_waiting_time);
    safe_printf("Average Turnaround Time: %.2f\n", metrics->avg_turnaround_time);
    safe_printf("Average Response Time:   %.2f\n", metrics->avg_response_time);
    safe_printf("CPU Utilization:         %.2f%%\n", metrics->cpu_utilization);
    safe_printf("Throughput:              %.2f processes/time unit\n", metrics->throughput);
    safe_printf("Context Switches:        %d\n", metrics->total_context_switches);
}

// Print simplified Gantt chart
void print_gantt_chart(scheduler_context_t *ctx) {
    safe_printf("\n=== Simplified Gantt Chart ===\n");
    safe_printf("Timeline: ");
    for (int t = 0; t <= ctx->current_time; t += 5) {
        safe_printf("%-5d", t);
    }
    safe_printf("\n");
    
    safe_printf("Execution: ");
    // This is a simplified representation
    for (int i = 0; i < ctx->process_count; i++) {
        safe_printf("[P%d] ", ctx->processes[i].pid);
    }
    safe_printf("\n");
}

// Demonstrate specific algorithm
void demonstrate_algorithm(scheduling_algorithm_t algorithm, const char *name, int time_quantum) {
    scheduler_context_t ctx;
    performance_metrics_t metrics;
    
    safe_printf("\n" "========================================\n");
    safe_printf("Demonstrating %s\n", name);
    safe_printf("========================================\n");
    
    initialize_scheduler_context(&ctx, algorithm, time_quantum);
    generate_process_workload(&ctx, 5, false); // Use predefined workload
    
    switch (algorithm) {
        case ALGORITHM_FCFS:
            schedule_fcfs(&ctx);
            break;
        case ALGORITHM_SJF:
            schedule_sjf(&ctx);
            break;
        case ALGORITHM_ROUND_ROBIN:
            schedule_round_robin(&ctx);
            break;
        case ALGORITHM_PRIORITY:
            schedule_priority(&ctx);
            break;
    }
    
    print_process_table(&ctx);
    calculate_performance_metrics(&ctx, &metrics);
    print_performance_metrics(name, &metrics);
    print_gantt_chart(&ctx);
}

// Compare all scheduling algorithms
void compare_algorithms(void) {
    safe_printf("\n" "=========================================\n");
    safe_printf("CPU Scheduling Algorithms Comparison\n");
    safe_printf("=========================================\n");
    
    scheduler_context_t contexts[MAX_ALGORITHMS];
    performance_metrics_t metrics[MAX_ALGORITHMS];
    const char *algorithm_names[] = {
        "FCFS", "SJF", "Round Robin", "Priority"
    };
    
    // Initialize and run all algorithms with same workload
    for (int i = 0; i < MAX_ALGORITHMS; i++) {
        initialize_scheduler_context(&contexts[i], (scheduling_algorithm_t)i, 3);
        generate_process_workload(&contexts[i], 5, false);
        
        switch (i) {
            case ALGORITHM_FCFS:
                schedule_fcfs(&contexts[i]);
                break;
            case ALGORITHM_SJF:
                schedule_sjf(&contexts[i]);
                break;
            case ALGORITHM_ROUND_ROBIN:
                schedule_round_robin(&contexts[i]);
                break;
            case ALGORITHM_PRIORITY:
                schedule_priority(&contexts[i]);
                break;
        }
        
        calculate_performance_metrics(&contexts[i], &metrics[i]);
    }
    
    // Print comparison table
    safe_printf("\nAlgorithm Performance Comparison:\n");
    safe_printf("Algorithm     | Avg Wait | Avg Turn | Avg Resp | CPU Util | Throughput\n");
    safe_printf("--------------------------------------------------------------------------\n");
    
    for (int i = 0; i < MAX_ALGORITHMS; i++) {
        safe_printf("%-12s | %8.2f | %8.2f | %8.2f | %7.2f%% | %9.2f\n",
                   algorithm_names[i],
                   metrics[i].avg_waiting_time,
                   metrics[i].avg_turnaround_time,
                   metrics[i].avg_response_time,
                   metrics[i].cpu_utilization,
                   metrics[i].throughput);
    }
    
    // Find best algorithm for each metric
    safe_printf("\nBest Performance by Metric:\n");
    
    int best_wait = 0, best_turnaround = 0, best_response = 0, best_utilization = 0;
    for (int i = 1; i < MAX_ALGORITHMS; i++) {
        if (metrics[i].avg_waiting_time < metrics[best_wait].avg_waiting_time)
            best_wait = i;
        if (metrics[i].avg_turnaround_time < metrics[best_turnaround].avg_turnaround_time)
            best_turnaround = i;
        if (metrics[i].avg_response_time < metrics[best_response].avg_response_time)
            best_response = i;
        if (metrics[i].cpu_utilization > metrics[best_utilization].cpu_utilization)
            best_utilization = i;
    }
    
    safe_printf("Best Average Waiting Time:    %s (%.2f)\n", 
               algorithm_names[best_wait], metrics[best_wait].avg_waiting_time);
    safe_printf("Best Average Turnaround Time: %s (%.2f)\n", 
               algorithm_names[best_turnaround], metrics[best_turnaround].avg_turnaround_time);
    safe_printf("Best Average Response Time:   %s (%.2f)\n", 
               algorithm_names[best_response], metrics[best_response].avg_response_time);
    safe_printf("Best CPU Utilization:         %s (%.2f%%)\n", 
               algorithm_names[best_utilization], metrics[best_utilization].cpu_utilization);
}

int main(void) {
    safe_printf("CPU Scheduling Algorithms Demonstration\n");
    safe_printf("======================================\n");
    safe_printf("This program demonstrates and compares basic CPU scheduling algorithms.\n");
    safe_printf("Each algorithm is tested with the same set of processes for fair comparison.\n");
    
    // Demonstrate each algorithm individually
    demonstrate_algorithm(ALGORITHM_FCFS, "First-Come-First-Serve", 0);
    demonstrate_algorithm(ALGORITHM_SJF, "Shortest Job First", 0);
    demonstrate_algorithm(ALGORITHM_ROUND_ROBIN, "Round Robin (q=3)", 3);
    demonstrate_algorithm(ALGORITHM_PRIORITY, "Priority Scheduling", 0);
    
    // Compare all algorithms
    compare_algorithms();
    
    safe_printf("\n=== Educational Summary ===\n");
    safe_printf("FCFS: Simple but can cause convoy effect with long processes\n");
    safe_printf("SJF: Optimal for average waiting time but requires burst time prediction\n");
    safe_printf("Round Robin: Good response time for interactive systems\n");
    safe_printf("Priority: Allows importance-based scheduling but may cause starvation\n");
    
    safe_printf("\nDemonstration completed successfully!\n");
    return 0;
} 