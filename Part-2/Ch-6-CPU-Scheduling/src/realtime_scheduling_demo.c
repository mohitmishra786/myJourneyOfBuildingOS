#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>

#define MAX_RT_TASKS 10
#define MAX_NAME_LENGTH 32
#define SIMULATION_TIME 100
#define HYPERPERIOD_LIMIT 1000

// Real-time task structure
typedef struct {
    int id;
    char name[MAX_NAME_LENGTH];
    int period;
    int execution_time;
    int deadline;
    int priority;          // For RMS (inverse of period)
    int next_release;
    int next_deadline;
    int remaining_execution;
    int instances_released;
    int instances_completed;
    int instances_missed;
    double utilization;
    bool active;
    bool deadline_missed;
} rt_task_t;

// Real-time scheduling context
typedef struct {
    rt_task_t tasks[MAX_RT_TASKS];
    int task_count;
    int current_time;
    int hyperperiod;
    double total_utilization;
    int total_deadline_misses;
    int total_preemptions;
    bool schedulable;
} rt_scheduler_t;

// Scheduling algorithms
typedef enum {
    RMS_SCHEDULING,
    EDF_SCHEDULING
} rt_algorithm_t;

// Function prototypes
void safe_printf(const char *format, ...);
void initialize_rt_scheduler(rt_scheduler_t *scheduler);
void add_rt_task(rt_scheduler_t *scheduler, int id, const char *name, int period, int exec_time, int deadline);
void generate_rt_workload(rt_scheduler_t *scheduler);
int calculate_hyperperiod(rt_scheduler_t *scheduler);
int gcd(int a, int b);
int lcm(int a, int b);
bool check_rms_schedulability(rt_scheduler_t *scheduler);
bool check_edf_schedulability(rt_scheduler_t *scheduler);
void reset_rt_scheduler(rt_scheduler_t *scheduler);
void simulate_rms_scheduling(rt_scheduler_t *scheduler);
void simulate_edf_scheduling(rt_scheduler_t *scheduler);
void release_periodic_tasks(rt_scheduler_t *scheduler);
rt_task_t *select_rms_task(rt_scheduler_t *scheduler);
rt_task_t *select_edf_task(rt_scheduler_t *scheduler);
void execute_task(rt_scheduler_t *scheduler, rt_task_t *task);
void check_deadlines(rt_scheduler_t *scheduler);
void print_rt_task_set(rt_scheduler_t *scheduler);
void print_rt_results(rt_scheduler_t *scheduler, rt_algorithm_t algorithm);
void print_schedulability_analysis(rt_scheduler_t *scheduler);
void print_timeline(rt_scheduler_t *scheduler, rt_algorithm_t algorithm);
void demonstrate_rms_scheduling(void);
void demonstrate_edf_scheduling(void);
void compare_rt_algorithms(void);

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

// Initialize real-time scheduler
void initialize_rt_scheduler(rt_scheduler_t *scheduler) {
    memset(scheduler, 0, sizeof(rt_scheduler_t));
    scheduler->current_time = 0;
    scheduler->task_count = 0;
    scheduler->total_utilization = 0.0;
    scheduler->schedulable = false;
}

// Add real-time task to scheduler
void add_rt_task(rt_scheduler_t *scheduler, int id, const char *name, int period, int exec_time, int deadline) {
    if (scheduler->task_count < MAX_RT_TASKS) {
        rt_task_t *task = &scheduler->tasks[scheduler->task_count];
        
        task->id = id;
        strcpy(task->name, name);
        task->period = period;
        task->execution_time = exec_time;
        task->deadline = (deadline == 0) ? period : deadline; // Implicit deadline if not specified
        task->priority = period; // RMS priority is inverse of period (lower value = higher priority)
        task->utilization = (double)exec_time / period;
        task->next_release = 0;
        task->next_deadline = task->deadline;
        task->remaining_execution = 0;
        task->instances_released = 0;
        task->instances_completed = 0;
        task->instances_missed = 0;
        task->active = false;
        task->deadline_missed = false;
        
        scheduler->total_utilization += task->utilization;
        scheduler->task_count++;
    }
}

