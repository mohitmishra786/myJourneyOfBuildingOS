# Chapter 4: Process Management - Source Code Examples

This directory contains C implementations demonstrating key concepts from Chapter 4 on Process Management. These examples provide practical illustrations of process concepts, scheduling algorithms, context switching, and process control mechanisms.

## Overview

The source code in this directory implements:

1. **Process Management Simulator** (`process_simulator.c`)
2. **Context Switching Demonstration** (`context_switch_demo.c`)

These implementations provide hands-on experience with process management concepts covered in the theoretical discussions.

## Files Description

### `process_simulator.c`
A comprehensive process management simulator that demonstrates:
- Process Control Block (PCB) structure and management
- Process state transitions (New, Ready, Running, Waiting, Terminated)
- Round-robin scheduling algorithm implementation
- Process queue management
- Basic process statistics and performance metrics

**Key Features:**
- Simulates multiple processes with different characteristics
- Implements time quantum-based scheduling
- Tracks process waiting times and turnaround times
- Provides real-time visualization of process state changes
- Calculates CPU utilization and scheduling statistics

### `context_switch_demo.c`
An interactive demonstration of context switching mechanisms:
- CPU context structure (registers, program counter, stack pointer)
- Context saving and restoration procedures
- Timer interrupt simulation
- Process priority handling
- Context switch overhead calculation

**Key Features:**
- Simulates realistic CPU context with registers and control structures
- Demonstrates the overhead cost of context switching
- Shows how timer interrupts trigger scheduling decisions
- Provides detailed context information for each process
- Calculates context switch performance metrics

### `Makefile`
Build configuration for compiling and running the examples:
- Builds both demonstration programs
- Provides individual and combined execution targets
- Includes cleanup and help functionality
- Generates demonstration output files

### `README.md`
This documentation file explaining the examples and their usage.

## Building the Examples

### Prerequisites
- GCC compiler with C99 support
- Make utility
- UNIX-like environment (Linux, macOS, WSL)

### Compilation
```bash
# Build all examples
make all

# Build individual examples
make process_simulator
make context_switch_demo

# Show available targets
make help
```

## Running the Demonstrations

### Process Management Simulator
```bash
# Run the process simulator
make run-process

# Or run directly
./process_simulator
```

**Expected Output:**
- Initial process information display
- Real-time simulation of process scheduling
- Process state transitions and timing information
- Final performance statistics

### Context Switch Demonstration
```bash
# Run the context switch demo
make run-context

# Or run directly
./context_switch_demo
```

**Expected Output:**
- Process context initialization
- Detailed context switch procedures
- Timer interrupt handling
- Context switch performance analysis

### Run All Demonstrations
```bash
# Run all demos and save output
make demo

# Run all demos interactively
make run-all
```

## Educational Value

### Process Management Simulator
This simulator helps understand:
- How operating systems manage multiple processes
- The role of Process Control Blocks in process management
- How scheduling algorithms affect system performance
- The relationship between time quanta and system responsiveness
- Process waiting times and turnaround time calculations

### Context Switch Demonstration
This demonstration illustrates:
- The complexity of context switching operations
- CPU state preservation and restoration
- The performance overhead of context switches
- How timer interrupts enable preemptive multitasking
- The trade-offs between context switching frequency and overhead

## Key Concepts Demonstrated

### Process States
```c
typedef enum {
    PROCESS_NEW = 0,
    PROCESS_READY = 1,
    PROCESS_RUNNING = 2,
    PROCESS_WAITING = 3,
    PROCESS_TERMINATED = 4
} process_state_t;
```

### Process Control Block
```c
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
```

### CPU Context
```c
typedef struct cpu_context {
    uint32_t registers[16];      /* General purpose registers */
    uint32_t program_counter;    /* Program counter */
    uint32_t stack_pointer;      /* Stack pointer */
    uint32_t status_flags;       /* Processor status flags */
    uint32_t instruction_count;  /* Instructions executed */
} cpu_context_t;
```

## Performance Metrics

The examples calculate and display various performance metrics:

### Process Scheduling Metrics
- **CPU Utilization**: Percentage of time CPU is busy
- **Turnaround Time**: Time from process arrival to completion
- **Waiting Time**: Time process spends in ready queue
- **Response Time**: Time from arrival to first execution

### Context Switch Metrics
- **Context Switch Count**: Total number of context switches
- **Overhead Cycles**: CPU cycles spent on context switching
- **Overhead Percentage**: Context switch overhead as percentage of total time

## Customization and Experiments

### Modifying Process Characteristics
You can experiment with different process configurations:
```c
/* Create processes with different burst times and priorities */
pcb_t* p1 = create_process(101, "CPU_Intensive", 20, 1, 0);  /* Long CPU burst */
pcb_t* p2 = create_process(102, "Interactive", 2, 3, 1);     /* Short burst, high priority */
pcb_t* p3 = create_process(103, "Background", 15, 1, 2);     /* Long burst, low priority */
```

### Adjusting Time Quantum
Experiment with different time quantum values:
```c
init_scheduler(&scheduler, 5);   /* Short time slice - more responsive */
init_scheduler(&scheduler, 20);  /* Long time slice - less overhead */
```

### Adding Scheduling Algorithms
The framework can be extended to implement different scheduling algorithms:
- First Come First Serve (FCFS)
- Shortest Job First (SJF)
- Priority Scheduling
- Multilevel Queue Scheduling

## Learning Objectives

After studying and running these examples, you should understand:

1. **Process Management Fundamentals**
   - Process lifecycle and state transitions
   - Role of Process Control Blocks
   - Process scheduling concepts

2. **Context Switching Mechanisms**
   - CPU context preservation and restoration
   - Context switch overhead and optimization
   - Timer-based preemptive scheduling

3. **Scheduling Algorithm Implementation**
   - Round-robin scheduling mechanics
   - Time quantum effects on performance
   - Process queue management

4. **Performance Analysis**
   - Calculating system performance metrics
   - Understanding trade-offs in process management
   - Analyzing context switch overhead

## Troubleshooting

### Common Build Issues
```bash
# If make fails, try manual compilation
gcc -Wall -Wextra -std=c99 -O2 -g -o process_simulator process_simulator.c

# For systems without make
gcc -o process_simulator process_simulator.c
gcc -o context_switch_demo context_switch_demo.c
```

### Runtime Issues
- Ensure sufficient terminal width for output formatting
- Some demonstrations include sleep delays for visualization
- Use Ctrl+C to interrupt long-running simulations

## Related Chapter Content

These examples complement the following sections in Chapter 4:
- Process Concepts and States
- Process Control Block (PCB)
- Context Switching
- Process Scheduling Algorithms

For complete theoretical background, refer to the corresponding markdown files in the chapter directory.

## Additional Resources

- Operating Systems: Three Easy Pieces (OSTEP)
- Modern Operating Systems by Andrew Tanenbaum
- Operating System Concepts by Silberschatz, Galvin, and Gagne
- Linux kernel source code for real-world implementations 