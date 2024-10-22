#include "alloc.h"

// List of all possible lexemmes
const char* LEXL[] = {
    "load", "store",
    "loadI",
    ",",
    "",
    "add", "sub", "mult", "lshift", "rshift",
    "output",
    "nop",
    "0",
    "0",
    "=>",
    "\\n",
    "invalid lexemme detected.", "overflows 2^31 - 1 limit.", "Invalid opcode.", "Invalid sentence.", "Too many ops in sentence."
};

uint32_t max_live = 0;

/* Function to format printing an IR Block Nicely(TM)*/
void print_vr(StateIR* ir) {
    uint32_t total = ir->groups[1].line_count;
    struct IRLine* line = ir->groups[1].oldest;
    for (uint32_t i = 0; i < total; ++i) {
        // Add code that reads each of the ops and converts as needed.
        switch(line->opcode) {
            case nop:
                printf("%s\n", LEXL[line->opcode]);
                break;
            case output:
                printf("%s\t %d \n", LEXL[line->opcode], line->op1.sr);
                break;
            case loadI:
                printf("%s\t %d => r%d \n", LEXL[line->opcode], line->op1.sr, line->op3.vr);
                break;
            case store:
            case load:
                printf("%s\tr%d => r%d \n", LEXL[line->opcode], line->op1.vr, line->op3.vr);
                break;
            case add:
            case sub:
            case mult:
            case lshift:
            case rshift:
                printf("%s\t r%d , r%d => r%d \n", LEXL[line->opcode], line->op1.vr, line->op2.vr, line->op3.vr);
                break;
        }
        line = line->next;
    }
}

/* Function to format printing an IR Block Nicely(TM)*/
void print_pr(StateIR* ir) {
    uint32_t total = ir->groups[1].line_count;
    struct IRLine* line = ir->groups[1].oldest;
    for (uint32_t i = 0; i < total; ++i) {
        // Add code that reads each of the ops and converts as needed.
        switch(line->opcode) {
            case nop:
                // printf("%s\n", LEXL[line->opcode]);
                break;
            case output:
                printf("%s\t %d \n", LEXL[line->opcode], line->op1.sr);
                break;
            case loadI:
                printf("%s\t %d => r%d \n", LEXL[line->opcode], line->op1.sr, line->op3.pr);
                break;
            case store:
            case load:
                printf("%s\tr%d => r%d \n", LEXL[line->opcode], line->op1.pr, line->op3.pr);
                break;
            case add:
            case sub:
            case mult:
            case lshift:
            case rshift:
                printf("%s\t r%d , r%d => r%d \n", LEXL[line->opcode], line->op1.pr, line->op2.pr, line->op3.pr);
                break;
        }
        line = line->next;
    }
}

/* Find the largest sr in the ILOC Intermediate Representation. */
uint32_t largest_sr(StateIR* ir) {
        uint32_t max_sr = 0;

        // Loop through IR.
        uint32_t total = ir->groups[1].line_count;
        struct IRLine* line = ir->groups[1].oldest;
        for (uint32_t i = 0; i < total; ++i) {
                // Add code that reads each of the ops and converts as needed.
                switch(line->opcode) {
                case nop:
                        break;
                case output:
                        break;
                case loadI:
                        if (line->op3.sr > max_sr) {
                                max_sr = line->op3.sr;
                        }
                        break;
                case store:
                case load:
                        if (line->op1.sr > max_sr) {
                                max_sr = line->op1.sr;
                        }
                        if (line->op3.sr > max_sr) {
                                max_sr = line->op3.sr;
                        }

                        
                        break;
                case add:
                case sub:
                case mult:
                case lshift:
                case rshift:
                        if (line->op1.sr > max_sr) {
                                max_sr = line->op1.sr;
                        }
                        if (line->op2.sr > max_sr) {
                                max_sr = line->op2.sr;
                        }
                        if (line->op3.sr > max_sr) {
                                max_sr = line->op3.sr;
                        }
                        break;
                }
                line = line->next;
        }

        return max_sr;
}


/* Rename all sr -> vr as presented in Lab2 Video. Return max_vr name 
 * if completed without error, -1 otherwise.
 */
