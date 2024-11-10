#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

// Memory management structures
#define PAGE_SIZE 4096
#define HEAP_SIZE (1024 * 1024) // 1MB heap

typedef struct {
    void* heap_start;
    void* heap_current;
    void* heap_end;
    size_t total_allocated;
} MemoryManager;

// Network stack structures
#define MAX_PACKETS 256
#define PACKET_SIZE 1514

typedef struct {
    uint8_t data[PACKET_SIZE];
    size_t length;
} Packet;

typedef struct {
    Packet rx_ring[MAX_PACKETS];
    Packet tx_ring[MAX_PACKETS];
    uint32_t rx_head;
    uint32_t rx_tail;
    uint32_t tx_head;
    uint32_t tx_tail;
} NetworkQueue;

// Event system
#define MAX_EVENTS 32

typedef void (*EventHandler)(void* data);

typedef struct {
    EventHandler handler;
    void* data;
    uint64_t trigger_time;
    bool active;
} Event;

typedef struct {
    Event events[MAX_EVENTS];
    uint32_t event_count;
    uint64_t current_time;
} EventSystem;

// Main unikernel structure
typedef struct {
    MemoryManager mm;
    NetworkQueue net_queue;
    EventSystem events;
    bool running;
} Unikernel;

// Memory management functions
void init_memory_manager(MemoryManager* mm) {
    static uint8_t heap[HEAP_SIZE] __attribute__((aligned(PAGE_SIZE)));
    mm->heap_start = heap;
    mm->heap_current = heap;
    mm->heap_end = heap + HEAP_SIZE;
    mm->total_allocated = 0;
}

void* allocate(MemoryManager* mm, size_t size) {
    size = (size + 7) & ~7; // 8-byte alignment
    
    if (mm->heap_current + size > mm->heap_end) {
        return NULL; // Out of memory
    }
    
    void* ptr = mm->heap_current;
    mm->heap_current += size;
    mm->total_allocated += size;
    return ptr;
}

// Network functions
void init_network_queue(NetworkQueue* nq) {
    nq->rx_head = 0;
    nq->rx_tail = 0;
    nq->tx_head = 0;
    nq->tx_tail = 0;
}

bool network_send_packet(NetworkQueue* nq, const void* data, size_t length) {
    if (length > PACKET_SIZE) {
        return false;
    }
    
    uint32_t next_head = (nq->tx_head + 1) % MAX_PACKETS;
    if (next_head == nq->tx_tail) {
        return false; // Queue full
    }
    
    memcpy(nq->tx_ring[nq->tx_head].data, data, length);
    nq->tx_ring[nq->tx_head].length = length;
    nq->tx_head = next_head;
    
    printf("Network Packet Sent (%zu bytes): %.*s\n", 
           length, (int)length, (const char*)data);
    
    return true;
}

void simulate_receive_packet(NetworkQueue* nq, const char* data) {
    size_t length = strlen(data);
    uint32_t next_head = (nq->rx_head + 1) % MAX_PACKETS;
    
    if (next_head != nq->rx_tail && length < PACKET_SIZE) {
        memcpy(nq->rx_ring[nq->rx_head].data, data, length);
        nq->rx_ring[nq->rx_head].length = length;
        nq->rx_head = next_head;
    }
}

bool network_receive_packet(NetworkQueue* nq, void* data, size_t* length) {
    if (nq->rx_head == nq->rx_tail) {
        return false; // No packets
    }
    
    memcpy(data, nq->rx_ring[nq->rx_tail].data, nq->rx_ring[nq->rx_tail].length);
    *length = nq->rx_ring[nq->rx_tail].length;
    nq->rx_tail = (nq->rx_tail + 1) % MAX_PACKETS;
    
    return true;
}

// Event system functions
void init_event_system(EventSystem* es) {
    es->event_count = 0;
    es->current_time = 0;
    memset(es->events, 0, sizeof(es->events));
}

