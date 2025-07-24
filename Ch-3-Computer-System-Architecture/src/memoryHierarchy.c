/*
 * Memory Hierarchy Simulation
 * Demonstrates cache memory, virtual memory, and memory management concepts
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define MAIN_MEMORY_SIZE (1024 * 1024)  /* 1MB main memory */
#define L1_CACHE_SIZE 1024               /* 1KB L1 cache */
#define L2_CACHE_SIZE 4096               /* 4KB L2 cache */
#define L3_CACHE_SIZE 16384              /* 16KB L3 cache */
#define CACHE_LINE_SIZE 64               /* 64 byte cache lines */
#define PAGE_SIZE 4096                   /* 4KB pages */
#define TLB_SIZE 64                      /* 64-entry TLB */

/* Memory access types */
typedef enum {
    ACCESS_READ,
    ACCESS_WRITE
} access_type_t;

/* Cache line structure */
typedef struct {
    uint32_t tag;
    uint8_t data[CACHE_LINE_SIZE];
    bool valid;
    bool dirty;
    uint64_t access_time;
    uint32_t access_count;
} cache_line_t;

/* Cache set structure */
typedef struct {
    cache_line_t* lines;
    uint32_t associativity;
} cache_set_t;

/* Cache controller structure */
typedef struct {
    cache_set_t* sets;
    uint32_t set_count;
    uint32_t line_size;
    uint32_t total_size;
    uint32_t associativity;
    uint32_t access_latency;
    
    /* Performance counters */
    uint64_t hit_count;
    uint64_t miss_count;
    uint64_t write_back_count;
    uint64_t total_accesses;
} cache_controller_t;

/* TLB entry structure */
typedef struct {
    uint32_t virtual_page;
    uint32_t physical_page;
    bool valid;
    bool dirty;
    bool accessed;
    uint8_t protection_flags;
} tlb_entry_t;

/* Page table entry */
typedef struct {
    uint32_t physical_page;
    bool present;
    bool writable;
    bool user_accessible;
    bool accessed;
    bool dirty;
} page_table_entry_t;

/* Memory hierarchy structure */
typedef struct {
    uint8_t* main_memory;
    cache_controller_t* l1_cache;
    cache_controller_t* l2_cache;
    cache_controller_t* l3_cache;
    tlb_entry_t* tlb;
    page_table_entry_t* page_table;
    
    /* Global performance counters */
    uint64_t memory_accesses;
    uint64_t tlb_hits;
    uint64_t tlb_misses;
    uint64_t page_faults;
    uint64_t total_cycles;
} memory_hierarchy_t;

/* Cache management functions */
cache_controller_t* create_cache(uint32_t total_size, uint32_t line_size, 
                                uint32_t associativity, uint32_t latency) {
    cache_controller_t* cache = malloc(sizeof(cache_controller_t));
    if (!cache) return NULL;
    
    cache->total_size = total_size;
    cache->line_size = line_size;
    cache->associativity = associativity;
    cache->access_latency = latency;
    cache->set_count = total_size / (line_size * associativity);
    
    /* Allocate cache sets */
    cache->sets = malloc(cache->set_count * sizeof(cache_set_t));
    if (!cache->sets) {
        free(cache);
        return NULL;
    }
    
    /* Initialize cache sets and lines */
    for (uint32_t i = 0; i < cache->set_count; i++) {
        cache->sets[i].associativity = associativity;
        cache->sets[i].lines = malloc(associativity * sizeof(cache_line_t));
        
        if (!cache->sets[i].lines) {
            /* Cleanup on failure */
            for (uint32_t j = 0; j < i; j++) {
                free(cache->sets[j].lines);
            }
            free(cache->sets);
            free(cache);
            return NULL;
        }
        
        /* Initialize cache lines */
        for (uint32_t j = 0; j < associativity; j++) {
            cache->sets[i].lines[j].valid = false;
            cache->sets[i].lines[j].dirty = false;
            cache->sets[i].lines[j].tag = 0;
            cache->sets[i].lines[j].access_time = 0;
            cache->sets[i].lines[j].access_count = 0;
            memset(cache->sets[i].lines[j].data, 0, CACHE_LINE_SIZE);
        }
    }
    
    /* Initialize performance counters */
    cache->hit_count = 0;
    cache->miss_count = 0;
    cache->write_back_count = 0;
    cache->total_accesses = 0;
    
    return cache;
}

