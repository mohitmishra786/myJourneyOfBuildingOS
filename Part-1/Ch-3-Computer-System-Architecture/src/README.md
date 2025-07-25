# Computer System Architecture - Source Code Examples

This directory contains practical C implementations that demonstrate the concepts covered in Chapter 3: Computer System Architecture.

## Files Overview

### `vonNeumannArch.c`
A complete Von Neumann architecture simulator that demonstrates:
- Stored program concept with unified memory
- Fetch-decode-execute cycle implementation
- Basic instruction set architecture
- Memory management and program execution
- Arithmetic and control flow operations

### `memoryHierarchy.c`
A comprehensive memory hierarchy simulation featuring:
- Multi-level cache systems (L1, L2, L3)
- Translation Lookaside Buffer (TLB) simulation
- Virtual memory management
- Cache replacement policies (LRU)
- Performance monitoring and analysis
- Different memory access patterns testing

## Building and Running

### Prerequisites
- GCC compiler (or any C99-compatible compiler)
- Make utility (optional, but recommended)
- Unix-like environment (Linux, macOS, or WSL on Windows)

### Quick Start

```bash
# Build all examples
make all

# Run all simulations
make run-all

# Build and run individual examples
make run-vonneumann
make run-memory

# Clean build artifacts
make clean

# Show help
make help
```

### Manual Compilation

If you prefer to compile manually without Make:

```bash
# Compile Von Neumann simulator
gcc -Wall -Wextra -std=c99 -O2 -g -o vonNeumannArch vonNeumannArch.c -lm

# Compile Memory Hierarchy simulator
gcc -Wall -Wextra -std=c99 -O2 -g -o memoryHierarchy memoryHierarchy.c -lm

# Run the programs
./vonNeumannArch
./memoryHierarchy
```

## Program Details

### Von Neumann Architecture Simulator

**What it demonstrates:**
- Basic CPU components (registers, ALU, control unit)
- Instruction encoding and decoding
- Memory operations (load/store)
- Arithmetic operations (add, subtract, multiply, divide)
- Control flow (jumps, conditional branches)
- Program execution with sample programs

**Sample Output:**
```
Von Neumann Architecture Simulator
===================================

=== Arithmetic Program Demo ===
Loaded program of 7 instructions

=== Starting CPU Execution ===
Cycle 0: PC=0x0000 LOAD R1, [R0 + 100] -> 0x00000005
Cycle 1: PC=0x0001 LOAD R2, [R0 + 101] -> 0x00000003
...
```

**Educational Value:**
- Understand how instructions are stored and executed
- See the fetch-decode-execute cycle in action
- Learn about instruction formats and encoding
- Observe program counter management and control flow

### Memory Hierarchy Simulator

**What it demonstrates:**
- Multi-level cache hierarchy (L1, L2, L3 caches)
- Cache hit/miss behavior and replacement policies
- Virtual memory and TLB operations
- Performance impact of different access patterns
- Memory hierarchy optimization techniques

**Sample Output:**
```
Memory Hierarchy Simulation
============================

=== Testing Sequential Access Pattern ===
Sequential access test completed

=== Testing Random Access Pattern ===
Random access test completed

=== Memory Hierarchy Performance ===
Overall Statistics:
  Total Memory Accesses: 2024
  Total Cycles: 15234
  Average Access Time: 7.52 cycles

L1 Cache Statistics:
  Total Accesses: 2024
  Hits: 1456 (71.94%)
  Misses: 568 (28.06%)
  ...
```

**Educational Value:**
- Understand cache behavior and locality of reference
- See the performance impact of cache misses
- Learn about virtual memory translation
- Observe how different access patterns affect performance

## Key Concepts Demonstrated

### 1. Von Neumann Architecture Principles
- **Stored Program Concept**: Instructions and data share the same memory space
- **Sequential Execution**: Instructions execute one after another unless control flow changes
- **Unified Memory Model**: Single memory space for both code and data

### 2. Memory Hierarchy Design
- **Locality of Reference**: Temporal and spatial locality exploitation
- **Cache Replacement**: LRU (Least Recently Used) policy implementation
- **Performance Trade-offs**: Speed vs. capacity vs. cost considerations

### 3. System Performance Analysis
- **Performance Metrics**: Hit rates, miss rates, average access times
- **Bottleneck Identification**: Understanding where performance is lost
- **Optimization Strategies**: How different techniques improve performance

## Extending the Examples

### Possible Enhancements for vonNeumannArch.c:
1. Add more instruction types (bit operations, floating-point)
2. Implement branch prediction mechanisms
3. Add instruction pipeline simulation
4. Include interrupt handling

### Possible Enhancements for memoryHierarchy.c:
1. Add write-through vs. write-back cache policies
2. Implement different replacement algorithms (FIFO, Random)
3. Add cache coherence protocols for multiprocessor simulation
4. Include prefetching mechanisms

## Learning Exercises

1. **Modify the Von Neumann simulator** to add new instructions or addressing modes
2. **Experiment with cache sizes** in the memory hierarchy simulator to see performance impacts
3. **Implement different replacement policies** and compare their effectiveness
4. **Add performance counters** to measure specific aspects of system behavior
5. **Create new test programs** to explore different computational patterns

## Troubleshooting

### Common Issues:

1. **Compilation Errors**: Ensure you have a C99-compatible compiler
2. **Math Library**: The `-lm` flag is required for mathematical functions
3. **Memory Access Violations**: All memory accesses are bounds-checked in the simulators
4. **Platform Differences**: Code is designed to be portable across Unix-like systems

### Debug Mode:
Both programs include extensive logging. To see more detailed output, you can modify the debug levels in the source code.

## Related Reading

These implementations complement the theoretical material in:
- Von Neumann Architecture chapter
- Memory Hierarchy chapter
- Instruction Execution Cycle chapter
- Overall Computer System Architecture concepts

The code serves as a practical foundation for understanding how modern computer systems implement these fundamental architectural principles. 