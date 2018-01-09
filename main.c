#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"

//// Defines

// For using registers as indexed array
#define REGISTER_COUNT 8
#define REGISTER_NAME_ARRAY { "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi" }

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
    int esp;
    int ebp;
    int esi;
    int edi;
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
    MEMORY memory;
    int memory_size;
    int pc;
    int step;
} STATE;

//// Forward declarations

void state_init(STATE *state);
BOOL state_allocate(STATE *state, int size);
void state_free(STATE *state);
BOOL state_compile(STATE *state, const char* filename);
void state_run(STATE *state, STATE *state_original);
BOOL state_clone(STATE *state_from, STATE *state_to);
void state_changes(STATE *state_old, STATE *state_now);
BOOL state_push(STATE *state, unsigned int val);
BOOL state_pop(STATE *state, unsigned int *val);

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
        if ((0 == an_parse_int(argv[2], &memory_size)) || (1 > memory_size) || (0 != memory_size % 4))
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

    // Make a copy
    STATE state_original = { 0 };
    if (0 == state_clone(&state, &state_original))
    {
        printf("[!] Could not clone state\n");
        state_free(&state);
        return 0;
    }

    // Run program
    state_run(&state, &state_original);

    // Log the changes
    state_changes(&state_original, &state);

    // Free memory
    state_free(&state);
    state_free(&state_original);

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

    // Possibly use something like this:
    // https://github.com/xsznix/js-y86/blob/master/js/y86.js : evalArgs
    // https://github.com/xsznix/js-y86/blob/master/js/syntax.js

    printf("[!] TODO: Compile '%s'\n", filename);

    // Manually compiled information from CMU.edu
    const int program[] = {
        0x30f40001, 0x000030f5, 0x00010000, 0x80240000,
        0x00000000, 0x0d000000, 0xc0000000, 0x000b0000,
        0x00a00000, 0xa05f2045, 0x30f00400, 0x0000a00f,
        0x30f21400, 0x0000a02f, 0x80420000, 0x002054b0,
        0x5f90a05f, 0x20455015, 0x08000000, 0x50250c00,
        0x00006300, 0x62227378, 0x00000050, 0x61000000,
        0x00606030, 0xf3040000, 0x00603130, 0xf3ffffff,
        0xff603274, 0x5b000000, 0x2045b05f, 0x90000000,
    };
    for (int i = 0; 32 > i; i++)
        an_int_bytes_big(program[i], state->memory + (i * 4));

    return 1;
}