bool add_event(EventSystem* es, EventHandler handler, void* data, uint64_t delay) {
    if (es->event_count >= MAX_EVENTS) {
        return false;
    }
    
    Event* event = &es->events[es->event_count++];
    event->handler = handler;
    event->data = data;
    event->trigger_time = es->current_time + delay;
    event->active = true;
    
    return true;
}

void process_events(EventSystem* es) {
    es->current_time++;
    
    for (uint32_t i = 0; i < es->event_count; i++) {
        Event* event = &es->events[i];
        if (event->active && event->trigger_time <= es->current_time) {
            event->handler(event->data);
            event->active = false;
        }
    }
}

// Timer handler
void timer_handler(void* data) {
    Unikernel* uk = (Unikernel*)data;
    const char* heartbeat = "heartbeat";
    network_send_packet(&uk->net_queue, heartbeat, strlen(heartbeat));
    add_event(&uk->events, timer_handler, uk, 10);
}

// HTTP handling
#define MAX_REQUEST_SIZE 2048
#define MAX_RESPONSE_SIZE 4096

typedef struct {
    char method[16];
    char path[256];
    char version[16];
    char headers[1024];
    char body[MAX_REQUEST_SIZE];
} HTTPRequest;

typedef struct {
    char version[16];
    int status_code;
    char status_text[32];
    char headers[1024];
    char body[MAX_RESPONSE_SIZE];
} HTTPResponse;

void handle_http_request(NetworkQueue* nq, const char* request_data, size_t length) {
    HTTPRequest req;
    HTTPResponse resp;
    memset(&req, 0, sizeof(req));
    memset(&resp, 0, sizeof(resp));
    
    sscanf(request_data, "%s %s %s", req.method, req.path, req.version);
    
    printf("Received HTTP Request: %s %s %s\n", req.method, req.path, req.version);
    
    strcpy(resp.version, "HTTP/1.1");
    if (strcmp(req.path, "/") == 0) {
        resp.status_code = 200;
        strcpy(resp.status_text, "OK");
        strcpy(resp.body, "Welcome to Unikernel Web Server!");
    } else {
        resp.status_code = 404;
        strcpy(resp.status_text, "Not Found");
        strcpy(resp.body, "Page not found");
    }
    
    char response_str[MAX_RESPONSE_SIZE];
    int response_len = snprintf(response_str, sizeof(response_str),
             "%s %d %s\r\n"
             "Content-Type: text/plain\r\n"
             "Content-Length: %zu\r\n"
             "\r\n"
             "%s",
             resp.version, resp.status_code, resp.status_text,
             strlen(resp.body), resp.body);
    
    if (response_len > 0 && (size_t)response_len < sizeof(response_str)) {
        network_send_packet(nq, response_str, response_len);
    }
}

// Network processing
void process_network(NetworkQueue* nq) {
    char packet_data[PACKET_SIZE];
    size_t packet_length;
    
    while (network_receive_packet(nq, packet_data, &packet_length)) {
        if (strstr(packet_data, "GET ") == packet_data ||
            strstr(packet_data, "POST ") == packet_data) {
            handle_http_request(nq, packet_data, packet_length);
        }
    }
}

// Initialize unikernel
void init_unikernel(Unikernel* uk) {
    init_memory_manager(&uk->mm);
    init_network_queue(&uk->net_queue);
    init_event_system(&uk->events);
    uk->running = true;
    
    add_event(&uk->events, timer_handler, uk, 10);
}

int main(void) {
    Unikernel uk;
    init_unikernel(&uk);
    
    printf("Unikernel initialized\n");
    
    const char* test_requests[] = {
        "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n",
        "GET /nonexistent HTTP/1.1\r\nHost: localhost\r\n\r\n"
    };
    
    int iterations = 0;
    while (uk.running && iterations < 50) {
        if (iterations % 20 == 0) {
            simulate_receive_packet(&uk.net_queue, test_requests[iterations / 20 % 2]);
        }
        
        process_events(&uk.events);
        process_network(&uk.net_queue);
        
        usleep(100000);  // Sleep for 100ms
        iterations++;
    }
    
    printf("Unikernel simulation completed\n");
    return 0;
}