/**
 * Function Declarations and Data Structures for Lab 1.
 */

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* CONSTANTS */

#define MAX_GROUPS 2
#define N 10
#define BUF_SIZE (2^(N + 1))
#define HALF_SIZE (2^N)
#define BLOCK_SIZE 2000

// TODO: Double check to see this works...
// i will always be positive, j up in the air...
#define MOD2N(i, j) ((j + i) & (N))

#ifndef SCANPARSE_H_INCLUDED
#define SCANPARSE_H_INCLUDED

// Mapping of part of speech to position in POS_NAMES
enum pos {
    MEMOP = 0,
    LOADI,
    COMMA,
    EoF,
    ARITHOP,
    OUTPUT,
    NOP,
    REGISTER,
    CONSTANT,
    INTO,
    EOL,
    ERROR
};

enum lex {
    load,
    store,
    loadI,
    comma,
    eof,
    add,
    sub,
    mult,
    lshift,
    rshift,
    output,
    nop,
    constant,
    reg,
    into,
    eol,
    spelling,
    overflow,
    invalid_op,
    invalid_sentence,
    extra_ops
};


/* STRUCTS */

// Token representing a single word.
typedef struct {
    uint8_t pos;   // part of speech
    uint32_t lex;  // anything in token_names (or const of below 2^31)
} token;

// Register information for a single operand
typedef struct {
    uint32_t sr;
    uint32_t vr;
    uint32_t pr;
    uint32_t nu;
} op;

/*
 * Struct representing a line
 * in the ILOC intermediate representation.
 */
struct IRLine {
    struct IRLine* prev;
    struct IRLine* next;
    uint8_t opcode;
    op op1;
    op op2;
    op op3;    
};

/*
 * Struct representing the head
 * of a double circularly-linked list.
 */
typedef struct {
    struct IRLine* oldest;  
    struct IRLine* newest;  
    uint64_t line_count;
} DummyHead;

typedef struct {
    uint32_t largest_group;
    DummyHead* groups;
    // IRLine* lines;
} StateIR;

typedef struct {
    uint32_t total_lines; 
    uint32_t line_index;
    uint8_t eof_found;
    FILE* file;
    //char buf[BUF_SIZE];
    StateIR* ir;
} ctx;

/* INTERFACE FOR SCANNER AND PARSER FUNCTIONS */

// Scanner functions
int init_ctx(ctx* context, char* filename);
void print_token(token* tok, uint32_t line_num);
char next_char(FILE* file);
void get_next_token(FILE* file, token* tok);
int scan(ctx* context);

// Parser Functions
int parse(ctx* context);
void print_error(uint32_t line_num, uint32_t reason);

/* ILOC IR REPRESENTATION CODE */
int init_IR(StateIR* self);
int make_lines(DummyHead * hp);
void add_line(DummyHead * hp, struct IRLine * bp);
struct IRLine* remove_line(DummyHead * hp);
void state_destruct(StateIR* self);
void print_ir(StateIR* ir);

#endif
