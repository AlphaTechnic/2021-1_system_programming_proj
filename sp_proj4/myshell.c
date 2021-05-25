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

typedef enum PIPE_TYPE {
    FIRST = 0,
    MIDDLE = 1,
    LAST = 2
}PIPE_TYPE;

typedef enum JOB_STATE{
    _undefined = 0,
    FG = 1,
    BG = 2,
    ST = 3
}JOB_STATE;

typedef struct JOB_info{
    int ind;
    pid_t  pid;
    JOB_STATE state;
    char cmdline[MAXLINE];
}JOB_info;
JOB_info JOBS[MAXJOBS];

int NUM_OF_CALLS = 0;
int VERBOSE = 0;

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, int *argc, char **argv);
int builtin_command(int argc, char **argv);

void change_dir(int argc, char **argv);
int pipe_command(char*cmd, char **argv, int input, PIPE_TYPE pipe_type, pid_t *pid);
void tokenize(char *cmd, char **argv);

void sigint_handler(int sig);
void sigtstp_handler(int sig);
void sigchld_handler(int sig);

void list_jobs();
int push_job(pid_t pid, int state, char *cmdline);
int delete_job(pid_t pid);
int update_job(pid_t pid, int state);
JOB_info *get_JOB_info_or_NULL(pid_t pid);
void waitfg(pid_t pid);

int main() {
    char cmdline[MAXLINE]; /* Command line */

    Signal(SIGINT, sigint_handler); // ctrl c
    Signal(SIGCHLD, sigchld_handler);
    Signal(SIGTSTP, sigtstp_handler); // ctrl z

    // init jobs
    for (int i = 1; i<MAXJOBS; i++){
        JOBS[i].ind = 0;
        JOBS[i].pid = 0;
        JOBS[i].state = _undefined;
        JOBS[i].cmdline[0] = '\0';
    }

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

    // for phase 1
    int argc;

    // for phase 2
    int p_flag = 0;
    int input = 0;
    PIPE_TYPE p_type = FIRST;

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
            if ((pid = fork()) == 0) { // child process
                if (bg) {
                    // BG job은 ctrl c, ctrl z에 반응하지 않음
                    Signal(SIGINT, SIG_IGN);
                    Signal(SIGTSTP, SIG_IGN);
                }

                char filename[MAXARGS] = "/bin/";
                strcat(filename, argv[0]);
                if (execve(filename, argv, environ) < 0) {    //ex) /bin/ls ls -al &
                    printf("%s: Command not found.\n", argv[0]);
                    exit(0);
                }

            }
            push_job(pid, (bg == 1 ? BG : FG), cmdline);
            if (!bg){
                printf("pid : %d\n", pid);
                waitfg(pid);
            }
            else{
                printf("%d %s", pid, cmdline);
            }
        }

        else { // pipeline command
            while (bar_pos != NULL) {
                *bar_pos = '\0';
                input = pipe_command(cmd, argv, input, p_type, &pid);

                cmd = bar_pos + 1;
                bar_pos = strchr(cmd, '|');
                p_type = MIDDLE;
            }
            pipe_command(cmd, argv, input, LAST, &pid);
            push_job(pid, (bg == 1 ? BG : FG), cmdline);

            // wait 처리
//            for (int i = 0; i < NUM_OF_CALLS - 1; i++) wait(NULL);
//            NUM_OF_CALLS = 0;

            if (!bg){
                printf("pid : %d\n", pid);
                waitfg(pid);
            }
            else{
                printf("%d %s", pid, cmdline);
            }
        }

        /* Parent waits for foreground job to terminate */
//        if (!bg) {
//            int status;
//            int return_val;
//            if ((return_val = waitpid(pid, &status, 0)) < -1){
//                printf("%d\n", pid);
//                printf("%d\n", status);
//                printf("%d\n", return_val);
//                unix_error("waitpid err!");
//            }
//            if (wait(NULL) < 0)
//                unix_error("waitpid err!");
//        }
//        else { //when there is background process!
//            printf("%d %s", pid, cmdline);
//        }
    }
}

