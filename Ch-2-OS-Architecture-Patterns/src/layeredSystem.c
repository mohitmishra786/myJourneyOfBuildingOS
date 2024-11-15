#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Layer 6: User Interface Layer
typedef struct {
    char command[100];
    void* data;
    int status;
} UIRequest;

// Layer 5: Program Execution Layer
typedef struct {
    int process_id;
    char* program_name;
    int priority;
} ProcessInfo;

// Layer 4: I/O Management Layer
typedef struct {
    int device_id;
    char* buffer;
    size_t size;
} IORequest;

// Layer 3: Memory Management Layer
typedef struct {
    void* address;
    size_t size;
    int protection;
} MemoryBlock;

// Layer 2: Process Management Layer
typedef struct {
    int pid;
    int state;
    void* resources;
} ProcessControl;

// Layer 1: Hardware Interface Layer
typedef struct {
    int interrupt_number;
    void* parameters;
} HardwareRequest;

// Layer interface definitions
typedef struct {
    void (*init)(void);
    int (*process_request)(void* request);
    void (*cleanup)(void);
} LayerInterface;

// Layer 6: User Interface Layer Implementation
void ui_layer_init(void) {
    printf("Initializing User Interface Layer\n");
}

int ui_layer_process(void* request) {
    UIRequest* req = (UIRequest*)request;
    printf("Processing UI request: %s\n", req->command);
    return 0;
}

void ui_layer_cleanup(void) {
    printf("Cleaning up User Interface Layer\n");
}

// Layer 5: Program Execution Layer Implementation
void program_layer_init(void) {
    printf("Initializing Program Execution Layer\n");
}

int program_layer_process(void* request) {
    ProcessInfo* info = (ProcessInfo*)request;
    printf("Executing program: %s with PID: %d\n", 
           info->program_name, info->process_id);
    return 0;
}

void program_layer_cleanup(void) {
    printf("Cleaning up Program Execution Layer\n");
}

// Layer 4: I/O Management Layer Implementation
void io_layer_init(void) {
    printf("Initializing I/O Management Layer\n");
}

int io_layer_process(void* request) {
    IORequest* req = (IORequest*)request;
    printf("Processing I/O request for device %d\n", req->device_id);
    return 0;
}

void io_layer_cleanup(void) {
    printf("Cleaning up I/O Management Layer\n");
}

// Layer 3: Memory Management Layer Implementation
void memory_layer_init(void) {
    printf("Initializing Memory Management Layer\n");
}

int memory_layer_process(void* request) {
    MemoryBlock* block = (MemoryBlock*)request;
    printf("Processing memory request for size: %zu\n", block->size);
    return 0;
}

void memory_layer_cleanup(void) {
    printf("Cleaning up Memory Management Layer\n");
}

// Layer 2: Process Management Layer Implementation
void process_layer_init(void) {
    printf("Initializing Process Management Layer\n");
}

int process_layer_process(void* request) {
    ProcessControl* proc = (ProcessControl*)request;
    printf("Processing request for PID: %d\n", proc->pid);
    return 0;
}

void process_layer_cleanup(void) {
    printf("Cleaning up Process Management Layer\n");
}

// Layer 1: Hardware Interface Layer Implementation
void hardware_layer_init(void) {
    printf("Initializing Hardware Interface Layer\n");
}

int hardware_layer_process(void* request) {
    HardwareRequest* req = (HardwareRequest*)request;
    printf("Processing hardware interrupt: %d\n", req->interrupt_number);
    return 0;
}

void hardware_layer_cleanup(void) {
    printf("Cleaning up Hardware Interface Layer\n");
}

// Layer interface array
LayerInterface layers[] = {
    {hardware_layer_init, hardware_layer_process, hardware_layer_cleanup},
    {process_layer_init, process_layer_process, process_layer_cleanup},
    {memory_layer_init, memory_layer_process, memory_layer_cleanup},
    {io_layer_init, io_layer_process, io_layer_cleanup},
    {program_layer_init, program_layer_process, program_layer_cleanup},
    {ui_layer_init, ui_layer_process, ui_layer_cleanup}
};

// System initialization
void init_system(void) {
    for (int i = 0; i < sizeof(layers)/sizeof(LayerInterface); i++) {
        layers[i].init();
    }
}

// System cleanup
void cleanup_system(void) {
    for (int i = sizeof(layers)/sizeof(LayerInterface) - 1; i >= 0; i--) {
        layers[i].cleanup();
    }
}

// Example usage
int main() {
    // Initialize all layers
    init_system();
    
    // Example requests for each layer
    HardwareRequest hw_req = {1, NULL};
    ProcessControl proc_req = {1001, 1, NULL};
    MemoryBlock mem_req = {NULL, 1024, 0};
    IORequest io_req = {2, NULL, 512};
    ProcessInfo prog_req = {1001, "test_program", 1};
    UIRequest ui_req = {"EXECUTE", NULL, 0};
    
    // Process requests through layers
    layers[0].process_request(&hw_req);
    layers[1].process_request(&proc_req);
    layers[2].process_request(&mem_req);
    layers[3].process_request(&io_req);
    layers[4].process_request(&prog_req);
    layers[5].process_request(&ui_req);
    
    // Cleanup all layers
    cleanup_system();
    
    return 0;
}