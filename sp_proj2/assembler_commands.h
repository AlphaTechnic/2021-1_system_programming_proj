//
// Created by 김주호 on 2021/03/29.
//

#ifndef SP_PROJ1_ASSEMBLER_COMMANDS_H
#define SP_PROJ1_ASSEMBLER_COMMANDS_H

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "utils.h"
#include "opcode_table_commands.h"

#define SYMBOL_LEN 100
#define LABEL_LEN 50
#define MNEMONIC_LEN 50
#define OPERAND_LEN 50
#define LINE_NUM_SCALE 5
#define MAX_LINE_NUM 1000000

#define M_RECORD_LEN 10 // T.001000.1E
#define TOTAL_M_RECORD_SIZE 100
#define ONELINE_T_RECORD_BYTE_SIZE 30
#define ONELINE_T_RECORD_LINE_SIZE 9+30*20+1 // T.001000.1D.(30BYTE).\0
#define OBJ_CODE_LEN 20
#define FORMAT4_TA_LEN 5


typedef struct SYM_node {
    char symbol[SYMBOL_LEN];
    int address;
    struct SYM_node *nxt;
} SYM_node;
SYM_node *SYMTAB_HEAD;
SYM_node *LATEST_SYMTAB;

int B_val;
int M_RECORD_NUM;
char M_RECORDS[TOTAL_M_RECORD_SIZE][M_RECORD_LEN];

// assemble 함수
OK_or_ERR assemble(char *filename);
OK_or_ERR pass1(FILE *fp, char *filename, int *PROGRAM_SIZE);
OK_or_ERR pass2(char *filename, int PROGRAM_SIZE);
OK_or_ERR make_obj_code(char *obj_code, int PC_val, char *MNEMONIC, char *OP1, char *OP2, int STARTING_ADDR);

// symtab 관련 함수
void print_symbols();
SYM_node *find_symbol_or_NULL(char *symbol);
void push_to_symtab(char *symbol, int addr);
void free_SYMTAB(SYM_node *head);


#endif //SP_PROJ1_ASSEMBLER_COMMANDS_H
