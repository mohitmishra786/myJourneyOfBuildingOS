# Makefile for Computer System Architecture Examples
# Chapter 3 - OS Learning Journey

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g
LDFLAGS = -lm

# Source files
SOURCES = vonNeumannArch.c memoryHierarchy.c
EXECUTABLES = $(SOURCES:.c=)

# Default target
all: $(EXECUTABLES)

# Individual targets
vonNeumannArch: vonNeumannArch.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

memoryHierarchy: memoryHierarchy.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Clean target
clean:
	rm -f $(EXECUTABLES) *.o

# Run targets
run-vonneumann: vonNeumannArch
	@echo "Running Von Neumann Architecture Simulator..."
	@echo "=================================="
	./vonNeumannArch

run-memory: memoryHierarchy
	@echo "Running Memory Hierarchy Simulation..."
	@echo "=================================="
	./memoryHierarchy

run-all: all
	@echo "Running all simulations..."
	@echo ""
	$(MAKE) run-vonneumann
	@echo ""
	@echo ""
	$(MAKE) run-memory

# Help target
help:
	@echo "Available targets:"
	@echo "  all              - Build all executables"
	@echo "  vonNeumannArch   - Build Von Neumann architecture simulator"
	@echo "  memoryHierarchy  - Build memory hierarchy simulator"
	@echo "  run-vonneumann   - Run Von Neumann simulator"
	@echo "  run-memory       - Run memory hierarchy simulator"
	@echo "  run-all          - Run all simulators"
	@echo "  clean            - Remove all built files"
	@echo "  help             - Show this help message"

# Phony targets
.PHONY: all clean run-vonneumann run-memory run-all help 