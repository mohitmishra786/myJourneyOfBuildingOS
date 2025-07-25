#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Constants
#define PAGE_SIZE 4096
#define MAX_MESSAGES 100
#define MAX_PROCESS_NAME 32

// Process Management Structures
typedef struct process {
    uint32_t pid;
    char name[MAX_PROCESS_NAME];
    struct process* next;
} process_t;

// Memory Management Structures
typedef struct page {
    uint8_t data[PAGE_SIZE];
    struct page* next;
} page_t;

// Inter-Process Communication Structures
typedef struct message {
    uint32_t sender_id;
    uint32_t receiver_id;
    uint8_t data[256];
} message_t;

// Global Variables
static process_t* process_list = NULL;
static uint32_t next_pid = 1;
static page_t* free_pages = NULL;
static message_t message_queue[MAX_MESSAGES];
static uint32_t queue_head = 0, queue_tail = 0;

// Process Management Functions
process_t* create_process(const char* name) {
    process_t* process = malloc(sizeof(process_t));
    if (!process) {
        printf("Failed to allocate process\n");
        return NULL;
    }
    
    process->pid = next_pid++;
    strncpy(process->name, name, MAX_PROCESS_NAME - 1);
    process->name[MAX_PROCESS_NAME - 1] = '\0';
    process->next = process_list;
    process_list = process;
    
    printf("Created process: %s (PID: %u)\n", process->name, process->pid);
    return process;
}

void schedule_process(process_t* process) {
    if (!process) {
        printf("Cannot schedule NULL process\n");
        return;
    }
    printf("Scheduling process: %s (PID: %u)\n", process->name, process->pid);
}

// Memory Management Functions
page_t* allocate_page(void) {
    if (free_pages) {
        page_t* page = free_pages;
        free_pages = free_pages->next;
        memset(page->data, 0, PAGE_SIZE);
        printf("Allocated page from free list\n");
        return page;
    }
    
    page_t* new_page = malloc(sizeof(page_t));
    if (!new_page) {
        printf("Failed to allocate new page\n");
        return NULL;
    }
    
    memset(new_page->data, 0, PAGE_SIZE);
    new_page->next = NULL;
    printf("Allocated new page\n");
    return new_page;
}

void free_page(page_t* page) {
    if (!page) {
        printf("Cannot free NULL page\n");
        return;
    }
    
    page->next = free_pages;
    free_pages = page;
    printf("Page freed and added to free list\n");
}

// Inter-Process Communication Functions
int send_message(message_t* msg) {
    if (!msg) {
        printf("Cannot send NULL message\n");
        return -1;
    }
    
    if ((queue_tail + 1) % MAX_MESSAGES == queue_head) {
        printf("Message queue is full\n");
        return -1;
    }
    
    message_queue[queue_tail] = *msg;
    queue_tail = (queue_tail + 1) % MAX_MESSAGES;
    printf("Message sent from PID %u to PID %u\n", msg->sender_id, msg->receiver_id);
    return 0;
}

message_t* receive_message(uint32_t receiver_id) {
    if (queue_head == queue_tail) {
        printf("Message queue is empty\n");
        return NULL;
    }
    
    message_t* msg = &message_queue[queue_head];
    if (msg->receiver_id != receiver_id) {
        printf("No messages for PID %u\n", receiver_id);
        return NULL;
    }
    
    queue_head = (queue_head + 1) % MAX_MESSAGES;
    printf("Message received by PID %u\n", receiver_id);
    return msg;
}

// Clean up function to free allocated memory
void cleanup(void) {
    // Free processes
    while (process_list) {
        process_t* temp = process_list;
        process_list = process_list->next;
        free(temp);
    }
    
    // Free pages
    while (free_pages) {
        page_t* temp = free_pages;
        free_pages = free_pages->next;
        free(temp);
    }
}

int main(void) {
    printf("Starting kernel simulation...\n\n");
    
    // Test process creation
    printf("=== Process Management Test ===\n");
    process_t* p1 = create_process("Process1");
    process_t* p2 = create_process("Process2");
    
    if (p1 && p2) {
        schedule_process(p1);
        schedule_process(p2);
    }
    
    printf("\n=== Memory Management Test ===\n");
    // Test memory management
    page_t* page1 = allocate_page();
    page_t* page2 = allocate_page();
    
    if (page1 && page2) {
        // Write some test patterns to pages
        memset(page1->data, 0xAA, 64);
        memset(page2->data, 0xBB, 64);
        printf("Written test patterns to pages\n");
        
        // Free pages
        free_page(page1);
        free_page(page2);
    }
    
    printf("\n=== IPC Test ===\n");
    // Test IPC
    message_t msg = {
        .sender_id = 1,
        .receiver_id = 2
    };
    strncpy((char*)msg.data, "Hello, Process 2!", 256);
    
    if (send_message(&msg) == 0) {
        message_t* received = receive_message(2);
        if (received) {
            printf("Message content: %s\n", received->data);
        }
    }
    
    // Clean up
    printf("\n=== Cleanup ===\n");
    cleanup();
    printf("Cleanup completed\n");
    
    return 0;
}
