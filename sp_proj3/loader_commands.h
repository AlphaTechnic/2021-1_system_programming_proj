//
// Created by 김주호 on 2021/04/17.
//

#ifndef ASSEMBLER_COMMANDS_C_LOADER_H
#define ASSEMBLER_COMMANDS_C_LOADER_H

#include "memory_commands.h"
#include "utils.h"
#include "assembler_commands.h"

#define SYMBOL_LEN 10
#define ESTAB_HASH_SIZE 20
#define MAX_RF_NUM 100

typedef struct ES_NODE{
    char name[SYMBOL_LEN];
    int addr;
    struct ES_NODE* nxt;
}ES_NODE;
ES_NODE *ESTAB[ESTAB_HASH_SIZE];

typedef struct BP_NODE{
    int addr;
    struct BP_NODE* nxt;
}BP_NODE;
BP_NODE *BPTAB;
int BP_CHK[MEM_SIZE];
int bp_visited;

int REG[REG_NUM + 1];
int PROG_ADDR;
int CS_ADDR;
int EXEC_ADDR;
int CS_LEN;
int TOTAL_LEN;

OK_or_ERR prog_addr(char* addr_hexstr);
OK_or_ERR loader(char filename);
void load_pass1(FILE *fp);
void load_pass2(FILE *fp);

void push_to_ESTAB(char *es_name, int es_addr);
ES_NODE *find_ESNODE_or_NULL(char *es_name);
void free_ESTAB();

#endif //ASSEMBLER_COMMANDS_C_LOADER_H
