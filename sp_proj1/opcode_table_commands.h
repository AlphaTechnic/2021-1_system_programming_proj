//
// Created by 김주호 on 2021/03/17.
//

#ifndef SP_PROJ1_OPCODE_TABLE_COMMANDS_H
#define SP_PROJ1_OPCODE_TABLE_COMMANDS_H

#include <stdio.h>
#include <string.h>
#include  <stdlib.h>

#define MAX_OPCODE_SIZE 3
#define MAX_INSTRUCTION_SIZE 10
#define MAX_FORMAT_SIZE 10
#define MAX_HASHTABLE_SIZE 20

typedef struct OPCODE_MNEMONIC_MAP {
    char opcode[MAX_OPCODE_SIZE];
    char mnemonic[MAX_INSTRUCTION_SIZE];
    char format[MAX_FORMAT_SIZE];
    struct OPCODE_MNEMONIC_MAP *nxt;
} OPCODE_MNEMONIC_MAP;
OPCODE_MNEMONIC_MAP* HASH_TABLE[MAX_HASHTABLE_SIZE];


int hash_func(char *mnemonic);
void get_opcode(char *mnemonic);
void init_hash_table(char *filename);
void opcodelist();
void free_hash_table();


#endif //SP_PROJ1_OPCODE_TABLE_COMMANDS_H
