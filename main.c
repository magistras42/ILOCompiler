/**
 * COMP 412 Lab 2 Driver Code
 */

#include "alloc.h"

void print_help() {
    printf("COMP Reference Allocator (Lab 2)\n");
    printf("Command Syntax:\n");
    printf("\t412alloc [flags] filename \n");
    printf("\n");
    printf("Required Arguments:\n");
    printf("\tk\t\tspecifies the number of registers available to the allocator and is in the range 3 <= k <= 64\n");
    printf("\tfilename\tis the path (absolute or relative) to the input file\n");
    printf("\n");
    printf("Optional Flags:\n");
    printf("\t-h\tprints this message\n");
    printf("\t-x\trenames registers and prints ILOC IR to command line\n");
    printf("\t k\tscans and parses filename ILOC, renames registers, then\n");
    printf("\t  \tallocates renames ILOC code into k physical registers,\n");
    printf("\t  \twhere 3 <= k <= 64\n");
}

int main(int argc, char **argv) {
    // Flags and their values
    uint8_t hflag = 0;
    uint8_t xflag = 0;
    uint8_t kflag = 0;
    uint8_t flags = 0;
    
    int status = 0;
    uint32_t k;
    ctx* context;
    StateIR* ir;
    Alloc_State* all;
    char *filename;

    // Check arguments
    for (int i = 1; i < argc; i+=2) {
        char* cop = argv[i];
        if (cop[0] == '-') {
            
            if (strchr(cop, 'x')) {
                xflag = i;
            }
            if (strchr(cop, 'h')) {
                hflag = i;
            }
        } else if (i == 1 && argc == 2) {
            break;
        } else {
            if (!kflag) {
                k = atoi(cop);
                if (k >= 3 && k <= 64) {
                    kflag = i;
                    continue;
                }
            }
            fprintf(stderr, "ERROR: Bad arguments.\n");
            hflag = 1;
            break;
        }
    }

    filename = argv[argc - 1];

    if (flags > 1) {
        fprintf(stderr, "ERROR: Multiple command-line flags found.\nTry '-h' for information on command-line syntax.\n");
    }

    if ((strlen(filename) < 1) || (argc <= 1)) {
        fprintf(stderr, "ERROR: Missing file path.\n");
        print_help();
        return status;
    }

    // Process Flags
    if (hflag) {
        print_help();
        return(0);
    }

    // Check to open file
    context = (ctx*) malloc(sizeof(ctx));
    if (context == NULL) {
        fprintf(stderr, "ERROR: Failed to initialize global context\n");
        return -1;
    }
    if ((status = init_ctx(context, filename))) {
        fprintf(stderr, "ERROR: Could not open input file.\n");
        print_help();
        return status;
    }

    // Initialize IR Representation
    ir = (StateIR*) malloc(sizeof(StateIR));
    if (ir == NULL) {
        fprintf(stderr, "ERROR: Failed to initialize IR State\n");
        fclose(context->file);
        return -1;
    }
    if ((status = init_IR(ir))) {
        fprintf(stderr, "ERROR: Failed to initialize IR State\n");
        fclose(context->file);
        return status;
    }

    context->ir = ir;

    if (xflag) {
        if ((status = parse(context)) >= 0) {
            if ((status = rename_reg(context->ir)) >= 0) {
                print_vr(context->ir);
                fclose(context->file);
                return 0;
            }
            printf("Failed to allocate intermediate arrays, run terminates.\n");
        }
        printf("Due to the syntax error, run terminates.\n");
        fclose(context->file);
        return status;
    }

    // TODO: code for lab2

    // Initialize Alloc Representation
    all = (Alloc_State*) malloc(sizeof(Alloc_State));
    if (all == NULL) {
        fprintf(stderr, "ERROR: Failed to initialize Alloc State\n");
        fclose(context->file);
        return -1;
    }

    if (kflag) {
        // printf("in kflag\n");
        if ((status = parse(context)) >= 0) {
            int max_vr;
            if ((max_vr = rename_reg(context->ir)) >= 0) {
                // printf("here!\n");
                uint32_t max2 = (uint32_t) max_vr;

                if (!(status = init_alloc(all, context->ir, max2, k))) {
                    reallocate(all, max_vr);
                    print_pr(context->ir);
                    fclose(context->file);
                    return 0;
                }
            }
            printf("Failed to allocate intermediate arrays, run terminates.\n");
        }
        printf("Due to the syntax error, run terminates.\n");
        fclose(context->file);
        return status;
    }

    // Close open file
    fclose(context->file);
    return 0;
}