void state_run(STATE *state, STATE *state_original)
{
    // Nothing passed?
    if (NULL == state)
        return;

    // No memory?
    if (0 >= state->memory_size)
        return;

    // Start at the beginning
    state->status = AOK;
    state->pc = 0;
    state->step = 0;

    // While there is no error
    while (AOK == state->status)
    {
        // Invalid PC address?
        if ((0 > state->pc) || (state->memory_size - 6 <= state->pc))
        {
            state->status = ADR;
            return;
        }

        // Increase step counter
        state->step++;

        // Get instruction and function
        unsigned char insfn = state->memory[state->pc];
        unsigned char ins = (insfn >> 4) & 0xF;
        unsigned char fn = insfn & 0xF;

        // Get arguments ready
        unsigned char rArB = state->memory[state->pc + 1];
        unsigned char rA = (rArB >> 4) & 0xF;
        unsigned char rB = rArB & 0xF;
        unsigned int dest = 0;
        an_bytes_int(state->memory + state->pc + 1, &dest);
        unsigned int val = 0;
        an_bytes_int(state->memory + state->pc + 2, &val);

        // Temp variables
        int pos;
        BOOL condition;
        unsigned int temp;

        // PC step size, default is 6
        int pc_step = 6;

        // Handle instruction
        switch(ins)
        {
            case 0: // halt
                // Invalid condition?
                if (0 != fn)
                {
                    state->status = INS;
                    return;
                }

                state->status = HLT;
                return;

            case 1: // nop
                // Invalid condition?
                if (0 != fn)
                {
                    state->status = INS;
                    return;
                }

                pc_step = 1;
                break;

            case 2: // rrmovl or cmovXX
                // Invalid registers?
                if ((0 > rA) || (REGISTER_COUNT <= rA) || (0 > rB) || (REGISTER_COUNT <= rB))
                {
                    state->status = INS;
                    return;
                }

                // Check condition based on flags
                // https://en.wikibooks.org/wiki/X86_Assembly/Control_Flow#Jump_Instructions
                switch (fn)
                {
                    case 0: // rrmovel
                        condition = 1;
                        break;
                    case 1: // cmovle
                        condition = (0 != state->codes.ZF) || (state->codes.SF != state->codes.OF);
                        break;
                    case 2: // cmovl
                        condition = (state->codes.SF != state->codes.OF);
                        break;
                    case 3: // cmove
                        condition = (0 != state->codes.ZF);
                        break;
                    case 4: // cmovne
                        condition = (0 == state->codes.ZF);
                        break;
                    case 5: // cmovge
                        condition = (0 != state->codes.ZF) || (state->codes.SF == state->codes.OF);
                        break;
                    case 6: // cmovg
                        condition = (0 == state->codes.ZF) && (state->codes.SF == state->codes.OF);
                        break;
                    default:
                        state->status = INS;
                        return;
                }

                // Perform move?
                if (0 != condition)
                    state->registers.ids[rB] = state->registers.ids[rA];

                pc_step = 2;
                break;

            case 3: // irmovl
                // Invalid condition?
                if (0 != fn)
                {
                    state->status = INS;
                    return;
                }

                // Invalid registers?
                if ((REGISTER_NONE != rA) || (0 > rB) || (REGISTER_COUNT <= rB))
                {
                    state->status = INS;
                    return;
                }

                // Perform move
                state->registers.ids[rB] = val;

                pc_step = 6;
                break;

            case 4: // rmmovl
                // Invalid condition?
                if (0 != fn)
                {
                    state->status = INS;
                    return;
                }

                // Invalid registers?
                if ((0 > rA) || (REGISTER_COUNT <= rA) || (0 > rB) || (REGISTER_COUNT <= rB))
                {
                    state->status = INS;
                    return;
                }

                // Memory position
                pos = state->registers.ids[rB] + val;

                // Invalid position?
                if ((0 > pos) || (state->memory_size - 4 <= pos))
                {
                    state->status = ADR;
                    return;
                }

                // Perform move
                an_int_bytes(state->registers.ids[rA], state->memory + pos);

                pc_step = 6;
                break;

            case 5: // mrmovl
                // Invalid condition?
                if (0 != fn)
                {
                    state->status = INS;
                    return;
                }

                // Invalid registers?
                if ((0 > rA) || (REGISTER_COUNT <= rA) || (0 > rB) || (REGISTER_COUNT <= rB))
                {
                    state->status = INS;
                    return;
                }

                // Memory position
                pos = state->registers.ids[rB] + val;

                // Invalid position?
                if ((0 > pos) || (state->memory_size - 4 <= pos))
                {
                    state->status = ADR;
                    return;
                }

                // Perform move
                an_bytes_int(state->memory + pos, state->registers.ids + rA);

                pc_step = 6;
                break;

            case 6: // OPl
                // Invalid registers?
                if ((0 > rA) || (REGISTER_COUNT <= rA) || (0 > rB) || (REGISTER_COUNT <= rB))
                {
                    state->status = INS;
                    return;
                }

                // Perform operation
                switch(fn)
                {
                    case 0: // addl
                        temp = state->registers.ids[rB] + state->registers.ids[rA];
                        state->codes.ZF = (0 == temp);
                        state->codes.SF = an_sign(temp);
                        state->codes.OF = ((1 == an_sign(state->registers.ids[rB])) && (0 == an_sign(temp)));
                        state->registers.ids[rB] = temp;
                        break;
                    case 1: // subl
                        temp = state->registers.ids[rB] - state->registers.ids[rA];
                        state->codes.ZF = (0 == temp);
                        state->codes.SF = an_sign(temp);
                        state->codes.OF = ((0 == an_sign(state->registers.ids[rB])) && (1 == an_sign(temp)));
                        state->registers.ids[rB] = temp;
                        break;
                    case 2: // andl
                        temp = state->registers.ids[rB] & state->registers.ids[rA];
                        state->codes.ZF = (0 == temp);
                        state->codes.SF = an_sign(temp);
                        state->codes.OF = 0;
                        state->registers.ids[rB] = temp;
                        break;
                    case 3: // xorl
                        temp = state->registers.ids[rB] ^ state->registers.ids[rA];
                        state->codes.ZF = (0 == temp);
                        state->codes.SF = an_sign(temp);
                        state->codes.OF = 0;
                        state->registers.ids[rB] = temp;
                        break;
                    default:
                        state->status = INS;
                        return;
                }

                pc_step = 2;
                break;

            case 7: // jXX
                // Check condition based on flags
                // https://en.wikibooks.org/wiki/X86_Assembly/Control_Flow#Jump_Instructions
                switch (fn)
                {
                    case 0: // jmp
                        condition = 1;
                        break;
                    case 1: // jle
                        condition = (0 != state->codes.ZF) || (state->codes.SF != state->codes.OF);
                        break;
                    case 2: // jl
                        condition = (state->codes.SF != state->codes.OF);
                        break;
                    case 3: // je
                        condition = (0 != state->codes.ZF);
                        break;
                    case 4: // jne
                        condition = (0 == state->codes.ZF);
                        break;
                    case 5: // jge
                        condition = (0 != state->codes.ZF) || (state->codes.SF == state->codes.OF);
                        break;
                    case 6: // jg
                        condition = (0 == state->codes.ZF) && (state->codes.SF == state->codes.OF);
                        break;
                    default:
                        state->status = INS;
                        return;
                }

                // Invalid address?
                if ((0 > dest) || (state->memory_size - 6 <= dest))
                {
                    state->status = ADR;
                    return;
                }

                // Perform move?
                if (0 != condition)
                {
                    state->pc = dest;
                    pc_step = 0;
                }
                else
                    pc_step = 5;

                break;

            case 8: // call
                // Invalid condition?
                if (0 != fn)
                {
                    state->status = INS;
                    return;
                }

                // Invalid address?
                if ((0 > dest) || (state->memory_size - 6 <= dest))
                {
                    state->status = ADR;
                    return;
                }

                // Try to push address to return to
                if (0 == state_push(state, state->pc + 5))
                {
                    state->status = ADR;
                    return;
                }

                // Move
                state->pc = dest;

                pc_step = 0;
                break;

            case 9: // ret
                // Invalid condition?
                if (0 != fn)
                {
                    state->status = INS;
                    return;
                }

                // Try to pop address to return to
                if (0 == state_pop(state, &pos))
                {
                    state->status = ADR;
                    return;
                }

                // Invalid address?
                if ((0 > pos) || (state->memory_size - 6 <= pos))
                {
                    state->status = ADR;
                    return;
                }

                // Move
                state->pc = pos;

                pc_step = 0;
                break;

            case 10: // pushl
                // Invalid condition?
                if (0 != fn)
                {
                    state->status = INS;
                    return;
                }

                // Invalid register?
                if ((0 > rA) || (REGISTER_COUNT <= rA) || (REGISTER_NONE != rB))
                {
                    state->status = INS;
                    return;
                }

                // Try to push value
                if (0 == state_push(state, state->registers.ids[rA]))
                {
                    state->status = ADR;
                    return;
                }

                pc_step = 2;
                break;

            case 11: // popl
                // Invalid condition?
                if (0 != fn)
                {
                    state->status = INS;
                    return;
                }

                // Invalid register?
                if ((0 > rA) || (REGISTER_COUNT <= rA) || (REGISTER_NONE != rB))
                {
                    state->status = INS;
                    return;
                }

                // Try to pop value
                if (0 == state_pop(state, state->registers.ids + rA))
                {
                    state->status = ADR;
                    return;
                }

                pc_step = 2;
                break;

            case 12: // iOPl
                // Invalid registers?
                if ((REGISTER_NONE != rA) || (0 > rB) || (REGISTER_COUNT <= rB))
                {
                    state->status = INS;
                    return;
                }

                // Perform operation
                switch(fn)
                {
                    case 0: // addl
                        temp = state->registers.ids[rB] + val;
                        state->codes.ZF = (0 == temp);
                        state->codes.SF = an_sign(temp);
                        state->codes.OF = ((1 == an_sign(state->registers.ids[rB])) && (0 == an_sign(temp)));
                        state->registers.ids[rB] = temp;
                        break;
                    case 1: // subl
                        temp = state->registers.ids[rB] - val;
                        state->codes.ZF = (0 == temp);
                        state->codes.SF = an_sign(temp);
                        state->codes.OF = ((0 == an_sign(state->registers.ids[rB])) && (1 == an_sign(temp)));
                        state->registers.ids[rB] = temp;
                        break;
                    case 2: // andl
                        temp = state->registers.ids[rB] & val;
                        state->codes.ZF = (0 == temp);
                        state->codes.SF = an_sign(temp);
                        state->codes.OF = 0;
                        state->registers.ids[rB] = temp;
                        break;
                    case 3: // xorl
                        temp = state->registers.ids[rB] ^ val;
                        state->codes.ZF = (0 == temp);
                        state->codes.SF = an_sign(temp);
                        state->codes.OF = 0;
                        state->registers.ids[rB] = temp;
                        break;
                    default:
                        state->status = INS;
                        return;
                }

                pc_step = 6;
                break;
            
            // TODO: Extra functions, such as enter (kinda) and leave

            default:
                state->status = INS;
                return;
        }

        // Increment PC
        state->pc += pc_step;
    }
}