// Generate real-time workload
void generate_rt_workload(rt_scheduler_t *scheduler) {
    // Example real-time task set
    add_rt_task(scheduler, 1, "Control", 10, 3, 10);    // High frequency control task
    add_rt_task(scheduler, 2, "Sensor", 15, 2, 15);     // Sensor reading task
    add_rt_task(scheduler, 3, "Display", 25, 4, 25);    // Display update task
    add_rt_task(scheduler, 4, "Network", 30, 5, 30);    // Network communication
    add_rt_task(scheduler, 5, "Logger", 50, 3, 50);     // Data logging task
}

// Calculate greatest common divisor
int gcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// Calculate least common multiple
int lcm(int a, int b) {
    return (a * b) / gcd(a, b);
}

// Calculate hyperperiod (LCM of all periods)
int calculate_hyperperiod(rt_scheduler_t *scheduler) {
    if (scheduler->task_count == 0) return 0;
    
    int hp = scheduler->tasks[0].period;
    for (int i = 1; i < scheduler->task_count; i++) {
        hp = lcm(hp, scheduler->tasks[i].period);
        if (hp > HYPERPERIOD_LIMIT) {
            return HYPERPERIOD_LIMIT; // Cap hyperperiod for simulation
        }
    }
    return hp;
}

// Check RMS schedulability using utilization bound
bool check_rms_schedulability(rt_scheduler_t *scheduler) {
    int n = scheduler->task_count;
    double utilization_bound = n * (pow(2.0, 1.0/n) - 1.0);
    
    safe_printf("RMS Utilization Bound Test:\n");
    safe_printf("Number of tasks: %d\n", n);
    safe_printf("Utilization bound: %.3f\n", utilization_bound);
    safe_printf("Total utilization: %.3f\n", scheduler->total_utilization);
    
    if (scheduler->total_utilization <= utilization_bound) {
        safe_printf("Result: SCHEDULABLE (utilization test passed)\n");
        return true;
    } else {
        safe_printf("Result: UNKNOWN (utilization test failed, need exact analysis)\n");
        return false;
    }
}

// Check EDF schedulability using utilization bound
bool check_edf_schedulability(rt_scheduler_t *scheduler) {
    safe_printf("EDF Utilization Test:\n");
    safe_printf("Total utilization: %.3f\n", scheduler->total_utilization);
    safe_printf("EDF utilization bound: 1.000\n");
    
    if (scheduler->total_utilization <= 1.0) {
        safe_printf("Result: SCHEDULABLE (utilization ≤ 1.0)\n");
        return true;
    } else {
        safe_printf("Result: NOT SCHEDULABLE (utilization > 1.0)\n");
        return false;
    }
}

// Reset scheduler for new simulation
void reset_rt_scheduler(rt_scheduler_t *scheduler) {
    scheduler->current_time = 0;
    scheduler->total_deadline_misses = 0;
    scheduler->total_preemptions = 0;
    
    for (int i = 0; i < scheduler->task_count; i++) {
        rt_task_t *task = &scheduler->tasks[i];
        task->next_release = 0;
        task->next_deadline = task->deadline;
        task->remaining_execution = 0;
        task->instances_released = 0;
        task->instances_completed = 0;
        task->instances_missed = 0;
        task->active = false;
        task->deadline_missed = false;
    }
}

// Release periodic tasks at their release times
void release_periodic_tasks(rt_scheduler_t *scheduler) {
    for (int i = 0; i < scheduler->task_count; i++) {
        rt_task_t *task = &scheduler->tasks[i];
        
        if (scheduler->current_time >= task->next_release) {
            // Release new instance
            if (task->active && task->remaining_execution > 0) {
                // Previous instance didn't complete - deadline miss
                task->instances_missed++;
                scheduler->total_deadline_misses++;
                safe_printf("  [DEADLINE MISS] Task %s missed deadline at time %d\n", 
                           task->name, scheduler->current_time);
            }
            
            task->active = true;
            task->remaining_execution = task->execution_time;
            task->instances_released++;
            task->next_release += task->period;
            task->next_deadline = scheduler->current_time + task->deadline;
            task->deadline_missed = false;
            
            safe_printf("  [RELEASE] Task %s released at time %d (deadline: %d)\n", 
                       task->name, scheduler->current_time, task->next_deadline);
        }
    }
}

