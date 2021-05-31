//
// Created by 김주호 on 2021/05/31.
//

#ifndef SP_PROJ4_MYSHELL_H
#define SP_PROJ4_MYSHELL_H

/* $begin shellmain */
#include "csapp.h"
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

#define MAXARGS   128
#define MAXJOBS   32

// pipe command의 유형을 3가지로 분류 (머리 - 몸통 - 꼬리)
// pipe structure : FIRST - MIDDLE - ... - MIDDLE - LAST
typedef enum PCMD_TYPE {
    FIRST = 0,
    MIDDLE = 1,
    LAST = 2
}PCMD_TYPE;

// JOB의 상태를 4가지로 분류
typedef enum JOB_STATE{
    _undefined = 0,
    FG = 1,
    BG = 2,
    ST = 3
}JOB_STATE;

// JOB의 정보
typedef struct JOB_info{
    int ind;
    pid_t pid;
    JOB_STATE state;
    char cmdline[MAXLINE];
}JOB_info;

// JOB 정보를 가지는 구조체 배열
JOB_info JOBS[MAXJOBS];

int PRINT_FOR_DEBUG = 0;

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, int *argc, char **argv);
int builtin_command(int argc, char **argv);

// phase 1
void change_dir(int argc, char **argv);

// phase 2
int pipe_command(char*cmd, char **argv, int input, PCMD_TYPE pipe_type, pid_t *pid);
void tokenize_for_pipe_command(char *cmd, char **argv);

// phase 3
void sigint_handler(int sig);
void sigtstp_handler(int sig);
void sigchld_handler(int sig);

// for JOB handling
void list_jobs();
int push_job(pid_t pid, int state, char *cmdline);
int delete_job(pid_t pid);
int update_job(pid_t pid, int state);
JOB_info *get_JOB_info_or_NULL(pid_t pid);
void waitfg(pid_t pid);


#endif //SP_PROJ4_MYSHELL_H
