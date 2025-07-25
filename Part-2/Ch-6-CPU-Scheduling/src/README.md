# CPU Scheduling Algorithms - Source Code Examples

This directory contains comprehensive C implementations of various CPU scheduling algorithms, designed to demonstrate the concepts, performance characteristics, and trade-offs discussed in Chapter 6 of our Operating Systems textbook.

## Overview

The source code examples provide practical implementations of:

1. **Basic Scheduling Algorithms** - FCFS, SJF, Round Robin, and Priority Scheduling
2. **Advanced Multilevel Scheduling** - Multilevel Queue and Multilevel Feedback Queue algorithms  
3. **Real-time Scheduling** - Rate Monotonic Scheduling (RMS) and Earliest Deadline First (EDF)

Each implementation includes performance analysis, comparison tools, and educational demonstrations to help understand the theoretical concepts through practical examples.

## Source Files

### 1. `scheduling_algorithms_demo.c`
**Basic CPU Scheduling Algorithms Implementation**

Demonstrates the four fundamental scheduling algorithms:
- **First-Come-First-Serve (FCFS)**: Simple FIFO scheduling
- **Shortest Job First (SJF)**: Optimal for minimizing average waiting time
- **Round Robin (RR)**: Time-sliced scheduling with configurable quantum
- **Priority Scheduling**: Priority-based with aging to prevent starvation

**Key Features:**
- Side-by-side performance comparison
- Gantt chart visualization
- Convoy effect demonstration
- Performance metrics calculation (waiting time, turnaround time, response time)
- Thread-safe implementation with detailed logging

### 2. `multilevel_queue_demo.c`
**Advanced Multilevel Scheduling Implementation**

Implements sophisticated scheduling approaches:
- **Multilevel Queue**: Fixed queue assignment based on process types
- **Multilevel Feedback Queue**: Dynamic queue migration with aging mechanisms

**Key Features:**
- Process classification (System, Interactive, Batch, Background)
- Dynamic process migration between queues
- Aging mechanism to prevent starvation
- Feedback-based behavior analysis
- Performance metrics per queue type
- Queue status monitoring and reporting

### 3. `realtime_scheduling_demo.c`
**Real-time Scheduling Algorithms Implementation**

Focuses on timing-critical scheduling algorithms:
- **Rate Monotonic Scheduling (RMS)**: Static priority based on task periods
- **Earliest Deadline First (EDF)**: Dynamic priority based on deadlines

**Key Features:**
- Schedulability analysis using utilization bounds
- Deadline miss detection and reporting
- Hyperperiod calculation for periodic task sets
- Mathematical schedulability tests
- Real-time task modeling with periods, deadlines, and execution times
- Preemption and context switch tracking

## Prerequisites

### Required Dependencies
- **GCC**: C compiler with C99 support
- **Make**: Build automation tool
- **Pthread Library**: POSIX threads support
- **Math Library**: Mathematical functions (libm)

### Platform Support
The code is designed to work on:
- **Linux**: Full feature support including real-time extensions
- **macOS**: Compatible with platform-specific adaptations
- **Other POSIX systems**: Basic functionality supported

### Optional Tools (for enhanced analysis)
- **Valgrind**: Memory leak detection and analysis
- **Cppcheck**: Static code analysis
- **Gcov**: Code coverage analysis
- **Clang-format**: Code formatting

## Building the Examples

### Basic Build
```bash
# Build all demonstrations
make all

# Build individual programs
make scheduling_algorithms_demo
make multilevel_queue_demo
make realtime_scheduling_demo
```

### Development Builds
```bash
# Debug build with symbols and debugging information
make debug

# Optimized release build
make release

# Check build dependencies
make check-deps
```

### Advanced Build Options
```bash
# Build with code coverage instrumentation
make coverage

# Static analysis of source code
make analyze

# Memory leak analysis (requires Valgrind)
make memcheck

# Performance analysis
make perf-test
```

## Running the Demonstrations

### Individual Demonstrations
```bash
# Basic scheduling algorithms comparison
./scheduling_algorithms_demo

# Multilevel queue scheduling
./multilevel_queue_demo

# Real-time scheduling analysis
./realtime_scheduling_demo
```

### Automated Testing
```bash
# Run all demonstrations with full output
make test

# Quick test with abbreviated output
make test-quick

# Interactive demonstration selector
make interactive

# Performance benchmarks
make benchmark
```

### Educational Mode
```bash
# Show available demonstrations
make demo

# Display comprehensive documentation
make docs

# Show all available make targets
make help
```

## Key Concepts Demonstrated

### 1. Basic Scheduling Algorithm Trade-offs

**First-Come-First-Serve (FCFS)**
- *Advantages*: Simple implementation, fair ordering, no starvation
- *Disadvantages*: Convoy effect, poor average waiting time for short jobs
- *Use Cases*: Batch processing systems, simple embedded systems

**Shortest Job First (SJF)**
- *Advantages*: Optimal average waiting time, good for batch systems
- *Disadvantages*: Requires burst time prediction, potential starvation
- *Use Cases*: Batch processing with known job characteristics

**Round Robin (RR)**
- *Advantages*: Good response time, fairness, no starvation
- *Disadvantages*: Context switching overhead, quantum selection challenges
- *Use Cases*: Interactive systems, time-sharing systems

