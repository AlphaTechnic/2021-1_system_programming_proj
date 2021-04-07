//
// Created by 김주호 on 2021/03/29.
//

#ifndef SP_PROJ1_ASSEMBLER_COMMANDS_H
#define SP_PROJ1_ASSEMBLER_COMMANDS_H

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdlib.h>

#include "utils.h"
#include "opcode_table_commands.h"

#define SYMBOL_LEN 100
#define NAME_LEN 100
#define LINE_LEN 100
#define LABEL_LEN 50
#define MNEMONIC_LEN 50
#define OPERAND_LEN 50
#define LINE_NUM_SCALE 5

#define M_RECORD_LEN 10 // T.001000.1E
#define TOTAL_M_RECORD_SIZE 100
#define ONELINE_T_RECORD_BYTE_SIZE 30
#define ONELINE_T_RECORD_LINE_SIZE 9+30*20+1 // T.001000.1D.(30BYTE).\0
#define OBJ_CODE_LEN 20
#define FORMAT4_LEN 5

typedef enum INSTRUCTION{
    _OPERATION = 0,
    _START = 1,
    _END = 2,
    _COMMENT = 3,
    _BASE = 4,
    _BYTE = 5,
    _WORD = 6,
    _RESB = 7,
    _RESW = 8,
    _ELSE = 9
} INSTRUCTION;

int B_val;

//typedef struct REG{
//    int number;
//    int address;
//} REG;
//REG* B_val;
//REG* PC;

typedef struct SYM_node {
    char symbol[SYMBOL_LEN];
    int address;
    struct SYM_node *nxt;
} SYM_node;
SYM_node *SYMTAB_HEAD;
SYM_node *LATEST_SYMTAB;

typedef enum REG_num{
    non_exist = -1,
    regA = 0,
    regX = 1,
    regL = 2,
    regB = 3,
    regS = 4,
    regT = 5,
    regF = 6,
    regPC = 8,
    regSW = 9
}REG_num;

int M_RECORD_NUM;
char M_RECORDS[TOTAL_M_RECORD_SIZE][M_RECORD_LEN];

OK_or_ERR assemble(char *filename);
void print_symbols();
INSTRUCTION line_split(char* line, char* label, char* mnemonic, char* op1, char* op2);
OK_or_ERR pass1(FILE* fp, char* filename, int* LENGTH);
INSTRUCTION get_instruction(char* mnemonic);
OK_or_ERR is_in_symtab(char *symbol);
void push_to_symtab(char* symbol, int addr);
OPCODE_MNEMONIC_MAP* get_opcode2(char *mnemonic);
int find_byte_len(char* constant);
void free_SYMTAB(SYM_node* head);
INSTRUCTION line_split2(char* line, int* LOCCTR, char* LABEL, char* MNEMONIC, char* OP1, char* OP2);
SYM_node *find_symbol(char* symbol);
OK_or_ERR make_obj_code_by_a_line(char *ret, int PC_val, char *MNEMONIC, char *OP1, char *OP2);
OK_or_ERR pass2(char *filename, int PROGRAM_SIZE);
REG_num get_REG_num(char* REG);

#endif //SP_PROJ1_ASSEMBLER_COMMANDS_H