BOOL state_clone(STATE *state_from, STATE *state_to)
{
    // Nothing passed?
    if ((NULL == state_from) || (NULL == state_to))
        return 0;
    
    state_to->registers = state_from->registers;
    state_to->codes = state_from->codes;
    state_to->status = state_from->status;

    // Try to copy memory if present
    if (0 < state_from->memory_size)
    {
        state_to->memory = malloc(state_from->memory_size);
        if (NULL == state_to->memory)
            return 0;
        memcpy(state_to->memory, state_from->memory, state_from->memory_size);
    }

    state_to->memory_size = state_from->memory_size;
    state_to->pc = state_from->pc;
    state_to->step = state_from->step;

    return 1;
}

void state_changes(STATE *state_old, STATE *state_now)
{
    // Nothing passed?
    if ((NULL == state_old) || (NULL == state_now))
        return;

    printf("Stopped in %d steps at PC = 0x%x.", state_now->step, state_now->pc);
    
    const char* st_names[STATUS_COUNT] = STATUS_NAME_ARRAY;
    BOOL st_valid = (_FIRST > state_now->status) || (_LAST < state_now->status);
    const char* st_str = st_valid ? "???" : st_names[state_now->status - _FIRST];
    printf("  Status '%s', CC Z=%d S=%d O=%d\n", st_str, state_now->codes.ZF, state_now->codes.SF, state_now->codes.OF);

    const char* reg_names[REGISTER_COUNT] = REGISTER_NAME_ARRAY;
    printf("Changes to registers:\n");
    for (int i = 0; REGISTER_COUNT > i; i++)
        if (state_old->registers.ids[i] != state_now->registers.ids[i])
            printf("%%%3s:   0x%08x      0x%08x\n", reg_names[i], state_old->registers.ids[i], state_now->registers.ids[i]);
    printf("\n");

    printf("Changes to memory:\n");
    for (int i = 0; state_now->memory_size / 4 > i; i++)
    {
        BOOL had_diff = 0;
        for (int j = i * 4; (i + 1) * 4 > j; j++)
            had_diff = had_diff || (state_old->memory[j] != state_now->memory[j]);
        if (0 != had_diff)
        {
            printf("0x%04x: 0x", i * 4);
            for (int j = (i + 1) * 4 - 1; i * 4 <= j; j--)
                printf("%02x", state_old->memory[j]);
            printf("      0x");
            for (int j = (i + 1) * 4 - 1; i * 4 <= j; j--)
                printf("%02x", state_now->memory[j]);
            printf("\n");
        }
    }
}

BOOL state_push(STATE *state, unsigned int val)
{
    // Nothing passed?
    if (NULL == state)
        return 0;
    
    // No memory?
    if (0 >= state->memory_size)
        return 0;
    
    // esp is valid position?
    int esp = state->registers.names.esp;
    if ((4 > esp) || (state->memory_size <= esp))
        return 0;
    
    // Subtract 4
    state->registers.names.esp -= 4;

    // Set value
    an_int_bytes(val, state->memory + state->registers.names.esp);
    
    return 1;
}

BOOL state_pop(STATE *state, unsigned int *val)
{
    // Nothing passed?
    if (NULL == state)
        return 0;
    
    // No memory?
    if (0 >= state->memory_size)
        return 0;
    
    // esp is valid position?
    int esp = state->registers.names.esp;
    if ((0 > esp) || (state->memory_size - 4 <= esp))
        return 0;

    // Get value
    an_bytes_int(state->memory + state->registers.names.esp, val);
    
    // Add 4
    state->registers.names.esp += 4;
    
    return 1;
}
