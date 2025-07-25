#include <stdint.h>     // For fixed-width integer types (uint32_t, etc.)
#include <stdbool.h>    // For boolean type
#include <stddef.h>     // For NULL definition
#include <stdio.h>      // For print functions
#include <stdlib.h>     // For malloc and free
#include <string.h>     // For memory operations

/* System call numbers - Used to identify different system services */
#define SYS_ALLOCATE_MEMORY 1    // Memory allocation request
#define SYS_CREATE_PROCESS  2    // Process creation request
#define SYS_READ_FILE      3     // File read request

/* Interrupt numbers - Hardware interrupt identifiers */
#define TIMER_INTERRUPT     32   // Regular timer tick
#define KEYBOARD_INTERRUPT  33   // Keyboard input received
#define PAGE_FAULT         14    // Memory access violation

/* Memory configuration */
#define PAGE_SIZE          4096  // Size of each memory page
#define TOTAL_MEMORY_PAGES 1024  // Total number of pages in system

/* Memory Management Structures */
typedef struct page {
    uint32_t flags;              // Page flags (free, used, etc.)
    uint32_t ref_count;          // Reference counter for shared pages
    struct page* next;           // Next page in free list
    uint8_t data[PAGE_SIZE];     // Actual page data
} page_t;

typedef struct {
    page_t* free_pages;          // List of free pages
    uint32_t total_pages;        // Total pages in system
    uint32_t used_pages;         // Currently used pages
    page_t* all_pages;           // Array of all pages
} memory_manager_t;

/* Process Management Structures */
typedef enum {
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_BLOCKED,
    PROCESS_TERMINATED
} process_state_t;

typedef struct process {
    uint32_t pid;                // Process ID
    process_state_t state;       // Current process state
    uint32_t* stack_pointer;     // Current stack pointer
    uint32_t* page_directory;    // Page directory for virtual memory
    struct process* next;        // Next process in list
} process_t;

typedef struct {
    process_t* current_process;   // Currently running process
    process_t* process_list;     // List of all processes
    uint32_t next_pid;           // Next available process ID
} process_manager_t;

/* File System Structures */
typedef struct {
    char name[256];              // File name
    uint32_t size;              // File size
    uint32_t permissions;       // Access permissions
    uint32_t inode;            // File system inode number
} file_t;

typedef struct {
    file_t* root_directory;     // Root directory entry
    uint32_t total_files;       // Total files in system
} filesystem_t;

/* Global Kernel State */
typedef struct {
    memory_manager_t* memory_manager;
    process_manager_t* process_manager;
    filesystem_t* filesystem;
    bool interrupts_enabled;
} kernel_t;

/* Global kernel instance */
kernel_t kernel;

/* Function declarations */
void switch_context(process_t* old, process_t* new);
void setup_interrupt_handlers(void);
void enable_interrupts(void);

/* Memory Management Implementation */
memory_manager_t* init_memory_manager(void) {
    memory_manager_t* mm = malloc(sizeof(memory_manager_t));
    if (!mm) {
        printf("Failed to allocate memory manager\n");
        return NULL;
    }

    // Allocate all pages
    mm->all_pages = malloc(sizeof(page_t) * TOTAL_MEMORY_PAGES);
    if (!mm->all_pages) {
        printf("Failed to allocate pages\n");
        free(mm);
        return NULL;
    }

    // Initialize free list
    mm->free_pages = &mm->all_pages[0];
    mm->total_pages = TOTAL_MEMORY_PAGES;
    mm->used_pages = 0;

    // Link all pages in free list
    for (uint32_t i = 0; i < TOTAL_MEMORY_PAGES - 1; i++) {
        mm->all_pages[i].next = &mm->all_pages[i + 1];
        mm->all_pages[i].ref_count = 0;
        mm->all_pages[i].flags = 0;
    }
    mm->all_pages[TOTAL_MEMORY_PAGES - 1].next = NULL;

    printf("Memory manager initialized with %u pages\n", TOTAL_MEMORY_PAGES);
    return mm;
}

void* allocate_page(memory_manager_t* mm) {
    if (!mm || !mm->free_pages) {
        printf("Memory allocation failed: No free pages\n");
        return NULL;
    }
    
    page_t* page = mm->free_pages;
    mm->free_pages = page->next;
    mm->used_pages++;
    
    page->ref_count = 1;
    page->flags = 0;
    page->next = NULL;
    
    printf("Allocated page at %p\n", (void*)page);
    return page;
}

void free_page(memory_manager_t* mm, void* page_addr) {
    if (!mm || !page_addr) return;

    page_t* page = (page_t*)page_addr;
    
    if (page->ref_count > 0) {
        page->ref_count--;
        if (page->ref_count == 0) {
            page->next = mm->free_pages;
            mm->free_pages = page;
            mm->used_pages--;
            printf("Freed page at %p\n", (void*)page);
        }
    }
}

/* Process Management Implementation */
process_manager_t* init_process_manager(void) {
    process_manager_t* pm = malloc(sizeof(process_manager_t));
    if (!pm) {
        printf("Failed to allocate process manager\n");
        return NULL;
    }

    pm->current_process = NULL;
    pm->process_list = NULL;
    pm->next_pid = 1;

    printf("Process manager initialized\n");
    return pm;
}

