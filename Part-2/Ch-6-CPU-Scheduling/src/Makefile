CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -O2 -D_GNU_SOURCE

# Platform-specific linking flags
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LDFLAGS = -lpthread -lrt -lm
else ifeq ($(UNAME_S),Darwin)
    LDFLAGS = -lpthread -lm
else
    LDFLAGS = -lpthread -lm
endif

TARGET_DIR = .
SRCDIR = .

SOURCES = scheduling_algorithms_demo.c multilevel_queue_demo.c realtime_scheduling_demo.c
OBJECTS = $(SOURCES:.c=.o)
TARGETS = scheduling_algorithms_demo multilevel_queue_demo realtime_scheduling_demo

all: $(TARGETS)
	@echo "All CPU scheduling demonstrations compiled successfully!"

scheduling_algorithms_demo: scheduling_algorithms_demo.o
	$(CC) -o $@ $< $(LDFLAGS)

multilevel_queue_demo: multilevel_queue_demo.o
	$(CC) -o $@ $< $(LDFLAGS)

realtime_scheduling_demo: realtime_scheduling_demo.o
	$(CC) -o $@ $< $(LDFLAGS)

# Generic object file rule
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Development and debugging targets
debug: CFLAGS += -DDEBUG -O0
debug: $(TARGETS)

release: CFLAGS += -DNDEBUG -O3
release: $(TARGETS)

# Testing targets
test: $(TARGETS)
	@echo "Running CPU Scheduling Algorithm Tests..."
	@echo "========================================"
	@echo "Testing Basic Scheduling Algorithms:"
	./scheduling_algorithms_demo
	@echo ""
	@echo "Testing Multilevel Queue Scheduling:"
	./multilevel_queue_demo
	@echo ""
	@echo "Testing Real-time Scheduling:"
	./realtime_scheduling_demo

# Quick test with limited output
test-quick: $(TARGETS)
	@echo "Quick CPU Scheduling Test:"
	@echo "=========================="
	@./scheduling_algorithms_demo | head -50
	@echo "... (output truncated) ..."

# Performance analysis
perf-test: $(TARGETS)
	@echo "Performance Analysis of Scheduling Algorithms:"
	@echo "=============================================="
	@if command -v time >/dev/null 2>&1; then \
		echo "Basic Algorithms Performance:"; \
		time -p ./scheduling_algorithms_demo > /dev/null; \
		echo ""; \
		echo "Multilevel Queue Performance:"; \
		time -p ./multilevel_queue_demo > /dev/null; \
		echo ""; \
		echo "Real-time Scheduling Performance:"; \
		time -p ./realtime_scheduling_demo > /dev/null; \
	else \
		echo "time command not available, running without timing"; \
		./scheduling_algorithms_demo > /dev/null; \
		./multilevel_queue_demo > /dev/null; \
		./realtime_scheduling_demo > /dev/null; \
	fi

# Memory analysis (if valgrind is available)
memcheck: $(TARGETS)
	@echo "Memory Analysis of CPU Scheduling Demonstrations:"
	@echo "================================================"
	@if command -v valgrind >/dev/null 2>&1; then \
		echo "Checking scheduling_algorithms_demo:"; \
		valgrind --leak-check=brief --error-exitcode=1 ./scheduling_algorithms_demo > /dev/null 2>&1 && echo "Memory: OK" || echo "Memory: Issues detected"; \
		echo "Checking multilevel_queue_demo:"; \
		valgrind --leak-check=brief --error-exitcode=1 ./multilevel_queue_demo > /dev/null 2>&1 && echo "Memory: OK" || echo "Memory: Issues detected"; \
		echo "Checking realtime_scheduling_demo:"; \
		valgrind --leak-check=brief --error-exitcode=1 ./realtime_scheduling_demo > /dev/null 2>&1 && echo "Memory: OK" || echo "Memory: Issues detected"; \
	else \
		echo "Valgrind not available, skipping memory analysis"; \
	fi

# Static analysis (if available)
analyze: $(SOURCES)
	@echo "Static Analysis of CPU Scheduling Code:"
	@echo "======================================"
	@if command -v cppcheck >/dev/null 2>&1; then \
		cppcheck --enable=warning,style,performance --std=c99 $(SOURCES); \
	else \
		echo "cppcheck not available, skipping static analysis"; \
	fi

# Code coverage (if gcov available)
coverage: CFLAGS += --coverage
coverage: LDFLAGS += --coverage
coverage: clean $(TARGETS)
	@echo "Generating Code Coverage Report:"
	@echo "==============================="
	@./scheduling_algorithms_demo > /dev/null
	@./multilevel_queue_demo > /dev/null
	@./realtime_scheduling_demo > /dev/null
	@if command -v gcov >/dev/null 2>&1; then \
		gcov $(SOURCES); \
		echo "Coverage files generated (.gcov)"; \
	else \
		echo "gcov not available, skipping coverage analysis"; \
	fi

# Educational targets
demo: $(TARGETS)
	@echo "CPU Scheduling Educational Demonstration"
	@echo "========================================"
	@echo ""
	@echo "This demonstration shows different CPU scheduling algorithms"
	@echo "and their performance characteristics."
	@echo ""
	@echo "1. Basic Scheduling Algorithms (FCFS, SJF, RR, Priority)"
	@echo "   Run: ./scheduling_algorithms_demo"
	@echo ""
	@echo "2. Multilevel Queue Scheduling"
	@echo "   Run: ./multilevel_queue_demo"
	@echo ""
	@echo "3. Real-time Scheduling (RMS, EDF)"
	@echo "   Run: ./realtime_scheduling_demo"
	@echo ""
	@echo "Choose which demonstration to run, or use 'make test' to run all."

