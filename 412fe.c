/**
 * COMP 412 Lab 1 Driver Code
 */


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "scanparse.h"

void print_help() {
    printf("COMP 412 ILOC Front End\n");
    printf("Command Syntax:\n");
    printf("\t./412fe [flags] filename\n");
    printf("\n");
    printf("Required Arguments:\n");
    printf("\tfilename is the path (absolute or relative) to the input file\n");
    printf("\n");
    printf("Optional Flags:\n");
    printf("\t-h\tprints this message\n");
    printf("\n");
    printf("At most one of the following three flags:\n");
    printf("\t-s\tprints tokens in token stream\n");
    printf("\t-p\tinvokes parser and reports on success or failure\n");
    printf("\t-r\tprints human readable version of parser's IR\n");
    printf("If none is specified, the default action is '-p'.\n");
}

int main(int argc, char **argv) {
    // Flags and their values
    uint8_t hflag = 0;
    uint8_t rflag = 0;
    uint8_t sflag = 0;
    uint8_t pflag = 0;
    uint8_t flags = 0;

    extern char *optarg;
    extern int optind, optopt;
    
    int status = 0;
    int arg;
    ctx* context;
    StateIR* ir;
    char *filename;

    while ((arg = getopt(argc, argv, ":hrps")) != -1) {
    switch(arg) {
        case 'h':
            hflag++;
            flags++;
            break;
        case 'r':
            rflag++;
            flags++;
            break;
        case 'p':
            pflag++;
            flags++;
            break;
        case 's':
            sflag++;
            flags++;
            break;
        case '?':
            fprintf(stderr, "Unrecognized option: -%c\n", optopt);
            print_help();
            status = -1;
            return status;
        }
    }

    // could check if filename uninitialized, if not then do this?
    filename = argv[argc - 1];

    if (flags > 1) {
        fprintf(stderr, "ERROR: Multiple command-line flags found.\nTry '-h' for information on command-line syntax.\n");
    }

    if ((filename == NULL) && (pflag || rflag || sflag)) {
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

    if (sflag && (flags < 2)) {
        status = scan(context);
        fclose(context->file);
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

    if (rflag) {
        if ((status = parse(context)) >= 0) {
            print_ir(context->ir);
            fclose(context->file);
            return 0;
        }
        printf("Due to the syntax error, run terminates.\n");
        fclose(context->file);
        return status;
    }

    if (pflag || (!rflag)) {
        if ((status = parse(context)) >= 0) {
            fprintf(stderr, "Parse succeeded. Processed %d operations.\n", status);
            fclose(context->file);
            return 0;
        }
        printf("Parse found errors.\n");
        fclose(context->file);
        return status;
    }

    // Close open file
    fclose(context->file);
    return 0;
}
