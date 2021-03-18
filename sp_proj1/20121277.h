//
// Created by 김주호 on 2021/03/10.
//

#ifndef SP_PROJ1_20121277_H
#define SP_PROJ1_20121277_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "shell_commands.h"
#include "memory_commands.h"
#include "opcode_table_commands.h"

#define MAX_INPUT_LEN 100
#define MAXNUM_OF_TOKEN 5
#define MAX_TOKEN_LEN 10
#define SUCCESS 1
#define FAIL 0

char INPUT[MAX_INPUT_LEN];
char INPUT_SPLIT[MAXNUM_OF_TOKEN][MAX_TOKEN_LEN];
char REFINED_INPUT[MAX_INPUT_LEN];
int NUM_OF_TOKENS;

typedef enum command {
    // 0 ~ 3 : shell command
    help_command = 0,
    dir_command,
    quit_command,
    history_command,

    // 4 ~ 7 : memory command
    dump_command,
    edit_command,
    fill_command,
    reset_command,

    // 8, 9 : opcode table command
    opcode_mnemonic_command,
    opcodelist_command,

    // 10 : INPUT error
    wrong_cmd
} command;

void init();
void refresh_input();
int input_split_by_comma();
command get_command();
int execute_cmd(command cmd);
void make_refined_input();

#endif //SP_PROJ1_20121277_H