int rename_reg(StateIR* ir) {
        uint32_t vr_name = 0;
        uint32_t max_sr = largest_sr(ir);

        // Allocate SRToVR and LU arrays.
        uint64_t arr_size = ((uint64_t) (largest_sr + 1) * sizeof(uint32_t));
        uint32_t* sr_to_vr = (uint32_t *) malloc(arr_size);
        uint32_t* last_updated = (uint32_t *) malloc(arr_size);

        // Make sure allocated properly.
        if ((sr_to_vr == NULL) || (last_updated == NULL)) {
                return -1;
        }

        // Initialize last_updated to 2^32 - 1 as "invalid"
        for (int i = 0; i <= max_sr; ++i) {
                sr_to_vr[i] = UINT32_MAX;
                last_updated[i] = UINT32_MAX;
        }

        // For each Op in block, bottom to top
        uint32_t index = ir->groups[1].line_count;
        struct IRLine* line = ir->groups[1].newest;

        uint32_t cur_live = 0;
        // should it be index - 1, i >= 0?
        for (int i = index; i > 0; i--) {
                op* o;
                op* o1;
                op* o2;
                op* o3;
                // For each operand O that line defines
                        // if srToVr[O.sr] = MAXINT (unused def)
                                // then srToVr[O.sr] = vrname++
                        // O.vr = srToVr[O.sr]
                        // O.nu = last_updated[O.sr]
                        // srToVr[O.sr] = MAXINT (kill op3)
                        // last_updated[O.sr] = MAXINT
                switch (line->opcode) {
                        case nop:
                        case output:
                        case store:
                                break;
                        case loadI:
                        case load:
                        case add:
                        case sub:
                        case mult:
                        case lshift:
                        case rshift:
                                // TODO - all in op3 1 def
                                o = &(line->op3);
                                if (sr_to_vr[o->sr] == UINT32_MAX) {
                                        sr_to_vr[o->sr] = vr_name++;
                                }
                                o->vr = sr_to_vr[o->sr];
                                o->nu = last_updated[o->sr];

                                sr_to_vr[o->sr] = UINT32_MAX;
                                last_updated[o->sr] = UINT32_MAX;
                                break;
                }
                
                // For each operand O that line uses
                        // if srToVr[O.sr] = MAXINT (last use)
                                // then srToVr[O.sr] = VRname++
                        // O.vr = srToVr[O.sr]
                        // O.nu = last_updated[O.sr]
                switch (line->opcode) {
                        case nop:
                        case output:
                        case loadI:
                                break;
                        case load:
                                // op1
                                o1 = &(line->op1);
                                if (sr_to_vr[o1->sr] == UINT32_MAX) {
                                        sr_to_vr[o1->sr] = vr_name++;
                                }
                                o1->vr = sr_to_vr[o1->sr];
                                o1->nu = last_updated[o1->sr];
                                last_updated[o1->sr] = i;
                                break;
                        case store:
                                // op1
                                o1 = &(line->op1);
                                if (sr_to_vr[o1->sr] == UINT32_MAX) {
                                        sr_to_vr[o1->sr] = vr_name++;
                                }
                                o1->vr = sr_to_vr[o1->sr];
                                o1->nu = last_updated[o1->sr];
                                last_updated[o1->sr] = i;
                                // op3
                                o3 = &(line->op3);
                                if (sr_to_vr[o3->sr] == UINT32_MAX) {
                                        sr_to_vr[o3->sr] = vr_name++;
                                }
                                o3->vr = sr_to_vr[o3->sr];
                                o3->nu = last_updated[o3->sr];
                                last_updated[o3->sr] = i;
                                break;
                        case add:
                        case sub:
                        case mult:
                        case lshift:
                        case rshift:
                                // op1
                                o1 = &(line->op1);
                                if (sr_to_vr[o1->sr] == UINT32_MAX) {
                                        sr_to_vr[o1->sr] = vr_name++;
                                }
                                o1->vr = sr_to_vr[o1->sr];
                                o1->nu = last_updated[o1->sr];
                                last_updated[o1->sr] = i;
                                // op2
                                o2 = &(line->op2);
                                if (sr_to_vr[o2->sr] == UINT32_MAX) {
                                        sr_to_vr[o2->sr] = vr_name++;
                                }
                                o2->vr = sr_to_vr[o2->sr];
                                o2->nu = last_updated[o2->sr];
                                last_updated[o2->sr] = i;
                                break;
                }

                // For each operand O that line uses
                        // last_updated[O.sr] = index (done above)

                // Calculate number of live registers
                cur_live = 0;
                for (int i = 0; i <= max_sr; ++i) {
                        if (sr_to_vr[i] != UINT32_MAX) {
                                cur_live++;
                        }
                }

                // Update max_live
                if (cur_live > max_live) {
                        max_live = cur_live;
                }

                line = line->prev;
        }

        // Clean up
        free(sr_to_vr);
        free(last_updated);

        return vr_name;
}

/**
 * Allocate entire context for 412alloc at once.
 */