// Select highest priority task for RMS
rt_task_t *select_rms_task(rt_scheduler_t *scheduler) {
    rt_task_t *selected = NULL;
    int highest_priority = INT_MAX;
    
    for (int i = 0; i < scheduler->task_count; i++) {
        rt_task_t *task = &scheduler->tasks[i];
        
        if (task->active && task->remaining_execution > 0) {
            if (task->priority < highest_priority) {
                highest_priority = task->priority;
                selected = task;
            }
        }
    }
    
    return selected;
}

// Select earliest deadline task for EDF
rt_task_t *select_edf_task(rt_scheduler_t *scheduler) {
    rt_task_t *selected = NULL;
    int earliest_deadline = INT_MAX;
    
    for (int i = 0; i < scheduler->task_count; i++) {
        rt_task_t *task = &scheduler->tasks[i];
        
        if (task->active && task->remaining_execution > 0) {
            if (task->next_deadline < earliest_deadline) {
                earliest_deadline = task->next_deadline;
                selected = task;
            }
        }
    }
    
    return selected;
}

// Execute selected task for one time unit
void execute_task(rt_scheduler_t *scheduler, rt_task_t *task) {
    if (task != NULL && task->remaining_execution > 0) {
        task->remaining_execution--;
        
        if (task->remaining_execution == 0) {
            // Task instance completed
            task->active = false;
            task->instances_completed++;
            safe_printf("  [COMPLETE] Task %s completed at time %d\n", 
                       task->name, scheduler->current_time + 1);
        }
    }
}

// Check for deadline violations
void check_deadlines(rt_scheduler_t *scheduler) {
    for (int i = 0; i < scheduler->task_count; i++) {
        rt_task_t *task = &scheduler->tasks[i];
        
        if (task->active && scheduler->current_time >= task->next_deadline && 
            task->remaining_execution > 0) {
            // Deadline missed
            if (!task->deadline_missed) {
                task->instances_missed++;
                scheduler->total_deadline_misses++;
                task->deadline_missed = true;
                safe_printf("  [DEADLINE MISS] Task %s missed deadline at time %d\n", 
                           task->name, scheduler->current_time);
            }
        }
    }
}

// Simulate RMS scheduling
void simulate_rms_scheduling(rt_scheduler_t *scheduler) {
    safe_printf("\n=== RMS Scheduling Simulation ===\n");
    
    reset_rt_scheduler(scheduler);
    scheduler->hyperperiod = calculate_hyperperiod(scheduler);
    int simulation_end = (scheduler->hyperperiod < SIMULATION_TIME) ? 
                        scheduler->hyperperiod : SIMULATION_TIME;
    
    rt_task_t *current_task = NULL;
    rt_task_t *previous_task = NULL;
    
    for (scheduler->current_time = 0; scheduler->current_time < simulation_end; scheduler->current_time++) {
        safe_printf("Time %d:\n", scheduler->current_time);
        
        // Release periodic tasks
        release_periodic_tasks(scheduler);
        
        // Check deadlines
        check_deadlines(scheduler);
        
        // Select task to execute (RMS: highest priority = shortest period)
        current_task = select_rms_task(scheduler);
        
        // Check for preemption
        if (current_task != previous_task && previous_task != NULL) {
            scheduler->total_preemptions++;
            safe_printf("  [PREEMPTION] Task %s preempted by %s\n", 
                       previous_task->name, current_task ? current_task->name : "IDLE");
        }
        
        // Execute selected task
        if (current_task != NULL) {
            safe_printf("  [EXECUTE] Task %s (remaining: %d)\n", 
                       current_task->name, current_task->remaining_execution);
            execute_task(scheduler, current_task);
        } else {
            safe_printf("  [IDLE] No task to execute\n");
        }
        
        previous_task = current_task;
    }
}

