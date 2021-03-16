//
// Created by 김주호 on 2021/03/14.
//

#include "memory_commands.h"


void dump(int num_of_tokens, char *l_or_NULL, char *r_or_NULL) {
    int S_dec, E_dec;
    int num_of_args = num_of_tokens - 1;
    int STATE = OK;

    // dump의 명령의 종류는 parameter의 종류에 따라 3가지
    // 1. dump  2. dump start   3. dump start, end
    switch (num_of_args) {
        case 0:
            set_actual_start_and_end(num_of_args, l_or_NULL, r_or_NULL, &S_dec, &E_dec);
            break;
        case 1:
            STATE = set_actual_start_and_end(num_of_args, l_or_NULL, r_or_NULL, &S_dec, &E_dec);
            break;
        case 2:
            STATE = set_actual_start_and_end(num_of_args, l_or_NULL, r_or_NULL, &S_dec, &E_dec);
            if (STATE == COMMA_ERR) printf("comma err! (use ',' between start address and end address)\n");
            break;
        default:
            printf("command err! too many args!\n");
            return;
    }
    if (STATE == RANGE_ERR) {
        printf("range err!\n");
        return;
    }
    print_memory(S_dec, E_dec);
    LAST_ADDR = E_dec;
}


int set_actual_start_and_end(int num_of_args, char *l_or_NULL, char *r_or_NULL, int *start_dec, int *end_dec) {
    // 1. arg가 0개인 case0    2. arg가 1개인 case1   3. arg가 2개인 case2
    switch (num_of_args) {
        case 0: // dump
            *start_dec = LAST_ADDR + 1 < MEM_SIZE ? LAST_ADDR + 1 : 0;
            *end_dec = *start_dec + 16 * 10 - 1 < MEM_SIZE ? *start_dec + 16 * 10 - 1 : MEM_SIZE - 1;
            break;
        case 1: // dump AA
            *start_dec = hexstr_to_dec(l_or_NULL);
            *end_dec = *start_dec + 16 * 10 - 1 < MEM_SIZE ? *start_dec + 16 * 10 - 1 : MEM_SIZE - 1;
            break;
        case 2: // dump AA, BB
            if (l_or_NULL[strlen(l_or_NULL) - 1] != ',') return COMMA_ERR;
            l_or_NULL[strlen(l_or_NULL) - 1] = '\0';
            *start_dec = hexstr_to_dec(l_or_NULL);
            *end_dec = hexstr_to_dec(r_or_NULL);
            break;
        default:
            printf("err! wrong parameter!\n");
            break;
    }
    if (*start_dec < 0 || *start_dec > MEM_SIZE || *end_dec < 0 || *end_dec > MEM_SIZE) return RANGE_ERR;
    if (*start_dec > *end_dec) return RANGE_ERR;
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


int edit(char *addr_hexstr, char *val_hexstr) {
    int addr_dec, val_dec;
    // hexstr to dec
    if (addr_hexstr[strlen(addr_hexstr) - 1] != ',') {
        printf("comma err! (use ',' between start address and end address)\n");
        return COMMA_ERR;
    }
    addr_hexstr[strlen(addr_hexstr) - 1] = '\0';
    addr_dec = hexstr_to_dec(addr_hexstr);
    val_dec = hexstr_to_dec(val_hexstr);

    // validate
    if (addr_dec < 0 || addr_dec >= MEM_SIZE) {
        printf("range err!\n");
        return RANGE_ERR;
    }
    if (val_dec < 0 || val_dec >= ONE_BYTE_SIZE) {
        printf("range err!\n");
        return RANGE_ERR;
    }
    MEMORY[addr_dec] = val_dec;
    return OK;
}

int fill(char* start_hexstr, char* end_hexstr, char* val_hexstr){
    int start_dec, end_dec, val_dec;

    if (start_hexstr[strlen(start_hexstr)-1] != ',') {
        printf("comma err! (use ',' between start address and end address)\n");
        return COMMA_ERR;
    }
    if (end_hexstr[strlen(end_hexstr)-1]!= ',') {
        printf("comma err! (use ',' between start address and end address)\n");
        return COMMA_ERR;
    }
    start_hexstr[strlen(start_hexstr)-1] = '\0'; end_hexstr[strlen(end_hexstr)-1]= '\0';
    start_dec = hexstr_to_dec(start_hexstr); end_dec = hexstr_to_dec(end_hexstr); val_dec = hexstr_to_dec(val_hexstr);

    if (start_dec < 0 || start_dec >= MEM_SIZE) {
        printf("range err!\n");
        return RANGE_ERR;
    }
    if (end_dec < 0 || end_dec >= MEM_SIZE) {
        printf("range err!\n");
        return RANGE_ERR;
    }
    if (start_dec > end_dec){
        printf("range err!\n");
        return RANGE_ERR;
    }
    if (val_dec < 0 || val_dec >= ONE_BYTE_SIZE){
        printf("range err!\n");
        return RANGE_ERR;
    }

    for (int i = start_dec; i<= end_dec; i++) MEMORY[i] = val_dec;
    return OK;
}

void reset(){
    for (int i=0; i<MEM_SIZE; i++){
        MEMORY[i] = 0;
    }
}