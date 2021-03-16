//
// Created by 김주호 on 2021/03/14.
//

#include "memory_commands.h"


void dump(int num_of_tokens, char *l_or_NULL, char *r_or_NULL) {
    int S, E;
    int num_of_args = num_of_tokens - 1;
    int STATE = OK;

    // dump의 명령의 종류는 parameter의 종류에 따라 3가지
    // 1. dump  2. dump start   3. dump start, end
    switch (num_of_args) {
        case 0:
            set_actual_start_and_end(num_of_args, l_or_NULL, r_or_NULL, &S, &E);
            break;
        case 1:
            STATE = set_actual_start_and_end(num_of_args, l_or_NULL, r_or_NULL, &S, &E);
            break;
        case 2:
            STATE = set_actual_start_and_end(num_of_args, l_or_NULL, r_or_NULL, &S, &E);
            if (STATE == COMMA_ERR) printf("comma err! (use ',' between start address and end address)\n");
            break;
        default:
            printf("command err! too many args!\n");
            return;
    }
    if (STATE == ADDR_ERR) {
        printf("addr err!\n");
        return;
    }
    if (STATE == RANGE_ERR) {
        printf("range err!\n");
        return;
    }
    print_memory(S, E);
    LAST_ADDR = E;
}


// output : True or False
int set_actual_start_and_end(int num_of_args, char *l_or_NULL, char *r_or_NULL, int *start, int *end) {
    // 1. arg가 0개인 case0    2. arg가 1개인 case1   3. arg가 2개인 case2
    switch (num_of_args) {
        case 0: // dump
            *start = LAST_ADDR + 1 < MEM_SIZE ? LAST_ADDR + 1 : 0;
            *end = *start + 16 * 10 - 1 < MEM_SIZE ? *start + 16 * 10 - 1 : MEM_SIZE - 1;
            break;
        case 1: // dump AA
            *start = hex_to_dec(l_or_NULL);
            *end = *start + 16 * 10 - 1 < MEM_SIZE ? *start + 16 * 10 - 1 : MEM_SIZE - 1;
            break;
        case 2: // dump AA, BB
            if (l_or_NULL[strlen(l_or_NULL) - 1] != ',') return COMMA_ERR;
            l_or_NULL[strlen(l_or_NULL) - 1] = '\0';
            *start = hex_to_dec(l_or_NULL);
            *end = hex_to_dec(r_or_NULL);
            break;
        default:
            printf("err! wrong parameter!\n");
            break;
    }
    if (*start < 0 || *start > 255 || *end < 0 || *end > 255) return RANGE_ERR;
    if (*start > *end) return RANGE_ERR;
    return OK;
}

void print_memory(int start, int end) {
    int begin_row = start / 16, end_row = end / 16;
    int MAX_COL_SIZE = 16;

    for (int r = begin_row; r <= end_row; r++) {
        printf("%05X  ", r * 16);
        for (int c = 0; c < MAX_COL_SIZE; c++) {
            int cur = r * MAX_COL_SIZE + c;
            if (cur < start || cur > end) printf("   ");
            else printf("%X%X ", (MEMORY[cur] & 0XF0) >> 4, MEMORY[cur] & 0XF);
        }
        printf(";  ");
        for (int c = 0; c < MAX_COL_SIZE; c++) {
            char ASCII = MEMORY[r * MAX_COL_SIZE + c];
            if (ASCII >= 32 && ASCII <= 126) printf("%c", ASCII);
            else printf(".");
        }
        printf("\n");
    }
}
