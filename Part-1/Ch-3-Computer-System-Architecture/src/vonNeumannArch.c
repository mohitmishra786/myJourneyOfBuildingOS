/*
 * Von Neumann Architecture Implementation
 * Demonstrates core concepts of stored-program computer architecture
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MEMORY_SIZE 4096
#define REGISTER_COUNT 16
#define INSTRUCTION_SIZE 4

/* Von Neumann Architecture Components */
typedef struct {
    uint32_t memory[MEMORY_SIZE];           /* Unified memory for instructions and data */
    uint32_t registers[REGISTER_COUNT];     /* CPU registers */
    uint32_t program_counter;               /* Current instruction address */
    uint32_t instruction_register;          /* Current instruction */
    uint32_t memory_address_register;       /* Memory access address */
    uint32_t memory_data_register;          /* Memory access data */
    bool running;                           /* Execution state */
} von_neumann_cpu_t;

/* Instruction format */
typedef struct {
    uint8_t opcode;      /* Operation code */
    uint8_t reg1;        /* First register */
    uint8_t reg2;        /* Second register */
    uint8_t reg3;        /* Third register */
    uint16_t immediate;  /* Immediate value */
} instruction_t;

/* Instruction opcodes */
enum opcodes {
    OP_NOP = 0,     /* No operation */
    OP_LOAD,        /* Load from memory */
    OP_STORE,       /* Store to memory */
    OP_ADD,         /* Addition */
    OP_SUB,         /* Subtraction */
    OP_MUL,         /* Multiplication */
    OP_DIV,         /* Division */
    OP_JMP,         /* Unconditional jump */
    OP_JEQ,         /* Jump if equal */
    OP_JNE,         /* Jump if not equal */
    OP_HALT         /* Halt execution */
};

/* CPU State Management */
von_neumann_cpu_t* create_cpu(void) {
    von_neumann_cpu_t* cpu = malloc(sizeof(von_neumann_cpu_t));
    if (!cpu) return NULL;
    
    /* Initialize memory to zero */
    memset(cpu->memory, 0, sizeof(cpu->memory));
    
    /* Initialize registers to zero */
    memset(cpu->registers, 0, sizeof(cpu->registers));
    
    /* Initialize control registers */
    cpu->program_counter = 0;
    cpu->instruction_register = 0;
    cpu->memory_address_register = 0;
    cpu->memory_data_register = 0;
    cpu->running = false;
    
    return cpu;
}

void destroy_cpu(von_neumann_cpu_t* cpu) {
    if (cpu) {
        free(cpu);
    }
}

/* Memory Operations */
uint32_t memory_read(von_neumann_cpu_t* cpu, uint32_t address) {
    if (address >= MEMORY_SIZE) {
        printf("Memory read error: address 0x%08X out of bounds\n", address);
        return 0;
    }
    
    cpu->memory_address_register = address;
    cpu->memory_data_register = cpu->memory[address];
    return cpu->memory_data_register;
}

void memory_write(von_neumann_cpu_t* cpu, uint32_t address, uint32_t data) {
    if (address >= MEMORY_SIZE) {
        printf("Memory write error: address 0x%08X out of bounds\n", address);
        return;
    }
    
    cpu->memory_address_register = address;
    cpu->memory_data_register = data;
    cpu->memory[address] = data;
}

/* Instruction Fetch-Decode-Execute Cycle */
uint32_t fetch_instruction(von_neumann_cpu_t* cpu) {
    /* Fetch instruction from memory at PC address */
    uint32_t instruction = memory_read(cpu, cpu->program_counter);
    cpu->instruction_register = instruction;
    
    /* Increment program counter */
    cpu->program_counter++;
    
    return instruction;
}

instruction_t decode_instruction(uint32_t raw_instruction) {
    instruction_t instruction;
    
    /* Decode instruction fields */
    instruction.opcode = (raw_instruction >> 24) & 0xFF;
    instruction.reg1 = (raw_instruction >> 20) & 0x0F;
    instruction.reg2 = (raw_instruction >> 16) & 0x0F;
    instruction.reg3 = (raw_instruction >> 12) & 0x0F;
    instruction.immediate = raw_instruction & 0xFFFF;
    
    return instruction;
}