void destroy_cache(cache_controller_t* cache) {
    if (!cache) return;
    
    if (cache->sets) {
        for (uint32_t i = 0; i < cache->set_count; i++) {
            free(cache->sets[i].lines);
        }
        free(cache->sets);
    }
    free(cache);
}

/* Address decomposition for cache */
typedef struct {
    uint32_t tag;
    uint32_t set_index;
    uint32_t block_offset;
} cache_address_t;

cache_address_t decompose_address(uint32_t address, cache_controller_t* cache) {
    cache_address_t result;
    
    uint32_t offset_bits = __builtin_ctz(cache->line_size);
    uint32_t index_bits = __builtin_ctz(cache->set_count);
    
    result.block_offset = address & ((1 << offset_bits) - 1);
    result.set_index = (address >> offset_bits) & ((1 << index_bits) - 1);
    result.tag = address >> (offset_bits + index_bits);
    
    return result;
}

/* LRU replacement policy */
cache_line_t* find_lru_victim(cache_set_t* set) {
    cache_line_t* victim = &set->lines[0];
    uint64_t oldest_time = victim->access_time;
    
    /* Find invalid line first */
    for (uint32_t i = 0; i < set->associativity; i++) {
        if (!set->lines[i].valid) {
            return &set->lines[i];
        }
    }
    
    /* Find LRU line */
    for (uint32_t i = 1; i < set->associativity; i++) {
        if (set->lines[i].access_time < oldest_time) {
            oldest_time = set->lines[i].access_time;
            victim = &set->lines[i];
        }
    }
    
    return victim;
}

/* Cache lookup and access */
cache_line_t* cache_lookup(cache_controller_t* cache, uint32_t address) {
    cache_address_t addr = decompose_address(address, cache);
    cache_set_t* set = &cache->sets[addr.set_index];
    
    cache->total_accesses++;
    
    /* Search for matching tag */
    for (uint32_t i = 0; i < set->associativity; i++) {
        cache_line_t* line = &set->lines[i];
        if (line->valid && line->tag == addr.tag) {
            /* Cache hit */
            line->access_time = cache->total_accesses;
            line->access_count++;
            cache->hit_count++;
            return line;
        }
    }
    
    /* Cache miss */
    cache->miss_count++;
    return NULL;
}

bool cache_access(cache_controller_t* cache, uint32_t address, 
                 access_type_t access_type, uint8_t* data, 
                 memory_hierarchy_t* mem_hierarchy) {
    cache_line_t* line = cache_lookup(cache, address);
    cache_address_t addr = decompose_address(address, cache);
    
    if (line) {
        /* Cache hit */
        if (access_type == ACCESS_READ) {
            memcpy(data, &line->data[addr.block_offset], 
                   sizeof(uint32_t));
        } else {
            memcpy(&line->data[addr.block_offset], data, 
                   sizeof(uint32_t));
            line->dirty = true;
        }
        mem_hierarchy->total_cycles += cache->access_latency;
        return true;
    } else {
        /* Cache miss - need to load from lower level */
        cache_set_t* set = &cache->sets[addr.set_index];
        cache_line_t* victim = find_lru_victim(set);
        
        /* Write back if dirty */
        if (victim->valid && victim->dirty) {
            cache->write_back_count++;
            /* In real system, would write to lower level */
        }
        
        /* Load new line */
        victim->valid = true;
        victim->dirty = false;
        victim->tag = addr.tag;
        victim->access_time = cache->total_accesses;
        victim->access_count = 1;
        
        /* Simulate loading from memory */
        uint32_t line_address = (address / cache->line_size) * cache->line_size;
        memcpy(victim->data, &mem_hierarchy->main_memory[line_address], 
               cache->line_size);
        
        /* Perform the access */
        if (access_type == ACCESS_READ) {
            memcpy(data, &victim->data[addr.block_offset], 
                   sizeof(uint32_t));
        } else {
            memcpy(&victim->data[addr.block_offset], data, 
                   sizeof(uint32_t));
            victim->dirty = true;
        }
        
        mem_hierarchy->total_cycles += cache->access_latency;
        return false;
    }
}

