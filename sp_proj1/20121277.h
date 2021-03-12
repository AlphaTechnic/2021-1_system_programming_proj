//
// Created by 김주호 on 2021/03/10.
//

#ifndef SP_PROJ1_20121277_H
#define SP_PROJ1_20121277_H

#include <stdio.h>
#include <string.h>

#define MAX_INPUT_LEN 50
#define MAXNUM_OF_TOKEN 5
#define MAX_TOKEN_LEN 10

char input[MAX_INPUT_LEN];
char input_split[MAXNUM_OF_TOKEN][MAX_TOKEN_LEN];
int NUM_OF_TOKEN;

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

    // 10, 11 : input error
    command_err,
    none_command
} command;


#endif //SP_PROJ1_20121277_H
