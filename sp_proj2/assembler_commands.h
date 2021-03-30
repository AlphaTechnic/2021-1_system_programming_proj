//
// Created by 김주호 on 2021/03/29.
//

#ifndef SP_PROJ1_ASSEMBLER_COMMANDS_H
#define SP_PROJ1_ASSEMBLER_COMMANDS_H

#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "utils.h"

#define MAX_SYMBOL_SIZE 100
#define MAX_NAME_SIZE 100
#define MAX_LINE_SIZE 100
#define MAX_LABEL_SIZE 50
#define MAX_OPCODE_SIZE 50
#define MAX_OPERAND_SIZE 50

typedef struct symbol_node {
    char symbol[MAX_SYMBOL_SIZE];
    int address;
    struct symbol_node *nxt;
} symbol_node;

symbol_node *SYMTAB_HEAD;
symbol_node *RECENT_SYMTAB_HEAD;

OK_or_ERR assemble(char *filename);
void symbol(char *);

#endif //SP_PROJ1_ASSEMBLER_COMMANDS_H
