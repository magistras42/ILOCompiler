/**
 * Function Declarations and Data Structures for Lab 1.
 */

#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "scanparse.h"

#ifndef ALLOC_H_INCLUDED
#define ALLOC_H_INCLUDED

/* CONSTANTS */

#define INVALID 65
#define ISREMAT 66
#define ISSPILLED 67
#define ISCLEAN 68

/* MACROS */

/* STRUCTS */

typedef struct {
    uint32_t total_lines; 
    uint32_t index;
    uint32_t max_pr;        // number of unreserved registers
    uint32_t spill_adr;
    uint32_t pr_count;      // actual number of physical registers
    uint32_t pr_stack_size;   
    uint32_t* pr_to_vr;
    uint32_t* prnu;
    uint32_t* pr_stack;
    uint32_t* vr_to_pr;
    uint32_t* vr_to_spill;
    uint32_t* vr_to_def;
    StateIR* ir;
    struct IRLine* cur;
} Alloc_State;

/* INTERFACE FOR RENAMING AND ALLOCATION FUNCTIONS */

// Register Renaming Functions
uint32_t largest_sr(StateIR* ir);
int rename_reg(StateIR* ir);

// Register Allocation Functions
void reallocate(Alloc_State* ctx, uint32_t max_vr);
uint32_t get_a_pr(Alloc_State* ctx, uint32_t vr, uint32_t nu, uint32_t marked);
void free_a_pr(Alloc_State* ctx, uint32_t pr);
void spill(Alloc_State* ctx, uint32_t pr);
void restore(Alloc_State* ctx, uint32_t vr, uint32_t nu);
void remat(Alloc_State* ctx, uint32_t vr, uint32_t pr);

/* ILOC IR REPRESENTATION CODE */
void print_vr(StateIR* ir);
void print_pr(StateIR* ir);

/* ALLOC_STATE MAINTENANCE CODE */
int init_alloc(Alloc_State* ctx, StateIR* ir, uint32_t max_vr, uint32_t max_pr);

/* STACK CODE???? OR DO POINTER THING */

#endif
