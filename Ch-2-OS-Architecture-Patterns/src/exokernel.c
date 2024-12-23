/* Core Exokernel Implementation */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

/* System Call Definitions */
#define SYSCALL_BIND_MEMORY 1
#define SYSCALL_BIND_DISK   2
#define SYSCALL_REVOKE     3

/* Global Variables */
#define MAX_PAGES 1024
#define MAX_BLOCKS 2048
static uint32_t current_process_id = 1;  // Changed from macro to variable

/* Resource Tracking Structures */
typedef struct {
    uint32_t resource_id;
    uint32_t owner_id;
    uint32_t permissions;
    void* physical_address;
} resource_binding_t;

typedef struct {
    resource_binding_t* bindings;
    uint32_t count;
    uint32_t capacity;
} resource_table_t;

/* Secure Binding Management */
typedef struct {
    uint32_t start_page;
    uint32_t page_count;
    uint32_t permissions;
} memory_binding_t;

typedef struct {
    uint32_t start_block;
    uint32_t block_count;
    uint32_t access_mask;
} disk_binding_t;

/* Global Resource Table */
resource_table_t* resource_table;

/* Utility Functions */
void* get_physical_address(uint32_t resource_id) {
    // Simulate physical address calculation
    return (void*)(uintptr_t)(0x1000 * resource_id);
}

bool are_pages_available(uint32_t start_page, uint32_t count) {
    // Simple simulation - check if pages are within bounds
    return (start_page + count <= MAX_PAGES);
}

bool are_blocks_available(uint32_t start_block, uint32_t count) {
    // Simple simulation - check if blocks are within bounds
    return (start_block + count <= MAX_BLOCKS);
}

void map_page_direct(uint32_t owner_id, uint32_t page) {
    printf("Mapping page %u for process %u\n", page, owner_id);
}

void send_revocation_notification(uint32_t owner_id, uint32_t resource_id) {
    printf("Notifying process %u about revocation of resource %u\n", 
           owner_id, resource_id);
}

/* Resource Management Implementation */
resource_table_t* create_resource_table(uint32_t capacity) {
    resource_table_t* table = malloc(sizeof(resource_table_t));
    if (!table) return NULL;
    
    table->bindings = malloc(sizeof(resource_binding_t) * capacity);
    if (!table->bindings) {
        free(table);
        return NULL;
    }
    
    table->capacity = capacity;
    table->count = 0;
    return table;
}

int bind_resource(resource_table_t* table, uint32_t resource_id, 
                 uint32_t owner_id, uint32_t permissions) {
    if (!table || table->count >= table->capacity) {
        return -1;  // Table full or invalid
    }
    
    // Check for existing bindings
    for (int i = 0; i < table->count; i++) {
        if (table->bindings[i].resource_id == resource_id) {
            return -2;  // Resource already bound
        }
    }
    
    // Create new binding
    resource_binding_t binding = {
        .resource_id = resource_id,
        .owner_id = owner_id,
        .permissions = permissions,
        .physical_address = get_physical_address(resource_id)
    };
    
    table->bindings[table->count++] = binding;
    return 0;
}

/* Memory Management */
int bind_memory_pages(uint32_t owner_id, memory_binding_t* binding) {
    if (!binding) return -1;
    
    // Verify pages are available
    if (!are_pages_available(binding->start_page, binding->page_count)) {
        printf("Pages %u to %u not available\n", 
               binding->start_page, 
               binding->start_page + binding->page_count - 1);
        return -1;
    }
    
    // Create secure binding
    for (uint32_t i = 0; i < binding->page_count; i++) {
        uint32_t page = binding->start_page + i;
        int result = bind_resource(resource_table, page, owner_id, 
                                 binding->permissions);
        if (result < 0) {
            printf("Failed to bind page %u\n", page);
            return result;
        }
        
        // Set up page table entries for direct access
        map_page_direct(owner_id, page);
    }
    
    printf("Successfully bound %u pages starting at page %u for process %u\n",
           binding->page_count, binding->start_page, owner_id);
    return 0;
}

