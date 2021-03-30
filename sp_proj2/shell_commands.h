//
// Created by 김주호 on 2021/03/12.
//

#ifndef SP_PROJ1_SHELL_COMMANDS_H
#define SP_PROJ1_SHELL_COMMANDS_H

/* 포함되는 파일 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include "utils.h"

/* 사용자 정의 data type */
typedef struct CMD {
    char cmd[100];
    struct CMD *nxt;
} CMD;

/* 전역 변수 */
CMD* HEAD;
CMD* TAIL;

/* 함수 원형 */
void help();
void dir();
void save_instructions(char* refined_cmd);
void history();
void free_log_of_instructions();

OK_or_ERR type(char* filename);

#endif //SP_PROJ1_SHELL_COMMANDS_H