void execute_instruction(von_neumann_cpu_t* cpu, instruction_t instruction) {
    switch (instruction.opcode) {
        case OP_NOP:
            /* No operation - do nothing */
            break;
            
        case OP_LOAD:
            /* Load value from memory to register */
            cpu->registers[instruction.reg1] = 
                memory_read(cpu, cpu->registers[instruction.reg2] + instruction.immediate);
            printf("LOAD R%d, [R%d + %d] -> 0x%08X\n", 
                   instruction.reg1, instruction.reg2, instruction.immediate,
                   cpu->registers[instruction.reg1]);
            break;
            
        case OP_STORE:
            /* Store register value to memory */
            memory_write(cpu, cpu->registers[instruction.reg2] + instruction.immediate,
                        cpu->registers[instruction.reg1]);
            printf("STORE R%d, [R%d + %d]\n", 
                   instruction.reg1, instruction.reg2, instruction.immediate);
            break;
            
        case OP_ADD:
            /* Add two registers */
            cpu->registers[instruction.reg1] = 
                cpu->registers[instruction.reg2] + cpu->registers[instruction.reg3];
            printf("ADD R%d, R%d, R%d -> 0x%08X\n", 
                   instruction.reg1, instruction.reg2, instruction.reg3,
                   cpu->registers[instruction.reg1]);
            break;
            
        case OP_SUB:
            /* Subtract two registers */
            cpu->registers[instruction.reg1] = 
                cpu->registers[instruction.reg2] - cpu->registers[instruction.reg3];
            printf("SUB R%d, R%d, R%d -> 0x%08X\n", 
                   instruction.reg1, instruction.reg2, instruction.reg3,
                   cpu->registers[instruction.reg1]);
            break;
            
        case OP_MUL:
            /* Multiply two registers */
            cpu->registers[instruction.reg1] = 
                cpu->registers[instruction.reg2] * cpu->registers[instruction.reg3];
            printf("MUL R%d, R%d, R%d -> 0x%08X\n", 
                   instruction.reg1, instruction.reg2, instruction.reg3,
                   cpu->registers[instruction.reg1]);
            break;
            
        case OP_DIV:
            /* Divide two registers */
            if (cpu->registers[instruction.reg3] != 0) {
                cpu->registers[instruction.reg1] = 
                    cpu->registers[instruction.reg2] / cpu->registers[instruction.reg3];
                printf("DIV R%d, R%d, R%d -> 0x%08X\n", 
                       instruction.reg1, instruction.reg2, instruction.reg3,
                       cpu->registers[instruction.reg1]);
            } else {
                printf("Division by zero error!\n");
                cpu->running = false;
            }
            break;
            
        case OP_JMP:
            /* Unconditional jump */
            cpu->program_counter = instruction.immediate;
            printf("JMP %d\n", instruction.immediate);
            break;
            
        case OP_JEQ:
            /* Jump if registers are equal */
            if (cpu->registers[instruction.reg1] == cpu->registers[instruction.reg2]) {
                cpu->program_counter = instruction.immediate;
                printf("JEQ R%d, R%d, %d (taken)\n", 
                       instruction.reg1, instruction.reg2, instruction.immediate);
            } else {
                printf("JEQ R%d, R%d, %d (not taken)\n", 
                       instruction.reg1, instruction.reg2, instruction.immediate);
            }
            break;
            
        case OP_JNE:
            /* Jump if registers are not equal */
            if (cpu->registers[instruction.reg1] != cpu->registers[instruction.reg2]) {
                cpu->program_counter = instruction.immediate;
                printf("JNE R%d, R%d, %d (taken)\n", 
                       instruction.reg1, instruction.reg2, instruction.immediate);
            } else {
                printf("JNE R%d, R%d, %d (not taken)\n", 
                       instruction.reg1, instruction.reg2, instruction.immediate);
            }
            break;
            
        case OP_HALT:
            /* Halt execution */
            printf("HALT\n");
            cpu->running = false;
            break;
            
        default:
            printf("Unknown opcode: 0x%02X\n", instruction.opcode);
            cpu->running = false;
            break;
    }
}

void cpu_cycle(von_neumann_cpu_t* cpu) {
    /* Fetch instruction */
    uint32_t raw_instruction = fetch_instruction(cpu);
    
    /* Decode instruction */
    instruction_t instruction = decode_instruction(raw_instruction);
    
    /* Execute instruction */
    execute_instruction(cpu, instruction);
}

/* Program Loading and Execution */
uint32_t encode_instruction(uint8_t opcode, uint8_t reg1, uint8_t reg2, 
                           uint8_t reg3, uint16_t immediate) {
    return ((uint32_t)opcode << 24) | 
           ((uint32_t)reg1 << 20) | 
           ((uint32_t)reg2 << 16) | 
           ((uint32_t)reg3 << 12) | 
           ((uint32_t)immediate);
}

void load_program(von_neumann_cpu_t* cpu, uint32_t* program, uint32_t program_size) {
    if (program_size > MEMORY_SIZE) {
        printf("Program too large for memory!\n");
        return;
    }
    
    /* Load program into memory starting at address 0 */
    for (uint32_t i = 0; i < program_size; i++) {
        memory_write(cpu, i, program[i]);
    }
    
    printf("Loaded program of %d instructions\n", program_size);
}

void run_cpu(von_neumann_cpu_t* cpu) {
    cpu->running = true;
    cpu->program_counter = 0;
    
    printf("\n=== Starting CPU Execution ===\n");
    
    uint32_t cycle_count = 0;
    while (cpu->running && cycle_count < 1000) {  /* Safety limit */
        printf("Cycle %d: PC=0x%04X ", cycle_count, cpu->program_counter);
        cpu_cycle(cpu);
        cycle_count++;
    }
    
    printf("=== CPU Execution Complete ===\n");
    printf("Total cycles: %d\n", cycle_count);
}

