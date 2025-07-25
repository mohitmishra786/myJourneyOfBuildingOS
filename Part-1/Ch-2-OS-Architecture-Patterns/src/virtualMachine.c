#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>  // Added for uint64_t and uint8_t types

// Structure to represent a virtual CPU
typedef struct {
    uint64_t registers[16];  // General purpose registers
    uint64_t rip;           // Instruction pointer
    uint64_t rflags;        // CPU flags
    uint64_t cr3;           // Page table base register
} VirtualCPU;

// Structure to represent a virtual machine
typedef struct {
    VirtualCPU vcpu;
    void* memory;           // Guest physical memory
    size_t memory_size;
    bool running;
} VirtualMachine;

// Initialize a new virtual machine
VirtualMachine* vm_create(size_t memory_size) {
    VirtualMachine* vm = (VirtualMachine*)malloc(sizeof(VirtualMachine));
    if (!vm) return NULL;

    // Allocate guest physical memory
    vm->memory = mmap(NULL, memory_size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (vm->memory == MAP_FAILED) {
        free(vm);
        return NULL;
    }

    vm->memory_size = memory_size;
    vm->running = false;

    // Initialize virtual CPU
    memset(&vm->vcpu, 0, sizeof(VirtualCPU));
    
    return vm;
}

// Load binary into VM memory
bool vm_load_binary(VirtualMachine* vm, const char* binary, size_t size, uint64_t address) {
    if (address + size > vm->memory_size) return false;
    memcpy((uint8_t*)vm->memory + address, binary, size);
    return true;
}

// Simple instruction emulation
void vm_emulate_instruction(VirtualMachine* vm) {
    uint8_t* code = (uint8_t*)vm->memory + vm->vcpu.rip;
    
    // Very basic instruction decoder
    switch (*code) {
        case 0x90:  // NOP
            printf("Executing NOP instruction at RIP: 0x%lx\n", vm->vcpu.rip);
            vm->vcpu.rip++;
            break;
        case 0xC3:  // RET
            printf("Executing RET instruction at RIP: 0x%lx\n", vm->vcpu.rip);
            vm->running = false;
            break;
        default:
            printf("Unknown instruction: 0x%02X at RIP: 0x%lx\n", *code, vm->vcpu.rip);
            vm->running = false;
    }
}

// Main VM execution loop
void vm_run(VirtualMachine* vm) {
    vm->running = true;
    vm->vcpu.rip = 0;  // Start execution from address 0

    printf("Starting VM execution...\n");
    while (vm->running) {
        vm_emulate_instruction(vm);
    }
    printf("VM execution completed.\n");
}

// Clean up VM resources
void vm_destroy(VirtualMachine* vm) {
    if (vm) {
        if (vm->memory) munmap(vm->memory, vm->memory_size);
        free(vm);
    }
}

int main() {
    // Create a VM with 1MB of memory
    VirtualMachine* vm = vm_create(1024 * 1024);
    if (!vm) {
        printf("Failed to create VM\n");
        return 1;
    }

    // Example program: NOP, NOP, RET
    uint8_t program[] = {0x90, 0x90, 0xC3};
    if (!vm_load_binary(vm, program, sizeof(program), 0)) {
        printf("Failed to load program\n");
        vm_destroy(vm);
        return 1;
    }

    // Run the VM
    vm_run(vm);

    // Clean up
    vm_destroy(vm);
    return 0;
}