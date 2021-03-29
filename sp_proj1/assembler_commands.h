//
// Created by 김주호 on 2021/03/29.
//

#ifndef SP_PROJ1_ASSEMBLER_COMMANDS_H
#define SP_PROJ1_ASSEMBLER_COMMANDS_H

#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "utils.h"

#define MAX_SYMBOL_LEN 100
#define MAX_NAME_LEN 100

typedef struct symbol_node{
    char symbol[MAX_SYMBOL_LEN];
    int address;
    struct symbol_node *nxt;
}symbol_node;

OK_or_ERR assemble(char* filename);
OK_or_ERR symbol(char*);

#endif //SP_PROJ1_ASSEMBLER_COMMANDS_H
