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
typedef struct OPCODE_MNEMONIC_MAP {
    int opcode;
    char mnemonic[MAX_INSTRUCTION_SIZE];
    char format[MAX_FORMAT_SIZE];
    struct OPCODE_MNEMONIC_MAP *nxt;
} OPCODE_MNEMONIC_MAP;

/* 전역 변수 */
OPCODE_MNEMONIC_MAP* HASH_TABLE[MAX_HASHTABLE_SIZE];

/* 함수 원형 */
int get_opcode(char *mnemonic);
OPCODE_MNEMONIC_MAP *get_opcode2(char *mnemonic);
void init_hash_table(char *filename);
void opcodelist();
void free_hash_table();


#endif //SP_PROJ1_OPCODE_TABLE_COMMANDS_H
