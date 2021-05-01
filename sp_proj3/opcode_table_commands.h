//
// Created by 김주호 on 2021/03/17.
//

#ifndef SP_PROJ1_OPCODE_TABLE_COMMANDS_H
#define SP_PROJ1_OPCODE_TABLE_COMMANDS_H

/* 포함되는 파일 */
#include <stdio.h>
#include <string.h>
#include  <stdlib.h>

#include "utils.h"

// #define MAX_OPCODE_SIZE 3
#define MAX_INSTRUCTION_SIZE 10
#define MAX_FORMAT_SIZE 10
#define MAX_HASHTABLE_SIZE 20

/* 사용자 정의 data type*/
typedef struct OP_NODE {
    int opcode;
    char mnemonic[MAX_INSTRUCTION_SIZE];
    char format[MAX_FORMAT_SIZE];
    struct OP_NODE *nxt_by_mnemonic;
    struct OP_NODE *nxt_by_opcode;
} OP_NODE;

/* 전역 변수 */
OP_NODE* OPTAB_by_mnemonic[MAX_HASHTABLE_SIZE];
OP_NODE* OPTAB_by_opcode[MAX_HASHTABLE_SIZE];

/* 함수 원형 */
OK_or_ERR get_opcode_by_mnemonic(char *mnemonic);
OP_NODE *get_opcode_or_NULL_by_mnemonic(char *mnemonic);
void init_OPTAB(char *filename);
void opcodelist();
void free_OPTAB();


#endif //SP_PROJ1_OPCODE_TABLE_COMMANDS_H
