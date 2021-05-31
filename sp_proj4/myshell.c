#include "myshell.h"

int main() {
    char cmdline[MAXLINE]; /* Command line */

    Signal(SIGINT, sigint_handler); // ctrl c
    Signal(SIGTSTP, sigtstp_handler); // ctrl z
    Signal(SIGCHLD, sigchld_handler);

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
        char* foo = fgets(cmdline, MAXLINE, stdin);
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
    PCMD_TYPE cmd_type = FIRST;
    int p_flag = 0;
    int input = 0;

    strcpy(buf, cmdline);
    bg = parseline(buf, &argc, argv);

    // is pipelined?
    char* cmd = cmdline;
    char cmdline_cpy[MAXLINE];
    strcpy(cmdline_cpy, cmdline);
    char* bar_pos;
    if ((bar_pos = strchr(cmd, '|')) != NULL) {
        p_flag = 1;
    }

    if (argv[0] == NULL)
        return;   /* Ignore empty lines */

    if (!builtin_command(argc, argv)) { // quit -> exit(0), & -> ignore, other -> pipe_command
        if (!p_flag) { // 단일 명령
            if ((pid = fork()) == 0) { // child process
                if (bg) {
                    // BG job은 ctrl c, ctrl z에 반응하지 않도록 조정
                    Signal(SIGINT, SIG_IGN);
                    Signal(SIGTSTP, SIG_IGN);
                }

                if (execvp(argv[0], argv) < 0){
                    printf("Err! There's no command : \"%s\"\n", argv[0]);
                    fflush(stdout);
                    exit(0);
                }
            }

            // 실행한 job의 내용을 저장
            push_job(pid, (bg == 1? BG : FG), cmdline);

            // foreground job이면 곧바로 wait
            if (!bg){
                waitfg(pid);
            }
            else{
                JOB_info* job_obj = get_JOB_info_or_NULL(pid);
                printf("[%d] %d\n", job_obj->ind, pid);
            }
        }

        // pipeline command
        // pipe structure : FIRST - MIDDLE - ... - MIDDLE - LAST
        else {
            while (bar_pos != NULL) {
                *bar_pos = '\0';

                // execute pipe command
                input = pipe_command(cmd, argv, input, cmd_type, &pid);

                cmd = bar_pos + 1;
                bar_pos = strchr(cmd, '|');
                cmd_type = MIDDLE;
            }
            // &가 붙은 background command일 경우, &를 제거하고 parameter로 넘겨줌
            char* ptr;
            if ((ptr = (strchr(cmd, '&')))) *ptr = '\0';
            pipe_command(cmd, argv, input, LAST, &pid);
            push_job(pid, (bg == 1? BG : FG), cmdline_cpy);

            // foreground job이면 바로 wait
            if (!bg){
                waitfg(pid);
            }
            else{
                JOB_info* job_obj = get_JOB_info_or_NULL(pid);
                printf("[%d] %d\n", job_obj->ind, pid);
            }
        }
    }
}