/* Disk Access */
int bind_disk_blocks(uint32_t owner_id, disk_binding_t* binding) {
    if (!binding) return -1;
    
    // Verify blocks are available
    if (!are_blocks_available(binding->start_block, binding->block_count)) {
        printf("Blocks %u to %u not available\n", 
               binding->start_block, 
               binding->start_block + binding->block_count - 1);
        return -1;
    }
    
    // Create secure binding
    for (uint32_t i = 0; i < binding->block_count; i++) {
        uint32_t block = binding->start_block + i;
        int result = bind_resource(resource_table, block, owner_id, 
                                 binding->access_mask);
        if (result < 0) {
            printf("Failed to bind block %u\n", block);
            return result;
        }
    }
    
    printf("Successfully bound %u blocks starting at block %u for process %u\n",
           binding->block_count, binding->start_block, owner_id);
    return 0;
}

/* Protection Checks */
bool verify_access(uint32_t owner_id, uint32_t resource_id, 
                  uint32_t requested_permission) {
    if (!resource_table) return false;
    
    for (int i = 0; i < resource_table->count; i++) {
        if (resource_table->bindings[i].resource_id == resource_id) {
            if (resource_table->bindings[i].owner_id != owner_id) {
                printf("Access denied: wrong owner for resource %u\n", 
                       resource_id);
                return false;
            }
            bool has_permission = (resource_table->bindings[i].permissions & 
                                 requested_permission) != 0;
            printf("Access %s for resource %u (requested permission: %u)\n",
                   has_permission ? "granted" : "denied", 
                   resource_id, requested_permission);
            return has_permission;
        }
    }
    printf("Access denied: resource %u not found\n", resource_id);
    return false;
}

/* Resource Revocation */
void revoke_resources(uint32_t owner_id) {
    if (!resource_table) return;
    
    printf("Revoking all resources for process %u\n", owner_id);
    
    for (int i = 0; i < resource_table->count; i++) {
        if (resource_table->bindings[i].owner_id == owner_id) {
            // Notify owner of revocation
            send_revocation_notification(owner_id, 
                                      resource_table->bindings[i].resource_id);
            
            // Remove binding
            for (int j = i; j < resource_table->count - 1; j++) {
                resource_table->bindings[j] = resource_table->bindings[j + 1];
            }
            resource_table->count--;
            i--;  // Adjust index after removal
        }
    }
}

/* System Call Interface */
void handle_syscall(uint32_t syscall_number, void* params) {
    switch (syscall_number) {
        case SYSCALL_BIND_MEMORY:
            memory_binding_t* mem_binding = (memory_binding_t*)params;
            bind_memory_pages(current_process_id, mem_binding);
            break;
            
        case SYSCALL_BIND_DISK:
            disk_binding_t* disk_binding = (disk_binding_t*)params;
            bind_disk_blocks(current_process_id, disk_binding);
            break;
            
        case SYSCALL_REVOKE:
            revoke_resources(*(uint32_t*)params);
            break;
            
        default:
            printf("Unknown syscall number: %u\n", syscall_number);
            break;
    }
}

/* Main function to demonstrate the exokernel simulation */
int main() {
    // Initialize resource table
    resource_table = create_resource_table(1000);
    if (!resource_table) {
        printf("Failed to create resource table\n");
        return 1;
    }
    
    // Simulate memory binding
    memory_binding_t mem_binding = {
        .start_page = 100,
        .page_count = 5,
        .permissions = 0x7  // Read/Write/Execute
    };
    handle_syscall(SYSCALL_BIND_MEMORY, &mem_binding);
    
    // Simulate disk binding
    disk_binding_t disk_binding = {
        .start_block = 500,
        .block_count = 10,
        .access_mask = 0x3  // Read/Write
    };
    handle_syscall(SYSCALL_BIND_DISK, &disk_binding);
    
    // Test access verification
    verify_access(current_process_id, 100, 0x1);  // Check read access to first page
    
    // Simulate resource revocation
    uint32_t process_to_revoke = current_process_id;  // Create a variable to hold the process ID
    handle_syscall(SYSCALL_REVOKE, &process_to_revoke);
    
    // Cleanup
    free(resource_table->bindings);
    free(resource_table);
    
    return 0;
}