void print_cpu_state(von_neumann_cpu_t* cpu) {
    printf("\n=== CPU State ===\n");
    printf("PC: 0x%04X  IR: 0x%08X\n", cpu->program_counter, cpu->instruction_register);
    printf("MAR: 0x%04X  MDR: 0x%08X\n", cpu->memory_address_register, cpu->memory_data_register);
    
    printf("Registers:\n");
    for (int i = 0; i < REGISTER_COUNT; i += 4) {
        printf("R%02d: 0x%08X  R%02d: 0x%08X  R%02d: 0x%08X  R%02d: 0x%08X\n",
               i, cpu->registers[i], i+1, cpu->registers[i+1],
               i+2, cpu->registers[i+2], i+3, cpu->registers[i+3]);
    }
    
    printf("Memory (first 32 words):\n");
    for (int i = 0; i < 32; i += 4) {
        printf("0x%04X: 0x%08X 0x%08X 0x%08X 0x%08X\n",
               i, cpu->memory[i], cpu->memory[i+1], cpu->memory[i+2], cpu->memory[i+3]);
    }
}

/* Demonstration Programs */
void demo_arithmetic_program(von_neumann_cpu_t* cpu) {
    printf("\n=== Arithmetic Program Demo ===\n");
    
    /* Program to calculate (5 + 3) * 2 */
    uint32_t program[] = {
        /* Load immediate values into registers */
        encode_instruction(OP_LOAD, 1, 0, 0, 100),  /* Load from memory[100] to R1 */
        encode_instruction(OP_LOAD, 2, 0, 0, 101),  /* Load from memory[101] to R2 */
        encode_instruction(OP_LOAD, 3, 0, 0, 102),  /* Load from memory[102] to R3 */
        
        /* Perform arithmetic operations */
        encode_instruction(OP_ADD, 4, 1, 2, 0),     /* R4 = R1 + R2 */
        encode_instruction(OP_MUL, 5, 4, 3, 0),     /* R5 = R4 * R3 */
        
        /* Store result */
        encode_instruction(OP_STORE, 5, 0, 0, 103), /* Store R5 to memory[103] */
        
        /* Halt */
        encode_instruction(OP_HALT, 0, 0, 0, 0)
    };
    
    /* Initialize data in memory */
    memory_write(cpu, 100, 5);  /* First operand */
    memory_write(cpu, 101, 3);  /* Second operand */
    memory_write(cpu, 102, 2);  /* Multiplier */
    
    load_program(cpu, program, sizeof(program) / sizeof(program[0]));
    run_cpu(cpu);
    
    printf("Result: %d (stored at memory[103])\n", memory_read(cpu, 103));
}

void demo_loop_program(von_neumann_cpu_t* cpu) {
    printf("\n=== Loop Program Demo ===\n");
    
    /* Program to sum numbers 1 through 5 */
    uint32_t program[] = {
        /* Initialize */
        encode_instruction(OP_LOAD, 1, 0, 0, 100),  /* R1 = counter (memory[100]) */
        encode_instruction(OP_LOAD, 2, 0, 0, 101),  /* R2 = sum (memory[101]) */
        encode_instruction(OP_LOAD, 3, 0, 0, 102),  /* R3 = limit (memory[102]) */
        
        /* Loop start (address 3) */
        encode_instruction(OP_ADD, 2, 2, 1, 0),     /* sum += counter */
        encode_instruction(OP_ADD, 1, 1, 0, 1),     /* counter++ (add immediate 1) */
        encode_instruction(OP_JNE, 1, 3, 3, 0),     /* if counter != limit, jump to 3 */
        
        /* Store result and halt */
        encode_instruction(OP_STORE, 2, 0, 0, 103), /* Store sum to memory[103] */
        encode_instruction(OP_HALT, 0, 0, 0, 0)
    };
    
    /* Initialize data */
    memory_write(cpu, 100, 1);  /* Counter starts at 1 */
    memory_write(cpu, 101, 0);  /* Sum starts at 0 */
    memory_write(cpu, 102, 6);  /* Loop until counter reaches 6 */
    
    load_program(cpu, program, sizeof(program) / sizeof(program[0]));
    run_cpu(cpu);
    
    printf("Sum of 1-5: %d (stored at memory[103])\n", memory_read(cpu, 103));
}

int main(void) {
    printf("Von Neumann Architecture Simulator\n");
    printf("===================================\n");
    
    /* Create CPU instance */
    von_neumann_cpu_t* cpu = create_cpu();
    if (!cpu) {
        printf("Failed to create CPU\n");
        return 1;
    }
    
    /* Run demonstration programs */
    demo_arithmetic_program(cpu);
    print_cpu_state(cpu);
    
    /* Reset CPU for next demo */
    memset(cpu->memory, 0, sizeof(cpu->memory));
    memset(cpu->registers, 0, sizeof(cpu->registers));
    
    demo_loop_program(cpu);
    print_cpu_state(cpu);
    
    /* Cleanup */
    destroy_cpu(cpu);
    
    return 0;
} 