// Simulate EDF scheduling
void simulate_edf_scheduling(rt_scheduler_t *scheduler) {
    safe_printf("\n=== EDF Scheduling Simulation ===\n");
    
    reset_rt_scheduler(scheduler);
    scheduler->hyperperiod = calculate_hyperperiod(scheduler);
    int simulation_end = (scheduler->hyperperiod < SIMULATION_TIME) ? 
                        scheduler->hyperperiod : SIMULATION_TIME;
    
    rt_task_t *current_task = NULL;
    rt_task_t *previous_task = NULL;
    
    for (scheduler->current_time = 0; scheduler->current_time < simulation_end; scheduler->current_time++) {
        safe_printf("Time %d:\n", scheduler->current_time);
        
        // Release periodic tasks
        release_periodic_tasks(scheduler);
        
        // Check deadlines
        check_deadlines(scheduler);
        
        // Select task to execute (EDF: earliest deadline)
        current_task = select_edf_task(scheduler);
        
        // Check for preemption
        if (current_task != previous_task && previous_task != NULL) {
            scheduler->total_preemptions++;
            safe_printf("  [PREEMPTION] Task %s preempted by %s\n", 
                       previous_task->name, current_task ? current_task->name : "IDLE");
        }
        
        // Execute selected task
        if (current_task != NULL) {
            safe_printf("  [EXECUTE] Task %s (deadline: %d, remaining: %d)\n", 
                       current_task->name, current_task->next_deadline, current_task->remaining_execution);
            execute_task(scheduler, current_task);
        } else {
            safe_printf("  [IDLE] No task to execute\n");
        }
        
        previous_task = current_task;
    }
}

// Print real-time task set
void print_rt_task_set(rt_scheduler_t *scheduler) {
    safe_printf("\n=== Real-time Task Set ===\n");
    safe_printf("ID  Name      Period  Exec  Deadline  Utilization\n");
    safe_printf("------------------------------------------------\n");
    
    for (int i = 0; i < scheduler->task_count; i++) {
        rt_task_t *task = &scheduler->tasks[i];
        safe_printf("%-3d %-8s %-6d %-5d %-8d %.3f\n",
                   task->id, task->name, task->period, task->execution_time,
                   task->deadline, task->utilization);
    }
    
    safe_printf("\nTotal Utilization: %.3f\n", scheduler->total_utilization);
    safe_printf("Hyperperiod: %d\n", scheduler->hyperperiod);
}

// Print scheduling results
void print_rt_results(rt_scheduler_t *scheduler, rt_algorithm_t algorithm) {
    const char *alg_name = (algorithm == RMS_SCHEDULING) ? "RMS" : "EDF";
    
    safe_printf("\n=== %s Scheduling Results ===\n", alg_name);
    safe_printf("Task  Released  Completed  Missed  Success Rate\n");
    safe_printf("-----------------------------------------------\n");
    
    for (int i = 0; i < scheduler->task_count; i++) {
        rt_task_t *task = &scheduler->tasks[i];
        double success_rate = (task->instances_released > 0) ? 
                             (double)task->instances_completed / task->instances_released * 100.0 : 0.0;
        
        safe_printf("%-5s %-8d %-9d %-7d %.1f%%\n",
                   task->name, task->instances_released, task->instances_completed,
                   task->instances_missed, success_rate);
    }
    
    safe_printf("\nTotal Deadline Misses: %d\n", scheduler->total_deadline_misses);
    safe_printf("Total Preemptions: %d\n", scheduler->total_preemptions);
    
    if (scheduler->total_deadline_misses == 0) {
        safe_printf("Result: ALL DEADLINES MET\n");
    } else {
        safe_printf("Result: %d DEADLINE VIOLATIONS\n", scheduler->total_deadline_misses);
    }
}

// Print schedulability analysis
void print_schedulability_analysis(rt_scheduler_t *scheduler) {
    safe_printf("\n=== Schedulability Analysis ===\n");
    
    safe_printf("\n--- Rate Monotonic Scheduling ---\n");
    bool rms_schedulable = check_rms_schedulability(scheduler);
    
    safe_printf("\n--- Earliest Deadline First ---\n");
    bool edf_schedulable = check_edf_schedulability(scheduler);
    
    safe_printf("\n=== Theoretical Comparison ===\n");
    safe_printf("RMS Schedulable: %s\n", rms_schedulable ? "YES" : "UNKNOWN");
    safe_printf("EDF Schedulable: %s\n", edf_schedulable ? "YES" : "NO");
    safe_printf("EDF can schedule any task set that RMS can schedule.\n");
    safe_printf("EDF achieves 100%% utilization bound vs RMS ~69.3%% bound.\n");
}

