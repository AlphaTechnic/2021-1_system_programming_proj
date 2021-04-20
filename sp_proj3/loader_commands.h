//
// Created by 김주호 on 2021/04/17.
//

#ifndef ASSEMBLER_COMMANDS_C_LOADER_H
#define ASSEMBLER_COMMANDS_C_LOADER_H

#include "memory_commands.h"
#include "utils.h"
#include "assembler_commands.h"


#define SYMBOL_LEN 10
#define HASH_LEN 20

typedef struct ES_NODE{
    char symbol_name[SYMBOL_LEN];
    int addr;
    struct ES_NODE* nxt;
}ES_NODE;
ES_NODE *ESTAB[HASH_LEN];

typedef struct BP_NODE{
    int addr;
    struct BP_NODE* nxt;
}BP_NODE;
BP_NODE *BPTAB;
int BP_CHK[MEM_SIZE];
int bp_visited;

int PROG_ADDR;
int CS_ADDR;
int EXEC_ADDR;
int CS_LEN;
int TOTAL_LEN;


#endif //ASSEMBLER_COMMANDS_C_LOADER_H
