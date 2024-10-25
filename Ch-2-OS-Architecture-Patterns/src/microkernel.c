#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>

/* Constants */
#define MAX_QUEUE_SIZE 100
#define MAX_CHANNELS 10
#define MAX_PROCESSES 50

/* Process States */
#define PROCESS_READY 1
#define PROCESS_WAITING 2
#define PROCESS_RUNNING 3

/* System Call Numbers */
#define SYSCALL_SEND_MESSAGE 1
#define SYSCALL_RECEIVE_MESSAGE 2
#define SYSCALL_CREATE_PROCESS 3

/* Interrupt Numbers */
#define TIMER_INTERRUPT 1
#define IPC_INTERRUPT 2

/* Message Structure for IPC */
typedef struct {
    uint32_t sender_id;
    uint32_t message_type;
    uint32_t size;
    char data[256];  // Fixed size data buffer for simplicity
} message_t;

/* Process Control Block */
typedef struct process {
    uint32_t pid;
    uint32_t state;
    message_t message_queue[MAX_QUEUE_SIZE];
    uint32_t queue_size;
    struct process* next;
    char name[32];  // Added name for better debugging
} process_t;

/* IPC Channel */
typedef struct {
    message_t messages[MAX_QUEUE_SIZE];
    uint32_t capacity;
    uint32_t count;
} ipc_channel_t;

/* System Parameters */
typedef struct {
    char* process_name;
    uint32_t priority;
    void* entry_point;
} create_process_params;

typedef struct {
    process_t* receiver;
    message_t* message;
} send_message_params;

/* Global State */
process_t* process_list = NULL;
process_t* current_process = NULL;
ipc_channel_t* channels = NULL;
uint32_t next_pid = 1;

/* Function Declarations */
void schedule_next_process(void);
void handle_ipc_interrupt(void);
void setup_interrupt_handlers(void);
void enable_interrupts(void);
void print_process_status(void);

/* Initialize IPC Channel */
ipc_channel_t* create_ipc_channel(uint32_t capacity) {
    printf("Creating IPC channel with capacity: %u\n", capacity);
    ipc_channel_t* channel = malloc(sizeof(ipc_channel_t));
    if (!channel) {
        printf("Failed to allocate IPC channel\n");
        return NULL;
    }
    channel->capacity = capacity;
    channel->count = 0;
    return channel;
}

/* Send Message */
int send_message(process_t* sender, process_t* receiver, message_t* message) {
    if (!sender || !receiver || !message) {
        printf("Invalid parameters in send_message\n");
        return -1;
    }

    printf("Sending message from PID %u to PID %u\n", sender->pid, receiver->pid);

    if (receiver->queue_size >= MAX_QUEUE_SIZE) {
        printf("Receiver's queue is full\n");
        return -1;
    }

    memcpy(&receiver->message_queue[receiver->queue_size], message, sizeof(message_t));
    receiver->queue_size++;

    if (receiver->state == PROCESS_WAITING) {
        printf("Waking up receiver process\n");
        receiver->state = PROCESS_READY;
    }

    return 0;
}

/* Receive Message */
message_t* receive_message(process_t* receiver) {
    if (!receiver) {
        printf("Invalid receiver in receive_message\n");
        return NULL;
    }

    printf("Process %u attempting to receive message. Queue size: %u\n", 
           receiver->pid, receiver->queue_size);

    if (receiver->queue_size == 0) {
        printf("No messages available, process going to wait state\n");
        receiver->state = PROCESS_WAITING;
        schedule_next_process();
        return NULL;
    }

    static message_t msg;  // Static to avoid returning stack address
    memcpy(&msg, &receiver->message_queue[0], sizeof(message_t));

    // Shift queue
    for (uint32_t i = 0; i < receiver->queue_size - 1; i++) {
        receiver->message_queue[i] = receiver->message_queue[i + 1];
    }
    receiver->queue_size--;

    printf("Message received successfully\n");
    return &msg;
}

/* Process Creation */
process_t* create_user_process(create_process_params* params) {
    if (!params || !params->process_name) {
        printf("Invalid process creation parameters\n");
        return NULL;
    }

    process_t* process = malloc(sizeof(process_t));
    if (!process) {
        printf("Failed to allocate process structure\n");
        return NULL;
    }

    process->pid = next_pid++;
    process->state = PROCESS_READY;
    process->queue_size = 0;
    strncpy(process->name, params->process_name, sizeof(process->name) - 1);
    process->name[sizeof(process->name) - 1] = '\0';

    // Add to process list
    process->next = process_list;
    process_list = process;

    printf("Created process: %s (PID: %u)\n", process->name, process->pid);
    return process;
}

/* Simplified scheduler */
void schedule_next_process(void) {
    if (!process_list) {
        printf("No processes to schedule\n");
        return;
    }

    if (current_process) {
        current_process = current_process->next;
    }
    if (!current_process) {
        current_process = process_list;
    }

    printf("Scheduled process: %s (PID: %u)\n", 
           current_process->name, current_process->pid);
}

/* Print current system status */
void print_process_status(void) {
    printf("\n=== System Status ===\n");
    process_t* p = process_list;
    while (p) {
        printf("Process: %s (PID: %u)\n", p->name, p->pid);
        printf("  State: %u\n", p->state);
        printf("  Queue Size: %u\n", p->queue_size);
        p = p->next;
    }
    printf("==================\n\n");
}

/* Main Kernel Initialization */
void init_microkernel(void) {
    printf("Initializing microkernel...\n");

    // Initialize IPC channels
    channels = create_ipc_channel(MAX_CHANNELS);
    if (!channels) {
        printf("Failed to initialize IPC channels\n");
        return;
    }

    // Create initial system processes
    create_process_params params[] = {
        {"file_server", 1, NULL},
        {"device_driver", 1, NULL},
        {"memory_manager", 1, NULL}
    };

    for (int i = 0; i < 3; i++) {
        if (!create_user_process(&params[i])) {
            printf("Failed to create system process: %s\n", params[i].process_name);
            return;
        }
    }

    printf("Microkernel initialization complete\n");
    print_process_status();
}

/* Example test function */
void test_microkernel(void) {
    printf("\nRunning microkernel tests...\n");

    // Create a test process
    create_process_params test_params = {"test_process", 1, NULL};
    process_t* test_process = create_user_process(&test_params);

    if (!test_process) {
        printf("Test failed: Could not create test process\n");
        return;
    }

    // Create and send a test message
    message_t test_msg = {
        .sender_id = 1,
        .message_type = 1,
        .size = sizeof("test"),
    };
    strncpy(test_msg.data, "test", sizeof(test_msg.data));

    if (send_message(process_list, test_process, &test_msg) != 0) {
        printf("Test failed: Could not send message\n");
        return;
    }

    // Receive and verify message
    message_t* received_msg = receive_message(test_process);
    if (!received_msg) {
        printf("Test failed: Could not receive message\n");
        return;
    }

    printf("Test message received: %s\n", received_msg->data);
    printf("Tests completed successfully\n");
}

int main(void) {
    printf("Starting microkernel system...\n");
    
    init_microkernel();
    test_microkernel();
    
    return 0;
}
