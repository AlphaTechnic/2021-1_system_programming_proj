//
// Created by 김주호 on 2021/03/10.
//

#ifndef SP_PROJ1_20121277_H
#define SP_PROJ1_20121277_H

/* 포함되는 파일 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "shell_commands.h"
#include "memory_commands.h"
#include "opcode_table_commands.h"
#include "assembler_commands.h"
#include "loader_commands.h"

/* 정의되는 상수 */
#define MAX_INPUT_LEN 100
#define MAXNUM_OF_TOKEN 5
#define MAX_TOKEN_LEN 15

typedef enum SUCCESS_or_FAIL {
    FAIL = 0,
    SUCCESS
}SUCCESS_or_FAIL;

/* 전역 변수 */
char INPUT[MAX_INPUT_LEN];
char INPUT_SPLIT[MAXNUM_OF_TOKEN][MAX_TOKEN_LEN];
char REFINED_INPUT[MAX_INPUT_LEN];
int NUM_OF_TOKENS;

/* 사용자 정의 data type */
typedef enum COMMAND {
    // 0 ~ 4 : shell commands
    HELP_CMD = 0,
    DIR_CMD,
    QUIT_CMD,
    HISTORY_CMD,
    TYPE_CMD,

    // 5 ~ 8 : memory commands
    DUMP_CMD,
    EDIT_CMD,
    FILL_CMD,
    RESET_CMD,

    // 9, 10 : opcode table commands
    OPCODE_MNEMONIC_CMD,
    OPCODELIST_CMD,

    // 11, 12 : assemble commands
    ASSEMBLE_CMD,
    SYMBOL_CMD,

    // 13, 14, 15, 16 : loader commands
    PROGADDR_CMD,
    LOADER_CMD,
    BP_CMD,
    RUN_CMD,

    // 17 : INPUT error
    WRONG_CMD
} COMMAND;

/* 함수 원형 */
void init();
void refresh_input();
SUCCESS_or_FAIL input_split_by_comma();
COMMAND get_command();
SUCCESS_or_FAIL execute_cmd(COMMAND cmd);
void make_refined_input();

#endif //SP_PROJ1_20121277_H
