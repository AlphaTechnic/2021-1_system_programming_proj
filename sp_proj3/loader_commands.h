//
// Created by 김주호 on 2021/04/17.
//

#ifndef ASSEMBLER_COMMANDS_C_LOADER_H
#define ASSEMBLER_COMMANDS_C_LOADER_H

#include "memory_commands.h"
#include "utils.h"
#include "assembler_commands.h"

#define MAX_FILES_NUM 5
#define MAX_FILENAME_LEN 15
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
BP_NODE *BP_LIST_HEAD;
int BP_CHK[MEM_SIZE];
int bp_visited;

int REG[REG_NUM + 1];
char CC; // Condition Code
int PROG_ADDR;
int CS_ADDR;
int FIRST_INSTRUCTION_ADDR;
int CS_LEN;
int TOTAL_LEN;

OK_or_ERR set_PROGADDR(char* addr_hexstr);
OK_or_ERR loader(char filenames[MAX_FILES_NUM][MAX_FILENAME_LEN]);
void load_pass1(FILE *fp);
void load_pass2(FILE *fp);

void push_to_ESTAB(char *es_name, int es_addr);
ES_NODE *find_ESNODE_or_NULL(char *es_name);
int LD_related_instruction(int ni, int TA, int num_of_bytes, int format);

OK_or_ERR bp_command(int num_of_tokens, char *addr_hexstr_or_claer_str);
void push_to_BPTAB(int addr);
OK_or_ERR run();
OK_or_ERR execute_instructions();
int LD_related_instruction(int ni, int TA, int format, int num_of_bytes);
int J_related_instruction(int ni, int TA, int format);
void ST_related_instruction(int ni, int TA, int tar_val, int format, int num_of_bytes);



#endif //ASSEMBLER_COMMANDS_C_LOADER_H
