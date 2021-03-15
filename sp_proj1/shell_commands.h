//
// Created by 김주호 on 2021/03/12.
//

#ifndef SP_PROJ1_SHELL_COMMANDS_H
#define SP_PROJ1_SHELL_COMMANDS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

typedef struct CMD {
    char cmd[100];
    struct CMD *nxt;
} CMD;
CMD* HEAD;
CMD* TAIL;

void help();
void dir();
void save_instructions(char* refined_cmd);
void history();

#endif //SP_PROJ1_SHELL_COMMANDS_H