/* TLB management */
tlb_entry_t* tlb_lookup(memory_hierarchy_t* mem_hierarchy, uint32_t virtual_page) {
    for (int i = 0; i < TLB_SIZE; i++) {
        tlb_entry_t* entry = &mem_hierarchy->tlb[i];
        if (entry->valid && entry->virtual_page == virtual_page) {
            mem_hierarchy->tlb_hits++;
            entry->accessed = true;
            return entry;
        }
    }
    
    mem_hierarchy->tlb_misses++;
    return NULL;
}

uint32_t translate_address(memory_hierarchy_t* mem_hierarchy, uint32_t virtual_address) {
    uint32_t virtual_page = virtual_address / PAGE_SIZE;
    uint32_t page_offset = virtual_address % PAGE_SIZE;
    
    /* Check TLB first */
    tlb_entry_t* tlb_entry = tlb_lookup(mem_hierarchy, virtual_page);
    if (tlb_entry) {
        return tlb_entry->physical_page * PAGE_SIZE + page_offset;
    }
    
    /* TLB miss - check page table */
    page_table_entry_t* pte = &mem_hierarchy->page_table[virtual_page];
    if (!pte->present) {
        /* Page fault */
        mem_hierarchy->page_faults++;
        printf("Page fault for virtual page %d\n", virtual_page);
        
        /* Simulate page loading */
        pte->present = true;
        pte->physical_page = virtual_page; /* Simple 1:1 mapping for demo */
        pte->writable = true;
        pte->user_accessible = true;
    }
    
    /* Update TLB */
    for (int i = 0; i < TLB_SIZE; i++) {
        tlb_entry_t* entry = &mem_hierarchy->tlb[i];
        if (!entry->valid) {
            entry->valid = true;
            entry->virtual_page = virtual_page;
            entry->physical_page = pte->physical_page;
            entry->accessed = true;
            entry->dirty = false;
            entry->protection_flags = 0;
            break;
        }
    }
    
    return pte->physical_page * PAGE_SIZE + page_offset;
}

/* Memory hierarchy access */
void memory_hierarchy_access(memory_hierarchy_t* mem_hierarchy, 
                           uint32_t virtual_address, 
                           access_type_t access_type, 
                           uint8_t* data) {
    mem_hierarchy->memory_accesses++;
    
    /* Translate virtual to physical address */
    uint32_t physical_address = translate_address(mem_hierarchy, virtual_address);
    
    /* Try L1 cache first */
    if (cache_access(mem_hierarchy->l1_cache, physical_address, 
                    access_type, data, mem_hierarchy)) {
        return; /* L1 hit */
    }
    
    /* Try L2 cache */
    if (cache_access(mem_hierarchy->l2_cache, physical_address, 
                    access_type, data, mem_hierarchy)) {
        return; /* L2 hit */
    }
    
    /* Try L3 cache */
    if (cache_access(mem_hierarchy->l3_cache, physical_address, 
                    access_type, data, mem_hierarchy)) {
        return; /* L3 hit */
    }
    
    /* Main memory access */
    if (access_type == ACCESS_READ) {
        memcpy(data, &mem_hierarchy->main_memory[physical_address], 
               sizeof(uint32_t));
    } else {
        memcpy(&mem_hierarchy->main_memory[physical_address], data, 
               sizeof(uint32_t));
    }
    
    mem_hierarchy->total_cycles += 100; /* Main memory latency */
}

