//
// Created by 김주호 on 2021/03/29.
//

#ifndef SP_PROJ1_ASSEMBLER_COMMANDS_H
#define SP_PROJ1_ASSEMBLER_COMMANDS_H

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <stdlib.h>

#include "utils.h"
#include "opcode_table_commands.h"

#define MAX_SYMBOL_SIZE 100
#define MAX_NAME_SIZE 100
#define MAX_LINE_SIZE 100
#define MAX_LABEL_SIZE 50
#define MAX_OPCODE_SIZE 50
#define MAX_OPERAND_SIZE 50
#define LINE_NUM_SCALE 5

typedef enum INSTRUCTION{
    OPERATION = 0,
    START = 1,
    END = 2,
    COMMENT = 3,
    BASE = 4,
    BYTE = 5,
    WORD = 6,
    RESB = 7,
    RESW = 8,
    ELSE = 9
} INSTRUCTION;

typedef struct SYM_node {
    char symbol[MAX_SYMBOL_SIZE];
    int address;
    struct SYM_node *nxt;
} SYM_node;

SYM_node *SYMTAB_HEAD;
SYM_node *LATEST_SYMTAB;

OK_or_ERR assemble(char *filename);
void symbol(char *);
INSTRUCTION line_split(char* line, char* label, char* mnemonic, char* op1, char* op2);
OK_or_ERR pass1(FILE* fp, char* filename, int* LENGTH);
INSTRUCTION get_instruction(char* mnemonic);
OK_or_ERR is_in_symtab(char *symbol);
void push_to_symtab(char* symbol, int addr);
OPCODE_MNEMONIC_MAP* get_opcode2(char *mnemonic);
int find_byte_len(char* constant);

#endif //SP_PROJ1_ASSEMBLER_COMMANDS_H
