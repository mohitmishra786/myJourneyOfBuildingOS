#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>

/* Simulated CPU Context */
typedef struct cpu_context {
    uint32_t registers[16];      /* General purpose registers */
    uint32_t program_counter;    /* Program counter */
    uint32_t stack_pointer;      /* Stack pointer */
    uint32_t status_flags;       /* Processor status flags */
    uint32_t instruction_count;  /* Instructions executed */
} cpu_context_t;

/* Process states for context switching */
typedef enum {
    PROC_NEW,
    PROC_READY,
    PROC_RUNNING,
    PROC_BLOCKED,
    PROC_TERMINATED
} proc_state_t;

/* Process structure for context switching */
typedef struct context_process {
    uint32_t pid;
    char name[32];
    proc_state_t state;
    cpu_context_t context;
    uint32_t priority;
    uint32_t time_slice;
    uint32_t remaining_time;
    uint32_t total_time;
    struct context_process* next;
} context_process_t;

/* Context switch simulator */
typedef struct context_switch_simulator {
    context_process_t* current_process;
    context_process_t* process_list;
    uint32_t context_switch_count;
    uint32_t total_overhead_cycles;
    uint32_t system_time;
    bool timer_interrupt_pending;
} cs_simulator_t;

/* Function prototypes */
void init_context_simulator(cs_simulator_t* sim);
context_process_t* create_context_process(uint32_t pid, const char* name, uint32_t priority);
void save_context(context_process_t* process);
void restore_context(context_process_t* process);
void perform_context_switch(cs_simulator_t* sim, context_process_t* next_process);
void timer_interrupt_handler(cs_simulator_t* sim);
void simulate_process_execution(context_process_t* process, uint32_t cycles);
void add_process_to_list(cs_simulator_t* sim, context_process_t* process);
context_process_t* select_next_process(cs_simulator_t* sim);
void print_process_context(context_process_t* process);
void print_context_switch_stats(cs_simulator_t* sim);
void run_context_switch_demo(cs_simulator_t* sim);

/* Initialize context switch simulator */
void init_context_simulator(cs_simulator_t* sim) {
    sim->current_process = NULL;
    sim->process_list = NULL;
    sim->context_switch_count = 0;
    sim->total_overhead_cycles = 0;
    sim->system_time = 0;
    sim->timer_interrupt_pending = false;
}

/* Create a new process for context switching */
context_process_t* create_context_process(uint32_t pid, const char* name, uint32_t priority) {
    context_process_t* process = malloc(sizeof(context_process_t));
    if (!process) return NULL;
    
    process->pid = pid;
    strncpy(process->name, name, sizeof(process->name) - 1);
    process->name[sizeof(process->name) - 1] = '\0';
    process->state = PROC_NEW;
    process->priority = priority;
    process->time_slice = 10; /* Default time slice */
    process->remaining_time = process->time_slice;
    process->total_time = 0;
    process->next = NULL;
    
    /* Initialize context with random values to simulate real state */
    for (int i = 0; i < 16; i++) {
        process->context.registers[i] = rand() % 1000;
    }
    process->context.program_counter = 0x1000 + pid * 0x100;
    process->context.stack_pointer = 0x8000 + pid * 0x1000;
    process->context.status_flags = 0x200; /* Basic status flags */
    process->context.instruction_count = 0;
    
    return process;
}

/* Save current process context (simulated) */
void save_context(context_process_t* process) {
    printf("  Saving context for process %s (PID %u)\n", process->name, process->pid);
    printf("    PC: 0x%08X, SP: 0x%08X\n", 
           process->context.program_counter, process->context.stack_pointer);
    printf("    Registers: R0=%u, R1=%u, R2=%u, R3=%u\n",
           process->context.registers[0], process->context.registers[1],
           process->context.registers[2], process->context.registers[3]);
    
    /* Simulate context save overhead */
    usleep(1000); /* 1ms overhead */
}

