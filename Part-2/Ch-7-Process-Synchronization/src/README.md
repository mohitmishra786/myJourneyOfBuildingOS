# Process Synchronization Demonstrations

This directory contains practical implementations of the synchronization concepts covered in Chapter 7.

## Programs

### 1. Race Condition Demo (`race_condition_demo.c`)
Demonstrates the problems that occur when multiple threads access shared data without proper synchronization.

**Features:**
- Unsafe counter increment (shows race conditions)
- Mutex-protected counter increment (safe)
- Atomic operations counter increment (lock-free)

**Usage:**
```bash
make run-race
```

### 2. Producer-Consumer Demo (`producer_consumer_demo.c`)
Implementation of the classic producer-consumer problem using semaphores and mutexes.

**Features:**
- Multiple producers and consumers
- Bounded buffer with semaphore synchronization
- Real-time statistics tracking

**Usage:**
```bash
make run-producer-consumer
```

### 3. Dining Philosophers Demo (`dining_philosophers_demo.c`)
Demonstrates different solutions to the dining philosophers problem and deadlock prevention.

**Features:**
- Monitor-based solution
- Asymmetric solution (deadlock prevention)
- Tanenbaum's solution (resource limitation)
- Timed simulation with statistics

**Usage:**
```bash
make run-philosophers
```

## Building and Running

### Build All Programs
```bash
make all
```

### Run Individual Programs
```bash
make run-race                # Race condition demonstration
make run-producer-consumer   # Producer-consumer implementation
make run-philosophers        # Dining philosophers solutions
```

### Run All Demonstrations
```bash
make run-all
```

### Clean Build Files
```bash
make clean
```

## Key Learning Points

### Race Conditions
- Observe how unsynchronized access leads to incorrect results
- Compare performance and correctness of different synchronization methods
- Understanding atomic operations vs. locks

### Producer-Consumer Pattern
- Proper use of semaphores for coordination
- Buffer management in concurrent systems
- Avoiding deadlock through correct semaphore ordering

### Dining Philosophers
- Deadlock conditions and prevention strategies
- Different approaches to resource allocation
- Monitor-based vs. semaphore-based solutions

## Expected Output Patterns

### Race Condition Demo
- Unsafe counter: Results vary and are usually less than expected
- Safe counter: Always produces correct results
- Atomic counter: Correct results with potentially better performance

### Producer-Consumer Demo
- Balanced production and consumption
- No buffer overflows or underflows
- Clean termination when all items are processed

### Dining Philosophers Demo
- No deadlock in any of the three solutions
- Fair access to resources across philosophers
- Statistics showing meal distribution

## Compilation Requirements

- GCC compiler with C99 support
- POSIX threads library (pthread)
- Semaphore support (part of POSIX)

The programs are designed to work on Unix-like systems (Linux, macOS) with standard POSIX compliance. 