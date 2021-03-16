//
// Created by 김주호 on 2021/03/14.
//

#ifndef SP_PROJ1_MEMORY_COMMANDS_H
#define SP_PROJ1_MEMORY_COMMANDS_H

#include <stdio.h>
#include <string.h>

#include "utils.h"

#define MEM_SIZE 1048577 // 16*65536+1
#define OK 1
#define ADDR_ERR -1
#define COMMA_ERR -2
#define RANGE_ERR -3

char MEMORY[MEM_SIZE];
int LAST_ADDR;

int set_actual_start_and_end(int num_of_args, char *l_or_NULL, char *r_or_NULL, int *start, int *end);
void dump(int num_of_tokens, char *l_or_NULL, char *r_or_NULL);
void print_memory(int start, int end);

#endif //SP_PROJ1_MEMORY_COMMANDS_H