/* Restore process context (simulated) */
void restore_context(context_process_t* process) {
    printf("  Restoring context for process %s (PID %u)\n", process->name, process->pid);
    printf("    PC: 0x%08X, SP: 0x%08X\n", 
           process->context.program_counter, process->context.stack_pointer);
    printf("    Registers: R0=%u, R1=%u, R2=%u, R3=%u\n",
           process->context.registers[0], process->context.registers[1],
           process->context.registers[2], process->context.registers[3]);
    
    /* Simulate context restore overhead */
    usleep(1000); /* 1ms overhead */
}

/* Perform context switch between processes */
void perform_context_switch(cs_simulator_t* sim, context_process_t* next_process) {
    printf("\n=== CONTEXT SWITCH (Time: %u) ===\n", sim->system_time);
    
    /* Save current process context if exists */
    if (sim->current_process) {
        printf("Switching FROM: %s (PID %u)\n", 
               sim->current_process->name, sim->current_process->pid);
        save_context(sim->current_process);
        
        /* Update process state */
        if (sim->current_process->state == PROC_RUNNING) {
            sim->current_process->state = PROC_READY;
            sim->current_process->remaining_time = sim->current_process->time_slice;
        }
    } else {
        printf("Switching FROM: NULL (initial context switch)\n");
    }
    
    /* Restore new process context */
    if (next_process) {
        printf("Switching TO: %s (PID %u)\n", next_process->name, next_process->pid);
        restore_context(next_process);
        
        /* Update process state */
        next_process->state = PROC_RUNNING;
        next_process->remaining_time = next_process->time_slice;
    }
    
    /* Update simulator state */
    sim->current_process = next_process;
    sim->context_switch_count++;
    sim->total_overhead_cycles += 10; /* Assume 10 cycles overhead */
    
    printf("Context switch completed (Total switches: %u)\n", sim->context_switch_count);
    printf("==============================\n\n");
}

/* Simulate timer interrupt */
void timer_interrupt_handler(cs_simulator_t* sim) {
    printf("*** TIMER INTERRUPT *** (Time: %u)\n", sim->system_time);
    
    if (sim->current_process) {
        printf("Process %s time slice expired\n", sim->current_process->name);
        sim->timer_interrupt_pending = true;
    }
}

/* Simulate process execution */
void simulate_process_execution(context_process_t* process, uint32_t cycles) {
    if (!process || process->state != PROC_RUNNING) return;
    
    /* Update process context to simulate execution */
    process->context.instruction_count += cycles;
    process->context.program_counter += cycles * 4; /* Assume 4 bytes per instruction */
    
    /* Modify some registers to simulate computation */
    process->context.registers[0] += cycles;
    process->context.registers[1] = process->context.registers[1] ^ cycles;
    
    process->total_time += cycles;
    process->remaining_time -= cycles;
    
    printf("Process %s executed %u cycles (Total: %u, Remaining slice: %u)\n",
           process->name, cycles, process->total_time, process->remaining_time);
}

/* Add process to the process list */
void add_process_to_list(cs_simulator_t* sim, context_process_t* process) {
    process->next = sim->process_list;
    sim->process_list = process;
    process->state = PROC_READY;
    printf("Added process %s (PID %u) to ready list\n", process->name, process->pid);
}

/* Select next process to run (simple round-robin) */
context_process_t* select_next_process(cs_simulator_t* sim) {
    context_process_t* current = sim->process_list;
    context_process_t* best_candidate = NULL;
    
    /* Find next ready process */
    while (current) {
        if (current->state == PROC_READY && current != sim->current_process) {
            if (!best_candidate || current->priority > best_candidate->priority) {
                best_candidate = current;
            }
        }
        current = current->next;
    }
    
    /* If no other ready process, continue with current */
    if (!best_candidate && sim->current_process && 
        sim->current_process->state == PROC_RUNNING) {
        return sim->current_process;
    }
    
    return best_candidate;
}