process_t* create_process(process_manager_t* pm) {
    if (!pm) return NULL;

    process_t* process = malloc(sizeof(process_t));
    if (!process) {
        printf("Process creation failed: Cannot allocate memory\n");
        return NULL;
    }
    
    process->pid = pm->next_pid++;
    process->state = PROCESS_READY;
    process->stack_pointer = NULL;
    process->page_directory = NULL;
    
    // Add to process list
    process->next = pm->process_list;
    pm->process_list = process;
    
    printf("Created process with PID %u\n", process->pid);
    return process;
}

void schedule_next_process(process_manager_t* pm) {
    if (!pm || !pm->current_process) {
        printf("No processes to schedule\n");
        return;
    }

    process_t* current = pm->current_process;
    
    // Simple round-robin scheduling
    if (current->next) {
        pm->current_process = current->next;
    } else {
        pm->current_process = pm->process_list;
    }
    
    printf("Switching from process %u to process %u\n", 
           current->pid, pm->current_process->pid);
    
    switch_context(current, pm->current_process);
}

/* Context Switching (Dummy implementation) */
void switch_context(process_t* old, process_t* new) {
    printf("Switching context from process %u to %u\n", old->pid, new->pid);
}

/* System Call Handler */
void handle_system_call(uint32_t syscall_number, void* params) {
    printf("Handling system call %u\n", syscall_number);
    
    switch (syscall_number) {
        case SYS_ALLOCATE_MEMORY:
            printf("Memory allocation system call\n");
            break;
        case SYS_CREATE_PROCESS:
            printf("Process creation system call\n");
            break;
        case SYS_READ_FILE:
            printf("File read system call\n");
            break;
        default:
            printf("Unknown system call %u\n", syscall_number);
            break;
    }
}

/* Interrupt Handler */
void interrupt_handler(uint32_t interrupt_number) {
    printf("Handling interrupt %u\n", interrupt_number);
    
    switch (interrupt_number) {
        case TIMER_INTERRUPT:
            printf("Timer interrupt\n");
            schedule_next_process(kernel.process_manager);
            break;
        case KEYBOARD_INTERRUPT:
            printf("Keyboard interrupt\n");
            break;
        case PAGE_FAULT:
            printf("Page fault\n");
            break;
        default:
            printf("Unknown interrupt %u\n", interrupt_number);
            break;
    }
}

/* Dummy implementations */
void setup_interrupt_handlers(void) {
    printf("Setting up interrupt handlers\n");
}

void enable_interrupts(void) {
    printf("Enabling interrupts\n");
}

/* Kernel cleanup */
void cleanup_kernel(void) {
    if (kernel.memory_manager) {
        if (kernel.memory_manager->all_pages) {
            free(kernel.memory_manager->all_pages);
        }
        free(kernel.memory_manager);
    }

    if (kernel.process_manager) {
        process_t* current = kernel.process_manager->process_list;
        while (current) {
            process_t* next = current->next;
            free(current);
            current = next;
        }
        free(kernel.process_manager);
    }

    if (kernel.filesystem) {
        free(kernel.filesystem);
    }
}

/* Kernel Initialization */
bool init_kernel(void) {
    printf("Initializing kernel...\n");
    
    // Initialize memory management
    kernel.memory_manager = init_memory_manager();
    if (!kernel.memory_manager) return false;
    
    // Initialize process management
    kernel.process_manager = init_process_manager();
    if (!kernel.process_manager) return false;
    
    // Initialize file system
    kernel.filesystem = malloc(sizeof(filesystem_t));
    if (!kernel.filesystem) return false;
    
    kernel.filesystem->root_directory = NULL;
    kernel.filesystem->total_files = 0;
    
    printf("File system initialized\n");
    
    // Set up interrupt handlers
    setup_interrupt_handlers();
    
    // Enable interrupts
    kernel.interrupts_enabled = true;
    enable_interrupts();
    
    printf("Kernel initialization complete\n");
    return true;
}

/* Main function for testing */
int main(void) {
    // Initialize the kernel
    if (!init_kernel()) {
        printf("Kernel initialization failed\n");
        cleanup_kernel();
        return 1;
    }
    
    // Create test processes
    printf("\nCreating test processes...\n");
    process_t* p1 = create_process(kernel.process_manager);
    if (!p1) {
        printf("Failed to create first process\n");
        cleanup_kernel();
        return 1;
    }

    process_t* p2 = create_process(kernel.process_manager);
    if (!p2) {
        printf("Failed to create second process\n");
        cleanup_kernel();
        return 1;
    }

    // Set current process
    kernel.process_manager->current_process = p1;
    
    // Test system calls
    printf("\nTesting system calls...\n");
    handle_system_call(SYS_ALLOCATE_MEMORY, NULL);
    handle_system_call(SYS_CREATE_PROCESS, NULL);
    handle_system_call(SYS_READ_FILE, NULL);
    
    // Test interrupts
    printf("\nTesting interrupts...\n");
    interrupt_handler(TIMER_INTERRUPT);
    interrupt_handler(KEYBOARD_INTERRUPT);
    interrupt_handler(PAGE_FAULT);
    
    // Cleanup
    cleanup_kernel();
    return 0;
}
