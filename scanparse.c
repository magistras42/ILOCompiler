/**
 * Scanner and Parser functionality for Lab 1.
 */

#include "scanparse.h"

/* CONSTANT ARRAYS */
// List of all token pos.
const char* POS_NAMES[] = {
    "MEMOP",
    "LOADI",
    "COMMA",
    "ENDFILE",
    "ARITHOP",
    "OUTPUT",
    "NOP",
    "REG",
    "CONST",
    "INTO",
    "NEWLINE",
    "ERROR"
};

// List of all possible lexemmes
const char* LEXEMMES[] = {
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

uint8_t rollback = 0;
uint8_t eof_found = 0;
char prev;

/* IR Representation */
int init_IR(StateIR* self) {
    self->largest_group = 0;
    struct IRLine* lines;

    // Allocate a block of memory(array of "Group" elements) of size "memory", assign pointer to group
    uint64_t group_memory = ((uint64_t)MAX_GROUPS) * sizeof(DummyHead);
    self->groups = (DummyHead*) malloc(group_memory);
    if (self->groups == NULL) {
        return -1;
    }
    self->groups[0].line_count = BLOCK_SIZE;

    // Allocate a block of all IRLines needed.
    uint64_t line_memory = ((uint64_t)BLOCK_SIZE) * sizeof(struct IRLine);
    lines = (struct IRLine*) malloc(line_memory);
    if (lines == NULL) {
        return -1;
    }

    // Initialize first line in doubly circular linked list.
    lines[0].prev = (struct IRLine *) &self->groups[0];
    lines[0].next = (struct IRLine *) &(lines[1]);
    // First element in group 0 doubly circular linked list is the first IRLine.
    self->groups[0].oldest = &lines[0];

    // Initialize other group in group array to be doubly circular linked list.
    self->groups[1].line_count = 0;
    self->groups[1].oldest = (struct IRLine *) &self->groups[1];
    self->groups[1].newest = (struct IRLine *) &self->groups[1];


    // Initialize all other lines to be included in doubly circular linked lists.
    struct IRLine *prev_line = &lines[0];
    for (uint32_t i = 1; i < BLOCK_SIZE; i++) {
        lines[i].prev = prev_line;
        // If lines[i] not last line in block, next should be lines[i+1]
        prev_line->next = &lines[i];
        prev_line = &lines[i];
    }
 

    // last line's next should be lines[0], group[0] should be last line.
    self->groups[0].newest = (struct IRLine *) &lines[BLOCK_SIZE - 1];
    lines[BLOCK_SIZE - 1].next = (struct IRLine *) &self->groups[0];
    // memory allocated on the heap = memory(groups) + memory(lines)
    return 0;
}

// Put more lines in ir->groups[0]
int make_lines(DummyHead * hp) {
    // Allocate a block of all IRLines needed.
    uint64_t line_memory = ((uint64_t)BLOCK_SIZE) * sizeof(struct IRLine);
    struct IRLine* lines = (struct IRLine*) malloc(line_memory);
    if (lines == NULL) {
        return -1;
    }

    hp->line_count += BLOCK_SIZE;

    // Initialize first line in doubly circular linked list.
    lines[0].prev = (struct IRLine *) hp;
    lines[0].next = (struct IRLine *) &(lines[1]);
    // First element in group 0 doubly circular linked list is the first IRLine.
    hp->oldest = &lines[0];
    // Initialize all other lines to be included in doubly circular linked lists.
    struct IRLine *prev_line = &lines[0];
    for (uint32_t i = 1; i < BLOCK_SIZE; i++) {
        lines[i].prev = prev_line;
        prev_line->next = &lines[i];
        prev_line = &lines[i];
    }
 

    // last line's next should be lines[0], group[0] should be last line.
    hp->newest = (struct IRLine *) &lines[BLOCK_SIZE - 1];
    lines[BLOCK_SIZE - 1].next = (struct IRLine *) hp;
    return 0;
}

/**
 * Insert a bucket after the head of a
 * double circularly linked list.
 */
void add_line(DummyHead * hp, struct IRLine * bp) {
    struct IRLine *temp = hp->newest;
    temp->next = bp;
    bp->prev = temp;
    bp->next = (struct IRLine *) hp;
    hp->newest = bp;
    if (hp->line_count == 0) {
        hp->oldest = bp;
    }
    hp->line_count++;
}

// i suspect that oldest isn't getting set properly

// TODO: For next assignment, will need to make remove general case

/**
 * Remove the oldest bucket from a
 * double circularly linked list.*/
struct IRLine* remove_line(DummyHead * hp) {
    struct IRLine *temp = hp->oldest;
    temp->next->prev = temp->prev;
    hp->oldest = temp->next;

    temp->next = NULL;
    temp->prev = NULL;

    hp->line_count--;
    return temp;
}

// only gets address,
struct IRLine* get_next_IR(StateIR* ir) {
    if (ir->groups->line_count == 0) {
        make_lines(ir->groups);
    }
    return ir->groups->oldest;
}

/**
 * Free lines and groups.*/
void state_destruct(StateIR* self) {
    // Free the memory allocated on the heap.
    free(self->groups);
    // TODO: For loop to destroy blocks of lines but rn no need bc use throughout
    // free(self->buckets);
    free(self);
}

/* Function to format printing an IR Block Nicely(TM)*/
void print_ir(StateIR* ir) {
    uint32_t total = ir->groups[1].line_count;
    struct IRLine* line = ir->groups[1].oldest;
    // printf("dummy head adr: %p\n", &ir->groups[1]);
    // printf("newest line: %p\n", ir->groups[1].newest);
    // printf("oldest line: %p\n", ir->groups[1].oldest);
    for (uint32_t i = 0; i < total; ++i) {
        // printf("next line: %p\n", line->next);
        // printf("prev line: %p\n", line->prev);
        // Add code that reads each of the ops and converts as needed.
        switch(line->opcode) {
            case nop:
                printf("%s\t[  ], [  ], [  ]\n", LEXEMMES[line->opcode]);
                break;
            case output:
                printf("%s\t[ val%d ], [  ], [  ]\n", LEXEMMES[line->opcode], line->op1.sr);
                break;
            case loadI:
                printf("%s\t[ val%d ], [  ], [ sr%d ]\n", LEXEMMES[line->opcode], line->op1.sr, line->op3.sr);
                break;
            case store:
            case load:
                printf("%s\t[ sr%d ], [  ], [ sr%d ]\n", LEXEMMES[line->opcode], line->op1.sr, line->op3.sr);
                break;
            case add:
            case sub:
            case mult:
            case lshift:
            case rshift:
                printf("%s\t[ sr%d ], [ sr%d ], [ sr%d ]\n", LEXEMMES[line->opcode], line->op1.sr, line->op2.sr, line->op3.sr);
                break;
        }
        line = line->next;
    }
}

/* SCANNING IMPLEMENTATION */

int init_ctx(ctx* context, char* filename) {
        context->total_lines = 1;
        context->line_index = 0;
        if ((context->file = fopen(filename, "r")) == NULL) {
                return -1;
        }
        // fread(context->buf, sizeof(char), BUF_SIZE, context->file);
        return 0;
}

void print_token(token* tok, uint32_t line_num) {
        if (tok->pos != CONSTANT && tok->pos != REGISTER) {
                printf("%i: < %s, \"%s\" >\n", line_num, POS_NAMES[tok->pos], LEXEMMES[tok->lex]);
        } else {
               printf("%i: < %s, \"%s%i\" >\n", line_num, POS_NAMES[tok->pos], tok->pos == CONSTANT ? "" : "r", tok->lex); 
        }
}

void print_error(uint32_t line_num, uint32_t reason) {
        fprintf(stderr, "ERROR %i:\t\"%s\" \n", line_num, LEXEMMES[reason]);
}

// Need to check out these inline shennanigans
// change to use the buffer
char next_char(FILE* file) {
        return fgetc(file);
}

// need to fix what happen if valid ends early in self terminator, not processing terminator...
// make helper function??
void get_next_token(FILE* file, token* tok) {
        uint64_t num = 0;
        char cur;
        if (rollback) {
            cur = prev;
            rollback = 0;
        } else {
            cur = next_char(file);
        }
        if ((cur == ' ') || (cur == '\t')) {
            while ((cur == ' ') || (cur == '\t')) {
                cur = next_char(file);
            }
        }
        switch (cur) {
            case 's':
                cur = next_char(file);
                if (cur == 't') {
                    cur = next_char(file);
                    if (cur == 'o') {
                        cur = next_char(file);
                        if (cur == 'r') {
                            cur = next_char(file);
                            if (cur == 'e') {
                                tok->pos = MEMOP;
                                tok->lex = store;
                            } else {
                                tok->pos = ERROR;
                                tok->lex = spelling;
                            }
                        } else {
                            tok->pos = ERROR;
                            tok->lex = spelling;
                        }
                    } else {
                        tok->pos = ERROR;
                        tok->lex = spelling; 
                    }
                } else if (cur == 'u') {
                    cur = next_char(file);
                    if (cur == 'b') {
                        tok->pos = ARITHOP;
                        tok->lex = sub;
                    }
                } else {
                    tok->pos = ERROR;
                    tok->lex = spelling;
                }
                break;
            case 'l':
                cur = next_char(file);
                if (cur == 'o') {
                    cur = next_char(file);
                    if (cur == 'a') {
                        cur = next_char(file);
                        if (cur == 'd') {
                            cur = next_char(file);
                            if (cur == 'I') {
                                tok->pos = LOADI;
                                tok->lex = loadI;
                            } else {
                                tok->pos = MEMOP;
                                tok->lex = load;
                                rollback = 1;
                                prev = cur;
                            }
                        } else {
                            tok->pos = ERROR;
                            tok->lex = spelling;
                        }
                    } else {
                        tok->pos = ERROR;
                        tok->lex = spelling;
                    }
                } else if (cur == 's') {
                    cur = next_char(file);
                    if (cur == 'h') {
                        cur = next_char(file);
                        if (cur == 'i') {
                            cur = next_char(file);
                            if (cur == 'f') {
                                cur = next_char(file);
                                if (cur == 't') {
                                    tok->pos = ARITHOP;
                                    tok->lex = lshift;
                                } else {
                                    tok->pos = ERROR;
                                    tok->lex = spelling;
                                }
                            } else {
                                tok->pos = ERROR;
                                tok->lex = spelling;
                            }
                        } else {
                            tok->pos = ERROR;
                            tok->lex = spelling;
                        }
                    } else {
                        tok->pos = ERROR;
                        tok->lex = spelling;
                    }
                } else {
                    tok->pos = ERROR;
                    tok->lex = spelling;
                }
                break;
            case 'r':
                cur = next_char(file);
                if (cur == 's') {
                    cur = next_char(file);
                    if (cur == 'h') {
                        cur = next_char(file);
                        if (cur == 'i') {
                            cur = next_char(file);
                            if (cur == 'f') {
                                cur = next_char(file);
                                if (cur == 't') {
                                    tok->pos = ARITHOP;
                                    tok->lex = lshift;
                                } else {
                                    tok->pos = ERROR;
                                    tok->lex = spelling;
                                }
                            } else {
                                tok->pos = ERROR;
                                tok->lex = spelling;
                            }
                        } else {
                            tok->pos = ERROR;
                            tok->lex = spelling;
                        }
                    } else {
                        tok->pos = ERROR;
                        tok->lex = spelling;
                    }
                } else if ((cur >= '0') && (cur <= '9')) {
                    num += (cur - '0');
                    cur = next_char(file);
                    while ((cur >= '0') && (cur <= '9')) {
                        num *= 10;
                        num += (cur - '0');
                        cur = next_char(file);
                    }
                    // TODO: Make more robust
                    if (num > UINT32_MAX) {
                        tok->pos = ERROR;
                        tok->lex = overflow;
                        break;
                    }
                    rollback = 1;
                    prev = cur;
                    tok->pos = REGISTER;
                    tok->lex = (uint32_t) num;
                } else {
                    tok->pos = ERROR;
                    tok->lex = spelling;
                }
                break;
            case 'm':
                cur = next_char(file);
                if (cur == 'u') {
                    cur = next_char(file);
                    if (cur == 'l') {
                        cur = next_char(file);
                        if (cur == 't') {
                            tok->pos = ARITHOP;
                            tok->lex = mult;
                        } else {
                            tok->pos = ERROR;
                            tok->lex = spelling;
                        }
                    } else {
                        tok->pos = ERROR;
                        tok->lex = spelling;
                    }
                } else {
                    tok->pos = ERROR;
                    tok->lex = spelling;
                }
                break;
            case 'a':
                cur = next_char(file);
                if (cur == 'd') {
                    cur = next_char(file);
                    if (cur == 'd') {
                        tok->pos = ARITHOP;
                        tok->lex = add;
                    } else {
                        tok->pos = ERROR;
                        tok->lex = spelling;
                    }
                } else {
                    tok->pos = ERROR;
                    tok->lex = spelling;
                }
                break;
            case 'n':
                cur = next_char(file);
                if (cur == 'o') {
                    cur = next_char(file);
                    if (cur == 'p') {
                        tok->pos = NOP;
                        tok->lex = nop;
                    } else {
                        tok->pos = ERROR;
                        tok->lex = spelling;
                    }
                } else {
                    tok->pos = ERROR;
                    tok->lex = spelling;
                }
                break;
            case 'o':
                cur = next_char(file);
                if (cur == 'u') {
                    cur = next_char(file);
                    if (cur == 't') {
                        cur = next_char(file);
                        if (cur == 'p') {
                            cur = next_char(file);
                            if (cur == 'u') {
                                cur = next_char(file);
                                if (cur == 't') {
                                    tok->pos = OUTPUT;
                                    tok->lex = output;
                                    break;
                                }
                            }
                        }
                    }
                }
                tok->pos = ERROR;
                tok->lex = spelling;
                break;
            case '=':
                cur = next_char(file);
                if (cur == '>') {
                    tok->pos = INTO;
                    tok->lex = into;
                } else {
                    tok->pos = ERROR;
                    tok->lex = spelling;
                }
                break;
            case '/':
                // comment behavior
                cur = next_char(file);
                if (cur == '/') {
                    while ((cur != '\n') && (cur != EOF)) {
                        cur = next_char(file);
                    }
                    if (cur == '\n') {
                        tok->pos = EOL;
                        tok->lex = eol;
                    } else {
                        tok->pos = EoF;
                        tok->lex = eof;
                    }
                } else {
                    tok->pos = ERROR;
                    tok->lex = spelling;
                }
                break;
            case ',':
                tok->pos = COMMA;
                tok->lex = comma;
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                // while loop for parsing numbers
                num += (cur - '0');
                cur = next_char(file);
                while ((cur >= '0') && (cur <= '9')) {
                    num *= 10;
                    num += (cur - '0');
                    cur = next_char(file);
                }
                // TODO: Make more robust
                if (num > UINT32_MAX) {
                    tok->pos = ERROR;
                    tok->lex = overflow;
                }
                tok->pos = CONSTANT;
                tok->lex = (uint32_t) num;

                rollback = 1;
                prev = cur;
                break;
            case '\n':
                // newline behavior
                tok->pos = EOL;
                tok->lex = eol;
                break;
            case '\r':
                // other newline
                cur = next_char(file);
                if (cur == '\n') {
                    tok->pos = EOL;
                    tok->lex = eol;
                } else {
                    tok->pos = ERROR;
                    tok->lex = spelling;
                }
                break;
            case EOF:
                // end of file behavior
                if (eof_found) {
                    tok->pos = EoF;
                    tok->lex = eof;
                } else {
                    tok->pos = EOL;
                    tok->lex = eol;
                    eof_found = 1;
                }
                break;
            default:
                tok->pos = ERROR;
                tok->lex = spelling;
        }
}

void scan_to_newline(FILE* file, token* tok) {
    char cur = rollback ? prev : next_char(file);
    rollback = 0;
    while ((cur != '\n') && (cur != EOF)) {
        cur = next_char(file);
    }
    if (cur == '\n') {
        tok->pos = EOL;
        tok->lex = eol;
    } else {
        tok->pos = EoF;
        tok->lex = eof;
    }
}

int scan(ctx* context) {
        token tok;
        int status = 0;
        get_next_token(context->file, &tok);
        while (tok.pos != EoF) {
                if (tok.pos == ERROR) {
                    print_error(context->total_lines, tok.lex);
                    scan_to_newline(context->file, &tok);
                } else {
                    print_token(&tok, context->total_lines);
                    if (tok.pos == EOL) {
                        context->total_lines++;
                    }
                    get_next_token(context->file, &tok);
                }
        }
        print_token(&tok, context->total_lines);
        return status;
}


/* PARSING IMPLEMENTATION */

// need to check if should record all ops (even if failed parsing)
// also need to go back and add checking for new line behavior 
// so right line num - if error, set to -1
int parse(ctx* context) {
        token tok;
        struct IRLine* line;
        uint8_t line_has_err = 0;
        int status = 0;
        int num_op = 0;
        FILE* file = context->file;
        StateIR* irs = context->ir;
        DummyHead* rep = &(irs->groups[1]);
        get_next_token(file, &tok);
        while (tok.pos != EoF) {
            // need to add check for if tok.pos = ERROR, go until find EOL or EOF - set num_op to -1
            // maybe have a flag like "line has errors" and if set, scan to end of line
            // that way could check if has errors and isn't already an EOL tok, find EOL token otherwise continue
            // while (line_has_err && ((tok.pos != EOL) && (tok.pos != EoF))) {
            //     printf("pos: %s\n", POS_NAMES[tok.pos]);
            //     get_next_token(file, &tok);
            // }
            if (line_has_err && tok.pos != EOL) {
                scan_to_newline(file, &tok);
            }
            if (tok.pos == EoF) {
                break;
            }
            line_has_err = 0;
            // now need to fix so getting new token, add the line_has_err behavior
            // at end can check if line_has_err and EOL so can increase line count
            // printf("pos: %s, lex: %s\n", POS_NAMES[tok.pos], LEXEMMES[tok.lex]);
            switch(tok.pos) {
                // only valid is MEMOP REG => REG
                case MEMOP:
                    // printf("memop lex: %s\n", LEXEMMES[tok.lex]);
                    line = get_next_IR(irs);
                    line->opcode = tok.lex;
                    get_next_token(file, &tok);
                    switch (tok.pos) {
                        case REGISTER:
                            // printf("memop lex: %s\n", LEXEMMES[tok.lex]);
                            line->op1.sr = tok.lex;
                            get_next_token(file, &tok);
                            if (tok.pos == INTO) {
                                // printf("memop lex: %s\n", LEXEMMES[tok.lex]);
                                get_next_token(file, &tok);
                                // printf("memop lex: %s\n", LEXEMMES[tok.lex]);
                                if (tok.pos == REGISTER) {
                                    line->op3.sr = tok.lex;
                                    get_next_token(file, &tok);
                                    if (tok.pos == EOL) {
                                        remove_line(irs->groups);
                                        add_line(rep, line);
                                        num_op++;
                                        context->total_lines++;
                                        break;
                                    } else if (tok.pos == ERROR) {
                                        line_has_err = 1;
                                        print_error(context->total_lines, tok.lex);
                                        print_error(context->total_lines, invalid_sentence);
                                    } else {
                                        line_has_err = 1;
                                        print_error(context->total_lines, extra_ops);
                                    }
                                } else if (tok.pos == ERROR) {
                                    line_has_err = 1;
                                    print_error(context->total_lines, tok.lex);
                                    print_error(context->total_lines, invalid_sentence);
                                } else {
                                    line_has_err = 1;
                                    print_error(context->total_lines, invalid_sentence);
                                }
                            } else if (tok.pos == ERROR) {
                                line_has_err = 1;
                                print_error(context->total_lines, tok.lex);
                                print_error(context->total_lines, invalid_sentence);
                            } else {
                                line_has_err = 1;
                                print_error(context->total_lines, invalid_sentence);
                            }
                            break;
                        case ERROR:
                            print_error(context->total_lines, tok.lex);
                        default:
                            line_has_err = 1;
                            print_error(context->total_lines, invalid_sentence);
                    }
                    break;
                case LOADI:
                    line = get_next_IR(irs);
                    line->opcode = tok.lex;
                    get_next_token(file, &tok);
                    switch (tok.pos) {
                        case CONSTANT:
                            line->op1.sr = tok.lex;
                            get_next_token(file, &tok);
                            switch (tok.pos) {
                                case INTO:
                                    get_next_token(file, &tok);
                                    switch (tok.pos) {
                                        case REGISTER:
                                            line->op3.sr = tok.lex;
                                            get_next_token(file, &tok);
                                            switch (tok.pos) {
                                                case EOL:
                                                    remove_line(irs->groups);
                                                    add_line(rep, line);
                                                    num_op++;
                                                    context->total_lines++;
                                                    // get_next_token(file, &tok);
                                                    break;
                                                case ERROR:
                                                    print_error(context->total_lines, tok.lex);
                                                default:
                                                    line_has_err = 1;
                                                    print_error(context->total_lines, extra_ops);
                                            }
                                            break;
                                        case ERROR:
                                            print_error(context->total_lines, tok.lex);
                                        default:
                                            line_has_err = 1;
                                            print_error(context->total_lines, invalid_sentence);
                                    }
                                    break;
                                case ERROR:
                                    print_error(context->total_lines, tok.lex);
                                default:
                                    line_has_err = 1;
                                    print_error(context->total_lines, invalid_sentence);
                            }
                            break;
                        case ERROR:
                            print_error(context->total_lines, tok.lex);
                        default:
                            line_has_err = 1;
                            print_error(context->total_lines, invalid_sentence);
                    }
                    break;
                case ARITHOP:
                    // TODO: ARITHOPS
                    line = get_next_IR(irs);
                    line->opcode = tok.lex;
                    get_next_token(file, &tok);
                    switch (tok.pos) {
                        case REGISTER:
                            line->op1.sr = tok.lex;
                            get_next_token(file, &tok);
                            switch (tok.pos) {
                                case COMMA:
                                    get_next_token(file, &tok);
                                    switch (tok.pos) {
                                        case REGISTER:
                                            line->op2.sr = tok.lex;
                                            get_next_token(file, &tok);
                                            switch (tok.pos) {
                                                case INTO:
                                                    get_next_token(file, &tok);
                                                    switch (tok.pos) {
                                                        case REGISTER:
                                                            line->op3.sr = tok.lex;
                                                            get_next_token(file, &tok);
                                                            switch(tok.pos) {
                                                                case EOL:
                                                                    remove_line(irs->groups);
                                                                    add_line(rep, line);
                                                                    num_op++;
                                                                    context->total_lines++;
                                                                    break;
                                                                case ERROR:
                                                                    print_error(context->total_lines, tok.lex);
                                                                default:
                                                                    line_has_err = 1;
                                                                    print_error(context->total_lines, invalid_sentence);
                                                            }
                                                            break;
                                                        case ERROR:
                                                            print_error(context->total_lines, tok.lex);
                                                        default:
                                                            line_has_err = 1;
                                                            print_error(context->total_lines, invalid_sentence);
                                                    }
                                                    break;
                                                case ERROR:
                                                    print_error(context->total_lines, tok.lex);
                                                default:
                                                    line_has_err = 1;
                                                    print_error(context->total_lines, invalid_sentence);
                                            }
                                            break;
                                        case ERROR:
                                            print_error(context->total_lines, tok.lex);
                                        default:
                                            line_has_err = 1;
                                            print_error(context->total_lines, invalid_sentence);
                                    }
                                    break;
                                case ERROR:
                                    print_error(context->total_lines, tok.lex);
                                default:
                                    line_has_err = 1;
                                    print_error(context->total_lines, invalid_sentence);
                            }
                            break;
                        case ERROR:
                            print_error(context->total_lines, tok.lex);
                        default:
                            line_has_err = 1;
                            print_error(context->total_lines, invalid_sentence);
                    }
                    break;
                case OUTPUT:
                    line = get_next_IR(irs);
                    line->opcode = tok.lex;
                    get_next_token(file, &tok);
                    switch (tok.pos) {
                        case CONSTANT:
                            line->op1.sr = tok.lex;
                            get_next_token(file, &tok);
                            if (tok.pos == EOL) {
                                remove_line(irs->groups);
                                add_line(rep, line);
                                context->total_lines++;
                                num_op++;
                                // get_next_token(file, &tok);
                                break;
                            } else if (tok.pos == ERROR) {
                                line_has_err = 1;
                                print_error(context->total_lines, tok.lex);
                                print_error(context->total_lines, invalid_sentence);
                            } else {
                                line_has_err = 1;
                                print_error(context->total_lines, extra_ops);
                            }
                            break;
                        case ERROR:
                            print_error(context->total_lines, tok.lex);
                        default:
                            line_has_err = 1;
                            print_error(context->total_lines, invalid_sentence);
                    }
                    break;
                case NOP:
                    line = get_next_IR(irs);
                    line->opcode = tok.lex;
                    get_next_token(file, &tok);
                    if (tok.pos == EOL) {
                        remove_line(irs->groups);
                        add_line(rep, line);
                        num_op++;
                        context->total_lines++;
                        // get_next_token(file, &tok);
                        break;
                    } else if (tok.pos == ERROR) {
                        line_has_err = 1;
                        print_error(context->total_lines, tok.lex);
                    } else {
                        line_has_err = 1;
                        print_error(context->total_lines, extra_ops);
                    }
                    break;
                case EOL:
                    context->total_lines++;
                    break;
                case ERROR:
                    print_error(context->total_lines, tok.lex);
                default:
                    line_has_err = 1;
                    print_error(context->total_lines, invalid_op);
            }
            if (line_has_err) {
                if (tok.pos == EOL) {
                    context->total_lines++;
                    line_has_err = 0;
                }
                status = -1;
            }
            get_next_token(file, &tok);
        }

        if (status == -1) {
            return status;
        }
        
        return num_op;
}