/* Print process context information */
void print_process_context(context_process_t* process) {
    const char* state_names[] = {"NEW", "READY", "RUNNING", "BLOCKED", "TERMINATED"};
    
    printf("\nProcess Context: %s (PID %u)\n", process->name, process->pid);
    printf("  State: %s\n", state_names[process->state]);
    printf("  Priority: %u\n", process->priority);
    printf("  Total execution time: %u cycles\n", process->total_time);
    printf("  Instructions executed: %u\n", process->context.instruction_count);
    printf("  Program Counter: 0x%08X\n", process->context.program_counter);
    printf("  Stack Pointer: 0x%08X\n", process->context.stack_pointer);
    printf("  Status Flags: 0x%08X\n", process->context.status_flags);
    printf("  Register R0: %u, R1: %u, R2: %u, R3: %u\n",
           process->context.registers[0], process->context.registers[1],
           process->context.registers[2], process->context.registers[3]);
}

/* Print context switch statistics */
void print_context_switch_stats(cs_simulator_t* sim) {
    printf("\n========== Context Switch Statistics ==========\n");
    printf("Total context switches: %u\n", sim->context_switch_count);
    printf("Total overhead cycles: %u\n", sim->total_overhead_cycles);
    printf("System time: %u\n", sim->system_time);
    
    if (sim->system_time > 0) {
        printf("Context switch overhead: %.2f%%\n", 
               (float)sim->total_overhead_cycles / sim->system_time * 100);
    }
    
    /* Print process statistics */
    context_process_t* process = sim->process_list;
    printf("\nProcess execution summary:\n");
    while (process) {
        printf("  %s: %u cycles (%.1f%% CPU)\n", 
               process->name, process->total_time,
               sim->system_time > 0 ? (float)process->total_time / sim->system_time * 100 : 0);
        process = process->next;
    }
    printf("=============================================\n");
}

/* Run context switching demonstration */
void run_context_switch_demo(cs_simulator_t* sim) {
    uint32_t simulation_cycles = 100;
    uint32_t time_slice = 10;
    
    printf("Starting Context Switch Demonstration\n");
    printf("Time slice: %u cycles\n", time_slice);
    printf("Total simulation: %u cycles\n\n", simulation_cycles);
    
    /* Create sample processes */
    context_process_t* p1 = create_context_process(101, "WebBrowser", 2);
    context_process_t* p2 = create_context_process(102, "TextEditor", 1);
    context_process_t* p3 = create_context_process(103, "Compiler", 3);
    
    /* Add processes to simulator */
    add_process_to_list(sim, p1);
    add_process_to_list(sim, p2);
    add_process_to_list(sim, p3);
    
    printf("\nInitial process contexts:\n");
    print_process_context(p1);
    print_process_context(p2);
    print_process_context(p3);
    
    /* Simulation loop */
    while (sim->system_time < simulation_cycles) {
        /* Check if we need to perform a context switch */
        if (!sim->current_process || sim->timer_interrupt_pending || 
            (sim->current_process && sim->current_process->remaining_time <= 0)) {
            
            context_process_t* next_process = select_next_process(sim);
            if (next_process != sim->current_process) {
                perform_context_switch(sim, next_process);
            }
            sim->timer_interrupt_pending = false;
        }
        
        /* Execute current process */
        if (sim->current_process) {
            uint32_t exec_cycles = (sim->current_process->remaining_time > 5) ? 5 : 
                                   sim->current_process->remaining_time;
            simulate_process_execution(sim->current_process, exec_cycles);
            
            /* Check for timer interrupt */
            if (sim->current_process->remaining_time <= 0) {
                timer_interrupt_handler(sim);
            }
        }
        
        sim->system_time += 5;
        
        /* Brief pause for demonstration */
        usleep(200000); /* 0.2 second */
    }
    
    /* Final statistics */
    print_context_switch_stats(sim);
}

int main(void) {
    srand(time(NULL));
    
    printf("Context Switching Demonstration\n");
    printf("==============================\n\n");
    
    cs_simulator_t simulator;
    init_context_simulator(&simulator);
    
    run_context_switch_demo(&simulator);
    
    /* Cleanup */
    context_process_t* current = simulator.process_list;
    while (current) {
        context_process_t* next = current->next;
        free(current);
        current = next;
    }
    
    return 0;
} 