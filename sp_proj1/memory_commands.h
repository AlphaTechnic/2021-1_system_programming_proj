//
// Created by 김주호 on 2021/03/14.
//

#ifndef SP_PROJ1_MEMORY_COMMANDS_H
#define SP_PROJ1_MEMORY_COMMANDS_H

#include <stdio.h>
#include <string.h>

#include "utils.h"

#define MEM_SIZE 1048576 // 16*65536
#define ONE_BYTE_SIZE 256 // FF(hex) == 255(dec)
#define OK 1
#define COMMA_ERR -1
#define RANGE_ERR -2

char MEMORY[MEM_SIZE];
int LAST_ADDR;

int set_actual_start_and_end(int num_of_args, char *l_or_NULL, char *r_or_NULL, int *start_dec, int *end_dec);
void dump(int num_of_tokens, char *l_or_NULL, char *r_or_NULL);
void print_memory(int start, int end);
int edit(char* addr_hexstr, char* val_hexstr);
int fill(char* start_hexstr, char* end_hexstr, char* val_hexstr);

#endif //SP_PROJ1_MEMORY_COMMANDS_H
