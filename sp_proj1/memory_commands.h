//
// Created by 김주호 on 2021/03/14.
//

#ifndef SP_PROJ1_MEMORY_COMMANDS_H
#define SP_PROJ1_MEMORY_COMMANDS_H

/* 포함되는 파일 */
#include <stdio.h>
#include <string.h>

#include "utils.h"

/* 정의되는 상수 */
#define MEM_SIZE 1048576 // 16*65536
#define ONE_BYTE_SIZE 256 // FF(hex) == 255(dec)

/* 전역 변수 */
char MEMORY[MEM_SIZE];
int LAST_ADDR;

/* 함수 원형 */
int set_actual_start_and_end(int num_of_args, char *l_or_NULL, char *r_or_NULL, int *start_dec, int *end_dec);
int dump(int num_of_tokens, char *l_or_NULL, char *r_or_NULL);
void print_memory(int start, int end);
int edit(char *addr_hexstr, char *val_hexstr);
int fill(char *start_hexstr, char *end_hexstr, char *val_hexstr);
void reset();

#endif //SP_PROJ1_MEMORY_COMMANDS_H