/* If first arg is a builtin command, pipe_command it and return true */
int builtin_command(int argc, char **argv) {
    char *cmd = argv[0];
    if (!strcmp(argv[0], "quit") || (!strcmp(argv[0], "exit"))) { /* quit command */
        exit(0);
    }
    if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
        return 1;

    if (strcmp(argv[0], "cd") == 0) {
        change_dir(argc, argv);
        return 1;
    }

    // 3333333
    if (!strcmp(argv[0], "jobs")){
        list_jobs();
        return 1;
    }

    // bg, fg, kill
    if (!strcmp(argv[0], "bg") || !strcmp(argv[0], "fg") || !strcmp(argv[0], "kill")) {
        int pid;

        if (argv[1] == NULL) {
            printf("Argument err!\n");
            return 1;
        }

        if (argv[1][0] == '%') argv[1] = &(argv[1][0]) + 1;
        int ind = atoi(argv[1]);
        pid = JOBS[ind].pid;
        if (ind < 1 || ind >= MAXJOBS){
            printf("Job index err!\n");
            return 1;
        }

        //pid = atoi(argv[1]);

        if (get_JOB_info_or_NULL(pid) != NULL) {
            if (!strcmp(cmd, "bg")) {
                Kill(pid, SIGSTOP);
                update_job(pid, BG);

                // [1]+ sleep 20 &
                int last_ind = strlen(JOBS[ind].cmdline) - 1; // 개행 문자를 가리키고 있음
                char buf[MAXLINE];
                strncpy(buf, JOBS[ind].cmdline, last_ind);
                buf[last_ind] = '\0';
                printf("[%d]+  %s &\n", ind, buf);
            }
            else if (!strcmp(cmd, "fg")) {
                printf("%s", JOBS[ind].cmdline);
                Kill(pid, SIGCONT);
                update_job(pid, FG);
                waitfg(pid);
            }
            else if (!strcmp(cmd, "kill")) {
                int status;
                delete_job(pid);
                if(waitpid(pid, &status, SIGKILL) < 0){
                    printf("%d\n", pid);
                    unix_error("waitfg: waitpid err!");
                }
            }
        }
        else
            printf("Job %d not found\n", pid);
        return 1;
    }

    // user-defined
//    if (strcmp(argv[0], "cd") == 0) {
//        change_dir(argc, argv);
//        return 1;
//    }
    return 0;                     /* Not a builtin command */
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, int *argc, char **argv) {
    char *delim;         /* Points to first space delimiter */
    //int argc;            /* Number of args */
    int bg = 0;              /* Background job? */

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
    int last_ind = strlen(argv[(*argc) - 1]) - 1;
    if ((bg = (*argv[(*argc) - 1] == '&')) != 0){
        argv[(*argc) - 1][1] = '\0';
        argv[--(*argc)] = NULL;
        return 1;
    }

    if (argv[(*argc)-1][0] != '\0' && argv[(*argc)-1][last_ind] == '&'){
        argv[(*argc)-1][last_ind] = '\0';
        argv[(*argc)] = NULL;
        return 1;
    }
    return bg;
}

/* $end parseline */

void change_dir(int argc, char **argv) {
    if ((argc != 2) || chdir(argv[1]) == -1) {
        printf("cd argument err!\n");
    }
}