int init_alloc(Alloc_State* ctx, StateIR* ir, uint32_t max_vr, uint32_t max_pr) {
        int status = 0;

        // Initialization of all but maps
        ctx->total_lines = ir->groups[1].line_count;
        ctx->index = 0;
        ctx->spill_adr = 32768;
        ctx->pr_count = max_pr;
        ctx->ir = ir;
        ctx->cur = ir->groups[1].oldest;

        // Check if there will be spills, reserve the last register if needed.
        if (max_live <= ctx->pr_count) {
                ctx->pr_stack_size = max_pr;
                ctx->max_pr = max_pr;
        } else {
                ctx->pr_stack_size = max_pr - 1;
                ctx->max_pr = max_pr - 1;
        }

        // Allocate pr_to_vr, prnu, pr_stack
        uint64_t pr_memory = (sizeof(uint32_t) * max_pr);
        ctx->pr_to_vr = (uint32_t *) malloc(pr_memory);
        ctx->pr_stack = (uint32_t *) malloc(pr_memory);
        ctx->prnu = (uint32_t *) malloc(pr_memory);

        // Allocate vr_to_pr, vr_to_spill, vr_to_def
        uint64_t vr_memory = (sizeof(uint32_t) * max_vr);
        ctx->vr_to_pr = (uint32_t *) malloc(vr_memory);
        ctx->vr_to_spill = (uint32_t *) malloc(vr_memory);
        ctx->vr_to_def = (uint32_t *) malloc(vr_memory);

        // Memory safety
        if (ctx->pr_stack == NULL || ctx->pr_to_vr == NULL || ctx->prnu == NULL || ctx->vr_to_pr == NULL || ctx->vr_to_spill == NULL) {
                return -1;
        }

        // Initialize pr-indexed maps
        for (int i = 0; i < ctx->max_pr; i++) {
                ctx->pr_to_vr[i] = UINT32_MAX;
                ctx->prnu[i] = UINT32_MAX;
                ctx->pr_stack[i] = ctx->max_pr - 1 - i;
        }

        // Initialize vr-indexed maps
        for (int i = 0; i < max_vr; i++) {
                ctx->vr_to_pr[i] = UINT32_MAX;
                // TODO: Should this just be INVALID???
                ctx->vr_to_spill[i] = 0;
                ctx->vr_to_def[i] = 0;
        }
        return status;
}

/**
 * Reallocate virtual registers to physical registers.
 */