/* Create memory hierarchy */
memory_hierarchy_t* create_memory_hierarchy(void) {
    memory_hierarchy_t* mem_hierarchy = malloc(sizeof(memory_hierarchy_t));
    if (!mem_hierarchy) return NULL;
    
    /* Allocate main memory */
    mem_hierarchy->main_memory = malloc(MAIN_MEMORY_SIZE);
    if (!mem_hierarchy->main_memory) {
        free(mem_hierarchy);
        return NULL;
    }
    
    /* Initialize main memory with test pattern */
    for (uint32_t i = 0; i < MAIN_MEMORY_SIZE / sizeof(uint32_t); i++) {
        ((uint32_t*)mem_hierarchy->main_memory)[i] = i;
    }
    
    /* Create cache levels */
    mem_hierarchy->l1_cache = create_cache(L1_CACHE_SIZE, CACHE_LINE_SIZE, 2, 1);
    mem_hierarchy->l2_cache = create_cache(L2_CACHE_SIZE, CACHE_LINE_SIZE, 4, 10);
    mem_hierarchy->l3_cache = create_cache(L3_CACHE_SIZE, CACHE_LINE_SIZE, 8, 20);
    
    if (!mem_hierarchy->l1_cache || !mem_hierarchy->l2_cache || !mem_hierarchy->l3_cache) {
        destroy_cache(mem_hierarchy->l1_cache);
        destroy_cache(mem_hierarchy->l2_cache);
        destroy_cache(mem_hierarchy->l3_cache);
        free(mem_hierarchy->main_memory);
        free(mem_hierarchy);
        return NULL;
    }
    
    /* Allocate TLB */
    mem_hierarchy->tlb = malloc(TLB_SIZE * sizeof(tlb_entry_t));
    if (!mem_hierarchy->tlb) {
        destroy_cache(mem_hierarchy->l1_cache);
        destroy_cache(mem_hierarchy->l2_cache);
        destroy_cache(mem_hierarchy->l3_cache);
        free(mem_hierarchy->main_memory);
        free(mem_hierarchy);
        return NULL;
    }
    
    /* Initialize TLB */
    for (int i = 0; i < TLB_SIZE; i++) {
        mem_hierarchy->tlb[i].valid = false;
    }
    
    /* Allocate page table */
    uint32_t page_count = MAIN_MEMORY_SIZE / PAGE_SIZE;
    mem_hierarchy->page_table = malloc(page_count * sizeof(page_table_entry_t));
    if (!mem_hierarchy->page_table) {
        free(mem_hierarchy->tlb);
        destroy_cache(mem_hierarchy->l1_cache);
        destroy_cache(mem_hierarchy->l2_cache);
        destroy_cache(mem_hierarchy->l3_cache);
        free(mem_hierarchy->main_memory);
        free(mem_hierarchy);
        return NULL;
    }
    
    /* Initialize page table */
    for (uint32_t i = 0; i < page_count; i++) {
        mem_hierarchy->page_table[i].present = false;
        mem_hierarchy->page_table[i].writable = true;
        mem_hierarchy->page_table[i].user_accessible = true;
        mem_hierarchy->page_table[i].accessed = false;
        mem_hierarchy->page_table[i].dirty = false;
        mem_hierarchy->page_table[i].physical_page = i;
    }
    
    /* Initialize performance counters */
    mem_hierarchy->memory_accesses = 0;
    mem_hierarchy->tlb_hits = 0;
    mem_hierarchy->tlb_misses = 0;
    mem_hierarchy->page_faults = 0;
    mem_hierarchy->total_cycles = 0;
    
    return mem_hierarchy;
}

void destroy_memory_hierarchy(memory_hierarchy_t* mem_hierarchy) {
    if (!mem_hierarchy) return;
    
    destroy_cache(mem_hierarchy->l1_cache);
    destroy_cache(mem_hierarchy->l2_cache);
    destroy_cache(mem_hierarchy->l3_cache);
    free(mem_hierarchy->main_memory);
    free(mem_hierarchy->tlb);
    free(mem_hierarchy->page_table);
    free(mem_hierarchy);
}