// Demonstrate RMS scheduling
void demonstrate_rms_scheduling(void) {
    safe_printf("\n" "========================================\n");
    safe_printf("Rate Monotonic Scheduling Demonstration\n");
    safe_printf("========================================\n");
    
    rt_scheduler_t scheduler;
    initialize_rt_scheduler(&scheduler);
    generate_rt_workload(&scheduler);
    
    print_rt_task_set(&scheduler);
    print_schedulability_analysis(&scheduler);
    
    safe_printf("\n=== Simulating RMS ===\n");
    simulate_rms_scheduling(&scheduler);
    print_rt_results(&scheduler, RMS_SCHEDULING);
}

// Demonstrate EDF scheduling
void demonstrate_edf_scheduling(void) {
    safe_printf("\n" "========================================\n");
    safe_printf("Earliest Deadline First Demonstration\n");
    safe_printf("========================================\n");
    
    rt_scheduler_t scheduler;
    initialize_rt_scheduler(&scheduler);
    generate_rt_workload(&scheduler);
    
    print_rt_task_set(&scheduler);
    
    safe_printf("\n=== Simulating EDF ===\n");
    simulate_edf_scheduling(&scheduler);
    print_rt_results(&scheduler, EDF_SCHEDULING);
}

// Compare RMS and EDF algorithms
void compare_rt_algorithms(void) {
    safe_printf("\n" "========================================\n");
    safe_printf("RMS vs EDF Comparison\n");
    safe_printf("========================================\n");
    
    rt_scheduler_t rms_scheduler, edf_scheduler;
    
    // Initialize both schedulers with same task set
    initialize_rt_scheduler(&rms_scheduler);
    initialize_rt_scheduler(&edf_scheduler);
    generate_rt_workload(&rms_scheduler);
    generate_rt_workload(&edf_scheduler);
    
    print_rt_task_set(&rms_scheduler);
    print_schedulability_analysis(&rms_scheduler);
    
    // Simulate both algorithms
    safe_printf("\n=== Running Both Algorithms ===\n");
    simulate_rms_scheduling(&rms_scheduler);
    simulate_edf_scheduling(&edf_scheduler);
    
    // Print comparison
    safe_printf("\n=== Performance Comparison ===\n");
    safe_printf("Metric                | RMS    | EDF\n");
    safe_printf("--------------------------------\n");
    safe_printf("Deadline Misses       | %-6d | %-6d\n", 
               rms_scheduler.total_deadline_misses, edf_scheduler.total_deadline_misses);
    safe_printf("Preemptions           | %-6d | %-6d\n", 
               rms_scheduler.total_preemptions, edf_scheduler.total_preemptions);
    
    safe_printf("\n=== Algorithm Characteristics ===\n");
    safe_printf("Rate Monotonic Scheduling (RMS):\n");
    safe_printf("+ Simple implementation (static priorities)\n");
    safe_printf("+ Predictable behavior\n");
    safe_printf("+ Well-established theory\n");
    safe_printf("- Lower utilization bound (~69.3%%)\n");
    safe_printf("- May miss deadlines that EDF could meet\n");
    
    safe_printf("\nEarliest Deadline First (EDF):\n");
    safe_printf("+ Optimal for single processor\n");
    safe_printf("+ 100%% utilization bound\n");
    safe_printf("+ Better average response time\n");
    safe_printf("- Higher implementation complexity\n");
    safe_printf("- Dynamic priority overhead\n");
}

int main(void) {
    safe_printf("Real-time CPU Scheduling Demonstration\n");
    safe_printf("======================================\n");
    safe_printf("This program demonstrates real-time scheduling algorithms\n");
    safe_printf("and analyzes their schedulability properties.\n");
    
    // Demonstrate individual algorithms
    demonstrate_rms_scheduling();
    demonstrate_edf_scheduling();
    
    // Compare algorithms
    compare_rt_algorithms();
    
    safe_printf("\n=== Key Takeaways ===\n");
    safe_printf("1. EDF is optimal for single-processor real-time scheduling\n");
    safe_printf("2. RMS provides simpler implementation but lower utilization\n");
    safe_printf("3. Both require careful schedulability analysis\n");
    safe_printf("4. Real-time systems prioritize predictability over average performance\n");
    
    safe_printf("\nDemonstration completed successfully!\n");
    return 0;
} 