void reallocate(Alloc_State* ctx, uint32_t max_vr) {
        // In main, call init_alloc
        uint32_t total = ctx->total_lines;
        struct IRLine* line;
        struct IRLine* temp;

        while (ctx->index < total) {
                op* o1;
                op* o2;
                op* o3;
                uint32_t pr1;
                uint32_t pr2;
                uint32_t pr3;
                line = ctx->cur;

                // Skip output and nops
                if ((line->opcode == nop) || (line->opcode == output)) {
                        // Next ILOC op
                        // for output, double check the print to make sure doing right print
                        ctx->index++;
                        ctx->cur = line->next;
                        
                        continue;
                }

                // op1
                if (line->opcode != loadI) {
                        o1 = &(line->op1);
                        pr1 = ctx->vr_to_pr[o1->vr];
                        // If pr invalid, restore
                        if (pr1 == UINT32_MAX) {
                                o1->pr = get_a_pr(ctx, o1->vr, o1->nu, INVALID);
                                if (ctx->vr_to_spill[o1->vr] >= 32768) {
                                        restore(ctx, o1->vr, o1->pr);
                                } else if (ctx->vr_to_spill[o1->vr] == INVALID) {
                                        remat(ctx, o1->vr, o1->pr);
                                } else {
                                        // fprintf(stderr, "ERROR: o1 VR_TO_SPILL problem, something wrong with spill.\n");
                                }
                        // Otherwise, update o1's pr
                        } else {
                                o1->pr = pr1;
                                ctx->prnu[pr1] = o1->nu;
                        }

                        // op2 and op3
                        if (line->opcode != store) {
                                // op2
                                if (line->opcode != load) {                               
                                        o2 = &(line->op2);
                                        pr2 = ctx->vr_to_pr[o2->vr];
                                        // If pr invalid, restore
                                        if (pr2 == UINT32_MAX) {
                                                o2->pr = get_a_pr(ctx, o2->vr, o2->nu, o1->pr);
                                                if (ctx->vr_to_spill[o2->vr] >= 32768) {
                                                        restore(ctx, o2->vr, o2->pr);
                                                } else if (ctx->vr_to_spill[o2->vr] == INVALID) {
                                                        remat(ctx, o2->vr, o2->pr);
                                                } else {
                                                        // fprintf(stderr, "ERROR: o2 VR_TO_SPILL problem, something wrong with spill.\n");
                                                }
                                        // Otherwise, update o2's pr
                                        } else {
                                                o2->pr = pr2;
                                                ctx->prnu[pr2] = o2->nu;
                                        }
                                }
                        } else {
                                // op3
                                o3 = &(line->op3);
                                pr3 = ctx->vr_to_pr[o3->vr];
                                // If pr invalid, restore
                                if (pr3 == UINT32_MAX) {
                                        o3->pr = get_a_pr(ctx, o3->vr, o3->nu, o1->pr);
                                        if (ctx->vr_to_spill[o3->vr] >= 32768) {
                                                restore(ctx, o3->vr, o3->pr);
                                        } else if (ctx->vr_to_spill[o3->vr] == INVALID) {
                                                remat(ctx, o3->vr, o3->pr);
                                        } else {
                                        // fprintf(stderr, "ERROR: o3 VR_TO_SPILL problem, something wrong with spill.\n");
                                        }
                                // Otherwise, update o3's pr
                                } else {
                                        o3->pr = pr3;
                                        ctx->prnu[pr3] = o3->nu;
                                }
                        }
                }

                // Check last uses for op1 and op2
                if (line->opcode != loadI) {
                        o1 = &(line->op1);
                        // If last use and no vr mapped to op1 pr, free.
                        if ((o1->nu == UINT32_MAX) && (ctx->pr_to_vr[o1->pr] != UINT32_MAX)) {
                                free_a_pr(ctx, o1->pr);
                        }
                }

                if (line->opcode > loadI) {
                        o2 = &(line->op2);
                        // If last use and no vr mapped to op2 pr, free.
                        if ((o2->nu == UINT32_MAX) && (ctx->pr_to_vr[o2->pr] != UINT32_MAX)) {
                                free_a_pr(ctx, o2->pr);
                        }
                }

                // Check last use for store on op3
                o3 = &(line->op3);
                // Check last use for store
                if (line->opcode == store) {
                        // If last use and no vr mapped to op3's pr, free
                        if ((o3->nu == UINT32_MAX) && (ctx->pr_to_vr[o3->pr] != UINT32_MAX)) {
                                free_a_pr(ctx, o3->pr);
                        }
                // Otherwise, allocate def for op3
                } else {
                        // loadI defs deferred until first use
                        if (line->opcode == loadI) {
                                ctx->vr_to_spill[o3->vr] = INVALID;
                                ctx->vr_to_def[o3->vr] = line->op1.sr;
                                temp = line->next;
                                remove_line_new(&(ctx->ir->groups[1]), ctx->cur);
                                add_line(ctx->ir->groups, ctx->cur);
                                ctx->index++;
                                ctx->cur = temp;
                                continue;
                        } else {
                                o3->pr = get_a_pr(ctx, o3->vr, o3->nu, INVALID);
                        }
                }

                // Next ILOC op
                ctx->index++;
                ctx->cur = line->next;
        }
}

/**
 * Get physical register for given virtual register vr.
 * Based on the next use of the vr, nu. Returns the
 * identity of the physical register found.
 */
uint32_t get_a_pr(Alloc_State* ctx, uint32_t vr, uint32_t nu, uint32_t marked) {
        uint32_t new_pr = UINT32_MAX;

        // Check for available pr's
        if (ctx->pr_stack_size > 0) {
                ctx->pr_stack_size--;
                new_pr = ctx->pr_stack[ctx->pr_stack_size];
        } else {
                uint32_t max_nu = UINT32_MAX;
                uint32_t max_pr = UINT32_MAX;
                for (int i = 0; i < ctx->max_pr; ++i) {
                        // ignore marked pr
                        if (i == marked) {
                                continue;
                        }

                        // so if invalid overflows to 0 :)
                        // also check that i not equal to the marked thing
                        // if prtonu[i]  + 1 >= max_nu + 1 && i != markedPr
                        if ((ctx->prnu[i] + 1) >= max_nu + 1) {
                                max_nu = ctx->prnu[i];
                                max_pr = i;
                        }
                }
                // Spill max_pr and update references if valid
                uint32_t temp = ctx->pr_to_vr[max_pr];
                if ((ctx->vr_to_spill[temp] < 32768) && (ctx->vr_to_spill[temp] != INVALID)) {
                        spill(ctx, max_pr);
                }
                // uint32_t temp = ctx->pr_to_vr[max_pr];
                ctx->vr_to_pr[temp] = UINT32_MAX;
                if ((ctx->vr_to_spill[temp] < 32768) && (ctx->vr_to_spill[temp] != INVALID)) {
                        ctx->vr_to_spill[temp] = ctx->spill_adr;
                        // Allignment for new spill address
                        ctx->spill_adr += 4;
                }
                new_pr = max_pr;
        }
        ctx->vr_to_pr[vr] = new_pr;
        ctx->pr_to_vr[new_pr] = vr;
        ctx->prnu[new_pr] = nu;
        return new_pr;
}