**Priority Scheduling**
- *Advantages*: Supports process importance levels, flexible policies
- *Disadvantages*: Potential starvation, priority inversion issues
- *Use Cases*: Real-time systems, multi-user environments

### 2. Advanced Multilevel Scheduling

**Multilevel Queue Advantages:**
- Process classification efficiency
- Tailored algorithms per process type
- Simple implementation and analysis

**Multilevel Feedback Queue Advantages:**
- Adaptive behavior based on process characteristics
- Automatic classification and optimization
- Starvation prevention through aging

### 3. Real-time Scheduling Guarantees

**Rate Monotonic Scheduling:**
- Static priority assignment (period = priority)
- Predictable behavior and analysis
- Utilization bound: n(2^(1/n) - 1) ≈ 69.3% for large n

**Earliest Deadline First:**
- Dynamic priority assignment (deadline = priority)
- Optimal single-processor scheduling
- Utilization bound: 100%

## Performance Analysis Features

### Metrics Calculated
- **Average Waiting Time**: Time spent in ready queue
- **Average Turnaround Time**: Total time from arrival to completion
- **Average Response Time**: Time from arrival to first execution
- **CPU Utilization**: Percentage of CPU time doing useful work
- **Throughput**: Processes completed per time unit
- **Context Switches**: Number of process switches
- **Deadline Misses**: Real-time constraint violations (real-time algorithms)

### Comparison Framework
The implementations provide comprehensive comparison capabilities:
- Side-by-side algorithm performance
- Best algorithm identification per metric
- Workload sensitivity analysis
- Scalability assessment

## Educational Value and Usage

### For Students
1. **Understand Algorithm Mechanics**: See exactly how each algorithm makes scheduling decisions
2. **Observe Performance Trade-offs**: Compare algorithms with identical workloads
3. **Experiment with Parameters**: Modify time quantums, priorities, and arrival patterns
4. **Analyze Real-world Scenarios**: Study different workload types and their impacts

### For Instructors
1. **Live Demonstrations**: Interactive examples for classroom use
2. **Assignment Base**: Foundation for extended projects and modifications
3. **Performance Analysis**: Tools for discussing algorithm trade-offs
4. **Practical Implementation**: Bridge between theory and real systems

### For Researchers
1. **Algorithm Prototyping**: Framework for testing new scheduling approaches
2. **Performance Benchmarking**: Standardized comparison methodology
3. **Workload Analysis**: Tools for studying different application patterns

## Extending and Modifying the Code

### Adding New Algorithms
1. Implement algorithm logic following existing patterns
2. Add performance metric collection
3. Integrate with comparison framework
4. Update Makefile and documentation

### Customizing Workloads
1. Modify workload generation functions
2. Add new process types or characteristics
3. Implement domain-specific requirements
4. Test with real application traces

### Performance Enhancements
1. Add parallel processing support
2. Implement more sophisticated data structures
3. Optimize for specific workload patterns
4. Add hardware-specific optimizations

## Common Issues and Solutions

### Build Issues
**Problem**: Missing pthread support
**Solution**: Install development packages (`build-essential` on Ubuntu, Xcode tools on macOS)

**Problem**: Math library linking errors
**Solution**: Ensure `-lm` flag is included in linking (handled by Makefile)

### Runtime Issues
**Problem**: Excessive output during demonstrations
**Solution**: Use `make test-quick` or redirect output to files

**Problem**: Platform-specific behavior differences
**Solution**: Code includes platform detection and adaptation

### Performance Issues
**Problem**: Long execution times for large task sets
**Solution**: Adjust simulation parameters or use smaller workloads for testing

## Memory Management and Thread Safety

### Memory Management
- All dynamic memory allocation is properly managed
- No memory leaks in normal execution paths
- Valgrind-clean implementation

### Thread Safety
- Thread-safe output functions prevent garbled display
- Proper synchronization for shared data structures
- Safe for concurrent execution scenarios

## Testing and Validation

### Unit Testing
Each algorithm implementation includes:
- Correctness verification with known results
- Edge case handling (empty queues, single processes)
- Boundary condition testing

### Integration Testing
- Cross-algorithm comparison validation
- Performance metric accuracy verification
- Platform compatibility testing

### Stress Testing
- Large process sets (hundreds of processes)
- Extended simulation periods
- Memory usage under load

## Contributing and Further Development

### Code Style
- Follow existing naming conventions
- Use consistent indentation (4 spaces)
- Add comprehensive comments for complex algorithms
- Include function documentation headers

### Documentation
- Update README for new features
- Add inline code comments
- Provide usage examples
- Document performance characteristics

### Testing
- Add test cases for new algorithms
- Verify cross-platform compatibility
- Include performance regression tests
- Validate educational effectiveness

## Conclusion

These CPU scheduling implementations provide a comprehensive foundation for understanding, analyzing, and comparing different scheduling approaches. They bridge the gap between theoretical concepts and practical implementation, offering both educational value and research utility.

The code demonstrates that while basic scheduling algorithms each have their strengths and weaknesses, real-world systems often require sophisticated combinations of techniques to achieve optimal performance across diverse workloads and requirements.

For further learning, experiment with different parameter values, workload characteristics, and algorithm combinations to gain deeper insights into CPU scheduling design and implementation. 