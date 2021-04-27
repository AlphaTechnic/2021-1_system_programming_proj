//
// Created by 김주호 on 2021/03/12.
//

#ifndef SP_PROJ1_UTILS_H
#define SP_PROJ1_UTILS_H

/* 포함되는 파일 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define LINE_LEN 100
#define NAME_LEN 100
#define MAX_HASH_SIZE 20
#define REG_NUM 9

typedef enum{
    ASSEMBLY_CODE_ERR = -4,
    FILE_ERR,
    RANGE_ERR,
    COMMA_ERR,
    OK = 1
} OK_or_ERR;

/* 사용자 정의 data type */
typedef enum {
    NO_COMMA = 0,
    YES_COMMA
} func_option;

typedef enum INSTRUCTION {
    _ELSE = -1,
    _OPERATION = 0,
    _START = 1,
    _END = 2,
    _COMMENT = 3,
    _BASE = 4,
    _BYTE = 5,
    _WORD = 6,
    _RESB = 7,
    _RESW = 8
} INSTRUCTION;

typedef enum REG_num {
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
} REG_num;

/* 전역 변수 */
func_option IS_COMMA;

/* 함수 원형 */
int get_dx_to_nxt_token(char *start_ptr);
int hexstr_to_decint(char *hexstr);
int hash_func(char *string, int max_hash_size);
INSTRUCTION line_split(char *line, char *LABEL, char *MNEMONIC, char *OP1, char *OP2);
INSTRUCTION line_split2(char *line, int *LOCCTR, char *LABEL, char *MNEMONIC, char *OP1, char *OP2);
INSTRUCTION get_instruction(char *mnemonic);
int get_byte_size(char *BYTE_operand);
OK_or_ERR file_open(char* filename, FILE** fp_itm, FILE** fp_lst, FILE** fp_obj);
REG_num get_REG_num(char *REG);


#endif //SP_PROJ1_UTILS_H