/**
 * Free physical register pr when not needed.
 */
void free_a_pr(Alloc_State* ctx, uint32_t pr) {
        uint32_t vr = ctx->pr_to_vr[pr];
        ctx->vr_to_pr[vr] = UINT32_MAX;
        ctx->pr_to_vr[pr] = UINT32_MAX;
        ctx->prnu[pr] = 0;
        ctx->pr_stack[ctx->pr_stack_size] = pr;
        ctx->pr_stack_size++;
}

/**
 * Spill contents of physical register pr into memory.
 */
void spill(Alloc_State* ctx, uint32_t pr) {
        DummyHead* rep = &(ctx->ir->groups[1]);
        struct IRLine* target = ctx->cur->prev;

        // Create a loadI for spill location address into reserved register.
        struct IRLine* new_load = get_next_IR(ctx->ir);
        remove_line(ctx->ir->groups);
        new_load->opcode = loadI;
        new_load->op1.sr = ctx->spill_adr;
        new_load->op1.vr = ctx->spill_adr;
        new_load->op1.pr = ctx->spill_adr;
        new_load->op1.nu = UINT32_MAX;
        new_load->op3.pr = ctx->pr_count - 1;
        new_load->op3.nu = UINT32_MAX;
        new_load->op3.nu = ctx->index;
        add_line_after(rep, target, new_load);

        // Create store for moving spilled value from pr to spill location.
        struct IRLine* new_store = get_next_IR(ctx->ir);
        remove_line(ctx->ir->groups);
        new_store->opcode = store;
        new_store->op1.vr = ctx->pr_to_vr[pr];
        new_store->op1.pr = pr;
        new_store->op1.nu = UINT32_MAX;
        new_store->op3.pr = ctx->pr_count - 1;
        new_store->op3.nu = UINT32_MAX;
        add_line_after(rep, new_load, new_store);
}

/**
 * Restore contents of physical register from memory to
 * virtual address vr with next use nu.
 */
void restore(Alloc_State* ctx, uint32_t vr, uint32_t nu) {
        DummyHead* rep = &(ctx->ir->groups[1]);
        struct IRLine* target = ctx->cur->prev;

        // Create a loadI to put spill location address into reserved register.
        struct IRLine* new_loadI = get_next_IR(ctx->ir);
        remove_line(ctx->ir->groups);
        new_loadI->opcode = loadI;
        new_loadI->op1.sr = ctx->vr_to_spill[vr];
        new_loadI->op1.vr = ctx->vr_to_spill[vr];
        new_loadI->op1.pr = ctx->vr_to_spill[vr];
        new_loadI->op3.pr = ctx->pr_count - 1;
        new_loadI->op3.nu = ctx->index;
        add_line_after(rep, target, new_loadI);

        // Create load for moving spilled value from spill location to pr.
        struct IRLine* new_load = get_next_IR(ctx->ir);
        remove_line(ctx->ir->groups);
        new_load->opcode = load;
        new_load->op1.pr = ctx->pr_count - 1;
        new_load->op3.vr = vr;
        new_load->op3.pr = ctx->vr_to_pr[vr];
        new_load->op3.nu = ctx->index;
        add_line_after(rep, new_loadI, new_load);
}

void remat(Alloc_State* ctx, uint32_t vr, uint32_t pr) {
        DummyHead* rep = &(ctx->ir->groups[1]);
        struct IRLine* target;
        target = ctx->cur->prev;

        uint32_t val = ctx->vr_to_def[vr];

        // Create a loadI to put spill location address into reserved register.
        struct IRLine* new_loadI = get_next_IR(ctx->ir);
        remove_line(ctx->ir->groups);
        new_loadI->opcode = loadI;
        new_loadI->op1.sr = val;
        new_loadI->op1.vr = val;
        new_loadI->op1.pr = val;
        new_loadI->op3.pr = pr;
        new_loadI->op3.nu = ctx->index;

        if (target == (struct IRLine *) rep) {
                target = ctx->cur;
                new_loadI->next = target;
                target->prev = new_loadI;
                new_loadI->prev = (struct IRLine *) rep;
                rep->oldest = new_loadI;
                rep->line_count++;
        } else {
                add_line_after(rep, target, new_loadI);
        }
}
