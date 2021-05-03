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

// bp가 걸려있는 memory cell의 주소에 true(1) 값을 저장시켜 bp가 걸려있음을 표시한다.
int BP_CHK[MEM_SIZE];
int BP_ADDR[MEM_SIZE];
int PRINT_FLAG;
int NUM_OF_BP;

// 레지스터와 프로그램 address와 길이 관리 변수
int REG[REG_NUM + 1];
char CC; // Condition Code
int PROGRAM_ADDR;
int CS_ADDR;
int CS_LEN;
int PROGRAM_LEN;
int FIRST_INSTRUCTION_ADDR;

// func about loader instruction
OK_or_ERR set_PROGADDR(char* addr_hexstr);
OK_or_ERR loader(char filenames[MAX_FILES_NUM][MAX_FILENAME_LEN]);
OK_or_ERR load_pass1(FILE *fp);
OK_or_ERR load_pass2(FILE *fp);

// func about ESTAB instruction
OK_or_ERR push_to_ESTAB(char *es_name, int es_addr);
ES_NODE *find_ESNODE_or_NULL(char *es_name);
void free_ESTAB();

// func about BP and run instruction
OK_or_ERR bp_command(int num_of_tokens, char *addr_hexstr_or_claer_str);
OK_or_ERR run();
OK_or_ERR execute_instructions();
int LD_related_instruction(int ni, int TA, int format, int num_of_bytes);
int J_related_instruction(int ni, int TA, int format);
void ST_related_instruction(int ni, int TA, int tar_val, int format, int num_of_bytes);

#endif //ASSEMBLER_COMMANDS_C_LOADER_H
