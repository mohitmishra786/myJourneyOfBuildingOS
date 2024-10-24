# Virtual Machines as an OS Pattern

Virtual Machine (VM) architecture represents a fundamental shift in how we think about operating system resources and isolation. By providing complete hardware virtualization, VMs enable multiple operating systems to coexist on the same physical hardware while maintaining strong isolation guarantees.

## Hypervisor Architecture

Hypervisors, also known as Virtual Machine Monitors (VMMs), come in two primary types:

Type 1 (Bare-metal):
• Runs directly on hardware
• Provides better performance
• Commonly used in server environments
• Examples: Xen, VMware ESXi

Type 2 (Hosted):
• Runs on top of a host operating system
• Easier to install and manage
• Better for desktop virtualization
• Examples: VirtualBox, VMware Workstation

## Resource Virtualization

The core challenge in VM implementation lies in efficiently virtualizing hardware resources while maintaining isolation. Modern hypervisors employ several key techniques:

Hardware-assisted Virtualization:
Modern processors provide specialized instructions (Intel VT-x, AMD-V) that significantly improve virtualization performance. These extensions handle sensitive instructions and provide hardware-level isolation between VMs.

Memory Virtualization:
Nested page tables (NPT) or Extended Page Tables (EPT) provide efficient memory virtualization by adding an additional layer of address translation between guest physical addresses and host physical addresses.

I/O Virtualization:
Modern hypervisors use a combination of techniques for I/O handling:
• Direct Device Assignment (PCIe passthrough)
• Para-virtualized drivers
• Device emulation

The choice of I/O virtualization strategy depends on performance requirements and hardware support.

```c
/* Basic Type-1 Hypervisor Implementation */

#include <stdint.h>
#include <stdbool.h>

/* VM Control Structure */
typedef struct {
    uint64_t vmcs_revision_id;
    uint32_t abort_indicator;
    uint8_t data[4088];  // VMX-specific control fields
} vmcs_t;

/* Virtual Machine State */
typedef struct {
    uint32_t vm_id;
    vmcs_t* vmcs;
    uint64_t guest_cr3;  // Guest page table base
    uint64_t ept_pointer;  // Extended Page Table pointer
    void* io_bitmap;
    bool running;
} vm_state_t;

/* VM Management Functions */
vm_state_t* create_vm(uint32_t memory_size, uint32_t vcpu_count) {
    vm_state_t* vm = malloc(sizeof(vm_state_t));
    
    // Allocate VMCS
    vm->vmcs = aligned_alloc(4096, sizeof(vmcs_t));
    
    // Set up Extended Page Tables
    vm->ept_pointer = setup_ept(memory_size);
    
    // Initialize I/O bitmap
    vm->io_bitmap = aligned_alloc(4096, IO_BITMAP_SIZE);
    memset(vm->io_bitmap, 0xFF, IO_BITMAP_SIZE);  // All I/O trapped
    
    return vm;
}

/* VM Entry/Exit Handlers */
void vm_entry_handler(vm_state_t* vm) {
    // Save host state
    save_host_state();
    
    // Load guest state from VMCS
    load_guest_state(vm->vmcs);
    
    // Enter VM mode
    __asm__ volatile("vmlaunch");
}

void vm_exit_handler(vm_state_t* vm) {
    uint32_t exit_reason = read_vmcs(VM_EXIT_REASON);
    
    switch (exit_reason) {
        case EXIT_REASON_IO_INSTRUCTION:
            handle_io_exit(vm);
            break;
            
        case EXIT_REASON_EPT_VIOLATION:
            handle_ept_violation(vm);
            break;
            
        case EXIT_REASON_EXTERNAL_INTERRUPT:
            handle_interrupt(vm);
            break;
    }
}

/* Memory Management */
uint64_t setup_ept(uint32_t memory_size) {
    // Allocate EPT page tables
    ept_pml4_t* pml4 = aligned_alloc(4096, sizeof(ept_pml4_t));
    
    // Set up identity mapping for physical memory
    for (uint64_t addr = 0; addr < memory_size; addr += PAGE_SIZE) {
        map_ept_page(pml4, addr, addr, EPT_READ | EPT_WRITE | EPT_EXECUTE);
    }
    
    return virt_to_phys(pml4);
}

/* I/O Handling */
void handle_io_exit(vm_state_t* vm) {
    io_exit_info_t exit_info;
    read_vmcs(VM_EXIT_QUALIFICATION, &exit_info);
    
    if (is_mmio_address(exit_info.port)) {
        handle_mmio(vm, exit_info);
    } else {
        emulate_io_instruction(vm, exit_info);
    }
}

/* Interrupt Management */
void handle_interrupt(vm_state_t* vm) {
    uint32_t interrupt_info;
    read_vmcs(VM_EXIT_INTERRUPT_INFO, &interrupt_info);
    
    if (is_external_interrupt(interrupt_info)) {
        // Handle external interrupt
        handle_external_interrupt(vm, interrupt_info);
    } else {
        // Inject interrupt into guest
        inject_interrupt(vm, interrupt_info);
    }
}

/* Device Emulation */
typedef struct {
    uint32_t device_id;
    void (*read_handler)(vm_state_t*, uint64_t, void*, size_t);
    void (*write_handler)(vm_state_t*, uint64_t, void*, size_t);
} virtual_device_t;

void emulate_device_access(vm_state_t* vm, uint64_t address, 
                          void* data, size_t size, bool is_write) {
    virtual_device_t* device = find_device(address);
    if (device) {
        if (is_write) {
            device->write_handler(vm, address, data, size);
        } else {
            device->read_handler(vm, address, data, size);
        }
    }
}

/* Main Hypervisor Loop */
void hypervisor_main() {
    // Initialize VMX operation
    enable_vmx();
    
    // Create initial VM
    vm_state_t* vm = create_vm(DEFAULT_VM_MEMORY, DEFAULT_VCPU_COUNT);
    
    // Main loop
    while (true) {
        if (vm->running) {
            vm_entry_handler(vm);
            // VM Exit occurs
            vm_exit_handler(vm);
        }
        
        // Check for new VM creation requests
        check_vm_requests();
        
        // Handle scheduling
        schedule_next_vm();
    }
}
```