/* Performance reporting */
void print_cache_stats(const char* name, cache_controller_t* cache) {
    if (cache->total_accesses == 0) {
        printf("%s Cache: No accesses\n", name);
        return;
    }
    
    double hit_rate = (double)cache->hit_count / cache->total_accesses;
    double miss_rate = (double)cache->miss_count / cache->total_accesses;
    
    printf("%s Cache Statistics:\n", name);
    printf("  Total Accesses: %llu\n", cache->total_accesses);
    printf("  Hits: %llu (%.2f%%)\n", cache->hit_count, hit_rate * 100);
    printf("  Misses: %llu (%.2f%%)\n", cache->miss_count, miss_rate * 100);
    printf("  Write-backs: %llu\n", cache->write_back_count);
    printf("  Access Latency: %d cycles\n", cache->access_latency);
}

void print_memory_hierarchy_stats(memory_hierarchy_t* mem_hierarchy) {
    printf("\n=== Memory Hierarchy Performance ===\n");
    
    printf("Overall Statistics:\n");
    printf("  Total Memory Accesses: %llu\n", mem_hierarchy->memory_accesses);
    printf("  Total Cycles: %llu\n", mem_hierarchy->total_cycles);
    printf("  Average Access Time: %.2f cycles\n", 
           (double)mem_hierarchy->total_cycles / mem_hierarchy->memory_accesses);
    
    printf("\nTLB Statistics:\n");
    printf("  TLB Hits: %llu\n", mem_hierarchy->tlb_hits);
    printf("  TLB Misses: %llu\n", mem_hierarchy->tlb_misses);
    printf("  Page Faults: %llu\n", mem_hierarchy->page_faults);
    
    if (mem_hierarchy->tlb_hits + mem_hierarchy->tlb_misses > 0) {
        double tlb_hit_rate = (double)mem_hierarchy->tlb_hits / 
                             (mem_hierarchy->tlb_hits + mem_hierarchy->tlb_misses);
        printf("  TLB Hit Rate: %.2f%%\n", tlb_hit_rate * 100);
    }
    
    printf("\n");
    print_cache_stats("L1", mem_hierarchy->l1_cache);
    printf("\n");
    print_cache_stats("L2", mem_hierarchy->l2_cache);
    printf("\n");
    print_cache_stats("L3", mem_hierarchy->l3_cache);
}

/* Test workloads */
void test_sequential_access(memory_hierarchy_t* mem_hierarchy) {
    printf("\n=== Testing Sequential Access Pattern ===\n");
    
    uint32_t data;
    for (uint32_t addr = 0; addr < 4096; addr += 4) {
        memory_hierarchy_access(mem_hierarchy, addr, ACCESS_READ, (uint8_t*)&data);
    }
    
    printf("Sequential access test completed\n");
}

void test_random_access(memory_hierarchy_t* mem_hierarchy) {
    printf("\n=== Testing Random Access Pattern ===\n");
    
    srand(time(NULL));
    uint32_t data;
    
    for (int i = 0; i < 1000; i++) {
        uint32_t addr = (rand() % (MAIN_MEMORY_SIZE / 4)) * 4;
        memory_hierarchy_access(mem_hierarchy, addr, ACCESS_READ, (uint8_t*)&data);
    }
    
    printf("Random access test completed\n");
}

void test_strided_access(memory_hierarchy_t* mem_hierarchy) {
    printf("\n=== Testing Strided Access Pattern ===\n");
    
    uint32_t data;
    uint32_t stride = CACHE_LINE_SIZE * 2; /* Skip every other cache line */
    
    for (uint32_t addr = 0; addr < 8192; addr += stride) {
        memory_hierarchy_access(mem_hierarchy, addr, ACCESS_READ, (uint8_t*)&data);
    }
    
    printf("Strided access test completed\n");
}

int main(void) {
    printf("Memory Hierarchy Simulation\n");
    printf("============================\n");
    
    /* Create memory hierarchy */
    memory_hierarchy_t* mem_hierarchy = create_memory_hierarchy();
    if (!mem_hierarchy) {
        printf("Failed to create memory hierarchy\n");
        return 1;
    }
    
    /* Run different access pattern tests */
    test_sequential_access(mem_hierarchy);
    test_random_access(mem_hierarchy);
    test_strided_access(mem_hierarchy);
    
    /* Print performance statistics */
    print_memory_hierarchy_stats(mem_hierarchy);
    
    /* Cleanup */
    destroy_memory_hierarchy(mem_hierarchy);
    
    return 0;
} 