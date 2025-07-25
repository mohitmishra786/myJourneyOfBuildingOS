# Makefile for Chapter 4 Process Management Examples
# Builds process management simulators and demonstrations

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g
LDFLAGS = 
TARGET_DIR = .
SOURCES = process_simulator.c context_switch_demo.c
TARGETS = process_simulator context_switch_demo

# Default target
all: $(TARGETS)

# Build process management simulator
process_simulator: process_simulator.c
	$(CC) $(CFLAGS) -o $(TARGET_DIR)/process_simulator process_simulator.c $(LDFLAGS)
	@echo "Built process management simulator"

# Build context switching demonstration
context_switch_demo: context_switch_demo.c
	$(CC) $(CFLAGS) -o $(TARGET_DIR)/context_switch_demo context_switch_demo.c $(LDFLAGS)
	@echo "Built context switching demonstration"

# Run process simulator
run-process: process_simulator
	@echo "Running Process Management Simulator..."
	@echo "======================================="
	./process_simulator

# Run context switch demo
run-context: context_switch_demo
	@echo "Running Context Switch Demonstration..."
	@echo "======================================"
	./context_switch_demo

# Run all demonstrations
run-all: run-process run-context

# Clean build artifacts
clean:
	rm -f $(TARGETS)
	@echo "Cleaned build artifacts"

# Create demonstration output
demo: all
	@echo "Chapter 4: Process Management Demonstrations"
	@echo "==========================================="
	@echo ""
	@echo "1. Process Management Simulator:"
	@echo "--------------------------------"
	@./process_simulator > process_simulation_output.txt 2>&1 || true
	@echo "Output saved to process_simulation_output.txt"
	@echo ""
	@echo "2. Context Switch Demonstration:"
	@echo "-------------------------------"
	@./context_switch_demo > context_switch_output.txt 2>&1 || true
	@echo "Output saved to context_switch_output.txt"
	@echo ""
	@echo "All demonstrations completed!"

# Help target
help:
	@echo "Available targets:"
	@echo "  all                - Build all programs"
	@echo "  process_simulator  - Build process management simulator"
	@echo "  context_switch_demo- Build context switching demonstration"
	@echo "  run-process        - Run process management simulator"
	@echo "  run-context        - Run context switching demonstration"
	@echo "  run-all            - Run all demonstrations"
	@echo "  demo               - Run all demos and save output"
	@echo "  clean              - Remove build artifacts"
	@echo "  help               - Show this help message"

.PHONY: all clean run-process run-context run-all demo help 