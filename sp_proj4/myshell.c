/* $begin shellmain */
#include "csapp.h"
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

#define MAXARGS   128

enum PIPE_TYPE {
    FIRST = 0,
    MIDDLE = 1,
    LAST = 2
};

int NUM_OF_CALLS = 0;

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, int *argc, char **argv);
int builtin_command(int argc, char **argv);

void change_dir(int argc, char **argv);
int pipe_command(char*cmd, char **argv, pid_t pid, int input, enum PIPE_TYPE pipe_type);
void tokenize(char *cmd, char **argv);

int main() {
    char cmdline[MAXLINE]; /* Command line */

    while (1) {
        /* Read */
        printf("CSE4100-SP-P#4> ");
        fgets(cmdline, MAXLINE, stdin);
        if (feof(stdin))
            exit(0);

        /* Evaluate */
        eval(cmdline);
    }
}
/* $end shellmain */

/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline) {
    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job pipe_command in bg or fg? */
    pid_t pid;           /* Process id */
    int argc;
    int p_flag = 0;
    int input = 0;
    enum PIPE_TYPE p_type = FIRST;

    strcpy(buf, cmdline);
    bg = parseline(buf, &argc, argv);

    // is pipelined?
    char* cmd = cmdline;
    char* bar_pos;
    if ((bar_pos = strchr(cmd, '|')) != NULL) {
        p_flag = 1;
    }

    if (argv[0] == NULL)
        return;   /* Ignore empty lines */
    if (!builtin_command(argc, argv)) { // quit -> exit(0), & -> ignore, other -> pipe_command
        if (!p_flag) {
            if ((pid = fork()) == 0) { // pipe_command child process
                char filename[MAXARGS] = "/bin/";
                strcat(filename, argv[0]);

                if (execve(filename, argv, environ) < 0) {    //ex) /bin/ls ls -al &
                    printf("%s: Command not found.\n", argv[0]);
                    exit(0);
                }
            }
        }

        else { // pipeline command
            while (bar_pos != NULL) {
                *bar_pos = '\0';
                input = pipe_command(cmd, argv, pid, input, p_type);

                cmd = bar_pos + 1;
                bar_pos = strchr(cmd, '|');
                p_type = MIDDLE;
            }
            input = pipe_command(cmd, argv, pid, input, LAST);

            // wait 처리
            for (int i = 0; i < NUM_OF_CALLS - 1; i++) wait(NULL);
            NUM_OF_CALLS = 0;
        }

        /* Parent waits for foreground job to terminate */
        if (!bg) {
            int status;
            //if (waitpid(pid, &status, 0) < 0)
            //    unix_error("waitpid err!");
            if (wait(NULL) < 0)
                unix_error("waitpid err!");
        }
        else { //when there is background process!
            printf("%d %s", pid, cmdline);
        }
    }
    return;
}

/* If first arg is a builtin command, pipe_command it and return true */
int builtin_command(int argc, char **argv) {
    if (!strcmp(argv[0], "quit") || (!strcmp(argv[0], "exit"))) /* quit command */
        exit(0);
    if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
        return 1;

    // user-defined
    if (strcmp(argv[0], "cd") == 0) {
        change_dir(argc, argv);
        return 1;
    }
    return 0;                     /* Not a builtin command */
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, int *argc, char **argv) {
    char *delim;         /* Points to first space delimiter */
    //int argc;            /* Number of args */
    int bg;              /* Background job? */

    buf[strlen(buf) - 1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
        buf++;

    /* Build the argv list */
    *argc = 0;
    while ((delim = strchr(buf, ' '))) {
        argv[(*argc)++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }
    argv[*argc] = NULL;

    if ((*argc) == 0)  /* Ignore blank line */
        return 1;

    /* Should the job pipe_command in the background? */
    if ((bg = (*argv[(*argc) - 1] == '&')) != 0)
        argv[--(*argc)] = NULL;

    return bg;
}

/* $end parseline */

void change_dir(int argc, char **argv) {
    if ((argc != 2) || chdir(argv[1]) == -1) {
        printf("cd argument err!\n");
    }
    return;
}

int pipe_command(char*cmd, char **argv, pid_t pid, int input, enum PIPE_TYPE pipe_type){
    tokenize(cmd, argv);
    if (argv[0] != NULL) NUM_OF_CALLS++;

    int fd[2];
    pipe(fd);
    pid = fork();

    if (pid == 0) { // child
        if (pipe_type == FIRST && input == 0){
            dup2(fd[1], STDOUT_FILENO);
        }
        else if (pipe_type == MIDDLE && input != 0){
            dup2(input, STDIN_FILENO);
            dup2(fd[1], STDOUT_FILENO);
        }
        else{ // LAST
            dup2(input, STDIN_FILENO);
        }
        if ((execvp(argv[0], argv)) == -1){
            printf("execvp err!\n");
            exit(1);
        }
    }

    if (input != 0) close(input);
    close(fd[1]);
    if (pipe_type == LAST) close(fd[0]);
    return fd[0];
}

void tokenize(char *cmd, char** argv){
    while (isspace(*cmd)) cmd++;
    char* next = strchr(cmd, ' ');
    int i = 0;

    while (next != NULL){
        next[0] = '\0';
        argv[i++] = cmd;
        while (isspace(*(next + 1))) next++;
        cmd = next + 1;
        next = strchr(cmd, ' ');
    }

    if (cmd[0] != '\0') {
        argv[i] = cmd;
        next = strchr(cmd, '\n');
        if (next[0] != '\0') next[0] = '\0';
        i++;
    }

    // grep 명령과 함께 입력되는 쌍따움표 "" 혹은 따움표 '' 처리
   if ((argv[i-1][0] == '\"' && argv[i-1][strlen(argv[i-1])-1] == '\"') ||
           (argv[i-1][0] == '\'' && argv[i-1][strlen(argv[i-1])-1] == '\'')){
        argv[i-1][strlen(argv[i-1])-1] = '\0';
        argv[i-1] = &(argv[i-1][0]) + 1;
    }
    argv[i] = NULL;
}