/* If first arg is a builtin command, run it and return true */
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

    // phase 3
    if (!strcmp(argv[0], "jobs")){
        list_jobs();
        return 1;
    }
    // bg, fg, kill
    if (!strcmp(argv[0], "bg") || !strcmp(argv[0], "fg") || !strcmp(argv[0], "kill")) {
        if (argv[1] == NULL) {
            printf("Argument err!\n");
            return 1;
        }

        // bg 혹은 fg의 인자로 %가 붙어있는 경우 이를 제거
        if (argv[1][0] == '%') argv[1] = &(argv[1][0]) + 1;
        int ind = atoi(argv[1]);
        int pid = JOBS[ind].pid;
        if (ind < 1 || ind >= MAXJOBS){
            printf("-bash: %s: %%%d: no such job!\n", argv[0], ind);
            return 1;
        }
        if (JOBS[ind].state == _undefined){
            printf("-bash: %s: %%%d: no such job!\n", argv[0], ind);
            return 1;
        }
        // JOB 구조체 배열에 저장된 job인지 check
        if (get_JOB_info_or_NULL(pid) != NULL) {
            if (!strcmp(cmd, "bg")) {
                // SIGCONT : stop 시그널에 의해 정지된 process를 재실행
                Kill(pid, SIGCONT);
                update_job(pid, BG);

                printf("[%d]+  %s", ind, JOBS[ind].cmdline);
            }
            else if (!strcmp(cmd, "fg")) {
                // SIGCONT : stop 시그널에 의해 정지된 process를 재실행
                Kill(pid, SIGCONT);
                update_job(pid, FG);

                printf("%s", JOBS[ind].cmdline);
                waitfg(pid);
            }
            else if (!strcmp(cmd, "kill")) {
                Kill(pid, SIGKILL);

                printf("[%d]+  terminated  %s", JOBS[ind].ind, JOBS[ind].cmdline);
                delete_job(pid);
            }
        }
        else
            printf("Job %d not found\n", pid);
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
    // command 끝에 &를 달고 있다면, argv가 이를 제거한 상태로 들고 있도록 작업
    int last_ind = (int)strlen(argv[(*argc) - 1]) - 1;
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


int pipe_command(char*cmd, char **argv, int input, PCMD_TYPE pipe_type, pid_t *pid){
    tokenize_for_pipe_command(cmd, argv);

    int fd[2];
    if (pipe(fd) < 0){
        printf("Piping err!\n");
        return -1;
    }
    *pid = fork();

    if ((*pid) == 0) { // child process
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


void tokenize_for_pipe_command(char *cmd, char** argv){
    // 공백 제거
    while (isspace(*cmd)) cmd++;

    char* nxt = strchr(cmd, ' ');
    int ind = 0;

    while (nxt != NULL){
        nxt[0] = '\0';
        argv[ind++] = cmd;
        while (isspace(*(nxt + 1))) nxt++;
        cmd = nxt + 1;
        nxt = strchr(cmd, ' ');
    }

    if (cmd[0] != '\0') {
        argv[ind] = cmd;
        nxt = strchr(cmd, '\n');
        if (nxt[0] != '\0') nxt[0] = '\0';
        ind++;
    }

    // grep 명령과 함께 입력되는 쌍따움표 "" 혹은 따움표에 대한 '' 처리
    if ((argv[ind - 1][0] == '\"' && argv[ind - 1][strlen(argv[ind - 1]) - 1] == '\"') ||
        (argv[ind - 1][0] == '\'' && argv[ind - 1][strlen(argv[ind - 1]) - 1] == '\'')){
        argv[ind - 1][strlen(argv[ind - 1]) - 1] = '\0';
        argv[ind - 1] = &(argv[ind - 1][0]) + 1;
    }
    argv[ind] = NULL;
}

void sigint_handler(int sig){
    // catch SIGINT
    if (PRINT_FOR_DEBUG)
        printf("SIGINT!!!!!!!!!\n");
}

void sigtstp_handler(int sig){
    // catch SIGTSTP
    if (PRINT_FOR_DEBUG)
        printf("SIGTSTP!!!!!!!\n");
}

void sigchld_handler(int sig){
    // catch SIGCHLD
    pid_t pid;
    int status;

    if (PRINT_FOR_DEBUG)
        printf("SIGCLD handler begin!!!!!!!\n");

    // WNOHANG
    // 기다리는 pid가 종료되지 않아서 즉시 종료 상태를 회수 할 수 없는 상황에서
    // 호출자는 차단되지 않고 반환값으로 0을 받음
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        delete_job(pid);
        //printf("\n종료됨!!\n");
        if (PRINT_FOR_DEBUG)
            printf("sigchld_handler에 의해 job %d 제거됨!!!!!\n", pid);
    }

    // REAP zombies
    if (!((pid == 0) || (pid == -1 && errno == ECHILD)))
        unix_error("sigchld_handler wait err!");

    if (PRINT_FOR_DEBUG)
        printf("SIGCLD handler end!!!!!!!\n");
}


int push_job(pid_t pid, int state, char *cmdline){
    for (int i = 1; i < MAXJOBS; i++){
        if (JOBS[i].pid == 0){
            JOBS[i].ind = i;
            JOBS[i].state = state;
            strcpy(JOBS[i].cmdline, cmdline);
            JOBS[i].pid = pid;
            return 1;
        }
    }
    printf("Too many jobs!\n");
    return 0;
}

int delete_job(pid_t pid){
    for (int i = 1; i < MAXJOBS; i++){
        if (JOBS[i].pid == pid) {
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
            if (state == BG){
                char buf[10] = " &\n";
                JOBS[i].cmdline[strlen(JOBS[i].cmdline) -1] = '\0';
                strcat(JOBS[i].cmdline, buf);
            }
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
    //char buf[MAXLINE];
    for (int i = 1; i < MAXJOBS; i++){
        if (JOBS[i].pid != 0){
            /* print job state */
            switch (JOBS[i].state) {
                case BG:
                    printf("[%d]+  ", i);
                    printf("Running                 ");
                    printf("%s", JOBS[i].cmdline);
                    break;
                case FG:
                    printf("[%d]+  ", i);
                    printf("Foreground              ");
                    printf("%s", JOBS[i].cmdline);
                    break;
                case ST:
                    printf("[%d]+  ", i);
                    printf("Stopped                 ");
                    printf("%s", JOBS[i].cmdline);
                    break;
                default:
                    printf("Internal err!\n");
                    return;
            }
            //sprintf(buf, "%s%s", buf, JOBS[i].cmdline);
            //printf("%s", buf);
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
       unix_error("waitpid err!");
    }

    // 1. "stop 시그널"을 받은 자식에 대한 처리
    // JOBS에서 중단된 FG의 state를 변경
    // WIFSTOPPED : 반환의 원인이 된 자식프로세스가 현재 "정지"되어 있다면 true(1)을 반환.
    if(WIFSTOPPED(status)){
        printf("\n[%d]+  ", job_info->ind);

        // WSTOPSIG(status) : 자식을 정지하도록 만든 signal의 숫자를 반환. WIFSTOPPED가 non-zero 일경우에만 사용가능
        // strsignal : 인자로 받은 시그널을 가리키는 이름을 문자열로 return
        printf("%s", strsignal(WSTOPSIG(status)));
        printf("                 %s", job_info->cmdline);

        update_job(pid, ST);
    }

    // 2. "terminate 시그널"을 받은 자식에 대한 처리
    else {
        // WIFSIGNALED(status) : 자식프로세스가 어떤 신호때문에 종료되었다면 참을 반환
        if (WIFSIGNALED(status)) {
            sprintf(buf, "\n[%d]- Terminated by signal", job_info->ind);
            psignal(WTERMSIG(status), buf);
        }
        delete_job(pid);
        if (PRINT_FOR_DEBUG)
            printf("Job pid : %d deleted!!!\n", pid);
    }
}