int pipe_command(char*cmd, char **argv, int input, PIPE_TYPE pipe_type, pid_t *pid){
    tokenize(cmd, argv);
    if (argv[0] != NULL) NUM_OF_CALLS++;

    int fd[2];
    pipe(fd);
    *pid = fork();

    if ((*pid) == 0) { // child
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

void sigint_handler(int sig){
    // catch SIGINT
    if (VERBOSE)
        printf("sigint_handler: shell caught SIGINT\n");
}

void sigtstp_handler(int sig){
    // catch SIGTSTP
    if (VERBOSE)
        printf("sigtstp_handler: shell caught SIGTSTP\n");
}

void sigchld_handler(int sig){
    // catch SIGCHLD
    pid_t pid;
    int status;

    if (VERBOSE)
        printf("sigchld_handler: entering \n");

    // WNOHANG
    // 기다리는 pid가 종료되지 않아서 즉시 종료 상태를 회수 할 수 없는 상황에서
    // 호출자는 차단되지 않고 반환값으로 0을 받음
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        delete_job(pid);
        if (VERBOSE)
            printf("sigchld_handler: job %d deleted\n", pid);
    }

    // REAP zombies
    if (!((pid == 0) || (pid == -1 && errno == ECHILD)))
        unix_error("sigchld_handler wait error");

    if (VERBOSE)
        printf("sigchld_handler: exiting\n");
}


int push_job(pid_t pid, int state, char *cmdline){
    for (int i = 1; i < MAXJOBS; i++){
        if (JOBS[i].pid == 0){
            JOBS[i].ind = i;
            JOBS[i].pid = pid;
            JOBS[i].state = state;
            strcpy(JOBS[i].cmdline, cmdline);
            return 1;
        }
    }
    printf("Too many jobs!\n");
    return 0;
}

int delete_job(pid_t pid){
    for (int i = 1; i < MAXJOBS; i++){
        if (JOBS[i].pid == pid){
            JOBS[i].ind = 0;
            JOBS[i].pid = 0;
            JOBS[i].state = _undefined;
            JOBS[i].cmdline[0] = '\0';
            return 1;
        }
    }
    return 0;
}

int update_job(pid_t pid, int state){
    for(int i = 1; i < MAXJOBS; i++){
        if (JOBS[i].pid == pid){
            JOBS[i].state = state;
            return 1;
        }
    }
    printf("Can't find job %d\n", pid);
    return 0;
}

JOB_info *get_JOB_info_or_NULL(pid_t pid){
    for (int i = 1; i < MAXJOBS; i++){
        if (JOBS[i].pid == pid){
            return &(JOBS[i]);
        }
    }
    return NULL;
}


void list_jobs(){
    char buf[MAXLINE];
    for (int i = 1; i < MAXJOBS; i++){
        if (JOBS[i].pid != 0){
            /* print pid */
            //printf("[%d]+  ", i);
            sprintf(buf, "[%d]+  ", i);

            /* print job state */
            switch (JOBS[i].state) {
                case BG:
                    sprintf(buf, "%sRunning                 ", buf);
                    break;
                case FG:
                    sprintf(buf, "%sForeground              ", buf);
                    break;
                case ST:
                    sprintf(buf, "%sStopped                 ", buf);
                    break;
                default:
                    sprintf(buf, "%sinternal error: job[%d].state=%d ", buf, i, JOBS[i].state);
            }
            /* print job command line */
            sprintf(buf, "%s%s", buf, JOBS[i].cmdline);
            printf("%s", buf);
            //[1]+  Running                 sleep 30 &
        }
    }
}


void waitfg(pid_t pid){
    int status;
    char buf[MAXLINE];

    if (pid == 0){
        return;
    }

    // WUNTRACED : 중단된 child 프로세스가 stopped인지 아닌지를 status에 기록해줌.
    JOB_info * job_info = get_JOB_info_or_NULL(pid);
    if(waitpid(pid, &status, WUNTRACED) < 0){
        printf("%d\n", pid);
        unix_error("waitfg: waitpid err!");
    }

    // JOBS에서 중단된 FG의 state를 변경
    // WIFSTOPPED : 반환의 원인이 된 자식프로세스가 현재 정지되어 있다면 true을 반환.
    if(WIFSTOPPED(status)){
        // WSTOPSIG(status) : 자식을 정지하도록 만든 signal의 숫자를 반환.
        // WIFSTOPPED가 non-zero 일경우에만 사용가능
        // strsignal : 인자로 받은 시그널을 가리키는 이름을 문자열로 return
        // 인자로 받은 시그널이 없으면 NULL을 return
        printf("--!\n");
        sprintf(buf, "\n[%d]+  ", job_info->ind);
        sprintf(buf, "%s%s", buf, strsignal(WSTOPSIG(status)));
        sprintf(buf, "%s                 %s", buf, job_info->cmdline);
        printf("%s", buf);

        update_job(pid, ST);
    }
    // FG job이 terminate되면, JOBS에서 지운다.
    else {
        printf("-!!\n");
        /* check if job was terminated by an uncaught signal */
        if (WIFSIGNALED(status)) {
            sprintf(buf, "\n[%d]- Terminated by signal", job_info->ind);
            psignal(WTERMSIG(status), buf);
        }
        delete_job(pid);
        if (VERBOSE)
            printf("waitfg: job %d deleted\n", pid);
    }
}