# Interactive demo runner
interactive: $(TARGETS)
	@echo "Interactive CPU Scheduling Demonstration"
	@echo "======================================"
	@echo "Select a demonstration:"
	@echo "1) Basic Scheduling Algorithms"
	@echo "2) Multilevel Queue Scheduling"
	@echo "3) Real-time Scheduling"
	@echo "4) Run all demonstrations"
	@echo ""
	@read -p "Enter your choice (1-4): " choice; \
	case $$choice in \
		1) ./scheduling_algorithms_demo ;; \
		2) ./multilevel_queue_demo ;; \
		3) ./realtime_scheduling_demo ;; \
		4) make test ;; \
		*) echo "Invalid choice" ;; \
	esac

# Benchmarking target
benchmark: $(TARGETS)
	@echo "CPU Scheduling Algorithm Benchmarks:"
	@echo "===================================="
	@echo "Running performance comparisons..."
	@echo ""
	@echo "Basic Algorithm Comparison:"
	@./scheduling_algorithms_demo | grep -A 10 "Performance Comparison"
	@echo ""
	@echo "Multilevel Queue Analysis:"
	@./multilevel_queue_demo | grep -A 5 "Performance Metrics"
	@echo ""
	@echo "Real-time Schedulability:"
	@./realtime_scheduling_demo | grep -A 5 "Schedulability"

# Documentation generation
docs:
	@echo "CPU Scheduling Documentation:"
	@echo "============================"
	@echo ""
	@echo "Source files and their purposes:"
	@echo "- scheduling_algorithms_demo.c: Basic scheduling algorithms"
	@echo "- multilevel_queue_demo.c: Advanced multilevel scheduling"
	@echo "- realtime_scheduling_demo.c: Real-time scheduling algorithms"
	@echo ""
	@echo "Build targets:"
	@echo "- make all: Build all demonstrations"
	@echo "- make test: Run all demonstrations"
	@echo "- make debug: Build with debug symbols"
	@echo "- make perf-test: Performance analysis"
	@echo "- make memcheck: Memory leak analysis"
	@echo "- make clean: Remove build artifacts"
	@echo ""
	@echo "Educational targets:"
	@echo "- make demo: Show available demonstrations"
	@echo "- make interactive: Interactive demonstration runner"
	@echo "- make benchmark: Performance benchmarks"

# Formatting (if clang-format available)
format:
	@if command -v clang-format >/dev/null 2>&1; then \
		echo "Formatting source code..."; \
		clang-format -i $(SOURCES); \
		echo "Code formatting completed"; \
	else \
		echo "clang-format not available, skipping formatting"; \
	fi

# Dependency checking
check-deps:
	@echo "Checking build dependencies:"
	@echo "==========================="
	@echo -n "GCC: "; command -v gcc >/dev/null && echo "Available" || echo "Missing"
	@echo -n "Make: "; command -v make >/dev/null && echo "Available" || echo "Missing"
	@echo -n "Pthread support: "; echo "#include <pthread.h>" | gcc -E - >/dev/null 2>&1 && echo "Available" || echo "Missing"
	@echo -n "Math library: "; echo "#include <math.h>" | gcc -E - >/dev/null 2>&1 && echo "Available" || echo "Missing"
	@echo ""
	@echo "Optional analysis tools:"
	@echo -n "Valgrind: "; command -v valgrind >/dev/null && echo "Available" || echo "Not installed"
	@echo -n "Cppcheck: "; command -v cppcheck >/dev/null && echo "Available" || echo "Not installed"
	@echo -n "Gcov: "; command -v gcov >/dev/null && echo "Available" || echo "Not installed"

# Help target
help:
	@echo "CPU Scheduling Demonstration Makefile"
	@echo "====================================="
	@echo ""
	@echo "Main targets:"
	@echo "  all          - Build all demonstrations"
	@echo "  test         - Run all demonstrations"
	@echo "  clean        - Remove build artifacts"
	@echo ""
	@echo "Development targets:"
	@echo "  debug        - Build with debug symbols"
	@echo "  release      - Build optimized release version"
	@echo "  analyze      - Run static analysis"
	@echo "  memcheck     - Check for memory leaks"
	@echo "  coverage     - Generate code coverage report"
	@echo ""
	@echo "Educational targets:"
	@echo "  demo         - Show available demonstrations"
	@echo "  interactive  - Interactive demonstration runner"
	@echo "  benchmark    - Run performance benchmarks"
	@echo "  docs         - Show documentation"
	@echo ""
	@echo "Utility targets:"
	@echo "  format       - Format source code"
	@echo "  check-deps   - Check build dependencies"
	@echo "  help         - Show this help message"

# Clean target
clean:
	rm -f $(OBJECTS) $(TARGETS)
	rm -f *.gcda *.gcno *.gcov
	rm -f core core.*
	@echo "Clean completed"

# Deep clean (including backup files)
distclean: clean
	rm -f *~ *.bak *.tmp
	rm -f .*.swp .*.swo
	@echo "Deep clean completed"

# Install target (optional)
install: $(TARGETS)
	@echo "Installing CPU scheduling demonstrations..."
	@mkdir -p ~/bin
	@cp $(TARGETS) ~/bin/
	@echo "Demonstrations installed to ~/bin/"
	@echo "Add ~/bin to your PATH to run from anywhere"

# Phony targets
.PHONY: all clean distclean test test-quick perf-test memcheck analyze coverage demo interactive benchmark docs format check-deps help install debug release

# Default target
.DEFAULT_GOAL := all 