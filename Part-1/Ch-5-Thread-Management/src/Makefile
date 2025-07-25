CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -O2

# Platform-specific linking flags
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LDFLAGS = -lpthread -lrt
else ifeq ($(UNAME_S),Darwin)
    LDFLAGS = -lpthread
else
    LDFLAGS = -lpthread
endif
TARGET_DIR = .
SRCDIR = .

# Source files
SOURCES = thread_models_demo.c thread_synchronization_demo.c pthread_programming_demo.c thread_pool_demo.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Executable targets
TARGETS = thread_models_demo thread_synchronization_demo pthread_programming_demo thread_pool_demo

# Default target
all: $(TARGETS)
	@echo "All thread management demos compiled successfully!"
	@echo "Available executables:"
	@echo "  - thread_models_demo: Demonstrates different thread models"
	@echo "  - thread_synchronization_demo: Shows synchronization mechanisms"
	@echo "  - pthread_programming_demo: POSIX threads programming examples"
	@echo "  - thread_pool_demo: Thread pool implementation and usage"

# Individual targets
thread_models_demo: thread_models_demo.o
	$(CC) -o $@ $< $(LDFLAGS)

thread_synchronization_demo: thread_synchronization_demo.o
	$(CC) -o $@ $< $(LDFLAGS)

pthread_programming_demo: pthread_programming_demo.o
	$(CC) -o $@ $< $(LDFLAGS)

thread_pool_demo: thread_pool_demo.o
	$(CC) -o $@ $< $(LDFLAGS)

# Object file compilation
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(OBJECTS) $(TARGETS)
	rm -rf *.dSYM
	@echo "Cleaned all build artifacts"

# Install target (copies executables to system location)
install: $(TARGETS)
	@echo "Installing thread management demos..."
	@mkdir -p /usr/local/bin/thread-demos
	@cp $(TARGETS) /usr/local/bin/thread-demos/
	@echo "Demos installed to /usr/local/bin/thread-demos/"

# Uninstall target
uninstall:
	@echo "Removing thread management demos..."
	@rm -rf /usr/local/bin/thread-demos
	@echo "Demos removed"

# Test target - runs all demos
test: $(TARGETS)
	@echo "Running thread management demonstrations..."
	@echo "=================================="
	@echo "Running Thread Models Demo:"
	@./thread_models_demo
	@echo "=================================="
	@echo "Running Thread Synchronization Demo:"
	@./thread_synchronization_demo
	@echo "=================================="
	@echo "Running Pthread Programming Demo:"
	@./pthread_programming_demo
	@echo "=================================="
	@echo "Running Thread Pool Demo:"
	@./thread_pool_demo
	@echo "All demos completed successfully!"

# Debug target - compiles with debug flags
debug: CFLAGS += -DDEBUG -g3 -O0
debug: $(TARGETS)
	@echo "Debug versions compiled successfully!"

# Performance target - compiles with optimization
performance: CFLAGS += -O3 -DNDEBUG -march=native
performance: $(TARGETS)
	@echo "Performance optimized versions compiled successfully!"

# Static analysis target
analyze:
	@echo "Running static analysis..."
	@which cppcheck > /dev/null 2>&1 && cppcheck --enable=all --std=c99 $(SOURCES) || echo "cppcheck not found, skipping static analysis"
	@which clang-tidy > /dev/null 2>&1 && clang-tidy $(SOURCES) -- $(CFLAGS) || echo "clang-tidy not found, skipping tidy analysis"

# Memory check target
memcheck: $(TARGETS)
	@echo "Running memory checks..."
	@which valgrind > /dev/null 2>&1 && echo "Running valgrind on thread_models_demo..." && valgrind --tool=memcheck --leak-check=full ./thread_models_demo || echo "valgrind not found, skipping memory check"

# Thread sanitizer target
tsan: CFLAGS += -fsanitize=thread -g
tsan: LDFLAGS += -fsanitize=thread
tsan: $(TARGETS)
	@echo "Thread sanitizer versions compiled successfully!"
	@echo "Run with: ./thread_models_demo (etc.) to detect threading issues"

# Address sanitizer target
asan: CFLAGS += -fsanitize=address -g
asan: LDFLAGS += -fsanitize=address
asan: $(TARGETS)
	@echo "Address sanitizer versions compiled successfully!"

# Help target
help:
	@echo "Available targets:"
	@echo "  all          - Build all demos (default)"
	@echo "  clean        - Remove all build artifacts"
	@echo "  test         - Build and run all demos"
	@echo "  debug        - Build debug versions"
	@echo "  performance  - Build optimized versions"
	@echo "  analyze      - Run static analysis tools"
	@echo "  memcheck     - Run memory analysis with valgrind"
	@echo "  tsan         - Build with thread sanitizer"
	@echo "  asan         - Build with address sanitizer"
	@echo "  install      - Install demos to system location"
	@echo "  uninstall    - Remove installed demos"
	@echo "  help         - Show this help message"

# Dependency tracking
-include $(SOURCES:.c=.d)

%.d: %.c
	@$(CC) $(CFLAGS) -MM $< > $@

.PHONY: all clean install uninstall test debug performance analyze memcheck tsan asan help 