/* $begin shellmain */
#include "csapp.h"
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<errno.h>

#define MAXARGS   128
#define MAXPATH   2048
pid_t PIDS[128];
int PIPE_FLAG = 0;

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, int *argc, char **argv);
int builtin_command(int argc, char **argv);

void change_dir(int argc, char **argv);
void list_segments(int argc, char **argv);
void make_dir(int argc, char **argv);
void remove_dir(int argc, char **argv);
void touch(int argc, char **argv);
void cat(int argc, char **argv);
void echo(int argc, char **argv);

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
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */
    int argc;

    strcpy(buf, cmdline);
    bg = parseline(buf, &argc, argv);
    if (argv[0] == NULL)
        return;   /* Ignore empty lines */
    if (!builtin_command(argc, argv)) { // quit -> exit(0), & -> ignore, other -> run
        if ((pid = fork()) == 0) { // run child process
            if (execve(argv[0], argv, environ) < 0) {    //ex) /bin/ls ls -al &
                printf("%s: Command not found.\n", argv[0]);
                exit(0);
            }
        }

        /* Parent waits for foreground job to terminate */
        if (!bg) {
            int status;
            if (waitpid(pid, &status, 0) < 0) unix_error("waitpid err!");
        }
        else { //when there is background process!
            printf("%d %s", pid, cmdline);
            int i = 0;
            int ctr = 1;
            while (ctr == 1) {
                // puts the pid into the PIDS array so we can keep track of all pids
                if (PIDS[i] == 0) {
                    PIDS[i] = pid;
                    ctr = 0;
                }
                else i++;
            }
        }
    }
    return;
}

/* If first arg is a builtin command, run it and return true */
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
    else if ((strcmp(argv[0], "ls")) == 0) {
        list_segments(argc, argv);
        return 1;
    }
    else if ((strcmp(argv[0], "mkdir")) == 0) {
        make_dir(argc, argv);
        return 1;
    }
    else if ((strcmp(argv[0], "rmdir")) == 0) {
        remove_dir(argc, argv);
        return 1;
    }
    else if ((strcmp(argv[0], "touch")) == 0) {
        touch(argc, argv);
        return 1;
    }
    else if ((strcmp(argv[0], "cat")) == 0) {
        cat(argc, argv);
        return 1;
    }
    else if ((strcmp(argv[0], "echo")) == 0) {
        echo(argc, argv);
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

    /* Should the job run in the background? */
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

void list_segments(int argc, char **argv) {
    DIR *dp = NULL;
    struct dirent *dent = NULL;
    char path[MAXPATH];

    if (argc == 1) strcpy(path, ".");
    dp = opendir(path);

    while ((dent = readdir(dp)) != NULL) {
        printf("%s      ", dent->d_name);
    }
    printf("\n");
    closedir(dp);
    return;
}

void make_dir(int argc, char **argv) {
    if ((argc != 2) || mkdir(argv[1], 0755) == -1) printf("mkdir argument err!\n");
    return;
}

void remove_dir(int argc, char **argv) {
    if ((argc != 2) || rmdir(argv[1]) == -1) printf("rmdir argument err!\n");
    return;
}

void touch(int argc, char **argv) {
    if (!(argc >= 2)) {
        printf("touch argument err!\n");
        return;
    }

    int fd;
    for (int i = 1; i < argc; i++) {
        fd = open(argv[i], O_RDWR | O_CREAT, 0664);
        close(fd);
    }
    return;
}

void cat(int argc, char **argv){
    int fdin;
    char *path = argv[1];
    char buf[MAXBUF];

    if (argc != 2){
        printf("cat argument err!\n");
        return;
    }

    if ((fdin = open(path, O_RDONLY)) == -1){
        printf("cat open() err!\n");
        return;
    }

    read(fdin, buf, sizeof(buf));
    printf("%s", buf);
    close(fdin);
    return;
}

void echo(int argc, char **argv){
    if (!(argc >= 1)) {
        printf("echo argument err!\n");
        return;
    }
    for (int i = 1; i < argc; i++){
        printf("%s%s", argv[i], (argc-i)? " ": "");
    }
    printf("\n");
    return;
}
