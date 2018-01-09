#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"

//// Defines

// For using registers as indexed array
#define REGISTER_COUNT 8
#define REGISTER_NAME_ARRAY { "eax", "ecx", "edx", "ebx", "esi", "edi", "esp", "ebp" }

// Special value
#define REGISTER_NONE 0xF

// Status information
#define STATUS_COUNT 4
#define STATUS_NAME_ARRAY { "AOK", "HLT", "ADR", "INS" }

// Default size of memory block in bytes
#define DEF_MEMORY_SIZE 1024

//// Type declarations

typedef struct _REGISTER_NAMES
{
    int eax;
    int ecx;
    int edx;
    int ebx;
    int esi;
    int edi;
    int esp;
    int ebp;
} REGISTER_NAMES;

typedef int REGISTER_ID;

typedef union _REGISTERS
{
    REGISTER_NAMES names;
    REGISTER_ID ids[REGISTER_COUNT];
} REGISTERS;

typedef struct _CONDITION_CODES
{
    BOOL ZF;
    BOOL SF;
    BOOL OF;
} CONDITION_CODES;

typedef enum _PROGRAM_STATUS
{
    AOK = 1,
    HLT,
    ADR,
    INS,
    _FIRST = AOK,
    _LAST = INS
} PROGRAM_STATUS;

typedef unsigned char *MEMORY;

typedef struct _STATE
{
    REGISTERS registers;
    CONDITION_CODES codes;
    PROGRAM_STATUS status;
    int pc;
    MEMORY memory;
    int memory_size;
} STATE;

//// Forward declarations

void state_init(STATE *state);
BOOL state_allocate(STATE *state, int size);
void state_free(STATE *state);
BOOL state_compile(STATE *state, const char* filename);
void state_run(STATE *state);
void state_print(STATE *state);
void state_memory(STATE *state, int lines);

//// Main function

int main(int argc, char** argv)
{
    // No source file?
    if (1 >= argc)
    {
        char* prog = (0 == argc) ? "program" : argv[0];
        printf("Usage: %s <source-file>\n", prog);
        return 0;
    }

    // Setup state
    STATE state = { 0 };
    state_init(&state);

    // Read arguments
    char* source_file = argv[1];
    int memory_size = DEF_MEMORY_SIZE;

    // Supplied memory size?
    if (3 <= argc)
        if ((0 == an_parse_int(argv[2], &memory_size)) || (1 > memory_size))
        {
            memory_size = DEF_MEMORY_SIZE;
            printf("[!] Invalid memory size: '%s'", argv[2]);
            printf(", using memory size of: %d\n", memory_size);
        }
        else
            printf("[-] Setting memory size to: %d\n", memory_size);
    
    // Allocate memory
    if (0 == state_allocate(&state, memory_size))
    {
        printf("[!] Failed to allocate memory\n");
        return 0;
    }
    
    // Try to compile from source file
    if (0 == state_compile(&state, source_file))
    {
        printf("[!] Failed to compile\n");
        state_free(&state);
        return 0;
    }

    // Run program
    state_run(&state);

    // Log the state
    state_print(&state);
    state_memory(&state, 8);

    // Free memory
    state_free(&state);

    return 0;
}

//// Definitions

void state_init(STATE *state)
{
    // Nothing passed?
    if (NULL == state)
        return;

    state->status = AOK;
}

BOOL state_allocate(STATE *state, int memory_size)
{
    // Nothing passed?
    if (NULL == state)
        return 0;

    // Invalid memory size?
    if (1 > memory_size)
        return 0;
    
    // Free existing
    state_free(state);
    
    // Try to allocate
    state->memory = malloc(memory_size);
    if (NULL == state->memory)
        return 0;
    
    // Zero out
    state->memory_size = memory_size;
    memset(state->memory, 0, memory_size);

    return 1;
}

void state_free(STATE *state)
{
    // Nothing passed?
    if (NULL == state)
        return;

    // Nothing allocated?
    if (0 >= state->memory_size)
        return;
    
    free(state->memory);
    state->memory_size = 0;
}

BOOL state_compile(STATE *state, const char* filename)
{
    // Nothing passed?
    if (NULL == state)
        return 0;

    // No memory?
    if (0 >= state->memory_size)
        return 0;

    printf("[!] TODO: Try compiling from '%s'\n", filename);

    // Manually compiled information from CMU.edu
    const int program[] = {
        0x30f40001, 0x000030f5, 0x00010000, 0x80240000,
        0x00000000, 0x0d000000, 0xc0000000, 0x000b0000,
        0x00a00000, 0xa05f2045, 0x30f00400, 0x0000a00f,
        0x30f21400, 0x0000a02f, 0x80420000, 0x002054b0,
        0x5f90a05f, 0x20455015, 0x08000000, 0x50250c00,
        0x00006300, 0x62227378, 0x00000056, 0x61000000,
        0x00606030, 0xf3040000, 0x00603130, 0xf3ffffff,
        0xff603274, 0x5b000000, 0x2045b05f, 0x90000000,
    };
    for (int i = 0; i < 32; i++)
        an_int_bytes_big(program[i], state->memory + (i * 4));

    return 1;
}

void state_run(STATE *state)
{
    printf("[!] TODO: Run the program\n");
}

void state_print(STATE *state)
{
    // Nothing passed?
    if (NULL == state)
        return;
    
    printf("===== Current state: =====\n");

    const char* reg_names[REGISTER_COUNT] = REGISTER_NAME_ARRAY;
    printf("Registers:\n");
    for (int i = 0; REGISTER_COUNT / 4 > i; i++) {
        for (int j = i * 4; ((i + 1) * 4 > j) && (REGISTER_COUNT > j); j++)
            printf("  %-3s:    0x%08x", reg_names[j], state->registers.ids[j]);
        printf("\n");
    }

    printf("Condition Codes:\n");
    printf("  ZF: %14s", an_bool_str(state->codes.ZF));
    printf("  SF: %14s", an_bool_str(state->codes.SF));
    printf("  OF: %14s\n", an_bool_str(state->codes.OF));

    printf("Program Counter:\n  PC:     0x%08x", state->pc);
    if (0 < state->memory_size)
    {
        printf("  Mem:  ");
        for (int i = 0; i < 6; i++)
        {
            int pos = state->pc + i;
            BOOL valid = (0 <= pos) && (state->memory_size > pos);
            if (valid)
                printf("%0.2x", state->memory[pos]);
            else
                printf("??");
        }
    }
    printf("\n");

    
    const char* st_names[STATUS_COUNT] = STATUS_NAME_ARRAY;
    BOOL st_valid = (_FIRST > state->status) || (_LAST < state->status);
    const char* st_str = st_valid ? "???" : st_names[state->status - _FIRST];
    printf("Status:\n  STR: %13s  VAL: %13d\n", st_str, state->status);
}

void state_memory(STATE *state, int lines)
{
    // Nothing passed?
    if (NULL == state)
        return;

    // No memory?
    if (0 >= state->memory_size)
        return;
    
    printf("Memory:\n");
    for (int i = 0; i < lines; i++)
    {
        printf("  Addr   0x%08x:", i * 16);
        for (int j = 0; j < 4; j++)
        {
            printf("  ");
            for (int k = 0; k < 4; k++)
            {
                int pos = i * 16 + j * 4 + k;
                BOOL valid = (0 <= pos) && (state->memory_size > pos);
                if (valid)
                    printf("%0.2x", state->memory[pos]);
                else
                    printf("??");
            }
        }
        printf("\n");
    }
}
