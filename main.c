#include <stdio.h>
#include <stdlib.h>

#include "helpers.h"

//// Defines

// For using registers as indexed array
#define REGISTER_COUNT 8

// Special value
#define REGISTER_NONE 0xF

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
    INS
} PROGRAM_STATUS;

typedef unsigned char *MEMORY;

typedef struct _STATE
{
    REGISTERS registers;
    CONDITION_CODES codes;
    PROGRAM_STATUS status;
    MEMORY memory;
    int memory_size;
} STATE;

//// Forward declarations

void state_init(STATE *state);
BOOL state_allocate(STATE *state, int size);
void state_free(STATE *state);
BOOL state_compile(STATE *state, const char* filename);

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
        goto cleanup;
    }

cleanup:

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
    
    state->memory_size = memory_size;
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
    return 0;
}
